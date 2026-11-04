#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
static void require(bool ok, const char *why) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", why);
        std::exit(1);
    }
}
int main() {
    using namespace mando;
    for (unsigned cutoff = 0; cutoff <= 290; ++cutoff)
        for (unsigned resonance = 0; resonance <= 128; ++resonance) {
            const dsp::Filter filter(
                dsp::pole(static_cast<int>(cutoff * 65536)),
                static_cast<int>(2147483647ull * resonance / 128), true);
            for (unsigned section = 0; section < 2; ++section) {
                const long long a = 2147483648ll + 2ll * filter.a1[section] +
                                    filter.a2[section];
                const long long b = 2147483648ll - 2ll * filter.a1[section] +
                                    filter.a2[section];
                require(a > 64 && b > 64 && filter.a2[section] >= 0,
                        "low-rate sections retain Schur stability margin");
            }
        }
    const double pi = 3.14159265358979323846;
    double largest_image = 0;
    for (unsigned bin : {33u, 65u, 129u, 257u, 385u, 511u})
        for (int cutoff : {1073741823, 2147483647}) {
            Voice voice = {};
            Shared shared;
            start(voice, shared, 1);
            int p[8] = {cutoff, 0, 0, 0, 0, 0, 1073741824, 0}, out[64];
            unsigned pitch[64];
            const unsigned count = 8192,
                           increment = bin * (0x80000000u / (count / 2));
            for (auto &value : pitch)
                value = increment;
            double real[3] = {}, imag[3] = {};
            unsigned frame = 0;
            for (unsigned block = 0; block < 512; ++block) {
                render(voice, shared, 1, out, p, pitch);
                require(voice.rate_mode == 1, "bass notes use low-rate core");
                if (block < 384)
                    continue;
                for (int value : out) {
                    for (unsigned order = 0; order < 3; ++order) {
                        const unsigned image =
                            (count / 2 - (order * 2 + 1) * bin + count) % count;
                        const double angle = 2 * pi * image * frame / count;
                        real[order] += value / 2147483648. * std::cos(angle);
                        imag[order] += value / 2147483648. * std::sin(angle);
                    }
                    ++frame;
                }
            }
            for (unsigned order = 0; order < 3; ++order) {
                const double amplitude =
                    2 * std::hypot(real[order], imag[order]) / count;
                require(amplitude < .0031623,
                        "reconstruction images stay below-50dBFS under bright "
                        "stress");
                if (amplitude > largest_image)
                    largest_image = amplitude;
            }
        }
    Voice v = {};
    Shared shared;
    start(v, shared, 1);
    int p[8] = {1503238553, 1073741824, 1073741824, 0,
                0,          1073741824, 1073741824, 0},
        out[64];
    unsigned pitch[64];
    for (unsigned block = 0; block < 1000; ++block) {
        for (auto &value : pitch)
            value = block < 500 ? 10737418u : 429496729u;
        const int previous = v.last_output;
        render(v, shared, 1, out, p, pitch);
        if (block == 500)
            require(std::abs(out[0] - previous) <= 16777217,
                    "rate handoff begins with a bounded crossfade step");
        for (int value : out)
            require(value >= -536870912 && value <= 536870912,
                    "rate handoff respects output headroom");
        if (block > 500)
            require(v.rate_mode == 0,
                    "upper register keeps full-rate synthesis");
    }
    std::printf("PASS: low-rate coefficient stability, reconstruction images "
                "%.9gFS (%.2fdBFS), and rate handoff\n",
                largest_image, 20 * std::log10(largest_image));
}
