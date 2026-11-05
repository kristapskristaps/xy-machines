#include "../engine.cpp"
#include "reference/wt_before_settled.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void require(bool value,const char* message){if(!value){std::puts(message);std::exit(1);}}
int main(){
    unsigned random=7719;
    for(unsigned scene=0;scene<8;++scene){
        atlas_dsp::macro_wavetable a={},b={};atlas_dsp::wt_init(&a);atlas_dsp::wt_init(&b);
        opxy::u8 p[7]={};opxy::i32 ao[32],aa[32],bo[32],ba[32];
        for(unsigned block=0;block<4096;++block){
            if(block==0 || block==2048 || (block>=3072 && block%17==0)){
                for(unsigned i=1;i<=3;++i){random=random*1664525u+1013904223u;p[i]=static_cast<opxy::u8>(random>>24);}
            }
            opxy::u32 inc=scene<4 ? 2678267u*(scene+1) : 25479940u;
            if(scene==5 && block>=3000)inc+=block*5171u;
            if(scene==6)inc=4295u;
            if(scene==7)inc=0x40000000u;
            atlas_dsp::wt_render(&a,p,inc,ao,aa,32);
            atlas_dsp::wt_render_reference(&b,p,inc,bo,ba,32);
            require(!std::memcmp(ao,bo,sizeof ao) && !std::memcmp(aa,ba,sizeof aa),"FAIL settled-coordinate path changed PCM");
            require(!std::memcmp(&a,&b,sizeof a),"FAIL settled-coordinate path changed oscillator/cache state");
        }
    }
    std::puts("PASS: 32,768 blocks match frozen PCM/state through long holds, control steps, automation, pitch motion and pitch limits");
}
