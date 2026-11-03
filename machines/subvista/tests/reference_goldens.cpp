#include "reference_engine.hpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
// Original fingerprints are preserved unchanged. The deliberate nearest-point
// candidate is tested quantitatively in fidelity.cpp and tools/fidelity.py.
// Audio and complete voice fingerprints from release commit1090e35.
const Hash reference[]={12477931361508671102ull,1348676243139264332ull,2461886212820253821ull,1206941142103748773ull,7956956313345814718ull,3896533567421244202ull};
Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
Hash scenario(unsigned setting){
    subvista_reference::Voice voice;subvista_reference::Shared shared;
    subvista_reference::initialize(voice,shared,7);
    i32 p[8]={1073741824,1879048191,0,1073741824,0,0,0,0};
    if(setting==1)p[2]=2147483647;
    if(setting==2)p[2]=1073741824;
    u32 pitch[64],random=7139; i32 output[64];
    Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==3 && block%31==0)
            for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        if(setting==4 && block%127==0)
            for(unsigned c=0;c<4;++c)p[c]=(block/127+c)%2?2147483647:0;
        if(setting==5 && block%97==0){
            subvista_reference::start(voice,shared,7);
            for(unsigned c=0;c<4;++c)p[c]=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        }
        for(unsigned i=0;i<64;++i){
            pitch[i]=25478335u;
            if(setting==3)pitch[i]=(random=random*1664525u+1013904223u)>>2;
            if(setting==4)pitch[i]=(block%3==0?0:block%3==1?25478335u:0x70000000u)+i;
        }
        subvista_reference::render(voice,shared,7,output,p,pitch);
        for(auto sample:output)hash=word(hash,static_cast<u32>(sample));
        static_assert(sizeof(voice)%sizeof(u32)==0,"word-shaped Subvista voice");
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
        if(actual!=reference[i]){std::fprintf(stderr,"FAIL: Subvista audio/state reference %u: %llu\n",i,actual);pass=false;}
    }
    if(!pass)return 1;
    std::puts("PASS: frozen original Subvista golden output and voice state across saw/pulse, settling, automation, pitch motion, extremes and retriggers");
}
