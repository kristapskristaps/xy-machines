#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

struct Tail { double seconds, late, final, peak; };

Tail measure(double decay) {
    lustre::State state;
    lustre::initialize(state);
    std::vector<opxy::i32> memory(lustre::kBufferWords, 0x76543210);
    const double settings[8] = {.9, 1, decay, 1, .18, .2, .25, .65};
    opxy::i32 parameters[8], left[64], right[64];
    for (unsigned j = 0; j < 8; ++j) parameters[j] = static_cast<opxy::i32>(settings[j] * 2147483647.);
    double bins[600] = {};
    opxy::u32 random = 12345;
    Tail result = {};
    for (unsigned frame = 0; frame < 60 * 44100; frame += 64) {
        for (unsigned i = 0; i < 64; ++i) {
            random = random * 1664525u + 1013904223u;
            left[i] = right[i] = frame + i < 44100 ? static_cast<opxy::i32>(random) / 4 : 0;
        }
        lustre::process(state, {memory.data(),lustre::kBufferWords}, left, right, parameters);
        for (unsigned i = 0; i < 64 && frame + i < 60 * 44100; ++i) {
            const double l = left[i] / 2147483648., r = right[i] / 2147483648.;
            bins[(frame + i) / 4410] += l*l + r*r;
            if (std::abs(l) > result.peak) result.peak = std::abs(l);
            if (std::abs(r) > result.peak) result.peak = std::abs(r);
        }
    }
    // Schroeder integration after the grain cloud has finished: T30 uses
    // the -5 to -35 dB section and extrapolates its slope to a 60 dB decay.
    for (unsigned i = 100; i < 150; ++i) result.late += bins[i];
    for (unsigned i = 550; i < 600; ++i) result.final += bins[i];
    for (unsigned i = 599; i > 0; --i) bins[i-1] += bins[i];
    const double reference = bins[30];
    unsigned five = 0, thirty_five = 0;
    for (unsigned i = 30; i < 600; ++i) {
        if (!five && bins[i] < reference * std::pow(10., -.5)) five = i;
        if (!thirty_five && bins[i] < reference * std::pow(10., -3.5)) thirty_five = i;
    }
    require(five && thirty_five > five, "tail crosses both decay thresholds");
    result.seconds = (thirty_five - five) * .2;
    return result;
}
}

int main() {
    const Tail normal = measure(.6), hall = measure(1);
    std::printf("Host-rendered T30 extrapolation: normal %.1fs, hall %.1fs; late %.6g, final %.6g, peak %.4f\n",
                normal.seconds, hall.seconds, hall.late, hall.final, hall.peak);
    require(hall.seconds >= 15, "maximum Decay provides a long hall tail");
    require(hall.seconds > normal.seconds * 2, "upper Decay range meaningfully extends the tail");
    require(hall.late > .0001, "hall remains audible ten seconds after excitation");
    require(hall.final < hall.late * .0001, "long hall fades without runaway or an audible noise floor");
    require(hall.peak < .95, "long hall retains headroom");
}
