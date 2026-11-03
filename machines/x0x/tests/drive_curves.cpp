#include "../dsp/drive_curves.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
int main(){
    int maximum_error=0;double worst_rms=0;
    for(unsigned amount=1;amount<128;++amount){
        double energy=0,error_energy=0;int previous=-32768;
        for(int sample=-32767;sample<=32767;++sample){
            const int old=x0x::dsp::drive(sample,amount),now=x0x::dsp::table_drive(sample,x0x::tables::drive_curves[amount]);
            const int error=std::abs(now-old);if(error>maximum_error)maximum_error=error;
            energy+=double(old)*old;error_energy+=double(old-now)*(old-now);
            if(now<previous || now<-32767 || now>32767){std::fprintf(stderr,"FAIL Drive monotonic/bounds\n");return 1;}
            previous=now;
        }
        const double rms=std::sqrt(error_energy/energy);if(rms>worst_rms)worst_rms=rms;
        if(rms>.001){std::fprintf(stderr,"FAIL Drive approximation exceeds 0.1%% RMS\n");return 1;}
    }
    std::printf("Drive curve maximum point error%d/32768, worst relative RMS%.8f\n",maximum_error,worst_rms);
    if(maximum_error>64)return 1;
}
