#include "../engine.cpp"
#include "cases.h"
#include <cstdio>
#include <cstdlib>
static void require(bool okay, const char *why) {
    if (!okay) {
        std::puts(why);
        std::exit(1);
    }
}
int main() {
    unsigned long long energy = 0;
    for (unsigned scene = 0; scene < 45; ++scene) {
        atlas::Voice voice = {};
        atlas::Shared shared = {};
        atlas::initialize(voice, shared, scene + 1);
        opxy::i32 p[8] = {}, output[64];
        opxy::u32 pitch[64];
        for (unsigned block = 0; block < 1536; ++block) {
            split_cases::parameters(scene, block, p);
            for (unsigned i = 0; i < 64; ++i)
                pitch[i] = split_cases::pitch(scene, block, i);
            if (scene == 43 && block % 11 == 0)
                atlas::start(voice, shared, scene + 1);
            atlas::render(voice, shared, scene + 1, output, p, pitch);
            for (auto sample : output) {
                require(sample >= -536870912 && sample <= 536870912, "FAIL output bound");
                energy += static_cast<unsigned long long>(
                    sample < 0 ? -static_cast<long long>(sample) : sample);
            }
            auto expected = pitch[32];
            if (expected < 4295)
                expected = 4295;
            if (expected > 0x40000000u)
                expected = 0x40000000u;
            require(voice.oscillator.e.wt.f == expected, "FAIL direct-rate pitch target");
            require(voice.oscillator.e.wt.build_at <= 132, "FAIL table preparation bound");
        }
    }
    require(energy > 0, "FAIL silent wavetable renderer");
    std::puts("PASS: 45 long native-rate wavetable scenes, pitch targets, control sweeps, glides, "
              "automation, retriggers and output bounds");
}
