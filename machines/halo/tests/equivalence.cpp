#include "plate_reference/fx.cpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;using opxy::u32;
using Hash=unsigned long long;
// Captured from 8641aec before changing Halo's DSP.
const Hash reference[]={2840770238351456209ull,11031747802177985088ull,
    8094262595374788695ull,16343497099083187646ull,
    4606473952324732194ull,3153414701709683826ull};
const i32 presets[][8]={
    {858993459,1181116006,257698038,107374182,966367642,1460288880,0,2147483647},
    {1395864371,1567663063,1417339207,214748365,816043786,1460288880,0,2147483647},
    {1675037245,1825361100,601295421,1546188226,1331439861,1116691496,0,2147483647}
};
Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
static i32 memory[halo_plate::buffer_words],left[64],right[64];
Hash scenario(unsigned setting){
    halo_plate::State state;halo_plate::initialize(state);
    for(auto& sample:memory)sample=0x3f3f3f3f;
    i32 parameters[8];std::memcpy(parameters,presets[setting%3],sizeof parameters);
    opxy::Buffer buffer={memory,halo_plate::buffer_words};
    opxy::Transport transport={1200,0,true};
    u32 random=7139,phase=0;Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<4096;++block){
        if(setting==3 && block%31==0){
            for(auto& value:parameters){random=random*1664525u+1013904223u;value=static_cast<i32>(random>>1);}
            transport.bpm_x10=block%3==0?0:block%3==1?400:3000;
        }
        if(setting==4 && block%127==0)
            for(unsigned control=0;control<8;++control)parameters[control]=(block/127+control)%2?2147483647:0;
        if(setting==5)parameters[6]=block>=512 && block<3072?2147483647:0;
        if(setting==3 && block==2048)halo_plate::initialize(state);
        for(unsigned i=0;i<64;++i){
            phase+=23192823u;
            left[i]=block<256 || setting>=3?static_cast<i32>(phase)/4:0;
            right[i]=block%113==0 && i==0?536870912:-left[i]/2;
        }
        halo_plate::process(state,buffer,transport,left,right,parameters);
        for(unsigned i=0;i<64;++i){hash=word(hash,static_cast<u32>(left[i]));hash=word(hash,static_cast<u32>(right[i]));}
    }
    static_assert(sizeof(state)%sizeof(u32)==0,"word-shaped Halo state");
    u32 words[sizeof(state)/sizeof(u32)];std::memcpy(words,&state,sizeof(state));
    for(auto value:words)hash=word(hash,value);
    for(auto value:memory)hash=word(hash,static_cast<u32>(value));
    return hash;
}
}
int main(){
    bool pass=true;
    for(unsigned scenario_index=0;scenario_index<6;++scenario_index){
        const Hash actual=scenario(scenario_index);
        if(actual!=reference[scenario_index]){
            std::fprintf(stderr,"FAIL: Halo audio/state/scratch reference %u: %llu\n",scenario_index,actual);pass=false;
        }
    }
    if(!pass)return 1;
    std::puts("PASS: exact Halo stereo output, state and scratch across presets, tails, automation, reset, extremes and Hold");
}
