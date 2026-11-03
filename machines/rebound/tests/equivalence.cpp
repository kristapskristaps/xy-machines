#include "../fx.cpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
// Audio, state and scratch fingerprints from release commit 1090e35.
const Hash reference[]={17394760409468720680ull,17813060166521520048ull,1589989219187387457ull,10594474545309177844ull,12509866974746303749ull,9498367433980541234ull};
static i32 memory[rebound::buffer_words],left[64],right[64];
Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
Hash scenario(unsigned setting){
    rebound::State state;rebound::initialize(state);
    for(auto& sample:memory)sample=0x3f3f3f3f;
    i32 p[8]={1073741824,1073741824,2147483647,1610612735,1073741824,0,2147483647,2147483647};
    if(setting==1){p[0]=0;p[1]=2147483647;}
    if(setting==5){p[0]=2147483647;p[2]=1073741824;p[5]=1073741824;p[6]=1073741824;}
    opxy::Buffer buffer={memory,rebound::buffer_words};
    opxy::Transport transport={1200,0,true};
    u32 random=7139,phase=0;Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==2 && block%31==0){
            for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
            transport.bpm_x10=block%3==0?0:block%3==1?400:3000;
        }
        if(setting==3 && block%127==0)
            for(unsigned c=0;c<8;++c)p[c]=(block/127+c)%2?2147483647:0;
        if(setting==4 && block==2048)rebound::initialize(state);
        if(setting==5 && block%257==0)for(unsigned c=1;c<7;++c)p[c]^=1;
        for(unsigned i=0;i<64;++i){
            phase+=23192823u;
            left[i]=block<256 || setting>=2?static_cast<i32>(phase)/2:0;
            right[i]=block%113==0 && i==0?1073741824:-left[i]/2;
        }
        rebound::process(state,buffer,transport,left,right,p);
        for(unsigned i=0;i<64;++i){hash=word(hash,static_cast<u32>(left[i]));hash=word(hash,static_cast<u32>(right[i]));}
    }
    static_assert(sizeof(state)%sizeof(u32)==0,"word-shaped Rebound state");
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
        if(actual!=reference[i]){std::fprintf(stderr,"FAIL: Rebound audio/state/scratch reference %u: %llu\n",i,actual);pass=false;}
    }
    if(!pass)return 1;
    std::puts("PASS: exact Rebound output, state and scratch across tails, feedback, automation, reset, extremes and settled controls");
}
