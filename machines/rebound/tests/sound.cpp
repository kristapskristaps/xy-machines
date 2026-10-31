#include "../fx.cpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

using opxy::i32;
using opxy::u32;
static const i32 full = 2147483647;
static const double pi = 3.14159265358979323846;
static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
static i32 normalized(double value) { return static_cast<i32>(value * full); }
static i32 option(unsigned index, unsigned count) {
    return static_cast<i32>((static_cast<opxy::i64>(2 * index + 1) * 2147483648LL) / (2 * count));
}
static unsigned feedback_index(double percent) {
    if (percent < 80) return static_cast<unsigned>(percent);
    if (percent <= 120) return 80 + static_cast<unsigned>((percent - 80) * 2);
    return 161 + static_cast<unsigned>(percent - 121);
}
struct Fixture {
    rebound::State state;
    std::vector<i32> words;
    i32 p[8];
    opxy::Transport transport;
    std::vector<double> left, right;
    Fixture(double feedback = 50, double pingpong = 1) :
        words(rebound::buffer_words, 0x60000000), transport{1500, 0, true} {
        rebound::initialize(state);
        p[0] = option(2, 8); p[1] = option(feedback_index(feedback), 191);
        p[2] = normalized(pingpong); p[3] = full;
        p[4] = option(50, 101); p[5] = 0; p[6] = full; p[7] = full;
    }
    template <class Input>
    void run(unsigned blocks, Input input, opxy::Buffer buffer) {
        i32 l[64], r[64];
        for (unsigned b = 0; b < blocks; ++b) {
            for (unsigned i = 0; i < 64; ++i) input(b * 64 + i, l[i], r[i]);
            rebound::process(state, buffer, transport, l, r, p);
            for (unsigned i = 0; i < 64; ++i) {
                require(std::abs(static_cast<opxy::i64>(l[i])) <= 2147483646LL &&
                        std::abs(static_cast<opxy::i64>(r[i])) <= 2147483646LL, "wet return exceeds ceiling");
                left.push_back(l[i] / 2147483648.0);
                right.push_back(r[i] / 2147483648.0);
            }
        }
    }
    template <class Input> void run(unsigned blocks, Input input) {
        run(blocks, input, opxy::Buffer{words.data(), static_cast<u32>(words.size())});
    }
};
static void silence(unsigned, i32& l, i32& r) { l = r = 0; }
static void impulse(unsigned n, i32& l, i32& r) { l = r = n == 0 ? normalized(0.2) : 0; }
static void burst(unsigned n, i32& l, i32& r) {
    l = r = n < 256 ? normalized(0.02 * std::sin(2 * pi * n / 32)) : 0;
}
static double peak(const std::vector<double>& samples, size_t from, size_t to) {
    double result = 0;
    for (size_t i = from; i < std::min(to, samples.size()); ++i)
        result = std::max(result, std::fabs(samples[i]));
    return result;
}
static double both(const Fixture& f, size_t from, size_t to) {
    return std::max(peak(f.left, from, to), peak(f.right, from, to));
}
static size_t first_repeat(unsigned size, unsigned fine, u32 bpm) {
    Fixture f(0); f.p[0] = option(size, 8); f.p[4] = option(fine, 101); f.transport.bpm_x10 = bpm;
    f.run(1100, impulse);
    for (size_t i = 0; i < f.left.size(); ++i) if (f.left[i] != 0) return i;
    return 0;
}
static double filtered_energy(unsigned hp, unsigned lp, double frequency) {
    Fixture f(100, 0); f.p[5] = option(hp, 33); f.p[6] = option(lp, 33);
    f.run(72, [frequency](unsigned n, i32& l, i32& r) {
        l = r = n < 4000 ? normalized(0.02 * std::sin(2 * pi * frequency * n / 44100)) : 0;
    });
    f.run(160, silence);
    double energy = 0;
    for (size_t i = 2 * 4410 + 512; i < 2 * 4410 + 3500; ++i) energy += f.left[i] * f.left[i];
    return energy;
}
int main() {
    // Unwritten scratch is never audible; a full ping-pong alternates repeats.
    Fixture bounce;
    bounce.run(300, impulse);
    const size_t d = 4410;
    require(both(bounce, 0, d) == 0, "stale scratch or dry audio heard before first repeat");
    require(std::fabs(bounce.left[d] - 0.2) < 1e-6, "first left repeat timing/amplitude");
    require(peak(bounce.right, d, d + 8) < 1e-7, "first repeat leaked right");
    require(std::fabs(bounce.right[2*d] / bounce.left[d] - 0.5) < 0.002, "50 percent loop gain");
    require(std::fabs(bounce.left[3*d] / bounce.right[2*d] - 0.5) < 0.002, "third repeat did not cross back");
    require(peak(bounce.left, 2*d, 2*d + 8) < 1e-6, "second repeat leaked left");

    Fixture stereo(0, 0);
    stereo.run(150, [](unsigned n, i32& l, i32& r) { l = n == 0 ? normalized(0.2) : 0; r = n == 0 ? normalized(-0.1) : 0; });
    require(std::fabs(stereo.left[d] - 0.2) < 1e-6 && std::fabs(stereo.right[d] + 0.1) < 1e-6,
            "zero PingPong must preserve stereo input");
    require(both(stereo, 2*d, stereo.left.size()) == 0, "zero feedback repeats twice");
    Fixture halfway(0, 0.5);
    halfway.run(150, impulse);
    require(std::fabs(halfway.left[d] - 0.2) < 1e-6 && std::fabs(halfway.right[d] - 0.1) < 1e-6,
            "partial PingPong injection");

    // Feedback determines decay, not the level of the first repeat. Check real
    // wet amplitudes as well as ratios: a missing headroom recovery passes ratios.
    for (double gain : {0.0, 10.0, 25.0, 50.0, 75.0}) {
        Fixture ordinary(gain, 0);
        ordinary.p[3] = normalized(0.65);
        ordinary.run(220, [](unsigned n, i32& l, i32& r) {
            l = r = n == 0 ? normalized(0.4) : 0;
        });
        require(std::fabs(ordinary.left[d] - 0.4 * 0.65) < 1e-6 &&
                std::fabs(ordinary.right[d] - 0.4 * 0.65) < 1e-6,
                "normal delay must return the first repeat at the requested wet level");
        require(std::fabs(ordinary.left[2*d] - 0.4 * 0.65 * gain / 100) < 0.0005,
                "low feedback repeats are too quiet or have the wrong decay");
        require(std::fabs(ordinary.left[3*d] - 0.4 * 0.65 * gain * gain / 10000) < 0.001,
                "third normal repeat has the wrong level");
    }

    const double beats[] = {0.125, 0.1875, 0.25, 0.375, 0.5, 0.75, 1, 2};
    for (unsigned i = 0; i < 8; ++i)
        require(std::fabs(first_repeat(i, 50, 1200) - beats[i] * 22050) <= 1, "tempo division timing");
    require(std::fabs(first_repeat(4, 100, 1200) - 13230) <= 1, "positive Fine timing");
    require(std::fabs(first_repeat(4, 0, 1200) - 8820) <= 1, "negative Fine timing");
    require(first_repeat(4, 50, 0) == first_repeat(4, 50, 1200), "missing tempo fallback");
    require(first_repeat(7, 0, 1) == rebound::line_frames - 2, "long delay clamp");
    require(first_repeat(0, 0, std::numeric_limits<u32>::max()) == 512, "short delay clamp");

    for (double gain : {99.5, 100.0, 100.5, 125.0, 150.0}) {
        Fixture f(gain);
        f.run(160, impulse);
        require(std::fabs(f.right[2*d] / f.left[d] - gain / 100) < 0.003,
                "shown feedback must match actual gain around and above unity");
    }
    Fixture unity(100, 0); unity.p[0] = option(0, 8);
    unity.run(1600, burst);
    require(both(unity, 90000, 100000) > 0.004, "unity feedback failed to sustain a quiet tone");

    Fixture hot(150); hot.p[0] = option(0, 8);
    hot.run(5000, burst);
    require(std::fabs(both(hot, 2205, 2205 + 256) - 0.02) < 1e-6,
            "the first repeat must retain the quiet seed's level above unity too");
    require(both(hot, 280000, 320000) > 0.2, "above-unity feedback did not build into sustained oscillation");
    hot.p[1] = option(feedback_index(50), 191);
    hot.run(3500, silence);
    require(both(hot, hot.left.size() - 22050, hot.left.size()) < 1e-5, "feedback cannot be pulled back after oscillation");

    Fixture dc(150, 0); dc.p[0] = option(0, 8);
    dc.run(7000, [](unsigned n, i32& l, i32& r) { l = r = n < 10000 ? normalized(0.5) : 0; });
    double mean = 0;
    for (size_t i = dc.left.size() - 44100; i < dc.left.size(); ++i) mean += dc.left[i];
    require(std::fabs(mean / 44100) < 0.01, "DC accumulated in the feedback loop");
    require(filtered_energy(32, 32, 100) < filtered_energy(0, 32, 100) * 0.02, "HPF does not remove bass from repeats");
    require(filtered_energy(0, 0, 8000) < filtered_energy(0, 32, 8000) * 0.02, "LPF does not darken repeats");

    // Wet Level and Dry never change the circulating loop.
    Fixture audible(125, 0), muted(125, 0);
    muted.p[3] = 0; muted.p[7] = 0;
    audible.run(1000, burst); muted.run(1000, burst);
    require(both(muted, 0, muted.left.size()) == 0, "Level zero is not silent");
    require(audible.words == muted.words, "wet Level or Dry changed feedback");
    muted.p[3] = full;
    muted.run(100, silence);
    require(both(muted, muted.left.size() - 6400, muted.left.size()) > 0.01, "muting Level killed the loop");

    rebound::initialize(bounce.state);
    bounce.left.clear(); bounce.right.clear();
    bounce.run(200, silence);
    require(both(bounce, 0, bounce.left.size()) == 0, "reset exposed previous delay contents");
    for (opxy::Buffer absent : {opxy::Buffer{nullptr, 0}, opxy::Buffer{bounce.words.data(), 1}}) {
        Fixture f;
        f.run(10, impulse, absent);
        require(both(f, 0, f.left.size()) == 0 && !f.state.written, "missing/short buffer must stay silent");
    }
    // Control motion and extreme inputs exercise the interpolation, filters and
    // wide saturation arithmetic. Run this fixture under ASan/UBSan as well.
    Fixture swept(150);
    u32 random = 123456789;
    for (unsigned block = 0; block < 1600; ++block) {
        if (block % 8 == 0) {
            for (unsigned p = 0; p < 8; ++p) {
                random = random * 1664525u + 1013904223u;
                swept.p[p] = static_cast<i32>(random & 0x7fffffff);
            }
            swept.transport.bpm_x10 = (block % 3) ? 300 + block * 2 : 0;
        }
        swept.run(1, [&random](unsigned, i32& l, i32& r) {
            random = random * 1664525u + 1013904223u;
            l = random & 1 ? std::numeric_limits<i32>::min() : full;
            r = random & 2 ? std::numeric_limits<i32>::min() : full;
        });
    }
    std::puts("PASS: Rebound timing, stereo, growth/recovery, filters, bounded output, reset and control sweeps");
}
