#include "display.h"
#include "controls.hpp"
#include "control_tables.hpp"

namespace vader {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;

void integer(char* out, u32 value, const char* suffix = "") {
    char digits[10];
    u32 count = 0, at = 0;
    do { digits[count++] = static_cast<char>('0' + value % 10); value /= 10; } while (value && count < 6);
    while (count) out[at++] = digits[--count];
    while (*suffix && at < header_capacity - 1) out[at++] = *suffix++;
    out[at] = 0;
}

void trace(Frame::path_type& path, i32 drive, u32 mode, i32 flash, bool pure) {
    const u32 d = static_cast<u32>(drive) >> 24;
    const u32 key = 1 + d * 4096 + mode * 256 + static_cast<u32>(flash >> 5);
    if (path.key == key) return;
    const i32 amplitude = 18 + flash / 180;
    for (u32 i = 0; i < 32; ++i) {
        const i32 angle = static_cast<i32>(i) * 3600 / 31;
        const i32 fundamental = rotation(angle).sin;
        const i32 partial = rotation(angle * (mode == 3 ? 5 : 3)).sin;
        i32 wave = fundamental;
        if (!pure) {
            wave = (fundamental * 127 + partial * static_cast<i32>(d) / 2) / (127 + static_cast<i32>(d) / 2);
            if (mode == 1 || mode == 2) wave += rotation(angle * 2).cos * static_cast<i32>(d) / 500;
        }
        const i32 x = -52 * unit + static_cast<i32>(i) * (104 * unit) / 31;
        const i32 y = wave * amplitude;
        path.points[i] = {x, y - 400};
        path.points[63 - i] = {x, y + 400};
    }
    path.key = key;
}
}

inline void display_reset(DisplayState& state) {
    state.header.reset();
    state.initialized = state.track = state.serial = 0;
    state.impact = state.edit = 0;
    for (u32 i = 0; i < 8; ++i) state.previous[i] = 0;
}

inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const bool snapshot = !state.initialized || state.track != input.track;
    const bool held = input.held_notes > 0 && input.held_notes <= 128;
    const i32 elapsed = clamp(input.elapsed, 0, 205);
    if (snapshot || input.reduced_motion) state.impact = state.edit = 0;
    else {
        // 20 ms half life, fully settled within 0.18 s; idle never runs a clock.
        const i32 loss = clamp(elapsed * 35, 0, unit);
        state.impact -= state.impact * loss >> 10;
        state.edit -= state.edit * loss >> 10;
        if (state.impact < 4) state.impact = 0;
        if (state.edit < 4) state.edit = 0;
        if (input.note_serial != state.serial) state.impact = unit;
        for (u32 i = 0; i < 8; ++i)
            if (input.slots[i] != state.previous[i]) state.edit = unit;
    }
    state.initialized = 1;
    state.track = input.track;
    state.serial = input.note_serial;
    for (u32 i = 0; i < 8; ++i) state.previous[i] = input.slots[i];
    const u32 mode = shape_index(input.slots[3]);
    const char* const titles[8] = {"punch", "fall", "drive", "shape", "click", "sub", "color", "phase"};
    const char* const modes[4] = {"round", "tube", "diode", "fold"};
    char values[8][header_capacity] = {};
    for (u32 i = 0; i < 8; ++i) header_amount(values[i], header_capacity, input.slots[i]);
    integer(values[0], (static_cast<u32>(input.slots[0] >> 16) * 48 + 16384) >> 15, "st");
    integer(values[1], (control_table(fall_milliseconds, input.slots[1]) + 500) / 1000, "ms");
    copy(values[3], header_capacity, modes[mode]);
    integer(values[7], (((static_cast<u32>(input.slots[7]) >> 16) * 8) >> 15) * 45);
    page_header(state.header, input, frame, titles, values, 2);

    trace(frame.paths[0], 0, 0, state.impact, true);
    trace(frame.paths[1], input.slots[2], mode, state.impact, false);
    place(frame, 0, 87 * unit, 120 * unit, unit, held ? 10 : 6, stroke);
    place(frame, 1, 393 * unit, 120 * unit, unit, held ? 15 : 11, stroke);

    const i32 flare = state.impact / 12;
    for (u32 side = 0; side < 2; ++side)
        place(frame, 10 + side, (240 + (side ? 1 : -1) * 25) * unit, (103 - flare / 32) * unit,
              unit + flare / 2, held ? 15 : 12, fill_and_stroke);
    place(frame, 12, 240 * unit, 123 * unit, unit, held ? 8 : 5, stroke);
    const i32 drive = input.slots[2] >> 24;
    const u32 active = static_cast<u32>((drive * 6 + 126) / 127);
    for (u32 side = 0; side < 2; ++side) {
        for (u32 i = 0; i < 6; ++i) {
            const u32 node = 13 + side * 6 + i;
            place(frame, node, (240 + (side ? 1 : -1) * (10 + static_cast<i32>(i) * 6)) * unit,
                  (147 + static_cast<i32>(i) * 2) * unit, unit + state.edit / 16,
                  i < active ? 13 : 3, fill_and_stroke);
            turn(frame, node, rotation(side ? -160 : 160));
        }
    }
    for (u32 ring = 0; ring < 4; ++ring) {
        const i32 progress = unit - state.impact;
        if (!state.impact || input.reduced_motion) { hide(frame, 25 + ring); continue; }
        place(frame, 25 + ring, 240 * unit, 123 * unit,
              unit + (progress * static_cast<i32>(ring + 1) >> 3),
              static_cast<u32>(clamp(10 - static_cast<i32>(ring) * 2 - progress / 140, 1, 10)), stroke);
    }
    for (u32 corner = 0; corner < 4; ++corner) {
        place(frame, 29 + corner, (corner & 1 ? 454 : 26) * unit, (corner & 2 ? 180 : 65) * unit,
              unit, state.edit ? 8 : 3, keep);
        turn(frame, 29 + corner, rotation(static_cast<i32>(corner) * 900));
    }
    for (u32 bar = 0; bar < 8; ++bar) {
        const i32 height = 2 + (drive * static_cast<i32>(8 - bar) / 100) + state.impact / (100 + static_cast<i32>(bar) * 50);
        place(frame, 33 + bar, (354 + static_cast<i32>(bar) * 11) * unit, 167 * unit,
              unit + clamp(height * 45, 0, unit), bar < static_cast<u32>(drive / 16 + 1) ? 12 : 3, fill_and_stroke);
    }
    const u32 slots[4] = {0, 1, 2, 5};
    for (u32 dial = 0; dial < 4; ++dial) {
        place(frame, 41 + dial, (40 + static_cast<i32>(dial) * 13) * unit, 169 * unit, unit, 7, stroke);
        turn(frame, 41 + dial, sweep(input.slots[slots[dial]], 1350));
    }
    place(frame, 45, 240 * unit, 185 * unit, unit, held ? 15 : 3, fill_and_stroke);
}
}
