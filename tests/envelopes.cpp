#include "../dsp/primitives.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
int main() {
    double worst = 0;
    opxy::i32 previous = 0;
    for (unsigned milliseconds = 4; milliseconds <= 6144; ++milliseconds) {
        const opxy::i32 coefficient = x0x::dsp::rate(milliseconds);
        require(coefficient >= previous && coefficient < 2147483647, "stable monotonic envelope coefficients");
        const double measured = std::log(0.001) / (std::log(double(coefficient) / 2147483647.0) * 44.1);
        const double error = std::fabs(measured / milliseconds - 1);
        if (error > worst) worst = error;
        require(error < 0.004, "T60 stays within 0.4% of its requested duration");
        previous = coefficient;
    }
    require(x0x::dsp::rate(0) == x0x::dsp::rate(4), "minimum duration is clamped");
    require(x0x::dsp::rate(0xffffffffu) == x0x::dsp::rate(6144), "maximum duration is clamped");
    std::printf("PASS: all 6,141 envelope durations, worst T60 error %.4f%%\n", worst * 100);
}
