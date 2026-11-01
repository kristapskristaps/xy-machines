#include "core.h"
#include "dsp.h"

namespace cyclops {
namespace {
template <bool Moving, bool Constant, bool Limited>
void render_audio(Voice& voice, opxy::Output __restrict output, opxy::Pitch __restrict pitch,
                  const dsp::Band& first, const dsp::Band& second,
                  const dsp::Oscillator& osc_a, const dsp::Oscillator& osc_b,
                  const dsp::Ladder& filter, const i32* target_wave,
                  u32 ratio, u32 octave, i32 balance, i32 drive) {
    using namespace dsp;
    const Ladder coefficients = filter;
    const Oscillator main_a = Moving ? Oscillator(first, 0) : osc_a;
    const Oscillator main_b = Moving ? Oscillator(second, 0) : osc_b;
    const Oscillator pulse_a(first, 32767), pulse_b(second, 32767);
    const u32 fixed_first = pitch[0] > 0x40000000u ? 0x40000000u : pitch[0];
    const u32 fixed_second = second_increment(fixed_first, ratio, octave);
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        if (Moving) {
            voice.wave[0] = smooth_wave(voice.wave[0], target_wave[0]);
            voice.wave[1] = smooth_wave(voice.wave[1], target_wave[1]);
        }
        const u32 increment = Constant ? fixed_first : pitch[i] > 0x40000000u ? 0x40000000u : pitch[i];
        const u32 other = Constant ? fixed_second : second_increment(increment, ratio, octave);
        const i32 a = balance == 32767 ? 0 : Moving
            ? morph(main_a, pulse_a, voice.phase[0], voice.wave[0]) : main_a.read(voice.phase[0]);
        const i32 b = balance == 0 ? 0 : Moving
            ? morph(main_b, pulse_b, voice.phase[1], voice.wave[1]) : main_b.read(voice.phase[1]);
        const i32 mixed = a + ((b - a) * balance >> 15);
        const i32 driven = mixed * drive >> 8;
        const i32 filtered = coefficients.step<Limited>(voice, driven);
        voice.phase[0] += increment;
        voice.phase[1] += other;
        const i32 charge = voice.dc_output * 32721 + voice.dc_residue;
        const i32 dc = filtered - voice.dc_input + (charge >> 15); // 10 Hz
        voice.dc_residue = static_cast<i32>(static_cast<u32>(charge) & 32767u);
        voice.dc_input = filtered;
        voice.dc_output = dc;
        output[i] = clamp(dc, -unity, unity) * 32768;
    }
}
template <bool Moving, bool Constant>
void dispatch_audio(Voice& voice, opxy::Output __restrict output, opxy::Pitch __restrict pitch,
                    const dsp::Band& first, const dsp::Band& second,
                    const dsp::Oscillator& osc_a, const dsp::Oscillator& osc_b,
                    const dsp::Ladder& filter, const i32* target_wave,
                    u32 ratio, u32 octave, i32 balance, i32 drive,
                    bool limited) {
    if (limited) render_audio<Moving, Constant, true>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive);
    else render_audio<Moving, Constant, false>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive);
}

}
inline void start(Voice& voice, Shared&, u32) {
    voice.phase[0] = 0;
    voice.phase[1] = 0x20000000u;
    for (unsigned i = 0; i < 4; ++i) voice.ladder[i] = 0;
    for (unsigned i = 0; i < 8; ++i) voice.controls[i] = 0;
    voice.ready = 0;
    voice.wave[0] = voice.wave[1] = 0;
    voice.dc_input = voice.dc_output = voice.dc_residue = 0;
    voice.cached_filter.g = 0;
}
inline void initialize(Voice& voice, Shared& shared, u32 owner) { start(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

inline void render(Voice& voice, Shared&, u32, opxy::Output __restrict output,
                   opxy::Parameters parameters, opxy::Pitch __restrict pitch) {
    using namespace dsp;
    for (unsigned j = 0; j < 8; ++j) {
        const i32 target = control(parameters[j]);
        if (!voice.ready || j == 3 || j == 6) voice.controls[j] = target;
        else voice.controls[j] += (target - voice.controls[j]) / 4;
    }
    const i32* p = voice.controls;
    const u32 waves = option(p[3], 4), octave = option(p[6], 3), ratio = tuning(p[4]);
    const i32 target_wave[2] = {waves >= 2 ? 32767 : 0, waves & 1 ? 32767 : 0};
    if (!voice.ready) {
        voice.wave[0] = target_wave[0]; voice.wave[1] = target_wave[1];
        voice.ready = 1;
    }
    u32 highest = 0;
    for (unsigned i = 0; i < opxy::block_frames; ++i) if (pitch[i] > highest) highest = pitch[i];
    if (highest > 0x40000000u) highest = 0x40000000u;
    // Select safe harmonic limits for every pitch in this block.
    const Band first(highest), second(second_increment(highest, ratio, octave));
    const i32 coefficient = pole(cutoff_position(p, pitch[0]));
    if (voice.cached_filter.g != coefficient || voice.cached_resonance != p[1]) {
        voice.cached_filter = Ladder(coefficient, p[1]);
        voice.cached_resonance = p[1];
    }
    const Ladder filter(voice.cached_filter);
    const i32 drive = 256 + multiply(p[2], 1792); // 1..8, Q8
    const i32 balance = p[5] >> 16;
    const Oscillator osc_a(first, target_wave[0]), osc_b(second, target_wave[1]);
    const bool changing_waves = voice.wave[0] != target_wave[0] || voice.wave[1] != target_wave[1];
    bool constant = true;
    for (unsigned i = 1; i < opxy::block_frames; ++i) {
        if (pitch[i] != pitch[0]) { constant = false; break; }
    }
    // Below quarter-rate cutoff, each integrator is a convex update. With
    // 8192 counts of state headroom, 64 samples of rounding cannot hit a rail.
    const bool limited = filter.needs_limits(voice);
    if (changing_waves) {
        if (constant) dispatch_audio<true, true>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive, limited);
        else dispatch_audio<true, false>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive, limited);
    } else {
        if (constant) dispatch_audio<false, true>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive, limited);
        else dispatch_audio<false, false>(voice, output, pitch, first, second, osc_a, osc_b, filter, target_wave, ratio, octave, balance, drive, limited);
    }
}
}
