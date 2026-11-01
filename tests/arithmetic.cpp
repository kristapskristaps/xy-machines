#include "../dsp/primitives.h"
#include <cstdio>
#include <cstdlib>
static void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
int main() {
    using namespace x0x::dsp;
    u32 random = 1;
    for (u32 i=0;i<2000000;++i) {
        random ^= random<<13; random ^= random>>17; random ^= random<<5;
        const u32 a=random, b=i*2654435761u;
        require(high_product(a,b)==u32((static_cast<unsigned long long>(a)*b)>>32),"full unsigned high product");
        const i32 value=i32(a);
        require(divide<3>(value)==value/3 && divide<6>(value)==value/6 && divide<23>(value)==value/23,
                "reciprocal divisions preserve signed rounding");
    }
    const i32 boundaries[]={i32(0x80000000u),i32(0x80000001u),-753641,-32768,-23,-6,-3,-1,0,1,3,6,23,32767,753641,0x7fffffff};
    for (auto value:boundaries)
        require(divide<3>(value)==value/3 && divide<6>(value)==value/6 && divide<23>(value)==value/23,
                "signed minimum and boundary divisions");
    for (u32 period=128;period<=815;++period)
        for (u32 numerator=0;numerator<=period;++numerator)
            require(ratio_q15(numerator,period,(32767u<<16)/period)==numerator*32767u/period,
                    "every clap burst ratio is unchanged");
    std::puts("PASS: 2,000,000 full-word products/divisions, signed boundaries and all clap burst ratios");
}
