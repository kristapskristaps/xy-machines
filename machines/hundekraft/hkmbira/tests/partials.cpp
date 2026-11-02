#include "../core.h"
#include <cstdio>

int main() {
    unsigned random=7139;
    for(unsigned trial=0;trial<1000;++trial)for(int count=1;count<=5;++count)for(int glide=0;glide<2;++glide) {
        hkmbira::port::Dsp dsp={};
        hkmbira::port::part_t parts[5];
        unsigned phases[5],expected_phases[5];
        int amplitudes[5],increments[5],steps[5],deltas[5],actual[128],expected[128]={};
        for(int k=0;k<count;++k) {
            phases[k]=expected_phases[k]=random=random*1664525u+1013904223u;
            amplitudes[k]=int((random=random*1664525u+1013904223u)&0x7fffffu)+0x800000;
            increments[k]=int((random=random*1664525u+1013904223u)&0x3fffffffu);
            deltas[k]=int((random=random*1664525u+1013904223u)&65535u)-32768;
            steps[k]=-int((random=random*1664525u+1013904223u)&32767u);
            parts[k]={&phases[k],unsigned(increments[k]),deltas[k],amplitudes[k],steps[k]};
        }
        for(int sample=0;sample<128;++sample)for(int pair=0;pair<count;pair+=2) {
            int sum=0;
            for(int k=pair;k<count && k<pair+2;++k) {
                expected_phases[k]+=unsigned(increments[k])+unsigned(glide?deltas[k]:0)*unsigned(sample);
                const bool single=pair+1==count;
                const int gain=(amplitudes[k]+steps[k]*(sample&~7))>>(single?9:10);
                sum+=hkmbira::port::sine_table[expected_phases[k]>>19]*gain;
            }
            expected[sample]+=sum>>(pair+1==count?16:15);
        }
        dsp.partials(actual,parts,count,glide);
        for(int sample=0;sample<128;++sample)if(actual[sample]!=expected[sample])return 1;
        for(int k=0;k<count;++k)if(phases[k]!=expected_phases[k])return 2;
    }
    std::puts("PASS: partial counts, accumulation, phase wrap, glide and eight-sample gain steps");
}
