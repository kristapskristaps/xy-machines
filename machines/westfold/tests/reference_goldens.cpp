#include "reference/engine.cpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
// Original golden literals are unchanged and checked on frozen exact97 DSP.
// The intentional half-difference DSP is tested in quality.cpp.
// Audio and complete voice fingerprints from release commit 1090e35.
const Hash reference[]={15481690983138528699ull,2934907969855482679ull,2739158391583318930ull,11181783214987345390ull,12647972495285902084ull,16446751716445956630ull};
Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
Hash scenario(unsigned setting){
    westfold_reference::Voice voice;westfold_reference::Shared shared;
    westfold_reference::initialize(voice,shared,7);
    i32 p[8]={1073741824,1073741824,1879048191,2147483647,1073741824,1073741824,1073741824,1073741824};
    if(setting==1)p[2]=2147483647;
    if(setting==2)p[2]=1073741824;
    u32 pitch[64],random=7139; i32 output[64];
    Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==3 && block%31==0)
            for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        if(setting==4 && block%127==0)
            for(unsigned c=0;c<8;++c)p[c]=(block/127+c)%2?2147483647:0;
        if(setting==5 && block%97==0){
            westfold_reference::start(voice,shared,7);
            for(unsigned c=0;c<8;++c)p[c]=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        }
        for(unsigned i=0;i<64;++i){
            pitch[i]=25478335u;
            if(setting==3)pitch[i]=(random=random*1664525u+1013904223u)>>2;
            if(setting==4)pitch[i]=(block%3==0?0:block%3==1?25478335u:0x70000000u)+i;
        }
        westfold_reference::render(voice,shared,7,output,p,pitch);
        for(auto sample:output)hash=word(hash,static_cast<u32>(sample));
        static_assert(sizeof(voice)%sizeof(u32)==0,"word-shaped Westfold voice");
        u32 words[sizeof(voice)/sizeof(u32)];std::memcpy(words,&voice,sizeof(voice));
        for(auto value:words)hash=word(hash,value);
    }
    return hash;
}
}
int main(){
    bool pass=true;
    for(unsigned i=0;i<6;++i){
        const Hash actual=scenario(i);
        if(actual!=reference[i]){std::fprintf(stderr,"FAIL: Westfold audio/state reference %u: %llu\n",i,actual);pass=false;}
    }
    if(!pass)return 1;
    std::puts("PASS: exact Westfold output and voice state across folding, settling, automation, pitch motion, extremes and retriggers");
}
