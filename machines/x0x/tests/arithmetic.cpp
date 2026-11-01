#include "../dsp/primitives.h"
#include <cstdio>
#include <cstdlib>
static void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
int main() {
    using namespace x0x::dsp;
    for (i32 value=-65536;value<=65536;++value)
        require(divide_signal<3>(value)==value/3 && divide_signal<6>(value)==value/6,
                "every signed signal division preserves rounding");
    for (i32 value=-23*32768;value<=23*32768;++value)
        require(divide_signal<23>(value)==value/23 && divide_signal<138>(value)==value/23/6,
                "every harmonic sum preserves both square and sixth-square rounding");
    for (i32 sample=-32767;sample<=32767;++sample) for (u32 amount=0;amount<128;++amount) {
        const i32 boosted=limit(sample+sample*i32(amount)/16), half=boosted/2;
        const i32 cube=(half*half/16384)*half/16384;
        require(drive(sample,amount)==(amount ? boosted-2*cube/3 : sample),
                "every Drive sample and control value is unchanged");
    }
    for (u32 period=128;period<=815;++period)
        for (u32 numerator=0;numerator<=period;++numerator)
            require(ratio_q15(numerator,period,(32767u<<16)/period)==numerator*32767u/period,
                    "every clap burst ratio is unchanged");
    std::puts("PASS: exhaustive signal/harmonic divisions, all Drive samples/controls and clap burst ratios");
}
