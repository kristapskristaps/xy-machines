#include "../engine.cpp"
#include "reference/voice.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstddef>
namespace {
using opxy::i32;using opxy::u32;using Hash=unsigned long long;
double minimum_rms=2,maximum_rms=0,maximum_error=0,minimum_edge=2,maximum_edge=0;
Hash word(Hash h,u32 v){return (h^v)*1099511628211ull;}
Hash state_hash(Hash h,const x0x::Voice& v){
    u32 prefix[offsetof(x0x::Voice,settings)/4];std::memcpy(prefix,&v,sizeof(prefix));
    for(auto x:prefix)h=word(h,x);
    u32 settings[offsetof(x0x::dsp::Settings,metal808)/4];std::memcpy(settings,&v.settings,sizeof(settings));
    for(auto x:settings)h=word(h,x);
    h=word(h,v.settings.metal808);h=word(h,v.configured);return word(h,v.configured_model);
}
Hash scenario(unsigned model,unsigned scenario){
    double old_energy=0,new_energy=0,error_energy=0,old_edge=0,new_edge=0,old_previous=0,new_previous=0;
    x0x::Voice v;x0x::Shared shared;x0x::initialize(v,shared,7);x0x::start(v,shared,7);x0x::drum_select(v,shared,7,model);
    x0x::Voice legacy=v;
    i32 p[8];const unsigned defaults[8]={x0x::family_raw(model,2),64,8,64,0,64,64,0};
    for(unsigned i=0;i<8;++i)p[i]=static_cast<i32>(defaults[i]<<24);
    if(scenario==1){p[1]=0;p[2]=p[4]=p[7]=127<<24;}
    if(scenario==2){p[1]=p[3]=p[6]=p[7]=127<<24;p[2]=0;}
    u32 random=7139,pitch[64]={};i32 output[64],reference[64];Hash hash=14695981039346656037ull;
    for(unsigned block=0;block<2048;++block){
        if(scenario==3 && block%17==0){for(unsigned i=1;i<8;++i)p[i]=static_cast<i32>(((random=random*1664525u+1013904223u)>>25)<<24);}
        if(scenario==3 && block%113==0){x0x::start(v,shared,7);x0x::start(legacy,shared,7);}
        if(scenario==4 && block%29==0){x0x::drum_select(v,shared,7,(block/29)%x0x::model_count);x0x::drum_select(legacy,shared,7,(block/29)%x0x::model_count);p[0]=static_cast<i32>(((block/29)*31%128)<<24);}
        if(scenario==5 && block%127==0){x0x::destroy(v,shared,7);x0x::initialize(v,shared,7);x0x::start(v,shared,7);x0x::drum_select(v,shared,7,model);x0x::destroy(legacy,shared,7);x0x::initialize(legacy,shared,7);x0x::start(legacy,shared,7);x0x::drum_select(legacy,shared,7,model);}
        x0x::render(v,shared,7,output,p,pitch);
        x0x::dsp::legacy_render(legacy,reference,p);
        for(unsigned i=0;i<64;++i){
            hash=word(hash,static_cast<u32>(reference[i]));
            const double a=reference[i]/2147483648.,b=output[i]/2147483648.;
            old_energy+=a*a;new_energy+=b*b;error_energy+=(a-b)*(a-b);
                            old_edge+=(a-old_previous)*(a-old_previous);new_edge+=(b-new_previous)*(b-new_previous);old_previous=a;new_previous=b;
        }
        hash=state_hash(hash,legacy);
    }
    const double rms=std::sqrt(new_energy/old_energy),error=std::sqrt(error_energy/old_energy);
    const double edge=std::sqrt((new_edge/new_energy)/(old_edge/old_energy));
    if(!std::isfinite(rms)||!std::isfinite(edge)||rms<.5||rms>2||edge<.5||edge>2){
        std::fprintf(stderr,"FAIL metal sound tolerance model %u scene %u: RMS%.6f error%.6f edge%.6f\n",model,scenario,rms,error,edge);return 0;
    }
    if(rms<minimum_rms)minimum_rms=rms;if(rms>maximum_rms)maximum_rms=rms;if(error>maximum_error)maximum_error=error;if(edge<minimum_edge)minimum_edge=edge;if(edge>maximum_edge)maximum_edge=edge;
    return hash;
}
}
int main(){
    // Captured from 23062df, before separating the metallic source into block passes.
    const Hash reference[]={1542250696603066529ull,
8630350155301343712ull,
17345336724957807745ull,
7651251946522557601ull,
16190117887712210192ull,
16893620402918631617ull,
6971723512627240264ull,
17068915107101797454ull,
499934552165723403ull,
12699098996896382410ull,
16190117887712210192ull,
2773143312283503685ull,
3907713153164939465ull,
9431457464825732992ull,
15995086127385320248ull,
18018778188333649982ull,
16190117887712210192ull,
3144723832051407192ull};
    unsigned at=0;
    for(unsigned model=x0x::closed_hat;model<=x0x::cymbal;++model)for(unsigned setting=0;setting<6;++setting){
        const Hash actual=scenario(model,setting);
        if(actual!=reference[at]){std::fprintf(stderr,"FAIL: metal block output/state model %u scenario %u: %llu\n",model,setting,actual);return 1;}
        ++at;
    }
    std::printf("Metal ratios RMS %.9f..%.9f; worst relative RMS difference %.9f\n",minimum_rms,maximum_rms,maximum_error);
    std::printf("Normalized edge ratio %.6f..%.6f\n",minimum_edge,maximum_edge);
    std::puts("PASS: frozen metallic PCM/state references plus quantitative new/legacy comparison across tails, extremes, automation, changes and resets");
}
