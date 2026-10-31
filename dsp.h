#pragma once
#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace cyclops { namespace dsp {
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
inline i32 linear(const i32* table, u32 position, unsigned bits) {
    const u32 index = position >> bits;
    const i32 fraction = static_cast<i32>(position & ((1u << bits) - 1));
    return table[index] + ((table[index + 1] - table[index]) * fraction >> bits);
}
inline i32 saturate(i32 x) {
    const u32 a = static_cast<u32>(x < 0 ? -x : x);
    const i32 y = a >= (8u << 14) ? unity : linear(tables::tanh_q14, a, 8);
    return x < 0 ? -y : y;
}
inline i32 log2_increment(u32 value) {
    if (!value) return 0;
    unsigned exponent = 0;
    u32 normalized = value;
    while (normalized > 1) { normalized >>= 1; ++exponent; }
    normalized = value << (31 - exponent);
    const u32 position = (normalized - 0x80000000u) >> 7;
    return static_cast<i32>(exponent * 65536) + interpolate(tables::log2_fraction, position, 16);
}
inline i32 cutoff_position(const i32* p, u32 pitch) {
    // C2 (65.406 Hz) is the key-tracking pivot. Q16 table steps.
    const i32 base = multiply(control(p[0]), 19005440); // 290 * 65536
    const i32 octaves = log2_increment(pitch) - 1481302;
    const i32 tracked = multiply(octaves, control(p[7])) * 32;
    return clamp(base + tracked, 0, 19005440);
}
inline i32 pole(i32 position) { return interpolate(tables::pole, static_cast<u32>(position), 16); }
inline u32 tuning(i32 detune) {
    return static_cast<u32>(interpolate(tables::detune_q20, static_cast<u32>(control(detune)) >> 7, 17));
}
inline u32 second_increment(u32 pitch, u32 ratio, u32 octave) {
    // Split Q20 product avoids 64-bit runtime helpers on Blackfin.
    const u32 delta = ratio - (1 << 20);
    u32 increment = pitch + (pitch >> 16) * delta / 16 + ((pitch & 65535u) * delta >> 20);
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
    const i32* const* bank = square ? tables::square : tables::saw;
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
struct Ladder {
    i32 g, resonance, inverse, weights[4];
    Ladder(i32 coefficient, i32 amount) : g(coefficient) {
        resonance = multiply(control(amount), 31949); // Q13 feedback 0..3.90
        const i32 g2 = g * g / 32768, g4 = g2 * g2 / 32768;
        const i32 feedback = g4 * resonance >> 13;
        weights[3] = 32768 - g;
        for (unsigned i = 3; i > 0; --i) weights[i - 1] = weights[i] * g >> 15;
        const u32 denominator = 32768u + static_cast<u32>(feedback);
        inverse = static_cast<i32>(0x40000000u / denominator);
    }
    i32 step(Voice& voice, i32 input) const {
        // TPT four-pole ladder: solve instantaneous feedback before updating
        // the trapezoidal integrators. A nonlinear input limits resonance.
        i32 memory = 0;
        for (unsigned s = 0; s < 4; ++s) memory += voice.ladder[s] * weights[s];
        memory >>= 15;
        i32 sample = saturate((((input - (memory * resonance >> 13)) >> 2) * inverse) >> 13);
        for (unsigned s = 0; s < 4; ++s) {
            // Retain sub-LSB integrator charge so low cutoff and quiet notes
            // do not stick in the dead band of a Q14 multiply.
            const i32 charge = (sample - voice.ladder[s]) * g + voice.residue[s];
            const i32 v = charge >> 15;
            voice.residue[s] = charge - v * 32768;
            sample = v + voice.ladder[s];
            voice.ladder[s] = clamp(sample + v, -2 * unity, 2 * unity);
        }
        return sample;
    }
};
}}
