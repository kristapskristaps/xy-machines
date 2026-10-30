#include "../engine.cpp"
#include <cmath>
#include <cstdio>
int main(){
    // A full convolution is an independent oracle for the unrolled half-band.
    unsigned random=1234567;
    for(unsigned trial=0;trial<10000;++trial){
        opxy::i32 samples[32];
        for(unsigned i=0;i<32;++i){random=random*1664525u+1013904223u;samples[i]=static_cast<opxy::i32>(random%98305u)-49152;}
        const unsigned at=trial&31u;
        long long reference=0;
        for(unsigned i=0;i<31;++i)reference+=static_cast<long long>(samples[(at-i)&31u])*acidline::tables::halfband31[i];
        if(acidline::decimate(samples,at,acidline::tables::halfband31)!=reference/16384){
            std::puts("FAIL half-band differs from full convolution");return 1;
        }
    }
    const double pi=3.14159265358979323846;
    double raw_real=0,raw_imag=0,aa_real=0,aa_imag=0;
    opxy::i32 history[32]={};opxy::u32 cursor=0;
    double sum=0;for(unsigned i=0;i<31;++i)sum+=acidline::tables::halfband31[i]/16384.0;
    if(std::fabs(sum-1)>.001)return 1;
    for(unsigned n=0;n<44100;++n){
        const opxy::i32 x=static_cast<opxy::i32>(.3*std::sin(2*pi*6000*n/44100)*65536);
        const double raw=acidline::distort(x,11469)/65536.0;
        opxy::i32 filtered=0;
        for(unsigned sub=0;sub<2;++sub){
            const opxy::i32 source=static_cast<opxy::i32>(.3*std::sin(2*pi*6000*(n*2+sub)/88200)*65536);
            history[cursor]=acidline::distort(source,11469);
            if(sub==1)filtered=acidline::decimate(history,cursor,acidline::tables::halfband31);
            cursor=(cursor+1)&31;
        }
        if(n>=4410){
            const double angle=2*pi*14100*n/44100;
            raw_real+=raw*std::cos(angle);raw_imag+=raw*std::sin(angle);
            const double aa=filtered/65536.0;aa_real+=aa*std::cos(angle);aa_imag+=aa*std::sin(angle);
        }
    }
    const double reduction=-10*std::log10((aa_real*aa_real+aa_imag*aa_imag)/(raw_real*raw_real+raw_imag*raw_imag));
    std::printf("2x oversampling: 6 kHz clipping fixture's 14.1 kHz alias reduced %.1f dB\n",reduction);
    if(reduction<25){std::puts("FAIL alias rejection");return 1;}
    return 0;
}
