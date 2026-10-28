// Copyright 2014 Emilie Gillet. MIT; see LICENSE.
// Integer adaptation of Clouds' grain.h and granular_sample_player.h.
#pragma once
#include "../core.h"
#include "tables.h"
#include <opxy/q31.hpp>
namespace vapour { namespace granular {
using opxy::i32;
using opxy::u32;
inline i32 clamp(i32 x, i32 low, i32 high) { return x < low ? low : x > high ? high : x; }
inline u32 random(State& state) { return state.random = state.random * 1664525u + 1013904223u; }
inline u32 control(i32 x) { return x < 0 ? 0u : static_cast<u32>(x) >> 16; }
inline i32 mul15(i32 a, i32 b) {
    // b is a nonnegative gain, at most 65535. Splitting the signed sample
    // keeps both products in 32 bits, including overshooting cubic coefficients.
    return (a >> 15) * b + ((a & 32767) * b >> 15);
}
inline i32 gain15(i32 sample, i32 gain) { return sample * gain >> 15; }
inline u32 lookup(const u32* table, u32 value) {
    const u32 index = value >> 7, fraction = value & 127u;
    const i32 a = static_cast<i32>(table[index]), b = static_cast<i32>(table[index + 1]);
    return static_cast<u32>(a + (b - a) * static_cast<i32>(fraction) / 128);
}
inline i32 sample(const State& state, const opxy::i32* ring, u32 index, unsigned channel) {
    index &= ring_frames - 1;
    if (index >= state.written) return 0;
    const u32 packed = static_cast<u32>(ring[index]);
    return static_cast<opxy::i16>(channel ? packed >> 16 : packed & 65535u);
}
inline i32 read(const State& state, const opxy::i32* ring, u32 phase, unsigned channel, bool high) {
    const u32 index = phase >> 16;
    const i32 t = static_cast<i32>((phase & 65535u) >> 1);
    if (!high) {
        const i32 a = sample(state,ring,index,channel), b = sample(state,ring,index+1,channel);
        return a + gain15(b-a,t);
    }
    const i32 xm1=sample(state,ring,index,channel), x0=sample(state,ring,index+1,channel);
    const i32 x1=sample(state,ring,index+2,channel), x2=sample(state,ring,index+3,channel);
    const i32 c=(x1-xm1)/2, v=x0-x1, w=c+v, a=w+v+(x2-x0)/2;
    const i32 b_neg=w+a;
    return clamp(mul15(mul15(mul15(a,t)-b_neg,t)+c,t)+x0,-32768,32767);
}
inline i32 envelope(Grain& grain, bool high = true) {
    const u32 p = grain.envelope >> 16;
    i32 gain = static_cast<i32>(p < 32768 ? p : 65535-p);
    if (grain.smoothness && high) {
        const u32 index=static_cast<u32>(gain)>>5, fraction=static_cast<u32>(gain)&31u;
        const i32 a=static_cast<i32>(tables::window[index]);
        const i32 b=static_cast<i32>(tables::window[index+1]);
        const i32 window=a+(b-a)*static_cast<i32>(fraction)/32;
        gain += gain15(window-gain,grain.smoothness);
    } else if (!grain.smoothness) {
        gain = clamp(gain * (grain.slope >> 15) + ((gain * (grain.slope & 32767)) >> 15), 0, 32767);
    }
    grain.envelope += grain.envelope_increment;
    return gain;
}
inline void schedule(State& state, Grain& grain, u32 size, u32 ratio, u32 position, u32 texture) {
    u32 width=lookup(tables::grain_size,size);
    if (ratio>65536u) {
        const u32 maximum=((ring_frames/4)<<12)/(ratio>>4);
        if (width>maximum) width=maximum;
    }
    width &= ~1u;
    const u32 consumed=(width * (ratio>>4)) >> 12;
    const u32 available=ring_frames-width-consumed;
    const u32 age=consumed+((available*position)>>15);
    grain.phase=((state.write-age)&(ring_frames-1))<<16;
    grain.increment=ratio;
    grain.envelope=0;
    grain.envelope_increment=0xffffffffu/width;
    grain.remaining=width;
    const u32 shape=texture<24576u ? texture*4u/3u : 32767u;
    grain.smoothness=shape>=16384u ? static_cast<i32>((shape-16384u)*2u) : 0;
    grain.slope=shape>=16384u ? 32768 : static_cast<i32>(536870912u/(shape+328u));
    const i32 pan=16384+(static_cast<i32>(random(state)>>17)-16384)*3/4;
    grain.left_gain=pan<16384 ? 32767 : 2*(32767-pan);
    grain.right_gain=pan<16384 ? 2*pan : 32767;
    state.size_hint += (static_cast<i32>(width)-static_cast<i32>(state.size_hint))/10;
}
}}
