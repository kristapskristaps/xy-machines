#pragma once
#include "../../vendor/macro.h"
#include <opxy/audio.hpp>
namespace sift_installed {
struct Voice {
    sift_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
    opxy::i16 history[48];
    opxy::u32 cache_read, history_write, resample_phase;
    opxy::i16 cycle[2][257];
    opxy::i32 cycle_mean[2], build_sum;
    opxy::u32 phase, active, valid, building, build_at, fade;
    opxy::u32 prepared[5];
    opxy::u32 ratio[3];
    opxy::i32 shape, bleed, inverse, mix, harmonic, morph;
};
struct Shared {};
void initialize(Voice &, Shared &, opxy::u32);
void start(Voice &, Shared &, opxy::u32);
void destroy(Voice &, Shared &, opxy::u32);
void render(Voice &, Shared &, opxy::u32, opxy::Output, opxy::Parameters, opxy::Pitch);
} // namespace sift

namespace sift_installed {
namespace {
using opxy::i32;
using opxy::u32;
unsigned normalized(i32 value) { return value < 0 ? 0u : static_cast<u32>(value) >> 24; }
u32 ratio16(u32 numerator, u32 denominator) {
    u32 whole = numerator / denominator, remainder = numerator % denominator, fraction = 0;
    if (whole >= 32)
        return 32u << 16;
    for (unsigned bit = 0; bit < 16; ++bit) {
        remainder <<= 1;
        fraction <<= 1;
        if (remainder >= denominator) {
            remainder -= denominator;
            fraction |= 1;
        }
    }
    return (whole << 16) | fraction;
}
i32 sample(const opxy::i16 *table, u32 phase) {
    const unsigned index = phase >> 24;
    return table[index] + ((static_cast<i32>(table[index + 1] - table[index]) *
                            static_cast<i32>((phase >> 9) & 32767u)) >>
                           15);
}
bool equal(const u32 *a, const u32 *b) {
    for (unsigned i = 0; i < 5; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}
void prepare(Voice &v, const u32 *key) {
    for (unsigned i = 0; i < 5; ++i)
        v.prepared[i] = key[i];
    const i32 harm = sift_dsp::k16(key[0]), timb = sift_dsp::k16(key[1]),
              morph = sift_dsp::k16(key[2]);
    const u32 root = key[4];
    const i32 base = 1653513 + (84 * timb - 45 * 65536) / 12;
    const u32 first = sift_dsp::inc_of_log2(base),
              second = sift_dsp::inc_of_log2(base + 4 * harm - 2 * 65536);
    const u32 zed = sift_dsp::inc_of_log2(sift_dsp::log2_q16(root) + 8 * timb);
    // Ratios above 32 harmonics exceed this prepared period's useful bandwidth.
    const u32 limit = root > 0x02000000u ? 0x33333333u : root * 25u;
    v.ratio[0] = ratio16(first > limit ? limit : first, root);
    v.ratio[1] = ratio16(second > limit ? limit : second, root);
    v.ratio[2] = ratio16(zed > limit ? limit : zed, root);
    i32 bleed = harm < 32768 ? (32768 - harm) * 2 - (harm == 0) : 0;
    bleed = static_cast<i32>((static_cast<u32>(bleed) * static_cast<u32>(131072 - bleed)) >> 16);
    v.bleed = bleed >> 1;
    v.inverse = static_cast<i32>(1073741824u / (32768u + v.bleed));
    const i32 tame = root >= 0x0aaaaaaau ? 0 : 65536 - static_cast<i32>((root >> 12) * 24 >> 4);
    v.shape = 21627 + (((morph - 21627) * (tame >> 1)) >> 15);
    v.harmonic = harm;
    v.morph = morph;
    v.mix = key[3] <= 55 ? 0 : key[3] >= 72 ? 32767 : static_cast<i32>(key[3] - 55) * 1927;
    v.build_at = 0;
    v.build_sum = 0;
    v.building = 1;
}
i32 point(const Voice &v, u32 phase, const sift_dsp::carrier_c &carrier) {
    const u32 index = phase >> 16;
    const i32 window = (sift_dsp::carrier24(&carrier, phase >> 1) + 256) >> 9;
    const i32 a = sift_dsp::sin32(index * v.ratio[0]);
    const i32 b = sift_dsp::sin32(index * v.ratio[1]);
    const i32 main = ((((window * (a + b + 2 * v.bleed)) >> 15) * v.inverse) >> 15) * 22938 >> 15;
    // Smooth half-cycle formant windows replace the discontinuity-correction
    // state machines. Morph shifts their duty; Harmonic warps the formant phase.
    const u32 half = phase << 1;
    const i32 window_z = (32767 - sift_dsp::sin32(half + 0x40000000u)) >> 1;
    const i32 envelope =
        ((window_z * (65535 - v.morph)) >> 16) + ((((window_z * window_z) >> 15) * v.morph) >> 16);
    const u32 warped = static_cast<u32>(v.harmonic) * 32768u;
    const i32 z = sift_dsp::sin32((half >> 17) * v.ratio[2] + warped);
    const i32 auxiliary = (((z * envelope) >> 15) * 19661) >> 15;
    return main + (((auxiliary - main) * v.mix) >> 15);
}
void build_tile(Voice &v) {
    sift_dsp::carrier_c carrier;
    sift_dsp::carrier_coefs(&carrier, v.shape);
    const unsigned end = v.build_at + 16 > 256 ? 256 : v.build_at + 16;
    auto *table = v.cycle[v.active ^ 1u];
    for (unsigned i = v.build_at; i < end; ++i) {
        // Three phase taps soften formant edges without an audio-rate SRC.
        const u32 phase = i << 24, spread = v.prepared[4] >> 2;
        const i32 value = (point(v, phase - spread, carrier) + 2 * point(v, phase, carrier) +
                           point(v, phase + spread, carrier)) /
                          4;
        table[i] = static_cast<opxy::i16>(value);
        v.build_sum += value;
    }
    v.build_at = end;
    if (end == 256) {
        table[256] = table[0];
        v.cycle_mean[v.active ^ 1u] = v.build_sum / 256;
        v.building = 0;
        v.valid = 1;
        v.fade = 256;
    }
}
} // namespace
void initialize(Voice &v, Shared &, u32 owner) {
    sift_dsp::macro_init(&v.oscillator);
    v.oscillator.rng = 0x9e3779b9u * owner + 1u;
    v.pending = 1;
    v.cache_read = 32;
    v.history_write = v.resample_phase = 0;
    for (auto &value : v.history)
        value = 0;
    v.phase = 0;
    v.active = v.valid = v.building = v.build_at = v.fade = 0;
    v.cycle_mean[0] = v.cycle_mean[1] = v.build_sum = 0;
    for (unsigned i = 0; i <= 256; ++i)
        v.cycle[0][i] = v.cycle[1][i] = static_cast<opxy::i16>(sift_dsp::sin32(i << 24) / 3);
}
void start(Voice &v, Shared &, u32) { v.pending = 1; }
void destroy(Voice &v, Shared &, u32) { v.pending = 0; }
void render(Voice &v, Shared &, u32, opxy::Output out, opxy::Parameters p, opxy::Pitch pitch) {
    u32 key[5];
    for (unsigned i = 0; i < 4; ++i)
        key[i] = normalized(p[i]);
    // Preparation uses the block pitch. Playback follows every incoming pitch sample.
    u32 frequency = (pitch[0] / 160u) * 147u + ((pitch[0] % 160u) * 147u) / 160u;
    if (frequency < 4295)
        frequency = 4295;
    if (frequency > 0x20000000u)
        frequency = 0x20000000u;
    key[4] = frequency;
    if (!v.building && !v.fade && (!v.valid || !equal(key, v.prepared)))
        prepare(v, key);
    if (v.building)
        build_tile(v);
    u32 phase = v.phase;
    for (unsigned i = 0; i < 64; ++i) {
        i32 signal = sample(v.cycle[v.active], phase) - v.cycle_mean[v.active];
        if (v.fade) {
            const i32 next = sample(v.cycle[v.active ^ 1u], phase) - v.cycle_mean[v.active ^ 1u];
            signal += ((next - signal) * static_cast<i32>(256 - v.fade)) >> 8;
            if (!--v.fade)
                v.active ^= 1u;
        }
        if (signal < -32768)
            signal = -32768;
        if (signal > 32767)
            signal = 32767;
        out[i] = signal * 16384;
        const u32 step = pitch[i] > 0x22d3b6a0u ? 0x22d3b6a0u : pitch[i];
        phase += step;
    }
    v.phase = phase;
    v.pending = 0;
}
} // namespace sift
