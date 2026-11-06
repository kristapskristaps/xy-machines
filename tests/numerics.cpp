#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr,"FAIL: %s\n",message);
        std::exit(1);
    }
}

opxy::i32 accepted_return(opxy::i32 value) {
    const opxy::i32 knee = 33554432;
    const bool negative = value < 0;
    const opxy::i32 magnitude = negative ? -value : value;
    if (magnitude > knee) {
        opxy::i32 shaped = 2*knee;
        if (magnitude < 3*knee) {
            const opxy::i32 excess = magnitude-knee;
            shaped = knee+excess-opxy::q31::multiply(excess*4,excess*4);
        }
        value = negative ? -shaped : shaped;
    }
    for (unsigned bit = 0; bit < 5; ++bit) value = opxy::q31::add(value,value);
    return value;
}

int main() {
    const opxy::i32 boundaries[] = {0,33554432,67108864,100663296,268435456};
    for (opxy::i32 boundary : boundaries) {
        for (opxy::i32 delta = -16; delta <= 16; ++delta) {
            const opxy::i32 value = boundary+delta;
            require(lustre::restore(value) == accepted_return(value),"positive return boundary");
            require(lustre::restore(-value) == accepted_return(-value),"negative return boundary");
        }
    }
    opxy::u32 random = 0x98123456u;
    for (unsigned i = 0; i < 2000000; ++i) {
        random = random*1664525u+1013904223u;
        const opxy::i32 value = static_cast<opxy::i32>(random&0x1fffffffu)-268435456;
        require(lustre::restore(value) == accepted_return(value),"return gain rounding");
    }
    const opxy::u32 pans[] = {4096,4097,8191,16384,24575,28670,28671};
    for (opxy::u32 window = 0; window < 32768; ++window) {
        for (opxy::u32 pan : pans) {
            const opxy::u32 weights[] = {65536-pan,32768+pan};
            for (opxy::u32 weight : weights) {
                const opxy::i32 gain = opxy::q31::multiply(window*16384,weight*32768);
                require(gain == static_cast<opxy::i32>(window*weight>>2),"grain pan gain rounding");
            }
        }
    }
    std::vector<opxy::i32> memory(lustre::kBufferWords);
    for (auto& value : memory) {
        random = random*1664525u+1013904223u;
        value = static_cast<opxy::i32>(random&0xffffffu)-0x800000;
    }
    const opxy::u32 writers[] = {0,1,127,4095,32766,32767};
    const opxy::u32 histories[] = {0,1,63,64,65,511,5000,12000,32767,32768};
    const opxy::i32 speeds[] = {-131072,-130999,-65536,-65000,65000,65536,130999,131072};
    for (auto write : writers) for (auto written : histories) for (auto speed : speeds) {
        const opxy::u32 ages[] = {0,1,2,63,64,65,32703,32704,32767,
                                  written, (written+1)&32767u,(written+2)&32767u,(written-1)&32767u};
        for (auto age : ages) for (opxy::u32 fraction : {0u,1u,32768u,65535u})
        for (opxy::u32 requested : {1u,2u,16u,31u,32u})
        for (opxy::u32 remaining : {1u,17u,31u,32u,33u}) {
            lustre::State state;
            lustre::initialize(state);
            state.write = write;
            state.written = written;
            auto& grain = state.grains[0];
            grain.position = (((write-age)&32767u)<<16)+fraction;
            grain.phase = random;
            grain.increment = 0xffffffffu/1323;
            grain.speed = speed;
            grain.pan = 24575;
            grain.remaining = remaining;
            auto reference = grain;
            opxy::i32 wet[2][32] = {}, expected[2][32] = {};
            const bool safe = lustre::history_safe(state,requested);
            const auto count = safe ? requested : 1u;
            const bool fractional = fraction || (static_cast<opxy::u32>(speed)&65535u);
            if (fractional)
                lustre::grain_span<false,true>(reference,memory.data(),write,written,count,expected[0],expected[1]);
            else
                lustre::grain_span<false,false>(reference,memory.data(),write,written,count,expected[0],expected[1]);
            lustre::cloud_span(state,memory.data(),wet,0,count,safe);
            for (unsigned ch = 0; ch < 2; ++ch) for (unsigned i = 0; i < 32; ++i)
                require(wet[ch][i] == expected[ch][i],"startup history classification");
            require(grain.position == reference.position && grain.phase == reference.phase &&
                    grain.remaining == reference.remaining && grain.increment == reference.increment &&
                    grain.speed == reference.speed && grain.pan == reference.pan,"silent grain advancement");
        }
    }
    std::puts("PASS: return boundaries, grain gains and startup history paths are exact");
}
