#include "../dsp.hpp"
#include "../display.cpp"
#include <cstdio>
int main() {
    hkwavetable::port::Dsp reference = {};
    for (int value = -32768; value <= 32767; ++value) {
        for (unsigned point = 0; point < 25; ++point) {
            const unsigned phase = point * 178956970u;
            const int expected = reference.wavetable_shape(value, phase);
            const int actual = hkwavetable::wave_at_display_point(value, point);
            if (actual != expected) {
                std::fprintf(stderr, "FAIL: display wave at Q15 %d point %u: %d != %d\n",
                             value, point, actual, expected);
                return 1;
            }
        }
    }
    std::puts("PASS: 1,638,400 display samples match the original oscillator formula");
}
