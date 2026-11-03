#include "core.h"
#include "dsp.h"
#include <opxy/q31.hpp>

namespace westfold {
using opxy::i32;
using opxy::u32;

inline void start(Voice& voice, Shared&, u32) {
    voice.phase = voice.mod_phase = voice.slope_phase = voice.initialized = 0;
    voice.envelope = opxy::q31::maximum;
    voice.gate = voice.low1 = voice.low2 = voice.dc_input = voice.dc_output = 0;
    for (unsigned c = 0; c < 8; ++c) voice.controls[c] = 0;
}
inline void initialize(Voice& voice, Shared& shared, u32 owner) { start(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

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
    const i32 decay = dsp::lookup(tables::decay, next[3]);
    const i32 ratio = dsp::lookup(tables::ratios, next[4]);
    const u32 slope_increment = static_cast<u32>(dsp::lookup(tables::rates, next[6]));
    const bool hold = next[3] >= 32600;
    const i32 depth = (next[5] * next[5] >> 15);
    const i32 next_envelope = hold ? opxy::q31::maximum : opxy::q31::multiply(voice.envelope, decay);
    const i32 envelope_step = (next_envelope - voice.envelope) / 64;

    // Reserve bandwidth for even overtones, PM sidebands and slope movement.
    // Eight harmonic limits crossfade instead of running a discontinuous folder
    // at the device sample rate.
    u32 maximum_pitch = 0;
    for (unsigned i = 0; i < opxy::block_frames; ++i)
        if (pitch[i] > maximum_pitch) maximum_pitch = pitch[i];
    if (maximum_pitch > 0x70000000u) maximum_pitch = 0x70000000u;
    const u32 mod_increment = dsp::ratio_increment(maximum_pitch, ratio);
    const u32 pm_budget = (mod_increment >> 15) * static_cast<u32>(depth) * 2u;
    const u32 motion_budget = (slope_increment >> 12) * static_cast<u32>(next[7]) / 8u;
    u32 budget = maximum_pitch + (pm_budget > 0x20000000u ? 0x20000000u : pm_budget);
    const dsp::Band clean_band = dsp::band(budget);
    budget += motion_budget > 0x10000000u ? 0x10000000u : motion_budget;
    const dsp::Band band = dsp::band(budget > 0x30000000u ? 0x60000000u : budget * 2u);
    const i32 even_guard = maximum_pitch <= 0x30000000u ? 32768
        : maximum_pitch >= 0x40000000u ? 0 : static_cast<i32>((0x40000000u - maximum_pitch) >> 13);
    const i32 fm_guard = maximum_pitch <= 0x10000000u ? 32768
        : maximum_pitch >= 0x40000000u ? 0 : dsp::third((0x40000000u - maximum_pitch) >> 13);
    u32 previous_pitch = pitch[0] > 0x70000000u ? 0x70000000u : pitch[0];
    u32 mod_step = dsp::ratio_increment(previous_pitch, ratio);
    i32 coefficient = dsp::lookup(tables::gate_coefficients, voice.gate);
    i32 fm = ((voice.controls[5] * voice.controls[5] >> 15) * fm_guard >> 15);
    const i32 next_fm = (depth * fm_guard >> 15);
    const i32 fm_step = (next_fm - fm) / 64;

    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        voice.controls[0] += step[0];
        voice.controls[1] += step[1];
        voice.controls[2] += step[2];
        voice.controls[7] += step[7];
        const i32* c = voice.controls;
        const i32 envelope = hold ? opxy::q31::maximum : voice.envelope;
        voice.gate += ((envelope >> 16) - voice.gate) >> 5;
        const i32 gate = voice.gate;
        const i32 cycle = dsp::slope(voice.slope_phase) * 2 - 32768;
        const i32 motion = (cycle * c[7] >> 15);
        const i32 folding = dsp::clamp((c[1] * (24576 + gate / 4) >> 15) + motion / 2, 0, 32767);
        const u32 fold = static_cast<u32>(folding) >> 11;
        const i32 fraction = (folding & 2047) * 16;

        fm += fm_step;
        const i32 offset = dsp::sine(voice.mod_phase) * fm;
        const u32 phase = voice.phase + static_cast<u32>(offset);
        i32 signal = dsp::clean(clean_band, phase);
        if(c[2]) {
            const i32 odd = dsp::wave(fold, fraction, band, phase);
            i32 shaped = odd;
            if(c[0]) {
                const i32 even = even_guard ? dsp::wave(fold, fraction, band, phase * 2u) * even_guard >> 15 : 0;
                shaped = dsp::mix(odd, even, c[0]);
            }
            signal = dsp::mix(signal, shaped, c[2]);
        }

        // Two gentle poles and gain close together as the strike fades.
        if (!(i & 7u)) coefficient = dsp::lookup(tables::gate_coefficients, gate);
        voice.low1 += ((signal - voice.low1) * coefficient >> 15);
        voice.low2 += ((voice.low1 - voice.low2) * coefficient >> 15);
        const i32 filtered = dsp::mix(voice.low2, voice.low1, 8192 + gate / 2);
        const i32 amplitude = (gate * (8192 + gate * 3 / 4) >> 15);
        const i32 sample = (filtered * amplitude >> 15);
        const i32 blocked = sample - voice.dc_input + voice.dc_output * 32690 / 32768;
        voice.dc_input = sample;
        voice.dc_output = dsp::clamp(blocked, -32768, 32767);
        output[i] = voice.dc_output * 8192;

        voice.envelope += envelope_step;
        const u32 increment = pitch[i] > 0x70000000u ? 0x70000000u : pitch[i];
        voice.phase += increment;
        if (increment != previous_pitch) {
            mod_step = dsp::ratio_increment(increment, ratio);
            previous_pitch = increment;
        }
        voice.mod_phase += mod_step;
        voice.slope_phase += slope_increment;
    }
    voice.envelope = next_envelope;
    for (unsigned c = 0; c < 8; ++c) voice.controls[c] = next[c];
}
}
