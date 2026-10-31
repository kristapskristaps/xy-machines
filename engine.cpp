#include "core.h"
#include "dsp.h"

namespace cyclops {
inline void start(Voice& voice, Shared&, u32) {
    voice.phase[0] = 0;
    voice.phase[1] = 0x20000000u;
    for (unsigned i = 0; i < 4; ++i) voice.ladder[i] = voice.residue[i] = 0;
    for (unsigned i = 0; i < 8; ++i) voice.controls[i] = 0;
    voice.ready = 0;
    voice.wave[0] = voice.wave[1] = 0;
    voice.dc_input = voice.dc_output = 0;
}
inline void initialize(Voice& voice, Shared& shared, u32 owner) { start(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

inline void render(Voice& voice, Shared&, u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
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
    const Ladder filter(pole(cutoff_position(p, pitch[0])), p[1]);
    const i32 drive = 256 + multiply(p[2], 1792); // 1..8, Q8
    const i32 balance = p[5] >> 16;
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        voice.wave[0] += (target_wave[0] - voice.wave[0]) >> 7;
        voice.wave[1] += (target_wave[1] - voice.wave[1]) >> 7;
        for (unsigned j = 0; j < 2; ++j) {
            const i32 left = target_wave[j] - voice.wave[j];
            if (left > -128 && left < 128) voice.wave[j] = target_wave[j];
        }
        const u32 increment = pitch[i] > 0x40000000u ? 0x40000000u : pitch[i];
        const u32 other = second_increment(increment, ratio, octave);
        const i32 a = balance == 32767 ? 0 : oscillator(voice.phase[0], first, voice.wave[0]);
        const i32 b = balance == 0 ? 0 : oscillator(voice.phase[1], second, voice.wave[1]);
        const i32 mixed = a + ((b - a) * balance >> 15);
        const i32 driven = mixed * drive >> 8;
        i32 filtered = filter.step(voice, driven);
        voice.phase[0] += increment;
        voice.phase[1] += other;
        // Mild compensation keeps a resonant bass patch useful.
        filtered += (filtered * (p[1] >> 16) >> 15);
        const i32 limited = saturate(filtered);
        const i32 dc = limited - voice.dc_input + (voice.dc_output * 32721 >> 15); // 10 Hz
        voice.dc_input = limited;
        voice.dc_output = dc;
        output[i] = clamp(dc, -unity, unity) * 32768;
    }
}
}
