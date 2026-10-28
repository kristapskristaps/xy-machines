#include "core.h"
#include "tables.hpp"
#include "controls.hpp"
#include "control_tables.hpp"
#include <opxy/q31.hpp>

namespace vader {
namespace {
using opxy::i32;
using opxy::u32;

i32 lookup(const opxy::i16* table, u32 phase) {
    const u32 at = phase >> 23;
    const i32 fraction = static_cast<i32>((phase >> 8) & 32767u);
    return table[at] + ((table[at + 1] - table[at]) * fraction >> 15);
}

// Remove the richer mip before its highest harmonic reaches 0.44 * Fs.
void mip(u32 increment, u32& band, i32& mix) {
    const u32 harmonics[6] = {1, 3, 7, 15, 31, 63};
    band = 5;
    while (band && increment >= 1889785610u / harmonics[band]) --band;
    mix = 32768;
    if (band) {
        const u32 end = 1889785610u / harmonics[band], begin = end / 2;
        if (increment > begin)
            mix = static_cast<i32>((end - increment) / ((end - begin) >> 15));
        if (mix > 32768) mix = 32768;
    }
}

i32 shaped(u32 mode, u32 drive, u32 band, i32 band_mix, u32 phase) {
    const u32 level = drive >> 15;
    const i32 mix = static_cast<i32>(drive & 32767u);
    const u32 next = level < 8 ? level + 1 : 8;
    const i32 a = lookup(waves[mode][level][band], phase);
    const i32 b = lookup(waves[mode][next][band], phase);
    const i32 rich = a + ((b - a) * mix >> 15);
    if (!band || band_mix == 32768) return rich;
    const i32 c = lookup(waves[mode][level][band - 1], phase);
    const i32 d = lookup(waves[mode][next][band - 1], phase);
    const i32 lean = c + ((d - c) * mix >> 15);
    return lean + ((rich - lean) * band_mix >> 15);
}

u32 swept(u32 pitch, i32 sweep, u32 ratio) {
    const u32 excess = (((ratio - 65536u) >> 5) * static_cast<u32>(sweep >> 16)) >> 10;
    const u32 factor = 65536u + excess;
    const i32 bounded = static_cast<i32>(pitch > 0x70000000u ? 0x70000000u : pitch);
    if (!excess) return static_cast<u32>(bounded);
    const i32 scaled = opxy::q31::multiply(bounded, static_cast<i32>(factor * 1024u));
    return scaled > 0x03800000 ? 0x70000000u : static_cast<u32>(scaled) * 32u;
}
}

inline void initialize(Voice& voice, Shared&, u32 owner) {
    voice.phase = 0;
    voice.pending = 1;
    voice.sweep = opxy::q31::maximum;
    voice.fade = voice.body = voice.body2 = voice.drive = voice.sub = voice.color = voice.boost = 0;
    voice.noise = owner * 2654435761u | 1u;
    voice.click = voice.click_low = 0;
}
inline void start(Voice& voice, Shared& shared, u32 owner) { initialize(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

// Stock AMP, glide, filter and velocity remain in the OP-XY voice path.
inline void render(Voice& voice, Shared&, u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
    const u32 mode = shape_index(parameters[3]);
    const u32 ratio = control_table(pitch_ratios, parameters[0]);
    const i32 decay = static_cast<i32>(control_table(fall_coefficients, parameters[1]));
    const i32 target_drive = (parameters[2] >> 16) * 8;
    const i32 target_sub = parameters[5] >> 16;
    const i32 target_color = static_cast<i32>(control_table(tone_coefficients, parameters[6]));
    // Boost spans 0..6 dB, using fixed headroom rather than a clipping stage.
    const i32 target_boost = static_cast<i32>(control_table(boost_gains, parameters[7]));
    if (voice.pending) {
        voice.pending = 0;
        voice.phase = 0;
        voice.drive = target_drive;
        voice.sub = target_sub;
        voice.color = target_color;
        voice.boost = target_boost;
        voice.click = parameters[4];
    }
    u32 maximum = 0;
    for (u32 i = 0; i < opxy::block_frames; ++i)
        if (pitch[i] > maximum) maximum = pitch[i];
    u32 band;
    i32 band_mix;
    mip(swept(maximum, voice.sweep, ratio), band, band_mix);
    const i32 drive_step = (target_drive - voice.drive) / 64;
    const i32 sub_step = (target_sub - voice.sub) / 64;
    const i32 color_step = (target_color - voice.color) / 64;
    const i32 boost_step = (target_boost - voice.boost) / 64;
    for (u32 i = 0; i < opxy::block_frames; ++i) {
        voice.drive += drive_step;
        voice.sub += sub_step;
        voice.color += color_step;
        voice.boost += boost_step;
        const i32 pure = lookup(waves[0][0][0], voice.phase);
        const i32 body = shaped(mode, static_cast<u32>(voice.drive), band, band_mix, voice.phase);
        // Q23 filter state resolves the bottom of the 20 Hz..open tone range.
        // The clean foundation bypasses both poles, preserving the sub at dark settings.
        voice.body += opxy::q31::multiply(body * 256 - voice.body, voice.color);
        voice.body2 += opxy::q31::multiply(voice.body - voice.body2, voice.color);
        const i32 filtered = voice.body2 >> 8;
        i32 sample = filtered + ((pure - filtered) * voice.sub >> 15);
        // High-passed noise impulse, with a fixed 2 ms decay.
        voice.noise = voice.noise * 1664525u + 1013904223u;
        const i32 noise = static_cast<i32>(voice.noise >> 17) - 16384;
        voice.click_low += (noise - voice.click_low) / 4;
        // Convex click/body mixing stays bounded without hard-clipping the bass.
        const i32 click_mix = voice.click >> 18;
        sample = (sample * (32768 - click_mix) + (noise - voice.click_low) * click_mix) >> 15;
        voice.click = opxy::q31::multiply(voice.click, 2123271099);
        if (voice.fade < 32768) {
            sample = sample * voice.fade >> 15;
            voice.fade += 768;
            if (voice.fade > 32768) voice.fade = 32768;
        }
        if (sample > 32767) sample = 32767;
        if (sample < -32767) sample = -32767;
        sample = sample * voice.boost >> 15;
        output[i] = sample * 32768;
        voice.phase += swept(pitch[i], voice.sweep, ratio);
        voice.sweep = opxy::q31::multiply(voice.sweep, decay);
    }
    voice.drive = target_drive;
    voice.sub = target_sub;
    voice.color = target_color;
    voice.boost = target_boost;
}
}
