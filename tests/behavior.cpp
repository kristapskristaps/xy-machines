#include "../fx.cpp"
#include <cstdio>
#include <cstring>
static halo::State a,b;
static opxy::i32 ma[halo::buffer_words],mb[halo::buffer_words],la[64],ra[64],lb[64],rb[64];
static opxy::i32 p[8]={1503238553,1717986918,1288490188,1073741824,1073741824,1460288880,0,0};
int main() {
    opxy::Buffer ba={ma,halo::buffer_words},bb={mb,halo::buffer_words};opxy::Transport t={1200,0,true};
    // Only the stock mixer owns Dry: its value cannot change the wet callback.
    for(unsigned block=0;block<1000;++block) {
        for(unsigned i=0;i<64;++i)la[i]=ra[i]=lb[i]=rb[i]=block==0&&i==0?536870912:0;
        p[7]=0;halo::process(a,ba,t,la,ra,p);p[7]=2147483647;halo::process(b,bb,t,lb,rb,p);
        if(std::memcmp(la,lb,sizeof la)||std::memcmp(ra,rb,sizeof ra))return 1;
    }
    if(halo::parameter(6,536870912)!=0 || halo::parameter(6,1610612736)!=32767)return 1;
    p[6]=1610612736;
    for(unsigned block=0;block<300;++block){for(unsigned i=0;i<64;++i)la[i]=ra[i]=lb[i]=rb[i]=0;halo::process(a,ba,t,la,ra,p);halo::process(b,bb,t,lb,rb,p);}
    if(a.controls[6]!=32767)return 1;
    double held_energy=0;
    for(unsigned block=0;block<1500;++block) {
        for(unsigned i=0;i<64;++i){la[i]=ra[i]=1073741824;lb[i]=rb[i]=0;}
        halo::process(a,ba,t,la,ra,p);halo::process(b,bb,t,lb,rb,p);
        if(std::memcmp(la,lb,sizeof la)||std::memcmp(ra,rb,sizeof ra))return 1;
        for(auto sample:la)held_energy+=double(sample)*sample;
    }
    if(held_energy<1e9)return 1;
    std::puts("PASS: dry belongs to mixer; Hold retains an audible tail and rejects new input");
}
