#include "../engine.cpp"
#include <cstdio>

int main() {
    for(opxy::i32 value=-33554431;value<=33554431;++value) {
        if(matter::excitation_fifth(value)!=value/5) {
            std::printf("FAIL: excitation division at %d\n",value);return 1;
        }
    }
    opxy::u32 random=72;
    for(unsigned i=0;i<1000000;++i) {
        random=random*1664525u+1013904223u;
        const opxy::i32 value=static_cast<opxy::i32>(random);
        const opxy::i32 clipped=matter::clamp(value,-16777216,16777215);
        const opxy::i32 biased=(clipped+static_cast<opxy::i32>(static_cast<opxy::u32>(clipped)>>31)*511)>>9;
        if(biased!=matter::clamp(value/512,-32768,32767))return 1;
        if(((value+static_cast<opxy::i32>(static_cast<opxy::u32>(value)>>31)*7)>>3)!=value/8)return 1;
    }
    std::puts("PASS: every bounded excitation fifth preserves signed truncation; one million saturated bow quantization cases");
}
