#pragma once
#include "tables.h"
#include "metal_stems.h"
#include <opxy/q31.hpp>

namespace x0x { namespace dsp {
using opxy::i32;
using opxy::u32;
static inline i32 limit(i32 value, i32 bound = 32767) { return value < -bound ? -bound : value > bound ? bound : value; }
// Q15 signals/coefficient products bounded to signed 32-bit arithmetic.
static inline i32 multiply(i32 a, i32 b) { return a * b / 32768; }
// Callers prove both operands fit signed16; keep the original truncation.
static inline i32 product16(i32 a,i32 b){return static_cast<opxy::i16>(a)*static_cast<opxy::i16>(b);}
static inline i32 multiply16(i32 a,i32 b){return product16(a,b)/32768;}
// Exact reciprocal division for bounded audio signals: |value| <= 65536 for
// /3 and /6, or <= 23*32768 for the harmonic sum's /23 and /138. The smaller
// signals fit a single unsigned product. Harmonic sums use one public Q31 DSP
// multiply, restoring the sign after its nonnegative product.
template <u32 divisor> static inline i32 divide_signal(i32 value) {
    static_assert(divisor == 3 || divisor == 6 || divisor == 23 || divisor == 138, "Signal divisor");
    const u32 magnitude = value < 0 ? 0u-u32(value) : u32(value);
    if(divisor==3 || divisor==6){
        const u32 quotient=magnitude*43691u>>(divisor==3?17:18);
        return value<0?-i32(quotient):i32(quotient);
    }
    const u32 reciprocal = (0x80000000u+divisor-1u)/divisor;
    const u32 bound = divisor <= 6 ? 65536u : 23u*32768u;
    static_assert(bound*(reciprocal*divisor-0x80000000u)<0x80000000u,"Exact bounded reciprocal");
    const i32 quotient = opxy::q31::multiply(i32(magnitude),i32(reciprocal));
    return value < 0 ? -quotient : quotient;
}
// Exact floor(numerator*32767/denominator), for 0 <= numerator <= denominator
// <= 65535. Cache floor((32767<<16)/denominator) when the control changes.
static inline u32 ratio_q15(u32 numerator, u32 denominator, u32 reciprocal) {
    const u32 quotient = numerator*reciprocal >> 16;
    return quotient + ((quotient+1)*denominator <= numerator*32767u);
}
static inline u32 scale(u32 value, u32 q16) {
    return (value >> 16) * q16 + ((value & 65535u) * (q16 >> 8) >> 8);
}
static inline u32 step(u32 hz) { return hz * 97392u; }
static inline i32 sine(u32 phase) {
    const u32 index = phase >> 23;
    const i32 a = tables::sine[index], b = tables::sine[index + 1];
    return a + multiply16(b - a, i32((phase >> 8) & 32767));
}
static inline i32 triangle(u32 phase) {
    const u32 folded = phase & 0x7fffffffu;
    return phase & 0x80000000u ? 32767 - i32(folded >> 15) : i32(folded >> 15) - 32768;
}
// Three odd partials keep the metallic source's intentional square character
// without an unbounded set of aliased harmonics.
static inline i32 square_partials(u32 phase) { return 15 * sine(phase) + 5 * sine(phase * 3) + 3 * sine(phase * 5); }
static inline i32 metallic_shape(u32 phase,u32 shape) {
    const u32 index=phase>>22;
    const i32 a=tables::metallic_square[shape][index],b=tables::metallic_square[shape][index+1];
    return a+multiply16(b-a,i32((phase>>7)&32767));
}
static inline i32 square(u32 phase) { return metallic_shape(phase,0); }
static inline i32 square_sixth(u32 phase) { return metallic_shape(phase,1); }
static inline i32 noise(u32& seed) {
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return i32(seed >> 17) - 16384;
}
static inline i32 rate(u32 milliseconds) {
    if (milliseconds < 4) milliseconds = 4;
    if (milliseconds > 6144) milliseconds = 6144;
    if (milliseconds < 8) return tables::decay[milliseconds - 4];
    u32 shift = 0;
    while ((milliseconds >> shift) >= 16) ++shift;
    const u32 at = 4 + shift * 8 + (milliseconds >> shift) - 8;
    const u32 fraction = milliseconds & ((1u << shift) - 1);
    return tables::decay[at] + i32(u32(tables::decay[at + 1] - tables::decay[at]) * fraction >> shift);
}
static inline i32 envelope(i32 value, i32 coefficient) { return opxy::q31::multiply(value, coefficient); }
static inline i32 drive(i32 sample, u32 amount) {
    if (!amount) return sample;
    const i32 boosted = limit(sample + product16(sample,i32(amount))/16);
    const i32 half = boosted / 2;
    const i32 cube = product16(product16(half,half)/16384,half)/16384;
    return boosted - divide_signal<3>(2 * cube);
}
}}
