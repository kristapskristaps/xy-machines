#pragma once
#include <opxy/audio.hpp>
#include "tables.h"

namespace westfold { namespace dsp {
using opxy::i32;
using opxy::u32;
inline i32 clamp(i32 x, i32 lo, i32 hi) { return x < lo ? lo : x > hi ? hi : x; }
inline i32 control(i32 x) { return x < 0 ? 0 : x >> 16; }
inline i32 mix(i32 a, i32 b, i32 t) { return a + ((b - a) * t >> 15); }
inline i32 read(const opxy::i16* table, u32 phase) {
    const u32 index = phase >> 23;
    const i32 fraction = static_cast<i32>((phase >> 8) & 32767u);
    return mix(table[index], table[index + 1], fraction);
}
inline i32 sine(u32 phase) { return read(tables::sine, phase); }
inline i32 lookup(const i32* table, i32 value) {
    const u32 position = static_cast<u32>(clamp(value, 0, 32767));
    const u32 index = position >> 8;
    const i32 difference = table[index + 1] - table[index];
    const i32 fraction = static_cast<i32>(position & 255u);
    // Split before multiplying: the short-decay coefficient steps exceed Q15.
    return table[index] + (difference >> 8) * fraction + ((difference & 255) * fraction >> 8);
}
// Q12 ratios keep both halves of the pitch multiply inside a 32-bit word.
inline u32 ratio_increment(u32 increment, i32 ratio) {
    const u32 high = (increment >> 16) * static_cast<u32>(ratio);
    if (high >= (0x30000000u >> 4)) return 0x30000000u;
    const u32 result = high * 16u + ((increment & 65535u) * static_cast<u32>(ratio) >> 12);
    return result > 0x30000000u ? 0x30000000u : result;
}
// A fast rise and rounded, longer fall, independent of pitch.
inline i32 slope(u32 phase) {
    const i32 p = static_cast<i32>(phase >> 16);
    const i32 ramp = p < 16384 ? p * 2 : ((65535 - p) * 21845 >> 15);
    return (ramp * ramp >> 15);
}
struct Band { u32 index; i32 fade; };
inline Band band(u32 increment) {
    u32 threshold = 0x60000000u >> 7;
    u32 index = 0;
    while (index < 7 && increment > threshold) { threshold *= 2; ++index; }
    Band result = {index, 0};
    if (index < 7 && increment > threshold / 2)
        result.fade = static_cast<i32>((increment - threshold / 2) / (threshold >> 16));
    result.fade = clamp(result.fade, 0, 32768);
    return result;
}
inline i32 wave(u32 fold, i32 fraction, const Band& band, u32 phase) {
    const i32 a = mix(read(tables::waves[band.index][fold], phase),
                      read(tables::waves[band.index][fold + 1], phase), fraction);
    if (!band.fade || band.index == 7) return a;
    const i32 b = mix(read(tables::waves[band.index + 1][fold], phase),
                      read(tables::waves[band.index + 1][fold + 1], phase), fraction);
    return mix(a, b, band.fade);
}
inline i32 clean(const Band& band, u32 phase) {
    const i32 a = read(tables::waves[band.index][0], phase);
    if (!band.fade || band.index == 7) return a;
    return mix(a, read(tables::waves[band.index + 1][0], phase), band.fade);
}
} }
