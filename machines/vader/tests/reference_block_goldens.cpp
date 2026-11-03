#include "reference_97.hpp"
#include <cstdio>
#include <cstring>
namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
Hash word(Hash h,u32 v){return (h^v)*1099511628211ull;}
Hash scenario(unsigned setting){
    vader_frozen::Voice voice;vader_frozen::Shared shared;vader_frozen::initialize(voice,shared,7);
    i32 p[8]={268435455,644245094,0,0,0,429496729,2147483647,2147483647};
    if(setting==1)p[2]=2147483647;
    if(setting==2){p[2]=1073741824;p[3]=1610612735;p[4]=2147483647;}
    u32 pitch[64],random=7139;i32 output[64];Hash h=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==3 && block%31==0)for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);
        if(setting==4 && block%127==0)for(unsigned c=0;c<8;++c)p[c]=(block/127+c)%2?2147483647:0;
        if(setting==5 && block%97==0){vader_frozen::start(voice,shared,7);for(auto& value:p)value=static_cast<i32>((random=random*1664525u+1013904223u)>>1);}
        for(unsigned i=0;i<64;++i){pitch[i]=25478335u;if(setting==3)pitch[i]=(random=random*1664525u+1013904223u)>>2;if(setting==4)pitch[i]=(block%3==0?0:block%3==1?25478335u:0xffffffffu)-i;}
        vader_frozen::render(voice,shared,7,output,p,pitch);
        for(auto sample:output)h=word(h,static_cast<u32>(sample));
        static_assert(sizeof(voice)%4==0,"word-shaped Vader voice");u32 words[sizeof(voice)/4];std::memcpy(words,&voice,sizeof(voice));for(auto value:words)h=word(h,value);
    }
    return h;
}
}
int main(){
    // Reference PCM and complete voice state captured from e429139.
    const Hash reference[]={3325169007062555743ull,
7295484003241281248ull,
5844422114680075597ull,
17418346220717631861ull,
8625207513067165984ull,
1686899697945756362ull};
    for(unsigned i=0;i<6;++i){const Hash actual=scenario(i);
        if(actual!=reference[i]){std::fprintf(stderr,"FAIL: Vader block-stage audio/state %u: %llu\n",i,actual);return 1;}
    }
    std::puts("PASS: frozen original Vader audio and full voice state across steady/ramping drive, click/sweep, automation, extremes and resets");
}
