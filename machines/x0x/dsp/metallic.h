#pragma once
#include "primitives.h"
#include "metallic_state.h"
#include "metallic_coefficients.h"

namespace x0x { namespace dsp {
// Signed Q14 products now use the DSP floor directly. Negative fractional
// terms differ from the preceding truncation by at most one Q14 count.
// Samples and coefficient magnitudes remain below32768, so scaling is safe.
__attribute__((always_inline)) static inline i32 metal_product(i32 sample,i32 coefficient) {
    if(!coefficient)return 0;
    return opxy::q31::multiply(sample*2,coefficient*65536);
}
// Each Q14 product fits i32. Scale before summing so resonant feedback never
// needs a wider accumulator; clamp every stored signal to the same bound.
__attribute__((always_inline)) static inline i32 metal_filter(MetalFilterState& s, const MetalCoefficients& c, i32 x) {
    const i32 y = limit(metal_product(x,c.b0) + metal_product(s.x1,c.b1) + metal_product(s.x2,c.b2)
                      - metal_product(s.y1,c.a1) - metal_product(s.y2,c.a2));
    s.x2 = s.x1; s.x1 = x; s.y2 = s.y1; s.y1 = y;
    return y;
}
// Polynomial edge correction retains the upper rectangular-wave harmonics
// needed by the 7.1 kHz band, while reducing folded oscillator edges.
static inline i32 metal_blep(u32 phase, u32 width, u32 reciprocal) {
    const u32 t = phase >> 16;
    if (t < width) {
        const i32 x = i32(t * reciprocal >> 16);
        return 2*x - multiply(x,x) - 32768;
    }
    if (t > 65536 - width) {
        const i32 x = (i32(t) - 65536) * i32(reciprocal) / 65536;
        return multiply(x,x) + 2*x + 32768;
    }
    return 0;
}
static inline i32 metal_pulse(u32 phase, u32 increment, u32 reciprocal) {
    const u32 duty = 2060725329u; // 47.98% nominal Schmitt-trigger duty.
    const u32 width = increment >> 16;
    return (phase < duty ? 8192 : -8192)
        + (metal_blep(phase,width,reciprocal) - metal_blep(phase-duty,width,reciprocal))/4;
}
static inline void metal_start(MetalState& state, u32* phases) {
    state = MetalState();
    state.fast = state.medium = 0x7fffffff;
    state.initialized = 1;
    // Stagger the bank: the physical oscillators are not phase-locked at a hit.
    static const u32 initial[6] = {0x183bc91du,0x9c3f742bu,0x5328ad49u,0xd74c1637u,0x328da561u,0xade78353u};
    for (u32 i = 0; i < 6; ++i) phases[i] = initial[i];
}
static inline i32 metal_source(MetalState& state, const u32* phases, const u32* increments,
                               const u32* reciprocals, bool cymbal, u32 tone, u32 body) {
    i32 cluster = 0;
    for (u32 i = 0; i < 6; ++i) cluster += divide_signal<6>(metal_pulse(phases[i],increments[i],reciprocals[i]));
    const i32 high = metal_filter(state.filters[1],high_band,cluster);
    const i32 bright = metal_filter(state.filters[3],hat_cut,high);
    i32 air = metal_filter(state.filters[4],air_cut,high);
    state.air_low += multiply(air-state.air_low,25432);
    air -= state.air_low;
    if (!cymbal) return limit((multiply(bright,32767-i32(tone)*160) + multiply(air,i32(tone)*512))*32);
    const i32 low = metal_filter(state.filters[0],low_band,cluster);
    const i32 bell = metal_filter(state.filters[2],low_cut,low);
    const i32 strike = multiply(bell,state.fast>>16);
    const i32 shimmer = multiply(bright,8192+(state.medium>>17));
    state.fast = envelope(state.fast,rate(180));
    state.medium = envelope(state.medium,rate(700));
    return limit((multiply(strike,8192+i32(body)*192) + shimmer + multiply(air,i32(tone)*512))*18);
}
}}
