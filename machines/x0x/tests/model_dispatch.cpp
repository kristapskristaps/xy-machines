#include "../engine.cpp"
#include "reference/before_model_dispatch.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>

int main(){
    using opxy::i32;using opxy::u32;u32 random=7719,combinations=0;
    for(u32 model=0;model<x0x::model_count;++model)for(u32 family=0;family<4;++family){
        if(!(x0x::family_masks[model]&(1u<<family)))continue;
        ++combinations;
        for(u32 scene=0;scene<4;++scene){
            x0x::Voice a={},b={};x0x::Shared shared={};x0x::initialize(a,shared,17);
            x0x::start(a,shared,17);x0x::drum_select(a,shared,17,model);b=a;
            i32 p[8],actual[64],expected[64];u32 pitch[64]={};
            for(u32 i=0;i<8;++i)p[i]=static_cast<i32>((scene==1?0:scene==2?127:64)<<24);
            p[0]=static_cast<i32>(x0x::family_raw(model,family))<<24;
            for(u32 block=0;block<512;++block){
                if(scene==3 && block%17==0)for(u32 i=1;i<8;++i)p[i]=static_cast<i32>(((random=random*1664525u+1013904223u)>>25)<<24);
                if(scene==3 && block%113==0){x0x::start(a,shared,17);x0x::start(b,shared,17);}
                if(scene==3 && block%127==0){
                    const u32 type=(model+block/127)%x0x::model_count;
                    x0x::drum_select(a,shared,17,type);x0x::drum_select(b,shared,17,type);
                }
                x0x::render(a,shared,17,actual,p,pitch);
                x0x::dsp::dispatch_baseline_render(b,expected,p);
                const bool same= !std::memcmp(actual,expected,sizeof actual)
                    && !std::memcmp(&a,&b,offsetof(x0x::Voice,settings))
                    && !std::memcmp(&a.settings,&b.settings,offsetof(x0x::dsp::Settings,metal808)+sizeof(bool))
                    && a.configured==b.configured && a.configured_model==b.configured_model;
                if(!same){std::printf("FAIL model dispatch model%u family%u scene%u block%u\n",model,family,scene,block);return 1;}
            }
        }
    }
    if(combinations!=47){std::puts("FAIL incomplete model/style matrix");return 1;}
    std::puts("PASS: all 47 styles match frozen PCM/state at control extremes, through automation, retriggers and live type changes");
}
