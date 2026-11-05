#pragma once
#include "../../vendor/macro.h"
#include <opxy/audio.hpp>
namespace tandem_installed {
struct Shape {
    opxy::u32 ratio, pulse, up, down;
    opxy::i32 triangle, square;
};
struct Voice {
    tandem_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
    opxy::i16 history[48];
    opxy::u32 cache_read, history_write, resample_phase;
    opxy::i16 cycle[2][2][257];
    opxy::i32 mean[2][2], sum[2];
    opxy::u32 phase[2], active, valid, building, build_at, fade;
    opxy::u32 key[5], detune;
    Shape pulse, primary, auxiliary, saw;
    opxy::i32 pulse_gain, saw_gain, mix;
};
struct Shared {};
void initialize(Voice &, Shared &, opxy::u32);
void start(Voice &, Shared &, opxy::u32);
void destroy(Voice &, Shared &, opxy::u32);
void render(Voice &, Shared &, opxy::u32, opxy::Output, opxy::Parameters, opxy::Pitch);
} // namespace tandem

namespace tandem_installed {
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
i32 read(const opxy::i16 *table, u32 phase) {
    const unsigned index = phase >> 24;
    return table[index] + ((static_cast<i32>(table[index + 1] - table[index]) *
                            static_cast<i32>((phase >> 9) & 32767u)) >>
                           15);
}
Shape shape(const tandem_dsp::macro_vso &source) {
    Shape s = {};
    s.ratio = ratio16(source.sf, source.mf ? source.mf : 1);
    s.pulse = static_cast<u32>(source.pw31) << 1;
    tandem_dsp::slopes(source.pw31, &s.up, &s.down);
    s.triangle = source.ws < 32768 ? 65536 - 2 * source.ws : 0;
    s.square = source.ws > 32768 ? 2 * (source.ws - 32768) : 0;
    return s;
}
i32 variable(const Shape &s, u32 phase) {
    const u32 slave = (phase >> 16) * s.ratio;
    return (tandem_dsp::vso_naive(slave, s.pulse, s.up, s.down, s.triangle, s.square) >> 12) -
           32768;
}
i32 saw(const Shape &s, u32 phase) {
    const i32 a = static_cast<i32>(phase >> 4), notch = 65536 - s.triangle;
    i32 value;
    if (phase < s.pulse)
        value = tandem_dsp::amt(a, notch) +
                tandem_dsp::amt(static_cast<i32>(((phase >> 16) * s.up) >> 3), s.triangle);
    else
        value =
            tandem_dsp::amt((1 << 28) + 53687091, notch) +
            tandem_dsp::amt((1 << 28) - static_cast<i32>((((phase - s.pulse) >> 16) * s.down) >> 3),
                            s.triangle);
    return tandem_dsp::fmac1(static_cast<i32>(static_cast<u32>(value) * 2u) - (1 << 28),
                             1789569707) >>
           13;
}
bool equal(const u32 *a, const u32 *b) {
    for (unsigned i = 0; i < 5; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}
void prepare(Voice &v, const u32 *key) {
    opxy::u8 p[7] = {};
    for (unsigned i = 0; i < 4; ++i) {
        v.key[i] = key[i];
        p[i + 1] = static_cast<opxy::u8>(key[i]);
    }
    v.key[4] = key[4];
    tandem_dsp::macro_va settings;
    tandem_dsp::va_init(&settings);
    i32 a[1], b[1];
    // The old engine still defines the knobs' tuning, gains, PWM and sync map.
    // Evaluate one setup sample; its four oscillators are not used for playback.
    tandem_dsp::va_render(&settings, p, key[4], a, b, 1, 1, 1);
    v.pulse = shape(settings.sync);
    v.primary = shape(settings.primary);
    v.auxiliary = shape(settings.auxiliary);
    v.saw = {};
    v.saw.pulse = static_cast<u32>(settings.saw.pw31) << 1;
    tandem_dsp::slopes(settings.saw.pw31, &v.saw.up, &v.saw.down);
    v.saw.triangle = settings.saw.ws;
    v.detune = ratio16(settings.auxiliary.mf, key[4]);
    v.pulse_gain = settings.sq_gain >> 9;
    v.saw_gain = settings.saw_gain >> 9;
    v.mix = key[3] <= 55 ? 0 : key[3] >= 72 ? 32767 : static_cast<i32>(key[3] - 55) * 1927;
    v.build_at = 0;
    v.sum[0] = v.sum[1] = 0;
    v.building = 1;
}
i32 point(const Voice &v, unsigned channel, u32 phase) {
    const i32 main = channel ? ((saw(v.saw, phase) * v.saw_gain) >> 15)
                             : ((variable(v.pulse, phase) * v.pulse_gain) >> 15);
    const i32 auxiliary =
        channel ? variable(v.auxiliary, phase) / 2 : -variable(v.primary, phase) / 2;
    return (main + (((auxiliary - main) * v.mix) >> 15)) * 26214 >> 15;
}
void build_tile(Voice &v) {
    const unsigned end = v.build_at + 16;
    for (unsigned channel = 0; channel < 2; ++channel) {
        auto *table = v.cycle[v.active ^ 1u][channel];
        // Frequency-aware phase taps replace per-sample BLEP corrections/SRC.
        u32 spread = v.key[4] >> 2;
        if (channel)
            spread = tandem_dsp::mulq16(spread, v.detune);
        for (unsigned i = v.build_at; i < end; ++i) {
            const u32 phase = i << 24;
            const i32 sample = (point(v, channel, phase - spread) + 2 * point(v, channel, phase) +
                                point(v, channel, phase + spread)) /
                               4;
            table[i] = static_cast<opxy::i16>(sample);
            v.sum[channel] += sample;
        }
        if (end == 256) {
            table[256] = table[0];
            v.mean[v.active ^ 1u][channel] = v.sum[channel] / 256;
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
    tandem_dsp::macro_init(&v.oscillator);
    v.oscillator.rng = 0x9e3779b9u * owner + 1;
    v.pending = 1;
    v.phase[0] = 0;
    v.phase[1] = 0x40000000u;
    v.active = v.valid = v.building = v.build_at = v.fade = 0;
    v.detune = 65536;
    for (unsigned bank = 0; bank < 2; ++bank)
        for (unsigned channel = 0; channel < 2; ++channel) {
            v.mean[bank][channel] = 0;
            for (unsigned i = 0; i <= 256; ++i)
                v.cycle[bank][channel][i] =
                    static_cast<opxy::i16>((static_cast<i32>((i & 255u) * 256u) - 32768) / 4);
        }
}
void start(Voice &v, Shared &, u32) { v.pending = 1; }
void destroy(Voice &v, Shared &, u32) { v.pending = 0; }
void render(Voice &v, Shared &, u32, opxy::Output output, opxy::Parameters p, opxy::Pitch pitch) {
    u32 key[5];
    for (unsigned i = 0; i < 4; ++i)
        key[i] = normalized(p[i]);
    u32 source = (pitch[0] / 160u) * 147u + ((pitch[0] % 160u) * 147u) / 160u;
    if (source < 4295)
        source = 4295;
    if (source > 0x40000000u)
        source = 0x40000000u;
    key[4] = source;
    if (!v.building && !v.fade && (!v.valid || !equal(key, v.key)))
        prepare(v, key);
    if (v.building)
        build_tile(v);
    u32 root = v.phase[0], detuned = v.phase[1];
    for (unsigned i = 0; i < 64; ++i) {
        i32 signal = read(v.cycle[v.active][0], root) - v.mean[v.active][0] +
                     read(v.cycle[v.active][1], detuned) - v.mean[v.active][1];
        if (v.fade) {
            const i32 next = read(v.cycle[v.active ^ 1u][0], root) - v.mean[v.active ^ 1u][0] +
                             read(v.cycle[v.active ^ 1u][1], detuned) - v.mean[v.active ^ 1u][1];
            signal += ((next - signal) * static_cast<i32>(256 - v.fade)) >> 8;
            if (!--v.fade)
                v.active ^= 1u;
        }
        if (signal < -32768)
            signal = -32768;
        if (signal > 32767)
            signal = 32767;
        output[i] = signal * 16384;
        const u32 step = pitch[i] > 0x40000000u ? 0x40000000u : pitch[i];
        root += step;
        const u32 auxiliary = static_cast<u32>(
            opxy::q31::wide(static_cast<i32>(step), static_cast<i32>(v.detune)) >> 16);
        detuned += auxiliary > 0x40000000u ? 0x40000000u : auxiliary;
    }
    v.phase[0] = root;
    v.phase[1] = detuned;
    v.pending = 0;
}
} // namespace tandem
