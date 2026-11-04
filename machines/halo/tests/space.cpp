#include "../fx.cpp"
#include "plate_reference/fx.cpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
static void require(bool ok, const char *text) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", text);
        std::exit(1);
    }
}
struct Metrics {
    double energy, late, correlation, density, t20;
};
template <class State, class Init, class Process>
Metrics measure(const int *parameters, Init initialize, Process process) {
    State state = {};
    initialize(state);
    std::vector<int> memory(
        std::max(halo::buffer_words, halo_plate::buffer_words));
    int p[8], l[64], r[64];
    for (unsigned i = 0; i < 8; ++i)
        p[i] = parameters[i];
    const unsigned frames = 44100 * 8, blocks = (frames + 63) / 64;
    std::vector<double> sums(blocks);
    double ll = 0, rr = 0, lr = 0, total = 0, late = 0;
    unsigned filled = 0, count = 0;
    for (unsigned block = 0; block < blocks; ++block) {
        for (unsigned i = 0; i < 64; ++i) {
            l[i] = block == 0 && i == 0 ? 536870912 : 0;
            r[i] = l[i];
        }
        process(
            state,
            opxy::Buffer{memory.data(), static_cast<unsigned>(memory.size())},
            opxy::Transport{1200, 0, true}, l, r, p);
        for (unsigned i = 0; i < 64; ++i) {
            const double a = l[i] / 2147483648., b = r[i] / 2147483648.,
                         e = a * a + b * b;
            total += e;
            sums[block] += e;
            if (block * 64 + i > 44100 / 3) {
                late += e;
                ll += a * a;
                rr += b * b;
                lr += a * b;
                if (std::abs(a) + std::abs(b) > 1e-6)
                    ++filled;
                ++count;
            }
        }
    }
    double remaining = 0;
    for (unsigned i = blocks; i-- > 0;) {
        remaining += sums[i];
        sums[i] = remaining;
    }
    unsigned start = 0, end = 0;
    for (unsigned i = 0; i < blocks; ++i) {
        if (!start && sums[i] < total * .316227766)
            start = i;
        if (!end && sums[i] < total * .00316227766)
            end = i;
    }
    return {total, late, lr / std::sqrt(ll * rr), double(filled) / count,
            (end - start) * 64. / 44100 * 3};
}
int main() {
    const int presets[3][8] = {{858993459, 1181116006, 257698038, 107374182,
                                966367642, 1460288880, 0, 2147483647},
                               {1395864371, 1567663063, 1417339207, 214748365,
                                816043786, 1460288880, 0, 2147483647},
                               {1675037245, 1825361100, 601295421, 1546188226,
                                1331439861, 1116691496, 0, 2147483647}};
    for (unsigned i = 0; i < 3; ++i) {
        auto a = measure<halo_plate::State>(presets[i], halo_plate::initialize,
                                            halo_plate::process);
        auto b =
            measure<halo::State>(presets[i], halo::initialize, halo::process);
        std::printf(
            "SPACE %u old_energy %.9g new_energy %.9g delta_db %.3f old_late "
            "%.9g new_late %.9g old_corr %.5f new_corr %.5f old_density %.5f "
            "new_density %.5f old_t60 %.3f new_t60 %.3f\n",
            i, a.energy, b.energy, 10 * std::log10(b.energy / a.energy), a.late,
            b.late, a.correlation, b.correlation, a.density, b.density, a.t20,
            b.t20);
        require(b.energy > a.energy * .5 && b.energy < a.energy * 2,
                "factory IR energy remains within3dB of the plate");
        require(b.late > a.late * .5 && b.late < a.late * 2,
                "late tail energy remains within3dB of the plate");
        require(b.t20 > a.t20 * .7 && b.t20 < a.t20 * 1.4,
                "factory decay time preserves the spacious scale");
        require(std::isfinite(b.energy) && b.energy > 1e-9 && b.late > 1e-10,
                "audible late stereo tail");
        require(std::abs(b.correlation) < .95,
                "late tail remains spatial rather than mono");
        require(b.density > .1, "late tail retains a useful diffusion density");
    }
}
