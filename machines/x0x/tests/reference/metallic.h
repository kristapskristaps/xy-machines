// Frozen97b6a9c test reference; only function names prefixed. Shared tables/types are unchanged.
#pragma once
namespace x0x { namespace dsp {
// Signals and coefficient magnitudes stay below 32768. Multiply magnitudes
// through the DSP primitive, then restore each Q14 term's truncation toward zero.
__attribute__((always_inline)) static inline i32 legacy_metal_product(i32 sample, i32 coefficient) {
    if(!coefficient)return 0;
    const i32 magnitude=sample<0?-sample:sample;
    const i32 gain=coefficient<0?-coefficient:coefficient;
    const i32 product=opxy::q31::multiply(magnitude*2,gain*65536);
    return (sample<0)!=(coefficient<0)?-product:product;
}
// Each Q14 product fits i32. Scale before summing so resonant feedback never
// needs a wider accumulator; clamp every stored signal to the same bound.
__attribute__((always_inline)) static inline i32 legacy_metal_filter(MetalFilterState& s, const MetalCoefficients& c, i32 x) {
    const i32 y = limit(legacy_metal_product(x,c.b0) + legacy_metal_product(s.x1,c.b1) + legacy_metal_product(s.x2,c.b2)
                      - legacy_metal_product(s.y1,c.a1) - legacy_metal_product(s.y2,c.a2));
    s.x2 = s.x1; s.x1 = x; s.y2 = s.y1; s.y1 = y;
    return y;
}
// Polynomial edge correction retains the upper rectangular-wave harmonics
// needed by the 7.1 kHz band, while reducing folded oscillator edges.
static inline i32 legacy_metal_blep(u32 phase, u32 width, u32 reciprocal) {
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
static inline i32 legacy_metal_pulse(u32 phase, u32 increment, u32 reciprocal) {
    const u32 duty = 2060725329u; // 47.98% nominal Schmitt-trigger duty.
    const u32 width = increment >> 16;
    return (phase < duty ? 8192 : -8192)
        + (legacy_metal_blep(phase,width,reciprocal) - legacy_metal_blep(phase-duty,width,reciprocal))/4;
}
static inline void legacy_metal_start(MetalState& state, u32* phases) {
    state = MetalState();
    state.fast = state.medium = 0x7fffffff;
    state.initialized = 1;
    // Stagger the bank: the physical oscillators are not phase-locked at a hit.
    static const u32 initial[6] = {0x183bc91du,0x9c3f742bu,0x5328ad49u,0xd74c1637u,0x328da561u,0xade78353u};
    for (u32 i = 0; i < 6; ++i) phases[i] = initial[i];
}
static inline i32 legacy_metal_source(MetalState& state, const u32* phases, const u32* increments,
                               const u32* reciprocals, bool cymbal, u32 tone, u32 body) {
    i32 cluster = 0;
    for (u32 i = 0; i < 6; ++i) cluster += divide_signal<6>(legacy_metal_pulse(phases[i],increments[i],reciprocals[i]));
    const i32 high = legacy_metal_filter(state.filters[1],high_band,cluster);
    const i32 bright = legacy_metal_filter(state.filters[3],hat_cut,high);
    i32 air = legacy_metal_filter(state.filters[4],air_cut,high);
    state.air_low += multiply(air-state.air_low,25432);
    air -= state.air_low;
    if (!cymbal) return limit((multiply(bright,32767-i32(tone)*160) + multiply(air,i32(tone)*512))*32);
    const i32 low = legacy_metal_filter(state.filters[0],low_band,cluster);
    const i32 bell = legacy_metal_filter(state.filters[2],low_cut,low);
    const i32 strike = multiply(bell,state.fast>>16);
    const i32 shimmer = multiply(bright,8192+(state.medium>>17));
    state.fast = envelope(state.fast,rate(180));
    state.medium = envelope(state.medium,rate(700));
    return limit((multiply(strike,8192+i32(body)*192) + shimmer + multiply(air,i32(tone)*512))*18);
}
}}
