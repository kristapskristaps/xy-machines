#include "../engine.cpp"
namespace x0x { namespace dsp {
static inline opxy::i32 installed_square_sixth(opxy::u32 phase){return metallic_shape(phase,1);}
}}
#define square_sixth installed_square_sixth
#include "reference/before_model_dispatch.h"
#undef square_sixth
#include <cstdio>
#include <cmath>
#include <cstdlib>
static void require(bool okay,const char* why){if(!okay){std::puts(why);std::exit(1);}}
int main(){
    using namespace x0x;using namespace x0x::dsp;
    double energy=0,error=0;int peak=0;
    for(opxy::u32 p=0,n=0;n<1048576;++n,p+=4093){
        const int a=installed_square_sixth(p),b=square_sixth(p),d=b-a;
        if(std::abs(d)>peak)peak=std::abs(d);
        energy+=double(a)*a;error+=double(d)*d;
    }
    require(peak<=8 && std::sqrt(error/energy)<.002,"FAIL cluster curve error");
    double maximum_error=0;
    for(unsigned model=closed_hat;model<=cymbal;++model)for(unsigned level=0;level<3;++level){
        Voice a={},b={};Shared shared={};initialize(a,shared,1);start(a,shared,1);drum_select(a,shared,1,model);b=a;
        opxy::i32 parameters[8],actual[64],expected[64];opxy::u32 pitch[64]={};
        for(auto& value:parameters)value=level==0?0:level==1?0x40000000:0x7fffffff;
        parameters[0]=0; // The 606 cluster path; other families are unchanged.
        double old_energy=0,new_energy=0,delta_energy=0;
        for(unsigned block=0;block<768;++block){
            render(a,shared,1,actual,parameters,pitch);dispatch_baseline_render(b,expected,parameters);
            for(unsigned i=0;i<64;++i){
                const double old=expected[i]/1073741824.,now=actual[i]/1073741824.,d=now-old;
                old_energy+=old*old;new_energy+=now*now;delta_energy+=d*d;
            }
        }
        const double rms=std::sqrt(delta_energy/(768*64));
        if(rms>maximum_error)maximum_error=rms;
        require(rms<.001,"FAIL cluster full-voice RMS change");
        if(old_energy>.0001*768*64){
            const double ratio=std::sqrt(new_energy/old_energy);
            require(ratio>.99 && ratio<1.01,"FAIL cluster loudness changed");
        }
    }
    std::printf("PASS: 1,048,576 cluster phases; peak error%d, relative RMS%.6f; nine complete percussion scenes max full-scale RMS%.6f\n",peak,std::sqrt(error/energy),maximum_error);
}
