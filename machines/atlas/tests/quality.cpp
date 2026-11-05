#include "../engine.cpp"
#include "cases.h"
#include "reference/engine_before_narrow.h"
#include <cstdio>
#include <cmath>
int main(){
    double maximum_error=0,min_ratio=2,max_ratio=0;
    for(unsigned scene=0;scene<45;++scene){
        atlas_reference::Voice a={};atlas::Voice b={};atlas_reference::Shared shared={};atlas::Shared fast_shared={};
        atlas_reference::initialize(a,shared,scene+1);atlas::initialize(b,fast_shared,scene+1);
        double reference_energy=0,candidate_energy=0,error=0;
        opxy::i32 p[8],old[64],now[64];opxy::u32 pitch[64];
        for(unsigned block=0;block<1536;++block){
            split_cases::parameters(scene,block,p);
            for(unsigned i=0;i<64;++i)pitch[i]=split_cases::pitch(scene,block,i);
            if(scene==43 && block%11==0){atlas_reference::start(a,shared,scene+1);atlas::start(b,fast_shared,scene+1);}
            atlas_reference::render(a,shared,scene+1,old,p,pitch);atlas::render(b,fast_shared,scene+1,now,p,pitch);
            const auto& x=a.oscillator.e.wt;const auto& y=b.oscillator.e.wt;
            if(x.phase!=y.phase || x.f!=y.f){std::puts("FAIL oscillator phase changed");return 1;}
            for(unsigned i=0;i<64;++i){
                const double delta=static_cast<double>(now[i])-old[i];
                reference_energy+=static_cast<double>(old[i])*old[i];candidate_energy+=static_cast<double>(now[i])*now[i];error+=delta*delta;
                if(now[i]<-536870912 || now[i]>536870912){std::puts("FAIL output bound");return 1;}
            }
        }
        const double relative=std::sqrt(error/(reference_energy+1));
        const double ratio=std::sqrt(candidate_energy/(reference_energy+1));
        if(relative>maximum_error)maximum_error=relative;
        if(ratio<min_ratio)min_ratio=ratio;if(ratio>max_ratio)max_ratio=ratio;
        if(relative>.001 || ratio<.995 || ratio>1.005){std::printf("FAIL scene%u error%.6f level%.6f\n",scene,relative,ratio);return 1;}
    }
    std::printf("PASS: 45 long Atlas scenes: maximum relative RMS error %.6f, level ratio %.6f..%.6f; phases unchanged\n",maximum_error,min_ratio,max_ratio);
}
