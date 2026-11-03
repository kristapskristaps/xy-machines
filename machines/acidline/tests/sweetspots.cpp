#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace {
void require(bool pass,const char* message){if(!pass){std::puts(message);std::exit(1);}}
const unsigned frames=8192;
void phrase(const opxy::i32* parameters,double* samples){
    acidline::Voice voice={};acidline::Shared shared={};
    acidline::initialize(voice,shared,1);
    opxy::u32 pitch[64];opxy::i32 output[64];
    for(unsigned i=0;i<64;++i)pitch[i]=12739262;
    for(unsigned block=0;block<frames/64;++block){
        if(block%32==0)acidline::start(voice,shared,1);
        acidline::render(voice,shared,1,output,parameters,pitch);
        for(unsigned i=0;i<64;++i)samples[block*64+i]=output[i]/2147483648.;
    }
}
}
int main(){
    // The cached transfer is an approximation of the preceding 1x shaper,
    // not a new golden hash. Check its worst point error over the complete
    // audio range and a drive/resonance grid, including both cache boundaries.
    int maximum_curve_error=0;
    acidline::Voice cached={};
    for(int r=0;r<=4;++r)for(int d=0;d<=4;++d){
        const int resonance=r*16383/4,drive=d*16383/4;
        acidline::prepare_shape(cached,resonance,drive);
        const int body=acidline::interpolate(acidline::tables::body,resonance)*65536;
        const int gain=acidline::distortion_gain(drive);
        int previous=-32769;double reference_energy=0,error_energy=0;
        for(int x=-32768;x<=32768;++x){
            const int direct=acidline::distort(acidline::limit(opxy::q31::multiply(x,body)*6,32768),drive,gain);
            const int actual=acidline::cached_shape(cached,x);
            const int error=std::abs(actual-direct);
            if(error>maximum_curve_error)maximum_curve_error=error;
            require(actual>=previous && actual>=-32768 && actual<=32768,"FAIL cached shaper monotonicity/headroom");
            previous=actual;reference_energy+=double(direct)*direct;error_energy+=double(actual-direct)*(actual-direct);
        }
        require(std::sqrt(error_energy/reference_energy)<.005,"FAIL cached shaper exceeds 0.5% whole-range RMS error");
    }
    std::printf("Cached shaper maximum point error %d /32768\n",maximum_curve_error);
    require(maximum_curve_error<=512,"FAIL cached shaper differs by over 1.56% peak scale");
    // Envelope depth has room to move through the upper knob range, instead
    // of becoming identical above the old early cutoff clamp.
    for(int base=0;base<16383;base+=511){
        int previous=-1;
        for(int depth=0;depth<=15360;depth+=1024){
            const int target=acidline::cutoff_target(base,16384,depth);
            require(target>previous && target<16383,"FAIL envelope has a dead zone before full depth");
            previous=target;
        }
    }
    double previous[frames],current[frames];
    for(unsigned control=0;control<8;++control){
        if(control==5)continue;
        opxy::i32 p[8]={0x20000000,0x50000000,0x40000000,0x50000000,0x40000000,0,0,0};
        p[control]=0;phrase(p,previous);
        double least=1e10;
        for(unsigned step=1;step<=8;++step){
            p[control]=static_cast<opxy::i32>(static_cast<unsigned long long>(0x7fffffff)*step/8);
            phrase(p,current);
            double difference=0,energy=0;
            for(unsigned i=0;i<frames;++i){
                require(std::isfinite(current[i]) && std::fabs(current[i])<=.5,"FAIL swept control output peak");
                difference+=(current[i]-previous[i])*(current[i]-previous[i]);energy+=current[i]*current[i];
                previous[i]=current[i];
            }
            require(energy>1e-6,"FAIL swept control goes silent");
            const double relative=std::sqrt(difference/energy);
            if(relative<least)least=relative;
            require(relative>.003,"FAIL adjacent control positions produce negligible sound change");
        }
        std::printf("Control %u: smallest adjacent waveform change %.3f RMS ratio\n",control,least);
    }
    std::puts("PASS: envelope headroom and seven continuous controls across nine positions");
}
