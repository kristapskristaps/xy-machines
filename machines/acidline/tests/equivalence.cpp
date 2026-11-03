#include "../engine.cpp"
#include "render_reference.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
using opxy::u32; using opxy::i32;
using Hash=unsigned long long;
static Hash mix(Hash hash,u32 word){return (hash^word)*1099511628211ull;}
static Hash block_hash(Hash hash,const acidline::Voice& voice,const i32* audio){
    for(unsigned i=0;i<64;++i)hash=mix(hash,u32(audio[i]));
    for(unsigned i=0;i<32;++i)if(voice.fir1[i]!=voice.fir1[i+32]){
        std::fprintf(stderr,"FAIL: mirrored half-band history\n");std::exit(1);
    }
    // Hash the original state layout; the extra history is an exact duplicate.
    const unsigned prefix=offsetof(acidline::Voice,fir1)/sizeof(u32)+32;
    static_assert(sizeof(voice)==(prefix+32+6)*sizeof(u32),"word-shaped voice state");
    u32 words[prefix+6];std::memcpy(words,&voice,prefix*sizeof(u32));
    std::memcpy(words+prefix,&voice.cursor1,6*sizeof(u32));
    for(auto word:words)hash=mix(hash,word);
    return hash;
}
static void check(unsigned index,Hash hash){
    if(hash!=render_reference[index]){std::fprintf(stderr,"FAIL: render/state reference %u changed\n",index);std::exit(1);}
}
int main(){
    const u32 values[]={0,1,15,31,63,64,95,126,127};unsigned cases=0;
    for(unsigned slot=0;slot<8;++slot)for(auto value:values){
        acidline::Voice voice;acidline::Shared shared;
        acidline::initialize(voice,shared,1);acidline::start(voice,shared,1);
        i32 p[8],out[64];u32 pitch[64];
        for(unsigned s=0;s<8;++s)p[s]=s==5?0x40000000:0x50000000;
        p[slot]=i32(Hash(value)*0x7fffffffu/127);
        Hash hash=14695981039346656037ull;
        for(unsigned block=0;block<32;++block){
            for(unsigned i=0;i<64;++i)pitch[i]=8123456u+(i+block*64)*7919u;
            if(block==16)acidline::start(voice,shared,1);
            acidline::render(voice,shared,1,out,p,pitch);hash=block_hash(hash,voice,out);
        }
        check(cases++,hash);
    }
    acidline::Voice voice;acidline::Shared shared;
    acidline::initialize(voice,shared,1);acidline::start(voice,shared,1);
    i32 p[8],out[64];u32 pitch[64],random=73;Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<2048;++block){
        for(auto& parameter:p){random=random*1664525u+1013904223u;parameter=i32(random&0x7fffffffu);}
        for(auto& step:pitch){random=random*1664525u+1013904223u;step=1000000+random%300000000;}
        if(block%127==0)acidline::start(voice,shared,1);
        acidline::render(voice,shared,1,out,p,pitch);hash=block_hash(hash,voice,out);
    }
    check(cases++,hash);
    if(cases!=73)return 1;

    std::puts("PASS: 73 exact render/state references, all controls, retrigger, pitch ramps and live modulation");
}
