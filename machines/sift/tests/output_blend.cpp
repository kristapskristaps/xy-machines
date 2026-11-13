#include "../engine.cpp"
#include "cases.h"
#include "reference/installed_before_mix.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool okay, const char *why) {
    if (!okay) {
        std::puts(why);
        std::exit(1);
    }
}
int main() {
    opxy::i32 p[8] = {}, a[64], b[64];
    opxy::u32 pitch[64];
    for (unsigned scene = 0; scene < 37; ++scene) {
        sift::Voice candidate = {};
        sift::Shared shared = {};
        sift::initialize(candidate, shared, 1);
        sift_installed::Voice installed = {};
        sift_installed::Shared oldshared = {};
        sift_installed::initialize(installed, oldshared, 1);
        split_cases::parameters(scene, 0, p);
        p[3] = 0;
        for (auto &value : pitch)
            value = 25479940;
        for (unsigned block = 0; block < 160; ++block) {
            sift::render(candidate, shared, 1, a, p, pitch);
            sift_installed::render(installed, oldshared, 1, b, p, pitch);
            require(!std::memcmp(a, b, sizeof a), "FAIL accepted main sound changed");
        }
    }
    opxy::i32 snapshots[9][64];
    double energies[9] = {};
    for (unsigned position = 0; position < 9; ++position) {
        sift::Voice v = {};
        sift::Shared shared = {};
        sift::initialize(v, shared, 1);
        split_cases::parameters(0, 0, p);
        p[3] = 0;
        for (auto &value : pitch)
            value = 25479940;
        for (unsigned block = 0; block < 64; ++block)
            sift::render(v, shared, 1, a, p, pitch);
        require(!v.building && !v.fade, "FAIL tables not settled");
        p[3] = static_cast<opxy::i32>(2147483647ull * position / 8);
        sift::render(v, shared, 1, a, p, pitch);
        require(!v.building && !v.fade, "FAIL mix knob rebuilt oscillator tables");
        require(v.mix == static_cast<opxy::i32>((static_cast<opxy::u32>(p[3]) + 32768u) >> 16),
                "FAIL incomplete knob travel");
        sift::render(v, shared, 1, snapshots[position], p, pitch);
        for (auto value : snapshots[position])
            energies[position] += double(value) * value;
    }
    for (unsigned position = 1; position < 8; ++position) {
        bool changed = false;
        for (unsigned i = 0; i < 64; ++i) {
            const auto from = snapshots[0][i], to = snapshots[8][i];
            const auto expected =
                static_cast<opxy::i64>(from) + (static_cast<opxy::i64>(to) - from) * position / 8;
            const auto error = static_cast<opxy::i64>(snapshots[position][i]) - expected;
            require(error >= -16384 && error <= 16384,
                    "FAIL blend is not continuous across full knob range");
            if (snapshots[position][i] != snapshots[position - 1][i])
                changed = true;
        }
        require(changed, "FAIL dead region in mix control");
        require(energies[position] > .2 * std::fmin(energies[0], energies[8]),
                "FAIL blend cancels to silence");
    }
    std::puts("PASS: accepted main PCM unchanged across37 scenes; full-range linear blend, "
              "fixed-phase response, no table rebuilding or midpoint collapse");
}
