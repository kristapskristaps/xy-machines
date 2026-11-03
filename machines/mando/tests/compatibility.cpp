#include "reference/engine.cpp"
#include <cstdio>
namespace {
const opxy::i32 patches[8][8] = { {1331439861, 0, 0, 0, 0, 1073741824, 1073741824, 0},
{1030792151, 322122547, 386547056, 0, 257698038, 1116691496, 1073741824, 858993459},
{622770258, 107374182, 0, 1879048191, 0, 1073741824, 0, 1395864371},
{1352914698, 1438814043, 687194767, 805306368, 343597384, 923417968, 1073741824, 751619276},
{1181116006, 644245094, 1288490188, 1342177279, 171798692, 1288490188, 0, 1073741824},
{1567663062, 708669604, 257698038, 1879048191, 429496729, 1073741824, 1073741824, 1395864371},
{1309965025, 2018634628, 171798692, 0, 0, 0, 1073741824, 2147483647},
{2147483647, 0, 0, 0, 515396075, 1073741824, 1073741824, 0} };
opxy::u32 fingerprint(unsigned patch) {
    mando::Voice voice = {}; mando::Shared shared;
    opxy::i32 parameters[8], output[64]; opxy::u32 pitch[64];
    opxy::u32 hash = 2166136261u, seed = 9137u + patch;
    const opxy::u32 notes[] = {0u, 3185120u, 5356534u, 10713067u, 42852281u, 171409123u, 685636492u};
    for (unsigned j = 0; j < 8; ++j) parameters[j] = patches[patch][j];
    mando::initialize(voice, shared, 1);
    for (unsigned block = 0; block < 256; ++block) {
        if (block % 128 == 0) mando::start(voice, shared, 1);
        if (block >= 128 && block % 8 == 0) for (unsigned j = 0; j < 8; ++j) {
            seed = seed * 1664525u + 1013904223u;
            parameters[j] = static_cast<opxy::i32>(seed >> 1);
        }
        for (unsigned i = 0; i < 64; ++i)
            pitch[i] = notes[(block / 16) % 7] + (block & 1u ? i * 12345u : 0u);
        mando::render(voice, shared, 1, output, parameters, pitch);
        for (unsigned i = 0; i < 64; ++i) hash = (hash ^ static_cast<opxy::u32>(output[i])) * 16777619u;
    }
    return hash;
}
}

int main() {
    // The saturation lookup must preserve the old integer interpolant everywhere,
    // including both boundaries of the fast range and the overload fallback.
    for (int x = -131200; x <= 131200; ++x) {
        const unsigned a = static_cast<unsigned>(x < 0 ? -x : x);
        const int positive = a >= 131072 ? 16384
            : mando::dsp::linear(mando::tables::tanh_q14, a, 8);
        const int expected = x < 0 ? -positive : positive;
        if (mando::dsp::saturate(x) != expected) {
            std::puts("FAIL: saturation curve changed"); return 1;
        }
    }
    // Captured from the auditioned B filter experiment before CPU optimization.
    const opxy::u32 expected[8] = {
        2732072389u, 139173317u, 3189185989u, 2357042629u,
        3703184837u, 2722373061u, 2208374213u, 46702021u
    };
    for (unsigned patch = 0; patch < 8; ++patch) if (fingerprint(patch) != expected[patch]) {
        std::printf("FAIL: auditioned B patch %u output changed\n", patch); return 1;
    }
    // The original two-word recurrence independently checks packing, rounding,
    // and clipping at both rails; only even states are reachable from start().
    const int states[] = {-32768, -24576, -2, 0, 2, 24576, 32768};
    const int residues[] = {0, 1, 32767};
    const int coefficients[] = {35, 8192, 16384, 16385, 22800};
    const int inputs[] = {-32768, -16384, -1, 0, 1, 16384, 32768};
    for (int z : states) for (int r : residues) for (int g : coefficients) for (int input : inputs) {
        mando::Voice packed = {};
        packed.ladder[0] = z / 2 * 32768 + r;
        const int charge = (input - z) * g + r;
        const int v = charge >> 15;
        const int next = mando::dsp::clamp(z + 2 * v, -32768, 32768);
        const int expected = next / 2 * 32768 + (charge & 32767);
        const mando::dsp::Ladder filter(g, 0);
        if (filter.stage<0, true>(packed, input) != z + v || packed.ladder[0] != expected) {
            std::puts("FAIL: original stage recurrence changed"); return 1;
        }
    }
    mando::Voice boundary = {};
    const mando::dsp::Ladder quarter(16384, 2147483647);
    const mando::dsp::Ladder above(16385, 2147483647);
    if (quarter.needs_limits(boundary) || !above.needs_limits(boundary)) {
        std::puts("FAIL: coefficient limit boundary"); return 1;
    }
    for (int sign = -1; sign <= 1; sign += 2) for (unsigned residue = 0; residue <= 32767; residue += 32767) {
        boundary.ladder[0] = sign * 12288 * 32768 + static_cast<opxy::i32>(residue);
        if (quarter.needs_limits(boundary)) { std::puts("FAIL: headroom boundary rejected"); return 1; }
        boundary.ladder[0] = sign * 12289 * 32768 + static_cast<opxy::i32>(residue);
        if (!quarter.needs_limits(boundary)) { std::puts("FAIL: insufficient headroom accepted"); return 1; }
    }
    // Check the no-clamp proof at the complete 64-sample boundary, including
    // worst-case fractional charge and arbitrary bounded initial pole states.
    opxy::u32 seed = 729;
    for (unsigned trial = 0; trial < 2048; ++trial) {
        mando::Voice limited = {}, fast = {};
        for (unsigned j = 0; j < 4; ++j) {
            seed = seed * 1664525u + 1013904223u;
            const opxy::i32 half = static_cast<opxy::i32>(seed % 24577) - 12288;
            limited.ladder[j] = half * 32768 + static_cast<opxy::i32>(seed >> 17);
        }
        fast = limited;
        const opxy::i32 g = trial & 1u ? 16384 : 35 + static_cast<opxy::i32>(seed % 16350);
        const mando::dsp::Ladder filter(g, static_cast<opxy::i32>(seed >> 1));
        if (filter.needs_limits(fast)) { std::puts("FAIL: bounded state rejected"); return 1; }
        for (unsigned i = 0; i < 64; ++i) {
            seed = seed * 1664525u + 1013904223u;
            const opxy::i32 input = static_cast<opxy::i32>(seed % 131073) - 65536;
            if (filter.step<true>(limited, input) != filter.step<false>(fast, input)) {
                std::puts("FAIL: state rail changed bounded output"); return 1;
            }
            for (unsigned j = 0; j < 4; ++j)
                if (limited.ladder[j] != fast.ladder[j]) {
                    std::puts("FAIL: state rail changed bounded charge"); return 1;
                }
        }
    }
    std::puts("PASS: auditioned B preset/automation fingerprints and 64-sample rail proof");
}
