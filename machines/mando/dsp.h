#pragma once
#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace mando { namespace dsp {
using opxy::q31::multiply;
const i32 unity = 1 << 14;
const i32 peak = 536870912;
inline i32 clamp(i32 x, i32 low, i32 high) { return x < low ? low : x > high ? high : x; }
inline i32 control(i32 x) { return x < 0 ? 0 : x; }
inline u32 option(i32 x, u32 count) {
    return (static_cast<u32>(control(x)) >> 16) * count >> 15;
}
inline i32 interpolate(const i32* table, u32 position, unsigned bits) {
    const u32 index = position >> bits;
    const i32 fraction = static_cast<i32>((position & ((1u << bits) - 1)) << (31 - bits));
    return table[index] + multiply(table[index + 1] - table[index], fraction);
}
// Signal tables use Q14; small coefficients avoid wide multiplies in audio.
template <class Sample>
inline i32 linear(const Sample* table, u32 position, unsigned bits) {
    const u32 index = position >> bits;
    const i32 fraction = static_cast<i32>(position & ((1u << bits) - 1));
    return table[index] + ((table[index + 1] - table[index]) * fraction >> bits);
}
inline i32 saturate(i32 x) {
    const u32 a = static_cast<u32>(x < 0 ? -x : x);
    const i32 y = a <= 4096u ? tables::tanh_prefix[a]
        : a >= (8u << 14) ? unity : linear(tables::tanh_q14, a, 8);
    return x < 0 ? -y : y;
}
inline i32 log2_increment(u32 value) {
    if (!value) return 0;
    unsigned exponent = 0;
    u32 leading = value;
    if (leading >> 16) { leading >>= 16; exponent += 16; }
    if (leading >> 8) { leading >>= 8; exponent += 8; }
    if (leading >> 4) { leading >>= 4; exponent += 4; }
    if (leading >> 2) { leading >>= 2; exponent += 2; }
    if (leading >> 1) ++exponent;
    const u32 normalized = value << (31 - exponent);
    const u32 position = (normalized - 0x80000000u) >> 7;
    return static_cast<i32>(exponent * 65536) + interpolate(tables::log2_fraction, position, 16);
}
inline i32 cutoff_position(const i32* p, u32 pitch) {
    // C2 (65.406 Hz) is the key-tracking pivot. Q16 table steps.
    const i32 base = multiply(control(p[0]), 19005440); // 290 * 65536
    if (!p[7]) return base;
    const i32 octaves = log2_increment(pitch) - 1481302;
    const i32 tracked = multiply(octaves, control(p[7])) * 32;
    return clamp(base + tracked, 0, 19005440);
}
inline i32 pole(i32 position) { return interpolate(tables::pole, static_cast<u32>(position), 16); }
inline u32 tuning(i32 detune) {
    return static_cast<u32>(interpolate(tables::detune_q20, static_cast<u32>(control(detune)) >> 7, 17));
}
// Glide needs this conversion per sample; avoid a native call in that loop.
__attribute__((always_inline)) inline u32 second_increment(u32 pitch, u32 ratio, u32 octave) {
    // Split Q20 product avoids 64-bit runtime helpers on Blackfin.
    const u32 delta = ratio - (1 << 20);
    u32 increment = pitch;
    if (delta) increment += (pitch >> 16) * delta / 16 + ((pitch & 65535u) * delta >> 20);
    if (octave == 0) increment >>= 1;
    if (octave == 2) increment = increment < 0x20000000u ? increment * 2 : 0x40000000u;
    return increment > 0x40000000u ? 0x40000000u : increment;
}
struct Band {
    u32 rich;
    i32 fade;
    explicit Band(u32 increment) : rich(8), fade(0) {
        while (rich && increment > (0x80000000u >> rich)) --rich;
        if (rich) {
            const u32 start = 0x40000000u >> rich;
            if (increment > start) {
                const u32 weight = (increment - start) << (rich + 1);
                fade = static_cast<i32>((weight > 0x7fffffffu ? 0x7fffffffu : weight) >> 16);
            }
        }
    }
};
inline i32 wave(u32 phase, const Band& band, bool square) {
    const opxy::i16* const* bank = square ? tables::square : tables::saw;
    const i32 first = linear(bank[band.rich], phase >> 12, 10);
    if (!band.rich || !band.fade) return first;
    const i32 second = linear(bank[band.rich - 1], phase >> 12, 10);
    return first + ((second - first) * band.fade >> 15);
}
inline i32 oscillator(u32 phase, const Band& band, i32 square) {
    if (square == 0) return wave(phase, band, false);
    if (square == 32767) return wave(phase, band, true);
    const i32 a = wave(phase, band, false), b = wave(phase, band, true);
    return a + ((b - a) * square >> 15);
}
// A stable waveform uses one bank; select its mip pointers before rendering.
struct Oscillator {
    const opxy::i16* rich;
    const opxy::i16* lean;
    i32 fade;
    Oscillator(const Band& band, i32 square) : fade(band.fade) {
        const opxy::i16* const* bank = square ? tables::square : tables::saw;
        rich = bank[band.rich];
        lean = bank[band.rich ? band.rich - 1 : 0];
    }
    i32 read(u32 phase) const {
        const i32 a = linear(rich, phase >> 12, 10);
        if (!fade) return a;
        const i32 b = linear(lean, phase >> 12, 10);
        return a + ((b - a) * fade >> 15);
    }
};
inline i32 smooth_wave(i32 current, i32 target) {
    current += (target - current) >> 7;
    const i32 left = target - current;
    return left > -128 && left < 128 ? target : current;
}
inline i32 morph(const Oscillator& saw, const Oscillator& pulse, u32 phase, i32 square) {
    if (square == 0) return saw.read(phase);
    const i32 b = pulse.read(phase);
    if (square == 32767) return b;
    const i32 a = saw.read(phase);
    return a + ((b - a) * square >> 15);
}
struct Ladder : FilterCoefficients {
    explicit Ladder(const FilterCoefficients& cached) : FilterCoefficients(cached) {}
    Ladder(i32 coefficient, i32 amount) {
        g = coefficient;
        compensation = control(amount) >> 16;
        resonance = multiply(control(amount), 31949); // Q13 feedback 0..3.90
        const i32 g2 = g * g / 32768, g4 = g2 * g2 / 32768;
        const i32 feedback = g4 * resonance >> 13;
        weights[3] = 32768 - g;
        for (unsigned i = 3; i > 0; --i) weights[i - 1] = weights[i] * g >> 15;
        const u32 denominator = 32768u + static_cast<u32>(feedback);
        inverse = static_cast<i32>(0x40000000u / denominator);
    }
    bool needs_limits(const Voice& voice) const {
        if (g > 16384) return true;
        for (unsigned i = 0; i < 4; ++i)
            if ((voice.ladder[i] >> 15) < -12288 || (voice.ladder[i] >> 15) > 12288) return true;
        return false;
    }
    template <bool Limited>
    __attribute__((always_inline)) inline i32 step(Voice& voice, i32 input) const {
        // TPT four-pole ladder: solve instantaneous feedback before updating
        // the trapezoidal integrators. A nonlinear input limits resonance.
        i32 corrected = (input >> 2) * 4;
        if (resonance) {
            const i32 memory = ((voice.ladder[0] >> 15) * weights[0]
                              + (voice.ladder[1] >> 15) * weights[1]
                              + (voice.ladder[2] >> 15) * weights[2]
                              + (voice.ladder[3] >> 15) * weights[3]) >> 14;
            corrected = (((input - (memory * resonance >> 13)) >> 2) * inverse) >> 13;
        }
        i32 sample = saturate(corrected);
        sample = stage<0, Limited>(voice, sample);
        sample = stage<1, Limited>(voice, sample);
        sample = stage<2, Limited>(voice, sample);
        // Keep overload and resonance compensation inside the ladder: the
        // final pole filters the generated harmonics before they leave it.
        sample += (sample * compensation >> 15);
        sample = saturate(sample);
        return stage<3, Limited>(voice, sample);
    }
    template <unsigned Index, bool Limited>
    __attribute__((always_inline)) inline i32 stage(Voice& voice, i32 sample) const {
        // Pole states start even, update by 2*v, and clamp to even rails.
        // A = (z/2)*32768 + residue therefore retains the old rounding exactly.
        const i32 half = voice.ladder[Index] >> 15;
        const i32 charge = voice.ladder[Index] + (sample - 2 * half) * g;
        const i32 next_half = charge >> 15;
        sample = half + next_half;
        voice.ladder[Index] = Limited
            ? clamp(next_half, -unity, unity) * 32768 + (charge & 32767) : charge;
        return sample;
    }
    i32 step(Voice& voice, i32 input) const { return step<true>(voice, input); }
};
}}
