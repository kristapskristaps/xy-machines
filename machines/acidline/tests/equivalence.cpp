#include "../engine.cpp"
#include "reference/engine.cpp"
#include "render_reference.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cmath>
using opxy::u32;using opxy::i32;using Hash=unsigned long long;
static Hash word(Hash hash,u32 value){return (hash^value)*1099511628211ull;}
static Hash legacy_hash(Hash hash,const acidline_legacy::Voice& voice,const i32* audio){
    for(unsigned i=0;i<64;++i)hash=word(hash,u32(audio[i]));
    const unsigned prefix=offsetof(acidline_legacy::Voice,fir1)/sizeof(u32)+32;
    u32 words[prefix+6];std::memcpy(words,&voice,prefix*sizeof(u32));
    std::memcpy(words+prefix,&voice.cursor1,6*sizeof(u32));
    for(auto value:words)hash=word(hash,value);return hash;
}
struct Metrics {double old_energy,new_energy,old_edge,new_edge,difference,old_last,new_last;};
static void accumulate(Metrics& m,const i32* old,const i32* now){
    for(unsigned i=0;i<64;++i){
        const double a=old[i]/2147483648.,b=now[i]/2147483648.;
        if(now[i]<-1073741824 || now[i]>1073741824){std::puts("FAIL new output peak");std::exit(1);}
        m.old_energy+=a*a;m.new_energy+=b*b;m.old_edge+=(a-m.old_last)*(a-m.old_last);
        m.new_edge+=(b-m.new_last)*(b-m.new_last);m.difference+=(a-b)*(a-b);m.old_last=a;m.new_last=b;
    }
}
int main(){
    const u32 values[]={0,1,15,31,63,64,95,126,127};unsigned cases=0;
    double min_level=1e9,max_level=0,min_brightness=1e9,max_brightness=0;
    int max_pole=0,max_last_pole=0;
    for(unsigned scenario=0;scenario<73;++scenario){
        acidline::Voice now;acidline::Shared ns;acidline::initialize(now,ns,1);acidline::start(now,ns,1);
        acidline_legacy::Voice old;acidline_legacy::Shared os;acidline_legacy::initialize(old,os,1);acidline_legacy::start(old,os,1);
        i32 p[8],a[64],b[64];u32 pitch[64],random=73;
        for(unsigned slot=0;slot<8;++slot)p[slot]=slot==5?0x40000000:0x50000000;
        if(scenario<72)p[scenario/9]=i32(Hash(values[scenario%9])*0x7fffffffu/127);
        Metrics m={};Hash hash=14695981039346656037ull;
        const unsigned blocks=scenario==72?2048:32;
        for(unsigned block=0;block<blocks;++block){
            if(scenario==72){
                for(auto& parameter:p){random=random*1664525u+1013904223u;parameter=i32(random&0x7fffffffu);}
                for(auto& increment:pitch){random=random*1664525u+1013904223u;increment=1000000+random%300000000;}
            }else for(unsigned i=0;i<64;++i)pitch[i]=8123456u+(i+block*64)*7919u;
            if((scenario<72 && block==16) || (scenario==72 && block%127==0)){
                acidline::start(now,ns,1);acidline_legacy::start(old,os,1);
            }
            acidline_legacy::render(old,os,1,a,p,pitch);acidline::render(now,ns,1,b,p,pitch);
            hash=legacy_hash(hash,old,a);accumulate(m,a,b);
            if(now.phase!=old.phase || now.sub_phase!=old.sub_phase || now.envelope!=old.envelope){
                std::puts("FAIL note, octave or contour timing changed");return 1;
            }
            for(auto pole:now.pole)if(std::abs(pole)>max_pole)max_pole=std::abs(pole);
            if(std::abs(now.pole[3])>max_last_pole)max_last_pole=std::abs(now.pole[3]);
        }
        if(hash!=render_reference[scenario]){std::printf("FAIL frozen legacy reference %u\n",scenario);return 1;}
        const double level=std::sqrt(m.new_energy/m.old_energy);
        const double brightness=std::sqrt((m.new_edge/m.new_energy)/(m.old_edge/m.old_energy));
        if(!std::isfinite(level)||!std::isfinite(brightness)||m.new_energy<1e-8){std::puts("FAIL silent/nonfinite sound comparison");return 1;}
        if(level<0.5 || level>2 || brightness<0.5 || brightness>2){
            std::printf("FAIL sound-comparison tolerance scene %u: RMS %.3f edge %.3f\n",scenario,level,brightness);return 1;
        }
        if(level<min_level)min_level=level;if(level>max_level)max_level=level;
        if(brightness<min_brightness)min_brightness=brightness;if(brightness>max_brightness)max_brightness=brightness;
        std::printf("scene %u: level %.3f brightness %.3f\n",scenario,level,brightness);++cases;
    }
    std::printf("SUMMARY %u frozen legacy scenes retained; new/old RMS %.3f..%.3f, normalized-edge %.3f..%.3f; block-end pole maxima %d / last %d\n",cases,min_level,max_level,min_brightness,max_brightness,max_pole,max_last_pole);
    // The budgeted 1x filter is intentionally different; compare audibility,
    // level and spectral motion instead of replacing the old audio hashes.
    if(max_last_pole>=262144 || max_pole>=16777216)return 1;
    return 0;
}
