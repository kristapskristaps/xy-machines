#include "core.h"
#include "dsp.h"
#include "coast_tables.h"
#include <opxy/q31.hpp>

namespace westfold {
using opxy::i32;
using opxy::u32;

inline void start(Voice& voice, Shared&, u32) {
    voice.phase = voice.mod_phase = voice.slope_phase = voice.initialized = 0;
    voice.envelope = opxy::q31::maximum;
    voice.gate = voice.low1 = voice.low2 = voice.dc_input = voice.dc_output = 0;
    for (unsigned c = 0; c < 8; ++c) voice.controls[c] = 0;
    for (unsigned h = 0; h < 9; ++h) voice.harmonics[h] = 0;
}
inline void initialize(Voice& voice, Shared& shared, u32 owner) { start(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

namespace {
// Fade each partial before Nyquist, including room for the modulation sidebands.
i32 harmonic_gain(u32 budget, u32 harmonic) {
    const u32 end = 0x78000000u / harmonic, begin = 0x58000000u / harmonic;
    if (budget <= begin) return 32768;
    if (budget >= end) return 0;
    return dsp::clamp(static_cast<i32>((end - budget) / ((end - begin) >> 15)), 0, 32768);
}

void coefficients(i32 fold, i32 overtone, i32 balance, const i32* gain, i32* weights) {
    const u32 frame = static_cast<u32>(fold) >> 11;
    const i32 fraction = (fold & 2047) * 16;
    const i32 parity = dsp::clamp(overtone * 3 / 2, 0, 32768);
    for (unsigned h = 0; h < 9; ++h) {
        // Each order belongs to just one family; the other coefficient is zero.
        const u32 family = h & 1u;
        const i32 partial = dsp::mix(coast::frames[family][frame][h], coast::frames[family][frame + 1][h], fraction);
        const i32 shaped = partial * (family ? parity : 32768 - parity) >> 15;
        weights[h] = dsp::mix(coast::clean[h], shaped, balance) * gain[h] >> 15;
    }
}

// Clenshaw evaluates all nine Chebyshev partials with one product per order.
// Q13 recurrence states and a Q15 oscillator keep every product inside i32.
// The generated frames bound sum(order * abs(coefficient)) below 214,000 Q15.
inline i32 harmonic_sample(i32 fundamental, i32* weights, const i32* step) {
    i32 next = 0, after = 0;
    for (unsigned h = 9; h-- > 0;) {
        weights[h] += step[h];
        const i32 current = (weights[h] >> 2) + (fundamental * next >> 14) - after;
        after = next; next = current;
    }
    return ((fundamental * next >> 15) - after) * 4;
}

}

inline void render(Voice& __restrict voice, Shared&, u32, opxy::Output __restrict output,
                   opxy::Parameters __restrict parameters, opxy::Pitch __restrict pitch) {
    i32 next[8], step[8];
    for (unsigned c = 0; c < 8; ++c) {
        const i32 target = dsp::control(parameters[c]);
        if (!voice.initialized) voice.controls[c] = target;
        next[c] = voice.controls[c] + (target - voice.controls[c]) / 2;
        step[c] = (next[c] - voice.controls[c]) / 64;
    }
    voice.initialized = 1;
    const bool hold = next[3] >= 32600;
    const i32 decay = dsp::lookup(tables::decay, next[3]);
    const i32 next_envelope = hold ? opxy::q31::maximum : opxy::q31::multiply(voice.envelope, decay);
    const i32 envelope_step = (next_envelope - voice.envelope) / 64;
    const i32 ratio = dsp::lookup(tables::ratios, next[4]);
    const u32 slope_increment = static_cast<u32>(dsp::lookup(tables::rates, next[6]));
    const i32 depth = next[5] * next[5] >> 15;
    const i32 previous_depth = voice.controls[5] * voice.controls[5] >> 15;
    u32 maximum_pitch = 0;
    for (unsigned i = 0; i < opxy::block_frames; ++i)
        if (pitch[i] > maximum_pitch) maximum_pitch = pitch[i];
    if (maximum_pitch > 0x70000000u) maximum_pitch = 0x70000000u;
    const u32 mod_increment = dsp::ratio_increment(maximum_pitch, ratio);
    const u32 pm_budget = (mod_increment >> 15) * static_cast<u32>(depth > previous_depth ? depth : previous_depth) * 2u;
    const u32 budget = maximum_pitch + (pm_budget > 0x20000000u ? 0x20000000u : pm_budget) + slope_increment * 2u;
    i32 gain[9], weight_step[9] = {};
    for (u32 h = 0; h < 9; ++h) gain[h] = harmonic_gain(budget, h + 1);
    const i32 fm_guard = maximum_pitch <= 0x10000000u ? 32768
        : maximum_pitch >= 0x40000000u ? 0 : dsp::third((0x40000000u - maximum_pitch) >> 13);
    i32 fm = previous_depth * fm_guard >> 15;
    const i32 fm_step = ((depth * fm_guard >> 15) - fm) / 64;
    u32 previous_pitch = maximum_pitch, mod_step = dsp::ratio_increment(previous_pitch, ratio);
    i32 coefficient = 0;

    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        voice.controls[0] += step[0]; voice.controls[1] += step[1];
        voice.controls[2] += step[2]; voice.controls[7] += step[7];
        const i32* c = voice.controls;
        const i32 envelope = hold ? opxy::q31::maximum : voice.envelope;
        voice.gate += ((envelope >> 16) - voice.gate) >> 4;
        const i32 gate = voice.gate;
        const i32 cycle = dsp::slope(voice.slope_phase) * 2 - 32768;
        if (!(i & 15u)) {
            const i32 motion = cycle * c[7] >> 15;
            const i32 fold = dsp::clamp((c[1] * (24576 + gate / 4) >> 15) + motion / 2, 0, 32767);
            i32 target[9]; coefficients(fold, c[0], c[2], gain, target);
            for (unsigned h = 0; h < 9; ++h) weight_step[h] = (target[h] - voice.harmonics[h]) / 16;
            coefficient = dsp::lookup(tables::gate_coefficients, gate);
        }
        fm += fm_step;
        const u32 phase = voice.phase + static_cast<u32>(dsp::sine(voice.mod_phase) * fm);
        const i32 fundamental = dsp::sine(phase);
        i32 signal = harmonic_sample(fundamental, voice.harmonics, weight_step);
        const i32 slope_mix = dsp::clamp((c[0] - 24576) * 4, 0, 32768) * c[2] >> 15;
        signal = dsp::mix(signal, signal * cycle >> 15, slope_mix);
        voice.low1 += (signal - voice.low1) * coefficient >> 15;
        voice.low2 += (voice.low1 - voice.low2) * coefficient >> 15;
        const i32 filtered = dsp::mix(voice.low2, voice.low1, 8192 + gate / 2);
        const i32 amplitude = gate * (8192 + gate * 3 / 4) >> 15;
        const i32 sample = filtered * amplitude >> 15;
        const i32 blocked = sample - voice.dc_input + voice.dc_output * 32690 / 32768;
        voice.dc_input = sample;
        voice.dc_output = dsp::clamp(blocked, -32768, 32767);
        output[i] = voice.dc_output * 8192;
        voice.envelope += envelope_step;
        const u32 increment = pitch[i] > 0x70000000u ? 0x70000000u : pitch[i];
        voice.phase += increment;
        if (increment != previous_pitch) { mod_step = dsp::ratio_increment(increment, ratio); previous_pitch = increment; }
        voice.mod_phase += mod_step;
        voice.slope_phase += slope_increment;
    }
    voice.envelope = next_envelope;
    for (unsigned c = 0; c < 8; ++c) voice.controls[c] = next[c];
}
}
