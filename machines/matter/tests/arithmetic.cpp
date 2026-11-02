#include "../engine.cpp"
#include <cstdio>

int main() {
    for(opxy::i32 value=-33554431;value<=33554431;++value) {
        if(matter::excitation_fifth(value)!=value/5) {
            std::printf("FAIL: excitation division at %d\n",value);return 1;
        }
    }
    const opxy::i32 edges[]={-2147483647-1,-2147483647,-1073741825,-65537,-32769,-1,0,1,32767,65535,1073741823,2147483647};
    for(opxy::i32 fraction=0;fraction<32768;++fraction)for(opxy::i32 signal:edges) {
        const opxy::i64 product=static_cast<opxy::i64>(signal)*fraction;
        const opxy::i32 expected=static_cast<opxy::i32>(product/32768-(product<0 && product%32768!=0));
        if(matter::fractional_mix(signal,fraction)!=expected)return 2;
    }
    opxy::u32 random=7139;
    for(unsigned i=0;i<2000000;++i) {
        const opxy::i32 signal=static_cast<opxy::i32>(random=random*1664525u+1013904223u);
        const opxy::i32 fraction=(random=random*1664525u+1013904223u)>>17;
        const opxy::i64 product=static_cast<opxy::i64>(signal)*fraction;
        const opxy::i32 expected=static_cast<opxy::i32>(product/32768-(product<0 && product%32768!=0));
        if(matter::fractional_mix(signal,fraction)!=expected)return 3;
    }
    std::puts("PASS: bounded excitation fifth and Q15 fractional mixes preserve exact rounding");
}
