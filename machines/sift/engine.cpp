#include "core.h"
#include "vendor/macro.cpp"

namespace sift {
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
        if (i != 3 && a[i] != b[i])
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
    // Ratios above 32 harmonics exceed this prepared period's useful bandwidth.
    const u32 limit = root > 0x02000000u ? 0x33333333u : root * 25u;
    v.ratio[0] = ratio16(first > limit ? limit : first, root);
    v.ratio[1] = ratio16(second > limit ? limit : second, root);
    // The alternate stays in the lower harmonic family instead of jumping up
    // eight octaves with Timbre. Its Hann window closes every carrier period.
    const u32 z_ratio = 65536u + ((static_cast<u32>(timb) * static_cast<u32>(timb)) >> 16) * 5u;
    const u32 safe_ratio = ratio16(0x33333333u, root);
    v.ratio[2] = z_ratio < safe_ratio ? z_ratio : safe_ratio;
    i32 bleed = harm < 32768 ? (32768 - harm) * 2 - (harm == 0) : 0;
    bleed = static_cast<i32>((static_cast<u32>(bleed) * static_cast<u32>(131072 - bleed)) >> 16);
    v.bleed = bleed >> 1;
    v.inverse = static_cast<i32>(1073741824u / (32768u + v.bleed));
    const i32 tame = root >= 0x0aaaaaaau ? 0 : 65536 - static_cast<i32>((root >> 12) * 24 >> 4);
    v.shape = 21627 + (((morph - 21627) * (tame >> 1)) >> 15);
    v.harmonic = harm;
    v.morph = morph;
    v.build_at = 0;
    v.build_sum[0] = v.build_sum[1] = 0;
    v.building = 1;
}
i32 point(const Voice &v, unsigned endpoint, u32 phase, const sift_dsp::carrier_c &carrier) {
    if (!endpoint) {
        const u32 index = phase >> 16;
        const i32 window = (sift_dsp::carrier24(&carrier, phase >> 1) + 256) >> 9;
        const i32 a = sift_dsp::sin32(index * v.ratio[0]), b = sift_dsp::sin32(index * v.ratio[1]);
        return (((((window * (a + b + 2 * v.bleed)) >> 15) * v.inverse) >> 15) * 22938) >> 15;
    }
    const i32 root = sift_dsp::sin32(phase);
    const i32 window = (32767 - sift_dsp::sin32(phase + 0x40000000u)) >> 1;
    const i32 envelope =
        ((window * (65535 - v.morph)) >> 16) + ((((window * window) >> 15) * v.morph) >> 16);
    const i32 warp = opxy::q31::multiply(root * 65536, v.harmonic * 8192);
    const i32 formant = sift_dsp::sin32((phase >> 16) * v.ratio[2] + static_cast<u32>(warp));
    const i32 resonance = (formant * envelope) >> 15;
    const i32 amount = 8192 + (v.morph >> 3);
    const i32 voiced = root + (((resonance - root) * amount) >> 15);
    return (voiced * 19661) >> 15;
}
i32 blended(const Voice &v, unsigned bank, u32 phase, i32 mix) {
    if (!mix)
        return sample(v.cycle[bank][0], phase) - v.cycle_mean[bank][0];
    if (mix == 32768)
        return sample(v.cycle[bank][1], phase) - v.cycle_mean[bank][1];
    const i32 main = sample(v.cycle[bank][0], phase) - v.cycle_mean[bank][0];
    const i32 alternate = sample(v.cycle[bank][1], phase) - v.cycle_mean[bank][1];
    return main + static_cast<i32>(opxy::q31::wide(alternate - main, mix) >> 15);
}
void build_tile(Voice &v) {
    sift_dsp::carrier_c carrier;
    sift_dsp::carrier_coefs(&carrier, v.shape);
    const unsigned end = v.build_at + 16 > 256 ? 256 : v.build_at + 16;
    for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
        auto *table = v.cycle[v.active ^ 1u][endpoint];
        for (unsigned i = v.build_at; i < end; ++i) {
            // Three phase taps soften formant edges without an audio-rate SRC.
            const u32 phase = i << 24, spread = v.prepared[4] >> 2;
            const i32 value = (point(v, endpoint, phase - spread, carrier) +
                               2 * point(v, endpoint, phase, carrier) +
                               point(v, endpoint, phase + spread, carrier)) /
                              4;
            table[i] = static_cast<opxy::i16>(value);
            v.build_sum[endpoint] += value;
        }
        if (end == 256) {
            table[256] = table[0];
            v.cycle_mean[v.active ^ 1u][endpoint] = v.build_sum[endpoint] / 256;
        }
    }
    v.build_at = end;
    if (end == 256) {
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
    v.mix = 0;
    for (unsigned bank = 0; bank < 2; ++bank)
        for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
            v.cycle_mean[bank][endpoint] = 0;
            for (unsigned i = 0; i <= 256; ++i)
                v.cycle[bank][endpoint][i] = static_cast<opxy::i16>(sift_dsp::sin32(i << 24) / 3);
        }
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
    const i32 target = p[3] < 0 ? 0 : static_cast<i32>((static_cast<u32>(p[3]) + 32768u) >> 16);
    const i32 mix_from = v.mix, mix_delta = target - mix_from;
    for (unsigned i = 0; i < 64; ++i) {
        const i32 mix = mix_from + ((mix_delta * static_cast<i32>(i + 1)) >> 6);
        i32 signal = blended(v, v.active, phase, mix);
        if (v.fade) {
            const i32 next = blended(v, v.active ^ 1u, phase, mix);
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
    v.mix = target;
    v.phase = phase;
    v.pending = 0;
}
} // namespace sift
