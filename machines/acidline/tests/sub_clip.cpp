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
    std::printf("Damp edge ratio %.4f\n",damped.edge/saw.edge);
    // The new ladder starts darker; still require at least 70% edge-energy reduction.
    require(damped.edge<saw.edge*.3,"FAIL damping does not tame brightness");
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
