#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
static void require(bool okay, const char *why) {
    if (!okay) {
        std::puts(why);
        std::exit(1);
    }
}
int main() {
    double minimum_low = 1, maximum_upper = 0;
    for (unsigned h : {0u, 63u, 127u})
        for (unsigned t : {0u, 63u, 127u})
            for (unsigned m : {0u, 63u, 127u})
                for (unsigned increment : {5356534u, 25479940u, 85704562u}) {
                    sift::Voice v = {};
                    sift::Shared shared = {};
                    sift::initialize(v, shared, 1);
                    opxy::i32 p[8] = {static_cast<opxy::i32>(h << 24),
                                      static_cast<opxy::i32>(t << 24),
                                      static_cast<opxy::i32>(m << 24), 2147483647},
                              output[64];
                    opxy::u32 pitch[64];
                    for (auto &value : pitch)
                        value = increment;
                    for (unsigned block = 0; block < 64; ++block)
                        sift::render(v, shared, 1, output, p, pitch);
                    const auto *wave = v.cycle[v.active][1];
                    double energy = 0, low = 0, upper = 0, unlocked = 0, note = 0, sub = 0;
                    for (unsigned i = 0; i < 256; ++i) {
                        const double sample = wave[i] - v.cycle_mean[v.active][1];
                        energy += sample * sample;
                    }
                    for (unsigned k = 1; k <= 128; ++k) {
                        double real = 0, imag = 0;
                        for (unsigned i = 0; i < 256; ++i) {
                            const double sample = wave[i] - v.cycle_mean[v.active][1],
                                         angle = 6.2831853071795864769 * k * i / 256;
                            real += sample * std::cos(angle);
                            imag += sample * std::sin(angle);
                        }
                        const double bin = (k == 128 ? 1. : 2.) * (real * real + imag * imag) / 256;
                        if (k == 1)
                            sub = bin;
                        if (k == 2)
                            note = bin;
                        if (k <= 2)
                            low += bin;
                        else
                            upper += bin;
                        if (k != 1 && k != 2 && k != 4 && k != 6)
                            unlocked += bin;
                    }
                    require(energy > 1000000, "FAIL alternate lost its body");
                    const double body = low / energy, air = upper / energy;
                    if (body < minimum_low)
                        minimum_low = body;
                    if (air > maximum_upper)
                        maximum_upper = air;
                    require(body > .9 && note / energy > .2 && sub / energy > .08,
                            "FAIL body lost note/sub weight");
                    require(unlocked / energy < .00001,
                            "FAIL body contains unlocked or noisy harmonics");
                    require(air < .1, "FAIL Body adds too much upper-harmonic energy");
                }
    std::printf("PASS: 81 Body patches: note/sub at least %.1f%% energy, upper harmonics at most "
                "%.2f%%; only locked tones\n",
                100 * minimum_low, 100 * maximum_upper);
}
