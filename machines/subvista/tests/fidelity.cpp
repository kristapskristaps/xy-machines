#include "../engine.cpp"
#include "reference_engine.hpp"
#include <cmath>
#include <cstdio>

int main() {
    const int values[]={0,1073741824,2147483647};
    const unsigned pitches[]={2678267u,5356535u,10713070u,42852281u,171409123u,681740841u};
    double worst_relative=0,worst_gain=0,worst_absolute=0;
    unsigned cases=0;
    for(unsigned setting=0;setting<84;++setting)for(unsigned increment:pitches) {
        subvista_reference::Voice old={};subvista_reference::Shared bs={};
        subvista::Voice now={};subvista::Shared as={};
        subvista_reference::initialize(old,bs,1);subvista::initialize(now,as,1);
        int p[8]={},a[64],b[64];unsigned pitch[64],random=7139;
        double reference_energy=0,candidate_energy=0,error_energy=0;
        for(unsigned block=0;block<256;++block) {
            for(unsigned i=0;i<4;++i) {
                p[i]=setting<81?values[(setting/(i==0?1:i==1?3:i==2?9:27))%3]:1073741824;
                if(setting==81)p[i]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
                if(setting==82){const unsigned step=(block*(i+1)*3)%256;p[i]=static_cast<int>((step<128?step:255-step)*16777215u);}
            }
            for(unsigned i=0;i<64;++i)pitch[i]=setting==83?increment/2+static_cast<unsigned>((static_cast<unsigned long long>(increment)*(block*64+i))/8192):increment;
            if(block==127 && setting>=81){subvista_reference::start(old,bs,1);subvista::start(now,as,1);}
            subvista_reference::render(old,bs,1,a,p,pitch);subvista::render(now,as,1,b,p,pitch);
            if(old.phase!=now.phase || old.detuned!=now.detuned || old.sub!=now.sub || old.envelope!=now.envelope ||
               old.a1!=now.a1 || old.a2!=now.a2 || old.a3!=now.a3){std::puts("FAIL: phase/control state changed");return 1;}
            for(unsigned i=0;i<64;++i) {
                if(b[i]<-1073741824 || b[i]>1073741824){std::puts("FAIL: headroom");return 1;}
                const double before=a[i]/2147483648.0,after=b[i]/2147483648.0,error=after-before;
                reference_energy+=before*before;candidate_energy+=after*after;error_energy+=error*error;
            }
        }
        const double rms=std::sqrt(reference_energy/16384),absolute=std::sqrt(error_energy/16384);
        if(absolute>worst_absolute)worst_absolute=absolute;
        if(rms>0.001) {
            const double relative=std::sqrt(error_energy/reference_energy);
            const double gain=std::fabs(10*std::log10(candidate_energy/reference_energy));
            if(relative>worst_relative)worst_relative=relative;
            if(gain>worst_gain)worst_gain=gain;
            if(relative>0.025 || gain>0.05){std::printf("FAIL: fidelity setting%u pitch%u relative%.6f gain%.6f\n",setting,increment,relative,gain);return 1;}
        }
        if(absolute>0.0015){std::printf("FAIL: absolute error%.8f\n",absolute);return 1;}
        ++cases;
    }
    std::printf("PASS: deliberate nearest1024 approximation, %u cases; worst RMS error%.6fFS /%.3f%%, gain%.5fdB; phase/control state exact\n",cases,worst_absolute,worst_relative*100,worst_gain);
}
