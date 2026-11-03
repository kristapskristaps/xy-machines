#include "../engine.cpp"
#include <cstdio>

int main() {
    const opxy::i32 bounds[]={-536870912,-536870911,-268435456,-16777216,-1,0,1,
                             16777216,268435456,536870911,536870912};
    const opxy::i32 factors[]={-2147483647-1,-2147483647,-1073741824,-1,0,1,
                              1073741824,2126008812,2147483647};
    for(opxy::i32 a:bounds)for(opxy::i32 b:factors) {
        if(matter::rounded_bow_product(a,b)!=matter_dsp::fmac1(a,b)) {
            std::printf("FAIL: bounded bow product at %d * %d\n",a,b);return 1;
        }
    }
    opxy::u32 rng=7139;
    for(unsigned i=0;i<1000000;++i) {
        rng=rng*1664525u+1013904223u;
        const opxy::i32 a=static_cast<opxy::i32>(rng%1073741825u)-536870912;
        rng=rng*1664525u+1013904223u;
        const opxy::i32 b=static_cast<opxy::i32>(rng);
        if(matter::rounded_bow_product(a,b)!=matter_dsp::fmac1(a,b)) {
            std::printf("FAIL: random bounded bow product at %d * %d\n",a,b);return 1;
        }
    }
    for(opxy::i32 value=-33554431;value<=33554431;++value) {
        if(matter::excitation_fifth(value)!=value/5) {
            std::printf("FAIL: excitation division at %d\n",value);return 1;
        }
    }
    std::puts("PASS: bounded bow products preserve legacy rounding; every bounded excitation fifth preserves signed truncation");
}
