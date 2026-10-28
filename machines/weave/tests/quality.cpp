#include "../vendor/macro.cpp"
#include <cstdio>
#include <cstdlib>
int main(){
    int maximum=0;
    for(int z=0;z<4;++z)for(int y=0;y<8;++y)for(int x=0;x<8;++x)
    for(int i=0;i<128;++i)for(int fraction=0;fraction<33;++fraction){
        const auto* t=weave_dsp::wt_wave(x,y,z);
        const int f=static_cast<int>(fraction*2147483647LL/32);
        const int xm1=t[i]*256,x0=t[i+1]*256,x1=t[i+2]*256,x2=t[i+3]*256;
        const int c=(x1-xm1)>>1,v=x0-x1,w=c+v,a=w+v+((x2-x0)>>1),bn=w+a;
        const int exact=weave_dsp::fmac1(weave_dsp::fmac1(weave_dsp::fmac1(a,f)-bn,f)+c,f)+x0;
        const int error=std::abs(weave_dsp::wt_hermite(t,i,f)-exact);
        if(error>maximum)maximum=error;
        if(error>4)return 1;
    }
    std::printf("PASS: all wavetable cubic segments; maximum rounding change %d Q8 table units\n",maximum);
}
