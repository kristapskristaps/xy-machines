#include "../engine.cpp"
#include <cmath>
#include <initializer_list>
#include <cstdio>
#include <cstdlib>
static void require(bool okay, const char *why) {
    if (!okay) {
        std::puts(why);
        std::exit(1);
    }
}
int main() {
    double minimum_fundamental = 1, maximum_upper = 0;
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
                    double energy = 0, fundamental = 0, upper = 0;
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
                            fundamental = bin;
                        if (k > 10)
                            upper += bin;
                    }
                    require(energy > 1000000, "FAIL alternate lost its body");
                    const double body = fundamental / energy, air = upper / energy;
                    if (body < minimum_fundamental)
                        minimum_fundamental = body;
                    if (air > maximum_upper)
                        maximum_upper = air;
                    require(body > .5, "FAIL alternate lost its pitched fundamental");
                    require(air < .025, "FAIL alternate dominated by upper harmonics");
                }
    std::printf("PASS: 81 alternate patches retain at least %.1f%% fundamental energy; above "
                "harmonic10 at most %.2f%%\n",
                100 * minimum_fundamental, 100 * maximum_upper);
}
