#include "../dsp.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static void require(bool ok, const char* message) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main() {
    hkelg::port::Dsp dsp={};
    for (int x=-98304;x<=98304;++x) {
        const unsigned a=x<0?unsigned(-x):unsigned(x);
        const int q=int(((a>>8)*43691u+(((a&255)*43691u)>>8))>>9);
        require(dsp.div3(x)==(x<0?-q:q), "complete three-saw division range");
    }
    for (int motion=0;motion<32768;++motion) {
        const std::int64_t rate=dsp.exp2q((motion*24800)>>15);
        require(dsp.lfo_increment(motion)==unsigned((373000*rate)>>12),
                "complete Q15 motion range");
    }
    const int edges[]={INT32_MIN,INT32_MIN+1,-1073741824,-65536,-32769,
                       -32768,-32767,-1,0,1,32767,32768,65535,1073741824,
                       INT32_MAX-1,INT32_MAX};
    for (int b=0;b<32768;++b) for (int a:edges) {
        const std::int64_t product=std::int64_t(a)*b;
        require(dsp.mul_q15(a,b)==(product>>15), "Q15 gain range and signed boundaries");
    }
    unsigned random=7139;
    for (unsigned trial=0;trial<500000;++trial) {
        const int a=int(random=random*1664525u+1013904223u);
        const int b=int((random=random*1664525u+1013904223u)>>17);
        require(dsp.mul_q15(a,b)==((std::int64_t(a)*b)>>15), "random full-width Q15 products");
    }
    std::puts("PASS: 196,609 saw quotients, 32,768 motion rates and 1,024,288 Q15 products");
}
