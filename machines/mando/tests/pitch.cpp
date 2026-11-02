#include "../engine.cpp"
#include <cstdio>

int main() {
    const opxy::u32 rail=0x40000000u;
    mando::Voice voice={};mando::Shared shared;
    mando::initialize(voice,shared,1);
    opxy::i32 parameters[8]={1331439861,0,0,0,0,1073741824,1073741824,0},output[64];
    opxy::u32 pitch[64],random=7139;
    for(unsigned mode=0;mode<6;++mode)for(unsigned pivot=0;pivot<64;++pivot) {
        if(pivot%17==0)mando::start(voice,shared,1);
        parameters[0]=pivot&1?2147483647:0;
        parameters[1]=parameters[2]=parameters[0];
        parameters[3]=static_cast<opxy::i32>((2u*(pivot&3u)+1u)*(2147483647u/8u));
        parameters[6]=static_cast<opxy::i32>((2u*(pivot%3u)+1u)*(2147483647u/6u));
        opxy::u32 expected=voice.phase[0];
        for(unsigned i=0;i<64;++i) {
            const opxy::u32 varying=rail+(random=random*1664525u+1013904223u)%0x30000000u;
            pitch[i]=mode==0?varying:mode==1?(i==pivot?rail-1:varying)
                :mode==2?(i==pivot?5356536u:5356535u):mode==3?(i==pivot?1u:0u)
                :mode==4?(i&1u?rail+32:rail-32):(random=random*1664525u+1013904223u);
            expected+=pitch[i]>rail?rail:pitch[i];
        }
        mando::render(voice,shared,1,output,parameters,pitch);
        if(voice.phase[0]!=expected) {
            std::printf("FAIL: phase advance at pitch mode %u pivot %u\n",mode,pivot);return 1;
        }
        for(auto value:output)if(value<-536870912 || value>536870912)return 1;
    }
    std::puts("PASS: constant, clamped, boundary, zero and random sample pitches retain exact phase advance");
}
