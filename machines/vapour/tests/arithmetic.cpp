#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>

static void require(bool ok, const char* message, opxy::i32 value) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s at %d\n", message, value);
        std::exit(1);
    }
}

int main() {
    const opxy::i32 samples[]={-536870912,-1048576,-524288,-1,0,1,524288,1048576,536870912};
    const opxy::i32 gains[]={0,1,32767,32768,65534,65535};
    for(opxy::i32 a:samples)for(opxy::i32 b:gains) {
        const opxy::i64 product=static_cast<opxy::i64>(a)*b;
        const opxy::i64 expected=product/32768-(product<0 && product%32768!=0);
        require(vapour::audio_gain15(a,b)==expected,"bounded audio product boundary",a);
    }
    opxy::u32 random=7139;
    for(unsigned i=0;i<1000000;++i) {
        random=random*1664525u+1013904223u;
        const opxy::i32 a=static_cast<opxy::i32>(random%1073741825u)-536870912;
        random=random*1664525u+1013904223u;
        const opxy::i32 b=static_cast<opxy::i32>(random>>16);
        const opxy::i64 product=static_cast<opxy::i64>(a)*b;
        const opxy::i64 expected=product/32768-(product<0 && product%32768!=0);
        require(vapour::audio_gain15(a,b)==expected,"bounded audio product reference",a);
        require(vapour::audio_gain15(a,b)==vapour::granular::mul15(a,b),"preceding split product",a);
    }
    // Entire reachable normalization-difference range, with margin, checked
    // against signed division rather than the reciprocal implementation.
    for (opxy::i32 value = -131072; value <= 131072; ++value) {
        require(vapour::normalization_step(value) == value / 100,
                "normalization quotient", value);
        const opxy::i64 product = static_cast<opxy::i64>(value) * 5;
        const opxy::i64 floor = product / 8 - (product < 0 && product % 8 != 0);
        require(vapour::allpass_gain(value) == floor, "allpass coefficient", value);
        require(vapour::allpass_gain(value) == vapour::granular::mul15(value, 20480),
                "original allpass coefficient", value);
    }
    // Exercise the bound using every texture/overlap value and active count.
    for (unsigned value = 0; value <= 32767; ++value) {
        const opxy::i32 window = static_cast<opxy::i32>(value < 12288 ? 32767 + value * 8 / 3 : 65534);
        for (unsigned active = 0; active <= vapour::grain_capacity; ++active) {
            const opxy::i32 base = static_cast<opxy::i32>(vapour::tables::normalization[active]);
            const opxy::i32 gain = base + vapour::granular::mul15(base,
                vapour::granular::mul15(window - 32767, 32767));
            require(gain >= 0 && gain <= 65534, "maximum normalization target", gain);
            const opxy::i32 low = base + vapour::granular::mul15(base,
                vapour::granular::mul15(32767, static_cast<opxy::i32>(value)));
            require(low >= base && low <= 65534, "normalization overlap bound", low);
        }
    }
    std::puts("PASS: exact normalization quotient, allpass coefficient and normalization bounds");
}
