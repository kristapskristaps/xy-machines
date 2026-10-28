#include "display.h"
namespace weave {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const char* const codes[15] = {"SHAP", "FM", "NOIS", "DUST", "KICK", "SNAR", "HAT", "GRAN", "CHRD", "SWRM", "WAVE", "ANLG", "MODL", "STRG", "6OP"};
u32 control(i32 word) { return word < 0 ? 0u : static_cast<u32>(word) >> 16; }
}
void display_reset(DisplayState& state) { state = DisplayState(); }
void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    state.notes.update(input);
    const u32 selected = control(input.slots[0]) * 15u >> 15;
    const i32 harmonic = control(input.slots[1]);
    const i32 timbre = control(input.slots[2]);
    const i32 morph = control(input.slots[3]);
    const i32 elapsed = clamp(input.elapsed, 0, 102);
    if (!input.reduced_motion)
        state.phase = (state.phase + static_cast<u32>(elapsed * (2 + (timbre >> 13)))) % 3600u;
    const i32 phase = input.reduced_motion ? 0 : static_cast<i32>(state.phase);
    const i32 held = input.held_notes <= 128 && input.held_notes ? 1 : 0;
    for (u32 strand = 0; strand < 4; ++strand) {
        Frame::path_type& path = frame.paths[strand];
        const i32 shift = static_cast<i32>(strand) * 900;
        const i32 cycles = 2 + harmonic / 8192;
        for (u32 i = 0; i < 64; ++i) {
            const i32 sample = i < 32 ? static_cast<i32>(i) : 63 - static_cast<i32>(i);
            const i32 x = -62 * unit + sample * 124 * unit / 31;
            const i32 angle = sample * 1800 * cycles / 31 + shift + phase;
            const i32 wave = sine(angle);
            const i32 twist = sine(sample * 3600 / 31 - phase + shift);
            const i32 amp = 13 + morph / 2048;
            path.points[i].x = x;
            path.points[i].y = wave * amp + twist * 6 + (i < 32 ? -unit / 3 : unit / 3);
        }
        path.key = 1u + state.phase + strand * 3600u + static_cast<u32>(harmonic / 1024) * 18000u + static_cast<u32>(morph / 256) * 720000u;
        place(frame, strand, 240 * unit, 119 * unit, 1800, 4 + strand, stroke);
    }
    for (u32 i = 0; i < 64; ++i) {
        const i32 x = 42 + static_cast<i32>(i & 31u) * 396 / 31;
        const i32 y = i < 32 ? 57 : 181;
        const i32 distance = static_cast<i32>(i & 31u) - 16;
        const i32 scale = (distance < 0 ? -distance : distance) > 12 ? 1024 : 512;
        place(frame, 4 + i, x * unit, y * unit, scale, (i & 7u) == 0 ? 3 : 1, stroke);
    }
    for (u32 i = 0; i < 4; ++i) {
        const i32 angle = phase + static_cast<i32>(i) * 900;
        const i32 x = 240 + sine(angle) * 104 / unit;
        const i32 y = 119 + sine(angle * 2 + 700) * (12 + morph / 2048) / unit;
        place(frame, 68 + i, x * unit, y * unit, held ? 1200 : 700, held ? 7 : 4, fill_and_stroke);
    }
    for (u32 i = 0; i < 15; ++i) {
        if (i == selected) place(frame, 72 + i, 264 * unit, 198 * unit, unit, 5, fill_and_stroke);
        else hide(frame, 72 + i);
    }
    hide(frame, held ? 87 : 88);
    place(frame, held ? 88 : 87, 405 * unit, 35 * unit, 768, held ? 7 : 3, fill_and_stroke);
    if (input.shown[0] == 0) header(frame, 0, codes[selected]);
}
}
