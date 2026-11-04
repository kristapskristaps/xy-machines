#pragma once
#include "baked_tables.h"
#include "core.h"
#include "four_pole_tables.h"
#include "half_rate_tables.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace mando {
namespace dsp {
using opxy::q31::multiply;
const i32 unity = 1 << 14;
const i32 peak = 536870912;
inline i32 clamp(i32 x, i32 low, i32 high) {
    return x < low ? low : x > high ? high : x;
}
inline i32 control(i32 x) { return x < 0 ? 0 : x; }
inline u32 option(i32 x, u32 count) {
    return (static_cast<u32>(control(x)) >> 16) * count >> 15;
}
inline i32 interpolate(const i32 *table, u32 position, unsigned bits) {
    const u32 index = position >> bits;
    const i32 fraction =
        static_cast<i32>((position & ((1u << bits) - 1)) << (31 - bits));
    return table[index] + multiply(table[index + 1] - table[index], fraction);
}
// Signal tables use Q14; small coefficients avoid wide multiplies in audio.
template <class Sample>
inline i32 linear(const Sample *table, u32 position, unsigned bits) {
    const u32 index = position >> bits;
    const i32 fraction = static_cast<i32>(position & ((1u << bits) - 1));
    return table[index] +
           ((table[index + 1] - table[index]) * fraction >> bits);
}
inline i32 saturate(i32 x) {
    const u32 a = static_cast<u32>(x < 0 ? -x : x);
    if (a <= 512u)
        return x; // The original integer interpolant is exactly linear here.
    if (a >= 131072u)
        return x < 0 ? -unity : unity;
    const unsigned at =
        a < 32768u ? (a + 8u) >> 4 : 2048u + ((a - 32768u + 64u) >> 7);
    const i32 value = baked_tables::tanh[at];
    return x < 0 ? -value : value;
}
// Uniform segments fit in one cache line group; endpoint and slope share a
// word.
inline i32 bass_saturate(i32 x) {
    const u32 a = static_cast<u32>(x < 0 ? -x : x);
    i32 y;
    if (a >= 65536u) {
        const u32 tail = 16373u + ((a - 65536u) >> 13);
        y = tail > 16384u ? 16384 : static_cast<i32>(tail);
    } else {
        static const u32 transfer[32] = {
            133496832u, 129501173u, 121769901u, 111417071u, 99360147u,
            86451071u,  73869478u,  61943053u,  51261630u,  41825228u,
            33896010u,  27211855u,  21707246u,  17185593u,  13646911u,
            10763535u,  8404403u,   6635059u,   5193368u,   4079335u,
            3161893u,   2506581u,   1916795u,   1523608u,   1195951u,
            933825u,    737231u,    540634u,    475106u,    344041u,
            278510u,    212978u};
        const u32 word = transfer[a >> 11];
        const opxy::i16 slope = static_cast<opxy::i16>(word >> 16);
        const opxy::i16 fraction = static_cast<opxy::i16>(a & 2047u);
        y = static_cast<i32>(word & 65535u) + (slope * fraction >> 11);
    }
    return x < 0 ? -y : y;
}
inline i32 bass_knee(i32 x) {
    const u32 a = static_cast<u32>(x < 0 ? -x : x);
    if (a <= 10240u)
        return x;
    if (a >= 18432u)
        return x < 0 ? -14336 : 14336;
    const opxy::i16 delta = static_cast<opxy::i16>(a - 10240u);
    const i32 y = 10240 + delta - (delta * delta >> 14);
    return x < 0 ? -y : y;
}
inline i32 output_knee(i32 x) {
    const u32 magnitude = static_cast<u32>(x < 0 ? -x : x);
    if (magnitude <= 10240u)
        return x;
    const i32 curved =
        10240 + (saturate(static_cast<i32>(magnitude - 10240u) * 4) >> 2);
    return x < 0 ? -curved : curved;
}
inline i32 wave_linear(const opxy::i16 *table, u32 phase) {
    const unsigned index = phase >> 22;
    const opxy::i16 fraction = static_cast<opxy::i16>((phase >> 12) & 1023u);
    const opxy::i16 slope =
        static_cast<opxy::i16>(table[index + 1] - table[index]);
    return table[index] + (slope * fraction >> 10);
}
inline i32 log2_increment(u32 value) {
    if (!value)
        return 0;
    unsigned exponent = 0;
    u32 leading = value;
    if (leading >> 16) {
        leading >>= 16;
        exponent += 16;
    }
    if (leading >> 8) {
        leading >>= 8;
        exponent += 8;
    }
    if (leading >> 4) {
        leading >>= 4;
        exponent += 4;
    }
    if (leading >> 2) {
        leading >>= 2;
        exponent += 2;
    }
    if (leading >> 1)
        ++exponent;
    const u32 normalized = value << (31 - exponent);
    const u32 position = (normalized - 0x80000000u) >> 7;
    return static_cast<i32>(exponent * 65536) +
           interpolate(tables::log2_fraction, position, 16);
}
inline i32 cutoff_position(const i32 *p, u32 pitch) {
    // C2 (65.406 Hz) is the key-tracking pivot. Q16 table steps.
    const i32 base = multiply(control(p[0]), 19005440); // 290 * 65536
    if (!p[7])
        return base;
    const i32 octaves = log2_increment(pitch) - 1481302;
    const i32 tracked = multiply(octaves, control(p[7])) * 32;
    return clamp(base + tracked, 0, 19005440);
}
inline i32 pole(i32 position) {
    return interpolate(tables::pole, static_cast<u32>(position), 16);
}
inline u32 tuning(i32 detune) {
    return static_cast<u32>(interpolate(
        tables::detune_q20, static_cast<u32>(control(detune)) >> 7, 17));
}
// Glide needs this conversion per sample; avoid a native call in that loop.
__attribute__((always_inline)) inline u32 second_increment(u32 pitch, u32 ratio,
                                                           u32 octave) {
    // Split Q20 product avoids 64-bit runtime helpers on Blackfin.
    const u32 delta = ratio - (1 << 20);
    u32 increment = pitch;
    if (delta)
        increment +=
            (pitch >> 16) * delta / 16 + ((pitch & 65535u) * delta >> 20);
    if (octave == 0)
        increment >>= 1;
    if (octave == 2)
        increment = increment < 0x20000000u ? increment * 2 : 0x40000000u;
    return increment > 0x40000000u ? 0x40000000u : increment;
}
struct Band {
    u32 rich;
    i32 fade;
    explicit Band(u32 increment) : rich(8), fade(0) {
        while (rich && increment > (0x80000000u >> rich))
            --rich;
        if (rich) {
            const u32 start = 0x40000000u >> rich;
            if (increment > start) {
                const u32 weight = (increment - start) << (rich + 1);
                fade = static_cast<i32>(
                    (weight > 0x7fffffffu ? 0x7fffffffu : weight) >> 16);
            }
        }
    }
};
inline i32 wave(u32 phase, const Band &band, bool square) {
    const opxy::i16 *const *bank = square ? tables::square : tables::saw;
    const i32 first = linear(bank[band.rich], phase >> 12, 10);
    if (!band.rich || !band.fade)
        return first;
    const i32 second = linear(bank[band.rich - 1], phase >> 12, 10);
    return first + ((second - first) * band.fade >> 15);
}
inline i32 oscillator(u32 phase, const Band &band, i32 square) {
    if (square == 0)
        return wave(phase, band, false);
    if (square == 32767)
        return wave(phase, band, true);
    const i32 a = wave(phase, band, false), b = wave(phase, band, true);
    return a + ((b - a) * square >> 15);
}
// A stable waveform uses one bank; select its mip pointers before rendering.
struct Oscillator {
    const opxy::i16 *rich;
    Oscillator(const Band &band, i32 square) {
        const unsigned mix = static_cast<unsigned>(band.fade + 8192) >> 14;
        rich = square ? baked_tables::square[band.rich][mix]
                      : baked_tables::saw[band.rich][mix];
    }
    i32 read(u32 phase) const { return wave_linear(rich, phase); }
};
inline i32 smooth_wave(i32 current, i32 target) {
    current += (target - current) >> 7;
    const i32 left = target - current;
    return left > -128 && left < 128 ? target : current;
}
inline i32 morph(const Oscillator &saw, const Oscillator &pulse, u32 phase,
                 i32 square) {
    if (square == 0)
        return saw.read(phase);
    const i32 b = pulse.read(phase);
    if (square == 32767)
        return b;
    const i32 a = saw.read(phase);
    return a + ((b - a) * square >> 15);
}
struct Filter : FilterCoefficients {
    explicit Filter(const FilterCoefficients &cached)
        : FilterCoefficients(cached) {}
    Filter(i32 coefficient, i32 amount, bool half = false) {
        const auto *bank = half ? half_rate_tables::coefficients
                                : four_pole_tables::coefficients;
        g = coefficient;
        unsigned low = 0, high = 73;
        while (high - low > 1) {
            const unsigned middle = (low + high) / 2;
            if (four_pole_tables::grid[middle] > g)
                high = middle;
            else
                low = middle;
        }
        const i32 fraction =
            static_cast<i32>(
                (static_cast<u32>(g - four_pole_tables::grid[low]) * 32768u) /
                static_cast<u32>(four_pole_tables::grid[high] -
                                 four_pole_tables::grid[low])) *
            65536;
        const unsigned row = static_cast<u32>(control(amount)) >> 26;
        const i32 weight = static_cast<i32>(
            (static_cast<u32>(control(amount)) & 67108863u) << 5);
        i32 result[6];
        for (unsigned i = 0; i < 6; ++i) {
            const i32 a =
                bank[row][low][i] +
                multiply(bank[row][high][i] - bank[row][low][i], fraction);
            const i32 b =
                bank[row + 1][low][i] +
                multiply(bank[row + 1][high][i] - bank[row + 1][low][i],
                         fraction);
            result[i] = a + multiply(b - a, weight);
        }
        b0[0] = result[0];
        a1[0] = result[1];
        a2[0] = result[2];
        b0[1] = result[3];
        a1[1] = result[4];
        a2[1] = result[5];
    }
    __attribute__((always_inline)) inline i32 step(Voice &voice,
                                                   i32 sample) const {
        i32 *h = voice.history;
        const i32 input = saturate(sample) * 1024;
        const i32 first =
            clamp(multiply(input + 2 * h[0] + h[1], b0[0]) -
                      multiply(h[2], a1[0]) * 2 - multiply(h[3], a2[0]),
                  -67108864, 67108864);
        const i32 second =
            clamp(multiply(first + 2 * h[2] + h[3], b0[1]) -
                      multiply(h[4], a1[1]) * 2 - multiply(h[5], a2[1]),
                  -67108864, 67108864);
        h[1] = h[0];
        h[0] = input;
        h[3] = h[2];
        h[2] = first;
        h[5] = h[4];
        h[4] = second;
        return output_knee(second >> 10);
    }
};
} // namespace dsp
} // namespace mando
