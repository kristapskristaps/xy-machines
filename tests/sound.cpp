#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
const double pi = 3.141592653589793;
const double defaults[8] = {.45, .65, .6, .65, 0, 0, 0, .65};
struct Result { double energy, late, final, side, peak; };
Result render(const double* settings, unsigned seed, bool silent, bool automate = false) {
    lustre::State state;
    lustre::initialize(state);
    std::vector<opxy::i32> memory(lustre::kBufferWords, static_cast<opxy::i32>(seed));
    opxy::Buffer buffer = {memory.data(), lustre::kBufferWords};
    opxy::i32 left[64], right[64], parameters[8];
    Result result = {};
    for (unsigned block = 0; block < 11025; ++block) {
        for (unsigned j = 0; j < 8; ++j)
            parameters[j] = static_cast<opxy::i32>((automate ? ((block / 17 + j) & 1) : settings[j]) * 2147483647.);
        for (unsigned i = 0; i < 64; ++i) {
            const double t = (block * 64 + i) / 44100.;
            const double x = !silent && t < 1.5 ? .45 * std::sin(2 * pi * 220 * t) * std::exp(-2 * t) : 0;
            left[i] = right[i] = static_cast<opxy::i32>(x * 2147483647.);
        }
        lustre::process(state, buffer, left, right, parameters);
        for (unsigned i = 0; i < 64; ++i) {
            const double l = left[i] / 2147483648., r = right[i] / 2147483648.;
            const double power = l*l + r*r;
            result.energy += power;
            result.side += (l-r)*(l-r);
            if (block > 1400 && block < 2800) result.late += power;
            if (block > 10000) result.final += power;
            if (std::abs(l) > result.peak) result.peak = std::abs(l);
            if (std::abs(r) > result.peak) result.peak = std::abs(r);
        }
    }
    return result;
}
}

int main() {
    require(sizeof(lustre::State) <= opxy::effect_buffered_state_bytes, "state fits the send slot");
    require(alignof(lustre::State) <= 4, "state alignment fits the send slot");
    require(render(defaults, 0x7fffffff, true).energy == 0, "fresh effect never exposes dirty scratch");
    const Result clean = render(defaults, 0, false);
    const Result dirty = render(defaults, 0x98765432, false);
    require(clean.energy == dirty.energy, "scratch contents do not affect sound");
    require(clean.energy > 1, "defaults produce an audible wet signal");
    require(clean.side > clean.energy * .001, "mono input blooms into stereo");
    require(clean.late > .001, "reverb persists after the source ends");
    require(clean.final < clean.late * .001, "default tail decays");
    double extremes[8] = {1,1,1,1,1,1,1,1};
    const Result maximum = render(extremes, 0x12345678, false);
    require(maximum.peak < .99, "maximum settings retain headroom on the fixture");
    require(maximum.final < maximum.late * .01, "maximum decay eventually fades");
    const Result moving = render(defaults, 0x456789ab, false, true);
    require(moving.peak < .99, "rapid automation remains bounded");
    for (unsigned j = 0; j < 8; ++j) {
        double low[8], high[8];
        for (unsigned k = 0; k < 8; ++k) low[k] = high[k] = defaults[k];
        low[j] = 0;
        high[j] = 1;
        const Result a = render(low, 0, false), b = render(high, 0, false);
        require(std::abs(a.energy - b.energy) > .001, "each control changes the audible fixture");
    }
    lustre::State state;
    lustre::initialize(state);
    opxy::i32 left[64], right[64], parameters[8] = {};
    for (unsigned i = 0; i < 64; ++i) left[i] = right[i] = 123456;
    lustre::process(state, {nullptr,0}, left, right, parameters);
    for (unsigned i = 0; i < 64; ++i) require(!left[i] && !right[i], "no scratch returns silence");
    // A reset must discard the previous effect's tail without a bulk clear.
    std::vector<opxy::i32> memory(lustre::kBufferWords, 0x55555555);
    for (unsigned j = 0; j < 8; ++j) parameters[j] = 0x7fffffff;
    for (unsigned block = 0; block < 18000; ++block) {
        for (unsigned i = 0; i < 64; ++i) {
            // Slow full-scale square waves excite the tank instead of being
            // removed by the input's anti-alias filter; finish with silence.
            left[i] = block < 9000 ? ((block / 8) & 1 ? 0x7fffffff : -0x7fffffff - 1) : 0;
            right[i] = block < 9000 ? ((block / 13) & 1 ? 0x7fffffff : -0x7fffffff - 1) : 0;
        }
        lustre::process(state, {memory.data(),lustre::kBufferWords}, left, right, parameters);
    }
    lustre::initialize(state);
    for (unsigned block = 0; block < 200; ++block) {
        for (unsigned i = 0; i < 64; ++i) left[i] = right[i] = 0;
        lustre::process(state, {memory.data(),lustre::kBufferWords}, left, right, parameters);
        for (unsigned i = 0; i < 64; ++i) require(!left[i] && !right[i], "reset scratch stays silent");
    }
    std::printf("PASS: dirty scratch, audible defaults, stereo bloom, fading tails, automation, state and missing buffer (peak %.4f)\n", maximum.peak);
}
