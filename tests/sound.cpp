#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
static opxy::i32 q(double value) { return static_cast<opxy::i32>(value * 2147483647.0 + 0.5); }
static std::vector<double> render(opxy::i32* parameters, opxy::u32 increment, unsigned blocks = 192,
                                 unsigned warmup = 64) {
    vader::Voice voice = {};
    vader::Shared shared = {};
    vader::initialize(voice, shared, 1);
    vader::start(voice, shared, 1);
    opxy::u32 pitches[64];
    opxy::i32 output[64];
    for (unsigned i = 0; i < 64; ++i) pitches[i] = increment;
    std::vector<double> samples;
    for (unsigned b = 0; b < blocks + warmup; ++b) {
        vader::render(voice, shared, 1, output, parameters, pitches);
        for (unsigned i = 0; i < 64; ++i) {
            require(output[i] >= -1073741824 && output[i] <= 1073741824, "declared peak");
            if (b >= warmup) samples.push_back(output[i] / 2147483648.0);
        }
    }
    return samples;
}
static double magnitude(const std::vector<double>& samples, double cycles) {
    double real = 0, imag = 0;
    for (unsigned i = 0; i < samples.size(); ++i) {
        const double phase = 6.283185307179586 * cycles * i;
        real += samples[i] * std::cos(phase);
        imag += samples[i] * std::sin(phase);
    }
    return 2 * std::sqrt(real * real + imag * imag) / samples.size();
}
int main() {
    opxy::i32 p[8] = {0, q(.5), 0, 0, 0, q(.25), q(1), 0};
    const opxy::u32 bass = 1u << 22; // coherent 43.0664 Hz
    const auto clean = render(p, bass);
    require(magnitude(clean, 1.0 / 1024) > .22, "useful clean sub level");
    require(magnitude(clean, 3.0 / 1024) < .0001, "clean default is a sine");
    // Exact stable tuning at bass and high pitches, independent of drive mode.
    for (unsigned m = 0; m < 4; ++m) {
        p[2] = q(1); p[3] = q((m + .5) / 4); p[5] = q(.25);
        const auto driven = render(p, bass);
        require(magnitude(driven, 1.0 / 1024) > .10, "drive retains the fundamental");
        require(magnitude(driven, 2.0 / 1024) + magnitude(driven, 3.0 / 1024) +
                magnitude(driven, 5.0 / 1024) > .015, "four drive modes add audible harmonics");
        double mean = 0; for (double sample : driven) mean += sample;
        require(std::abs(mean / driven.size()) < .001, "asymmetric modes remain DC free");
        // At 5 kHz the legal harmonic set is constrained. Energy outside the
        // legal harmonic bins catches folded partials and table image aliases.
        const auto treble = render(p, 233u << 21, 32);
        double total = 0, legal = 0;
        for (double sample : treble) total += sample * sample;
        total /= treble.size();
        for (unsigned h = 1; h * 233 < 1024; ++h) {
            const double a = magnitude(treble, h * 233.0 / 2048);
            legal += a * a / 2;
        }
        require((total - legal) / total < .0001, "upper register suppresses alias energy");
    }
    p[2] = q(1); p[5] = q(1);
    const auto protected_sub = render(p, bass);
    require(magnitude(protected_sub, 3.0 / 1024) < .0001, "full Sub restores a clean fundamental");

    // Pitch drop starts high, descends monotonically, and settles on the key.
    vader::Voice voice = {};
    vader::Shared shared = {};
    opxy::u32 pitch[64]; for (unsigned i = 0; i < 64; ++i) pitch[i] = bass;
    opxy::i32 output[64];
    p[0] = q(1); p[1] = q(0); p[2] = 0; p[4] = q(1); p[5] = q(.25);
    vader::start(voice, shared, 1);
    double last = 20;
    for (unsigned b = 0; b < 150; ++b) {
        const opxy::u32 phase = voice.phase;
        vader::render(voice, shared, 1, output, p, pitch);
        const double ratio = static_cast<opxy::u32>(voice.phase - phase) / (64.0 * bass);
        require(ratio <= last + .001 && ratio >= .999, "pitch falls monotonically to key");
        if (b == 0) require(ratio > 10, "48 st punch begins near four octaves");
        last = ratio;
    }
    require(std::abs(last - 1) < .001, "pitch settles accurately");
    require(voice.click < 100, "click impulse ends");
    // Retrigger uses deterministic phase/noise; voices never share state.
    vader::Voice a = {}, b = {};
    vader::start(a, shared, 7); vader::start(b, shared, 7);
    opxy::i32 other[64];
    for (unsigned block = 0; block < 50; ++block) {
        vader::render(a, shared, 7, output, p, pitch);
        vader::render(b, shared, 7, other, p, pitch);
        require(!std::memcmp(output, other, sizeof(output)), "deterministic retrigger");
    }
    // Exercise all parameter corners and the entire unsigned pitch contract.
    unsigned seed = 98765;
    for (unsigned corner = 0; corner < 256; ++corner) {
        for (unsigned i = 0; i < 8; ++i) p[i] = corner & (1u << i) ? q(1) : 0;
        vader::start(a, shared, 7);
        for (unsigned block = 0; block < 5; ++block) {
            for (unsigned i = 0; i < 64; ++i) {
                seed = seed * 1664525u + 1013904223u; pitch[i] = seed;
            }
            vader::render(a, shared, 7, output, p, pitch);
            for (auto sample : output)
                require(sample >= -1073741824 && sample <= 1073741824, "corner peak");
        }
    }
    std::puts("PASS: clean sub, drive harmonics, DC, alias rejection, punch tuning, click decay, retrigger and 256 corners.");
}
