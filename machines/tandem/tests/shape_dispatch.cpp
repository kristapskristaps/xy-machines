#include "../engine.cpp"
#include "reference/vso_before_shape_dispatch.h"
#include <cstdio>
#include <cstring>
int main(){
    unsigned random=7719;
    const int shapes[]={0,16384,32768,49152,65536};
    for(unsigned scene=0;scene<16;++scene){
        tandem_dsp::macro_vso a={},b={};tandem_dsp::vso_init(&a,scene&2?0x40000000u:0);b=a;
        opxy::i32 actual[32],expected[32];int shape=shapes[scene%5];
        for(unsigned block=0;block<2048;++block){
            if(block%127==0)shape=shapes[(scene+block/127)%5];
            if(block>=1536 && block%7==0)shape=static_cast<int>((random=random*1664525u+1013904223u)%65537);
            opxy::u32 mf=scene&4?2678267u:25479940u;
            if(scene&8)mf=4295u+block*75001u;
            const opxy::u32 sf=mf*(1+scene%4);
            const opxy::i32 pw=static_cast<opxy::i32>((random=random*1664525u+1013904223u)>>1);
            tandem_dsp::vso_render(&a,scene&1,mf,sf,pw,shape,actual,32);
            tandem_dsp::vso_reference(&b,scene&1,mf,sf,pw,shape,expected,32);
            if(std::memcmp(actual,expected,sizeof actual)||std::memcmp(&a,&b,sizeof a)){
                std::printf("FAIL shape specialization scene%u block%u\n",scene,block);return 1;
            }
        }
    }
    std::puts("PASS: 32,768 variable-shape blocks match frozen PCM/state through waveform boundaries, sync, width and pitch motion");
}
