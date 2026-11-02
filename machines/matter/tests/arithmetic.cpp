#include "../engine.cpp"
#include <cstdio>

int main() {
    for(opxy::i32 value=-33554431;value<=33554431;++value) {
        if(matter::excitation_fifth(value)!=value/5) {
            std::printf("FAIL: excitation division at %d\n",value);return 1;
        }
    }
    std::puts("PASS: every bounded excitation fifth preserves signed truncation");
}
