#include "display.h"
#include "dsp_common.h"
#include "display_tables.h"

namespace westfold {
namespace {
using opxy::i32;
using opxy::u32;
using namespace opxy::draw;

void number(char* out, u32 value, const char* suffix) {
    char reverse[10];
    u32 count = 0, at = 0;
    do { reverse[count++] = static_cast<char>('0' + value % 10); value /= 10; } while (value && count < 9);
    while (count) out[at++] = reverse[--count];
    for (u32 i = 0; suffix[i] && at < 7; ++i) out[at++] = suffix[i];
    out[at] = 0;
}
void decimal(char* out, u32 tenths, const char* suffix) {
    number(out, tenths / 10, "");
    u32 at = 0;
    while (out[at]) ++at;
    if (at < 5) { out[at++] = '.'; out[at++] = static_cast<char>('0' + tenths % 10); }
    for (u32 i = 0; suffix[i] && at < 7; ++i) out[at++] = suffix[i];
    out[at] = 0;
}
void headers(const Input& input, Frame& frame) {
    for (u32 i = 0; i < 4; ++i) {
        const u32 slot = input.shown[i] < 8 ? input.shown[i] : i;
        const i32 value = dsp::control(input.parameters[i]);
        if (slot == 3) {
            if (value >= 32600) copy(frame.header[i], 8, "hold");
            else {
                const u32 ms = static_cast<u32>(dsp::lookup(draw_tables::durations_ms, value));
                if (ms < 1000) number(frame.header[i], ms, "ms");
                else decimal(frame.header[i], (ms + 50) / 100, "s");
            }
        } else if (slot == 4) {
            decimal(frame.header[i], static_cast<u32>((dsp::lookup(draw_tables::ratios, value) * 10 + 2048) / 4096), "");
        } else if (slot == 6) {
            const u32 rate = static_cast<u32>(dsp::lookup(draw_tables::rates, value));
            const u32 tenths = (rate / 1024) * 4410 / 41943;
            if (tenths < 1000) decimal(frame.header[i], tenths, "Hz");
            else number(frame.header[i], (tenths + 5) / 10, "Hz");
        } else number(frame.header[i], static_cast<u32>((value * 100 + 16384) / 32768), "");
    }
}

i32 ribbon_wave(u32 fold,i32 fraction,u32 phase) {
    return dsp::mix(dsp::read(draw_tables::waves[fold],phase),
                    dsp::read(draw_tables::waves[fold+1],phase),fraction);
}

void ribbon(Frame::path_type& path, u32 key, i32 fold, i32 overtone, i32 balance,
            i32 amount, u32 phase, bool clean_wave) {
    if (path.key == key) return;
    const u32 index = static_cast<u32>(fold) >> 11;
    const i32 fraction = (fold & 2047) * 16;
    for (u32 i = 0; i < 32; ++i) {
        const u32 p = i * (0xffffffffu / 31u);
        const u32 mod = static_cast<u32>(dsp::read(draw_tables::sine, p * 2u + phase) * amount / 2);
        const i32 fundamental = dsp::read(draw_tables::waves[0], p + mod);
        const i32 odd = ribbon_wave(index, fraction, p + mod);
        const i32 even = ribbon_wave(index, fraction, (p + mod) * 2u);
        const i32 y = clean_wave ? fundamental : dsp::mix(fundamental, dsp::mix(odd, even, overtone), balance);
        const i32 x = -58 * unit + static_cast<i32>(i) * 116 * unit / 31;
        const i32 height = y * 20 / 32768;
        path.points[i] = {x, -height * unit - 180};
        path.points[63 - i] = {x, -height * unit + 180};
    }
    path.key = key;
}
}

inline void display_reset(DisplayState& state) {
    state.notes.reset();
    state.track = state.serial = state.initialized = state.phase = state.gate = state.geometry = 0;
    state.age = 60000;
}

inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const i32 dt = clamp(input.elapsed, 0, 102);
    const bool baseline = !state.initialized || input.track != state.track;
    state.notes.update(input);
    const bool onset = !baseline && input.note_serial != state.serial;
    if (baseline) { state.age = 60000; state.gate = 0; state.phase = 0; }
    if (onset) state.age = 0;
    if (state.age < 60000) state.age += static_cast<u32>(dt);
    state.serial = input.note_serial;
    state.track = input.track;
    state.initialized = 1;
    const i32 overtone = dsp::control(input.slots[0]);
    const i32 fold = dsp::control(input.slots[1]);
    const i32 balance = dsp::control(input.slots[2]);
    const i32 decay = dsp::control(input.slots[3]);
    const i32 fm = dsp::control(input.slots[5]);
    const i32 motion = dsp::control(input.slots[7]);
    if (!input.reduced_motion) {
        const u32 rate = static_cast<u32>(dsp::lookup(draw_tables::rates, dsp::control(input.slots[6])));
        state.phase += rate * static_cast<u32>(dt * 44100 / 1024);
    }
    const u32 duration = static_cast<u32>(dsp::lookup(draw_tables::durations_ms, decay));
    const u32 age_index = state.age * 125u / (duration ? duration : 1);
    state.gate = decay >= 32600 && input.held_notes && input.held_notes <= 128 ? 1024
        : static_cast<u32>(draw_tables::visual_decay[age_index > 255 ? 255 : age_index]);
    if (input.reduced_motion) state.gate = input.held_notes && input.held_notes <= 128 ? 1024 : 0;
    const i32 cycle = input.reduced_motion ? 0 : dsp::slope(state.phase) * 2 - 32768;
    const i32 animated_fold = dsp::clamp(fold * (24576 + static_cast<i32>(state.gate) * 8) / 32768
                                       + cycle * motion / 65536, 0, 32767);
    const u32 key = ++state.geometry ? state.geometry : ++state.geometry;
    ribbon(frame.paths[0], key, animated_fold, overtone, balance, fm * fm / 32768,
           input.reduced_motion ? 0 : state.phase, false);
    ribbon(frame.paths[1], key, 0, 0, 0, 0, 0, true);
    ribbon(frame.paths[2], key, animated_fold, overtone, balance, fm * fm / 32768,
           input.reduced_motion ? 0 : state.phase, false);
    place(frame, 0, 240 * unit, 109 * unit, 2 * unit, 3, stroke);
    place(frame, 1, 240 * unit, 109 * unit, 2 * unit, 7, stroke);
    place(frame, 2, 240 * unit, 109 * unit, 2 * unit, state.gate > 500 ? 12 : 10, stroke);

    Frame::path_type& contour = frame.paths[3];
    for (u32 i = 0; i < 32; ++i) {
        const i32 x = -34 * unit + static_cast<i32>(i) * 68 * unit / 31;
        const i32 y = i < 3 ? static_cast<i32>(i) * 11 / 2
            : draw_tables::visual_decay[(i - 3) * 8] * 11 / 1024;
        contour.points[i] = {x, -y * unit - 100};
        contour.points[63 - i] = {x, -y * unit + 100};
    }
    contour.key = key;
    place(frame, 3, 413 * unit, 189 * unit, unit, 9, stroke);

    const u32 fi = static_cast<u32>(animated_fold) >> 11;
    const i32 ff = (animated_fold & 2047) * 16;
    for (u32 harmonic = 1; harmonic <= 32; ++harmonic) {
        const i32 odd = harmonic & 1 ? dsp::mix(draw_tables::spectrum[fi][harmonic], draw_tables::spectrum[fi + 1][harmonic], ff) : 0;
        const u32 doubled = harmonic / 2;
        const i32 even = !(harmonic & 1) ? dsp::mix(draw_tables::spectrum[fi][doubled], draw_tables::spectrum[fi + 1][doubled], ff) : 0;
        i32 level = dsp::mix(odd, even, overtone);
        level = dsp::mix(harmonic & 1 ? draw_tables::spectrum[0][harmonic] : 0, level, balance);
        const i32 height = 80 + dsp::lookup(draw_tables::visual_spectrum, level);
        place(frame, 3 + harmonic, static_cast<i32>(139 + harmonic * 6) * unit,
              193 * unit, height, harmonic & 1 ? 10 : 7, stroke);
    }
    place(frame, 36, 63 * unit, 109 * unit, unit + static_cast<i32>(state.gate) / 8, 0, keep);
    for (u32 i = 0; i < 12; ++i) {
        place(frame, 37 + i, 413 * unit, 109 * unit,
              512 + static_cast<i32>(state.gate) / 2, i % 2 ? 8 : 10, stroke);
        turn(frame, 37 + i, rotation(static_cast<i32>(i) * 300));
    }
    for (u32 i = 0; i < 8; ++i) {
        if (!state.gate || input.reduced_motion) { hide(frame, 49 + i); continue; }
        const u32 travel = ((state.phase >> 23) + i * 128) & 511u;
        const i32 x = i < 4 ? 84 + static_cast<i32>(travel) * 34 / 511
                             : 362 + static_cast<i32>(travel) * 26 / 511;
        place(frame, 49 + i, x * unit, 109 * unit, 700, i < 4 ? 7 : 10, fill_and_stroke);
    }
    place(frame, 57, 413 * unit, 109 * unit, 400 + static_cast<i32>(state.gate) / 2,
          state.gate > 400 ? 12 : 4, fill_and_stroke);
    headers(input, frame);
    place(frame, 58, (383 + overtone * 62 / 32768) * unit, 54 * unit, unit, 10, fill_and_stroke);
    place(frame, 59, 23 * unit, 187 * unit, unit, motion > 1000 ? 10 : 4, fill_and_stroke);
    place(frame, 60, 23 * unit, 200 * unit, unit, fm > 1000 ? 7 : 4, fill_and_stroke);
}
}
