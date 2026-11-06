// Audio signatures from accepted commit 6de273f. Covers startup, history wraps,
// feedback, silence, all-max controls, automation and the reverb clock wrapping.
#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

void require(bool condition, unsigned test, unsigned long long actual) {
    if (!condition) {
        std::fprintf(stderr,"FAIL: accepted audio stream %u changed (%llu)\n",test,actual);
        std::exit(1);
    }
}

unsigned long long stream(unsigned test) {
    lustre::State state;
    lustre::initialize(state);
    if (test == 5) state.reverb_write = 0xfffffff0u;
    std::vector<opxy::i32> memory(lustre::kBufferWords, 0x34567812);
    const double presets[4][8] = {
        {.45,.65,.6,.65,0,0,0,.65}, {.5,.85,.72,.8,.35,.1,.2,.72},
        {.9,1,1,1,.18,.2,.25,.65}, {1,1,1,1,1,1,1,1}
    };
    opxy::i32 parameters[8], left[64], right[64];
    opxy::u32 random = 123456;
    unsigned long long digest = 14695981039346656037ull;
    for (unsigned block = 0; block < 6000; ++block) {
        for (unsigned j = 0; j < 8; ++j) {
            const double value = test == 4 ? ((block/17+j)&1) : presets[test<4 ? test : 2][j];
            parameters[j] = static_cast<opxy::i32>(value*2147483647.);
        }
        for (unsigned i = 0; i < 64; ++i) {
            random = random*1664525u+1013904223u;
            const opxy::i32 value = static_cast<opxy::i32>(random & 0x3fffffffu)-0x20000000;
            left[i] = block < 3000 ? value : 0;
            right[i] = block < 3000 ? value/2 : 0;
        }
        lustre::process(state, {memory.data(),lustre::kBufferWords}, left,right,parameters);
        for (unsigned i = 0; i < 64; ++i) {
            digest = (digest ^ static_cast<opxy::u32>(left[i])) * 1099511628211ull;
            digest = (digest ^ static_cast<opxy::u32>(right[i])) * 1099511628211ull;
        }
    }
    return digest;
}

int main() {
    const unsigned long long expected[] = {
        14667542933649493253ull, 17503455584369474053ull, 15353162884857559653ull,
        10725287907247887301ull, 1034581004368163333ull, 15353162884857559653ull
    };
    for (unsigned i = 0; i < 6; ++i) {
        const unsigned long long actual = stream(i);
        require(actual == expected[i],i,actual);
    }
    std::puts("PASS: all six accepted audio streams are bit exact");
}
