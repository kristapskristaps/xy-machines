#pragma once
#include <opxy/audio.hpp>
namespace split_cases {
const unsigned count=45;
inline void parameters(unsigned scene,unsigned block,opxy::i32* p) {
    const opxy::i32 defaults[4]={858993459,1073741823,858993459,0};
    for(unsigned i=0;i<4;++i)p[i]=defaults[i];
    if(scene>=1 && scene<=36)p[(scene-1)/9]=static_cast<opxy::i32>(2147483647ull*((scene-1)%9)/8);
    if(scene==42)for(unsigned i=0;i<4;++i)p[i]=static_cast<opxy::i32>(((block*1664525u+(i+1)*1013904223u)*747796405u)>>1);
    if(scene==44)for(unsigned i=0;i<4;++i)p[i]=block<32?0:block<64?2147483647:defaults[i];
}
inline opxy::u32 pitch(unsigned scene,unsigned block,unsigned sample) {
    if(scene==37)return 685636492u;
    if(scene==38)return 2678267u;
    if(scene==39)return 0;
    if(scene==40)return 0xffffffffu;
    if(scene==41)return 18000000u+block*75001u+sample*4097u;
    return 25479940u;
}
inline unsigned long long hash(unsigned long long h,opxy::u32 word){return (h^word)*1099511628211ull;}
}
