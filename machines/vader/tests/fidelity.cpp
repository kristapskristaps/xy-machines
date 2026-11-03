#include "../engine.cpp"
#include "reference_97.hpp"
#include <cmath>
#include <cstdio>

int main() {
    const unsigned pitches[]={2678267u,10713070u,85704562u,681740841u};
    double worst_error=0,worst_gain=0;
    unsigned cases=0;
    for(unsigned setting=0;setting<295;++setting)for(unsigned increment:pitches) {
        vader_frozen::Voice old={};vader_frozen::Shared bs={};
        vader::Voice now={};vader::Shared as={};
        vader_frozen::initialize(old,bs,1);vader::initialize(now,as,1);
        int p[8],a[64],b[64];unsigned pitch[64],random=7139;
        double old_energy=0,new_energy=0,error_energy=0;
        for(unsigned block=0;block<256;++block) {
            const int defaults[]={268435456,644245094,0,0,0,429496729,2147483647,2147483647};
            for(unsigned i=0;i<8;++i)p[i]=setting<256?(setting&(1u<<i)?2147483647:0):defaults[i];
            if(setting>=256 && setting<292) {
                const unsigned drive=(setting-256)%9,mode=(setting-256)/9;
                p[2]=static_cast<int>((2147483647ull*drive)/8);
                p[3]=static_cast<int>((2147483647ull*(2*mode+1))/8);
            }
            if(setting==292)for(auto& value:p)value=static_cast<int>((random=random*1664525u+1013904223u)>>1);
            if(setting==293)for(unsigned i=0;i<8;++i){const unsigned step=(block*(i+1)*3)%256;p[i]=static_cast<int>((step<128?step:255-step)*16777215u);}
            for(unsigned i=0;i<64;++i)pitch[i]=setting==294?increment/2+static_cast<unsigned>((static_cast<unsigned long long>(increment)*(block*64+i))/8192):increment;
            if(block==127 && setting>=292){vader_frozen::start(old,bs,1);vader::start(now,as,1);}
            vader_frozen::render(old,bs,1,a,p,pitch);vader::render(now,as,1,b,p,pitch);
            if(old.phase!=now.phase || old.pending!=now.pending || old.sweep!=now.sweep || old.fade!=now.fade ||
               old.drive!=now.drive || old.sub!=now.sub || old.color!=now.color || old.boost!=now.boost ||
               old.noise!=now.noise || old.click!=now.click || old.click_low!=now.click_low){std::puts("FAIL: phase/control state");return 1;}
            for(unsigned i=0;i<64;++i) {
                if(b[i]<-1073741824 || b[i]>1073741824){std::puts("FAIL: headroom");return 1;}
                const double before=a[i]/2147483648.0,after=b[i]/2147483648.0,error=after-before;
                old_energy+=before*before;new_energy+=after*after;error_energy+=error*error;
            }
        }
        const double rms=std::sqrt(old_energy/16384),error=std::sqrt(error_energy/16384);
        const double budget=0.03*rms>0.000125?0.03*rms:0.000125;
        if(error>worst_error)worst_error=error;
        if(error>budget || error>0.01){std::printf("FAIL: approximation setting%u pitch%u error%.7f budget%.7f\n",setting,increment,error,budget);return 1;}
        if(rms>0.001) {
            const double gain=std::fabs(10*std::log10(new_energy/old_energy));
            if(gain>worst_gain)worst_gain=gain;
            if(gain>0.06){std::printf("FAIL: RMS change%.6fdB\n",gain);return 1;}
        }
        ++cases;
    }
    std::printf("PASS: deliberate nearest512 drive approximation, %u cases, worst RMS error%.7fFS/gain%.5fdB; phase/control state exact\n",cases,worst_error,worst_gain);
}
