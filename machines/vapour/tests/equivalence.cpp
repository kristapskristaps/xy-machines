#include "../fx.cpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
// Complete output, state and scratch fingerprints from release 1090e35.
const Hash reference[]={3646798092898674940ull,15257104986192471119ull,18137657036631423530ull,2518943475243336310ull,12926295899465381604ull,11780095626165470929ull};
static i32 memory[vapour::ring_frames+sizeof(vapour::Workspace)/4],left[64],right[64];
Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
Hash scenario(unsigned setting){
    vapour::State state;vapour::initialize(state);
    for(auto& sample:memory)sample=0x3f3f3f3f;
    i32 p[8]={1073741824,1073741824,1073741824,536870912,1073741824,1073741824,0,2147483647};
    if(setting==4){p[3]=2147483647;p[4]=2147483647;}
    opxy::Buffer buffer={memory,sizeof(memory)/sizeof(memory[0])};
    u32 random=7139,phase=0;Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==1)p[6]=block>=512 && block<3072?2147483647:0;
        if(setting==2 && block%31==0)
            for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        if(setting==3 && block%127==0)
            for(unsigned c=0;c<8;++c)p[c]=(block/127+c)%2?2147483647:0;
        if(setting==4 && block==2048)vapour::initialize(state);
        if(setting==5 && block%257==0){p[1]^=1073741823;p[2]^=2147483647;p[4]^=2147483647;}
        for(unsigned i=0;i<64;++i){
            phase+=23192823u;
            left[i]=block<256 || setting>=2?static_cast<i32>(phase)/2:0;
            right[i]=block%113==0 && i==0?1073741824:-left[i]/2;
        }
        vapour::process(state,buffer,left,right,p);
        for(unsigned i=0;i<64;++i){hash=word(hash,static_cast<u32>(left[i]));hash=word(hash,static_cast<u32>(right[i]));}
    }
    static_assert(sizeof(state)%sizeof(u32)==0,"word-shaped Vapour state");
    u32 words[sizeof(state)/sizeof(u32)];std::memcpy(words,&state,sizeof(state));
    for(auto value:words)hash=word(hash,value);
    for(auto value:memory)hash=word(hash,static_cast<u32>(value));
    return hash;
}
}
int main(){
    bool pass=true;
    for(unsigned i=0;i<6;++i){
        const Hash actual=scenario(i);
        if(actual!=reference[i]){std::fprintf(stderr,"FAIL: Vapour audio/state/scratch reference %u: %llu\n",i,actual);pass=false;}
    }
    if(!pass)return 1;
    std::puts("PASS: exact Vapour output, state and scratch across tails, Freeze/resume, automation, reset, density and interpolation modes");
}
