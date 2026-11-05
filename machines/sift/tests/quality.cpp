#include "../engine.cpp"
#include "cases.h"
#include "reference/grain_before_q15.h"
#include <cstdio>
#include <cmath>
int main(){
    double maximum_error=0,min_ratio=2,max_ratio=0;
    for(unsigned scene=0;scene<45;++scene){
        sift_reference::Voice a={};sift::Voice b={};sift_reference::Shared shared={};sift::Shared fast_shared={};
        sift_reference::initialize(a,shared,scene+1);sift::initialize(b,fast_shared,scene+1);
        double reference_energy=0,candidate_energy=0,error=0;
        opxy::i32 p[8],old[64],now[64];opxy::u32 pitch[64];
        for(unsigned block=0;block<1536;++block){
            split_cases::parameters(scene,block,p);
            for(unsigned i=0;i<64;++i)pitch[i]=split_cases::pitch(scene,block,i);
            if(scene==43 && block%11==0){sift_reference::start(a,shared,scene+1);sift::start(b,fast_shared,scene+1);}
            sift_reference::render(a,shared,scene+1,old,p,pitch);sift::render(b,fast_shared,scene+1,now,p,pitch);
            const auto& x=a.oscillator.e.grain;const auto& y=b.oscillator.e.grain;
            if(x.gc[0]!=y.gc[0]||x.gc[1]!=y.gc[1]||x.gf[0]!=y.gf[0]||x.gf[1]!=y.gf[1]||x.zc!=y.zc||x.zd!=y.zd||x.zf!=y.zf){std::puts("FAIL oscillator phase changed");return 1;}
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
    std::printf("PASS: 45 long Sift scenes: maximum relative RMS error %.6f, level ratio %.6f..%.6f; phases unchanged\n",maximum_error,min_ratio,max_ratio);
}
