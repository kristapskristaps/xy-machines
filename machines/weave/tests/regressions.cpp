#include "../vendor/macro.cpp"
#include <cmath>
#include <cstdio>
int main() {
    using namespace weave_dsp;
    double previous=0;
    for(unsigned fraction=0;fraction<65536;++fraction) {
        const uf result=dx_pow2_2(static_cast<opxy::i32>(fraction));
        const double value=std::ldexp(result.m/1073741824.0,result.e);
        const double f=fraction/65536.0,expected=1+f*(.6565+.3435*f);
        if(value<previous || std::fabs(value-expected)>.0001) {
            std::printf("FAIL: FM exponential at fraction %u\n",fraction);return 1;
        }
        previous=value;
    }
    macro_dxvoice lfo={};lfo.p.lwave=4;
    opxy::u32 rng=1;
    for(unsigned i=0;i<4096;++i) {
        lfo.lph=i*1048576u;
        dx_lfo_step(&lfo,&rng,1);
        const double expected=.5+.5*std::sin(6.283185307179586*(i/4096.0+.5));
        if(lfo.lval<0 || lfo.lval>1073741824 || std::fabs(lfo.lval/1073741824.0-expected)>.00002) {
            std::printf("FAIL: sine LFO at phase %u\n",i);return 1;
        }
    }
    macro_voice voice;macro_init(&voice);macro_trig(&voice);
    opxy::u8 p[7]={80,64,64,64,0};opxy::i16 out[32];
    for(unsigned block=0;block<5000;++block) {
        for(unsigned i=1;i<5;++i)p[i]=static_cast<opxy::u8>((rng=rng*1664525u+1013904223u)>>25);
        macro_render(&voice,p,(rng&0x3fffffffu)+1,out,32);
        for(unsigned axis=0;axis<3;++axis)
            if(voice.e.wt.pre[axis]<0 || voice.e.wt.pre[axis]>(7<<24)) {
                std::puts("FAIL: wavetable smoother escaped its bank range");return 1;
            }
    }
    std::puts("PASS: FM exponential, sine LFO peak and rapidly moving wavetable banks");
}
