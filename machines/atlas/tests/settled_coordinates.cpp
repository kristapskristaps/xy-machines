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
        opxy::u8 p[7]={};opxy::i16 actual[32];opxy::i32 bo[32],ba[32];
        for(unsigned block=0;block<4096;++block){
            if(block==0 || block==2048 || (block>=3072 && block%17==0)){
                for(unsigned i=1;i<=3;++i){random=random*1664525u+1013904223u;p[i]=static_cast<opxy::u8>(random>>24);}
            }
            opxy::u32 inc=scene<4 ? 2678267u*(scene+1) : 25479940u;
            if(scene==5 && block>=3000)inc+=block*5171u;
            if(scene==6)inc=4295u;
            if(scene==7)inc=0x40000000u;
            const opxy::i32 mix=static_cast<opxy::i32>((scene%3)==0?0:(scene%3)==1?32767:16384);
            atlas_dsp::wt_render_fused(&a,p,inc,actual,32,mix);
            atlas_dsp::wt_render_reference(&b,p,inc,bo,ba,32);
            for(unsigned i=0;i<32;++i){
                opxy::i32 x=bo[i]<-65535?-65535:bo[i]>65535?65535:bo[i];
                opxy::i32 y=ba[i]<-65535?-65535:ba[i]>65535?65535:ba[i];
                x=(x*19661)>>15;y=(y*19661)>>15;
                x=x<-32768?-32768:x>32767?32767:x;y=y<-32768?-32768:y>32767?32767:y;
                const opxy::i32 expected=mix==0?x:mix>=32767?y:x+(((y-x)*mix)>>15);
                const opxy::i32 error=actual[i]-expected;
                require(error>=-2 && error<=2,"FAIL fused output exceeds two signed16 rounding units");
            }
            require(!std::memcmp(&a,&b,sizeof a),"FAIL settled-coordinate path changed oscillator/cache state");
        }
    }
    std::puts("PASS: 32,768 blocks preserve frozen oscillator state within two PCM rounding units through long holds, control steps, automation, pitch motion and pitch limits");
}
