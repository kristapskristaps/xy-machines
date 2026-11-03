#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace {
void require(bool ok,const char* reason){if(!ok){std::puts(reason);std::exit(1);}}
struct Spectrum {double fundamental,sub,second,third,edge;};
Spectrum measure(opxy::i32 wave,opxy::i32 sub,opxy::i32 damp){
    acidline::Voice voice={};acidline::Shared shared={};
    acidline::initialize(voice,shared,1);acidline::start(voice,shared,1);
    opxy::i32 parameters[8]={0x7fffffff,0,0,0,0,wave,sub,damp},output[64];
    opxy::u32 pitch[64];for(unsigned i=0;i<64;++i)pitch[i]=16777216;
    double real[4]={},imag[4]={},edge=0,previous=0;
    const double frequencies[4]={1./256,1./512,2./256,3./256};
    for(unsigned block=0;block<192;++block){
        acidline::render(voice,shared,1,output,parameters,pitch);
        for(unsigned i=0;i<64;++i){
            const double x=output[i]/2147483648.;
            if(block>=64){
                const unsigned n=block*64+i;
                for(unsigned k=0;k<4;++k){const double angle=6.283185307179586*frequencies[k]*n;real[k]+=x*std::cos(angle);imag[k]+=x*std::sin(angle);}
                edge+=(x-previous)*(x-previous);
            }
            previous=x;
        }
    }
    return {real[0]*real[0]+imag[0]*imag[0],real[1]*real[1]+imag[1]*imag[1],
        real[2]*real[2]+imag[2]*imag[2],real[3]*real[3]+imag[3]*imag[3],edge};
}
}
int main(){
    const unsigned fractions[]={0,1,2,7,15,16,31,32,63,64,127,128,255,256,511,512,
        1023,1024,2047,2048,4095,4096,8191,8192,16382,16383};
    for(unsigned band=0;band<8;++band){
        for(unsigned index=0;index<256;++index)for(unsigned half=0;half<2;++half){
            const auto packed=acidline::tables::saw_square[band][index][half];
            const unsigned at=index+half*256;
            require(static_cast<opxy::i16>(packed&65535u)==acidline::tables::saw[band][at] &&
                static_cast<opxy::i16>(packed>>16)==acidline::tables::saw[band][at+1]-acidline::tables::saw[band][at],
                "FAIL packed saw value or slope");
        }
        for(unsigned index=0;index<512;++index)for(auto fraction:fractions){
            const opxy::u32 phase=(index<<23)+(fraction<<9);
            const int expected=(acidline::lookup(acidline::tables::saw[band],phase)-
                acidline::lookup(acidline::tables::saw[band],phase+0x80000000u))/2;
            require(acidline::square_lookup(band,phase)==expected,"FAIL paired square interpolation boundary");
        }
    }
    opxy::u32 phase_random=7139;
    for(unsigned trial=0;trial<1000000;++trial){
        phase_random=phase_random*1664525u+1013904223u;
        const unsigned band=trial&7u;
        const int expected=(acidline::lookup(acidline::tables::saw[band],phase_random)-
            acidline::lookup(acidline::tables::saw[band],phase_random+0x80000000u))/2;
        require(acidline::square_lookup(band,phase_random)==expected,"FAIL randomized paired square interpolation");
    }
    for(int x=-100000;x<=100000;++x){
        const int bounded=x<-49152?-49152:x>49152?49152:x;
        const int half=bounded/2,square=half*half/16384;
        require(acidline::soft(x)==bounded-(bounded*square/65536)/3,
            "FAIL division-free saturation differs from the original curve");
    }
    for(int amount=0;amount<=16383;amount+=1023){
        int previous=-32769;
        for(int x=-32768;x<=32768;x+=8){
            const int y=acidline::distort(x,amount);
            require(y>=previous,"FAIL clipping folds back");
            require(y>=-32768 && y<=32768,"FAIL clipping peak");
            require(std::abs(y+acidline::distort(-x,amount))<=1,"FAIL clipping introduces asymmetry beyond one rounding unit");
            require(amount || y==x,"FAIL zero drive changes the signal");
            previous=y;
        }
    }
    const Spectrum saw=measure(0,0,0),triangle=measure(0x40000000,0,0),square=measure(0x7fffffff,0,0);
    require(saw.second/saw.fundamental>.1,"FAIL saw lacks even harmonics");
    require(triangle.third/triangle.fundamental<saw.third/saw.fundamental*.3,"FAIL triangle is not smoother than saw");
    require(square.second/square.fundamental<.0001,"FAIL square has strong even harmonics");
    const Spectrum low=measure(0,0x7fffffff,0),damped=measure(0,0,0x7fffffff);
    require(low.sub>low.fundamental,"FAIL octave-down sub is inaudible");
    require(saw.sub<saw.fundamental*.0001,"FAIL sub is present when off");
    require(damped.edge<saw.edge*.1,"FAIL damping does not tame brightness");
    acidline::Voice voice={};acidline::Shared shared={};
    acidline::initialize(voice,shared,1);acidline::start(voice,shared,1);
    opxy::i32 parameters[8]={},output[64];opxy::u32 pitch[64];
    unsigned random=73;unsigned long long accumulated=0;
    for(unsigned block=0;block<2048;++block){
        for(unsigned p=0;p<8;++p){random=random*1664525u+1013904223u;parameters[p]=static_cast<opxy::i32>(random&0x7fffffffu);}
        for(unsigned i=0;i<64;++i){random=random*1664525u+1013904223u;pitch[i]=1000000+(random%300000000);accumulated+=pitch[i];}
        acidline::render(voice,shared,1,output,parameters,pitch);
        require(voice.sub_phase==static_cast<opxy::u32>(accumulated/2),"FAIL sub pitch or glide differs from one octave down");
        require(voice.phase==static_cast<opxy::u32>(accumulated),"FAIL main pitch ramp changed");
        for(unsigned i=0;i<64;++i)require(output[i]>=-1073741824 && output[i]<=1073741824,"FAIL modulated output peak");
    }
    std::puts("PASS: monotonic symmetric soft clip, saw/triangle/square spectra, octave sub, damping, randomized modulation and exact pitch ramps");
}
