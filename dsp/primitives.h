#pragma once
#include "tables.h"
#include <opxy/q31.hpp>

namespace x0x { namespace dsp {
using opxy::i32;
using opxy::u32;
static inline i32 limit(i32 value, i32 bound = 32767) { return value < -bound ? -bound : value > bound ? bound : value; }
// Q15 signals/coefficient products bounded to signed 32-bit arithmetic.
static inline i32 multiply(i32 a, i32 b) { return a * b / 32768; }
static inline u32 scale(u32 value, u32 q16) {
    return (value >> 16) * q16 + ((value & 65535u) * (q16 >> 8) >> 8);
}
static inline u32 step(u32 hz) { return hz * 97392u; }
static inline i32 sine(u32 phase) {
    const u32 index = phase >> 23;
    const i32 a = tables::sine[index], b = tables::sine[index + 1];
    return a + multiply(b - a, i32((phase >> 8) & 32767));
}
static inline i32 triangle(u32 phase) {
    const u32 folded = phase & 0x7fffffffu;
    return phase & 0x80000000u ? 32767 - i32(folded >> 15) : i32(folded >> 15) - 32768;
}
// Three odd partials keep the metallic source's intentional square character
// without an unbounded set of aliased harmonics.
static inline i32 square(u32 phase) { return (15 * sine(phase) + 5 * sine(phase * 3) + 3 * sine(phase * 5)) / 23; }
static inline i32 noise(u32& seed) {
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return i32(seed >> 17) - 16384;
}
static inline i32 rate(u32 milliseconds) {
    if (milliseconds < 4) milliseconds = 4;
    if (milliseconds > 6144) milliseconds = 6144;
    const u32 at = milliseconds >> 2, fraction = milliseconds & 3;
    return tables::decay[at] + (tables::decay[at + 1] - tables::decay[at]) / 4 * i32(fraction);
}
static inline i32 envelope(i32 value, i32 coefficient) { return opxy::q31::multiply(value, coefficient); }
static inline i32 drive(i32 sample, u32 amount) {
    if (!amount) return sample;
    const i32 boosted = limit(sample * (1 + i32(amount) / 16));
    const i32 half = boosted / 2;
    const i32 cube = (half * half / 16384) * half / 16384;
    return boosted - 2 * cube / 3;
}
}}
