#include "../engine.cpp"
#include <cstdio>
#include <cmath>

int main() {
    // Cubic delay weights sum to unity; their largest absolute row sum is
    // 40960. Signed-16 history therefore cannot overflow its signed-32 sum.
    static_assert(32768LL*40960<2147483647LL,"Cubic delay accumulator headroom");
    matter::BowDelay delay={};
    for(unsigned i=0;i<1024;++i)delay.samples[0][i]=-32768;
    for(unsigned write=0;write<1024;++write)for(unsigned length=1;length<1024;++length) {
        if(matter::delay_sample<8>(delay,0,write,length)!=-32768 ||
           matter::delay_sample<4>(delay,0,write,length)!=-32768 ||
           matter::delay_sample<2>(delay,0,write,length)!=-32768 ||
           matter::delay_sample<1>(delay,0,write,length)!=-32768)return 1;
    }
    // Probe the reachable input range and saturated states, rather than forcing
    // arbitrary full-scale input through an internal bounded-input primitive.
    // Normal input <= 140928610. Bow input <= 2^29 + friction(135056200)
    // + one delayed mode(16609443), below 725 million. Half-gain <= .6 Q31.
    opxy::u32 random=91;
    for(unsigned i=0;i<200000;++i) {
        random=random*1664525u+1013904223u;
        opxy::i32 s1=static_cast<opxy::i32>(random),s2=static_cast<opxy::i32>(~random);
        const opxy::i32 input=static_cast<opxy::i32>(random%1450000001u)-725000000;
        random=random*1664525u+1013904223u;
        const opxy::i32 coefficient=static_cast<opxy::i32>(random>>1);
        const opxy::i32 half_gain=static_cast<opxy::i32>(random%1288490189u);
        matter::resonate(s1,s2,coefficient,2147483616,half_gain,input);
    }
    for(unsigned rate=2;rate<=8;rate*=2)for(unsigned index=0;index<353;++index) {
        const double f=index/1024.,pi=3.14159265358979323846;
        const double gain=index?std::pow(rate*std::sin(pi*f/rate)/std::sin(pi*f),3):1;
        const unsigned actual=rate==2?matter::rate_gain_2[index]:rate==4?matter::rate_gain_4[index]:matter::rate_gain_8[index];
        if(std::fabs(actual/16777216.-gain)>.00000004)return 1;
        if(index<352) {
            const unsigned next=rate==2?matter::rate_gain_2[index+1]:rate==4?matter::rate_gain_4[index+1]:matter::rate_gain_8[index+1];
            if(next<actual || next-actual>131071)return 1;
            for(unsigned fraction=0;fraction<65536;fraction+=257) {
                const opxy::i64 expected=actual+(static_cast<opxy::i64>(next-actual)*fraction>>16);
                if(matter::gain_between(actual,next,fraction)!=expected)return 1;
            }
        }
    }
    // Summing rate groups before interpolation is linear except for integer
    // division ordering: at most N*(Rate-1) mixed units between endpoints.
    for(unsigned rate=2;rate<=8;rate*=2)for(unsigned n=1;n<=24;++n)for(unsigned trial=0;trial<64;++trial) {
        opxy::i32 before[8]={},after[8]={},sum_last=0,sum_current=0;
        for(unsigned mode=0;mode<n;++mode) {
            random=random*1664525u+1013904223u;opxy::i32 last=static_cast<opxy::i32>(random%134217729u)-67108864;
            random=random*1664525u+1013904223u;const opxy::i32 current=static_cast<opxy::i32>(random%134217729u)-67108864;
            sum_last+=last;sum_current+=current;
            if(rate==2)matter::upsample<2>(current,last,before,true);
            if(rate==4)matter::upsample<4>(current,last,before,true);
            if(rate==8)matter::upsample<8>(current,last,before,true);
        }
        if(rate==2)matter::upsample<2>(sum_current,sum_last,after,false);
        if(rate==4)matter::upsample<4>(sum_current,sum_last,after,false);
        if(rate==8)matter::upsample<8>(sum_current,sum_last,after,false);
        for(unsigned i=0;i<rate;++i)if(std::abs(before[i]-after[i])>static_cast<int>(n*(rate-1)))return 1;
        if(before[rate-1]!=after[rate-1])return 1;
    }
    std::puts("PASS: delay unity/headroom, bounded-input saturated resonator analytic response tables and grouped-sum error bounds");
}
