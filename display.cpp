#include "display.h"
#include "controls.hpp"
#include "control_tables.hpp"

namespace vader {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 kDuration = 184; // 180 ms, Q10 seconds.

i32 remaining(i32 age) {
    const i32 t = clamp(unit - age * unit / kDuration, 0, unit);
    return ((t * t) >> 10) * t >> 10;
}
i32 ease(i32 age) { return unit - remaining(age); }
i32 follow(i32 value, i32 target, i32 dt) {
    const i32 result = value + ((target - value) * clamp(dt * 45, 0, unit) >> 10);
    return result - target < 4 && target - result < 4 ? target : result;
}
void integer(char* out, u32 value, const char* suffix = "") {
    char digits[10];
    u32 count = 0, at = 0;
    do { digits[count++] = static_cast<char>('0' + value % 10); value /= 10; } while (value && count < 6);
    while (count) out[at++] = digits[--count];
    while (*suffix && at < header_capacity - 1) out[at++] = *suffix++;
    out[at] = 0;
}

void trace(Frame::path_type& path, i32 drive, u32 mode, i32 age, bool pure) {
    const u32 d = static_cast<u32>(drive >> 3);
    // Geometry and its cache key use exactly the same quantized inputs.
    age = (age / 3) * 3;
    const u32 key = 1 + d * 2048 + mode * 512 + static_cast<u32>(age);
    if (path.key == key) return;
    const i32 impact = remaining(age), travel = ease(age);
    const i32 center = -48 + (travel * 96 >> 10);
    for (u32 i = 0; i < 32; ++i) {
        const i32 x = -52 * unit + static_cast<i32>(i) * (104 * unit) / 31;
        const i32 angle = static_cast<i32>(i) * 3600 / 31;
        const i32 fundamental = rotation(angle).sin;
        const i32 partial = rotation(angle * (mode == 3 ? 5 : 3)).sin;
        i32 wave = fundamental;
        if (!pure) {
            wave = (fundamental * 127 + partial * static_cast<i32>(d) / 2) / (127 + static_cast<i32>(d) / 2);
            if (mode == 1 || mode == 2) wave += rotation(angle * 2).cos * static_cast<i32>(d) / 500;
        }
        const i32 distance = x / unit - center;
        const i32 absolute = distance < 0 ? -distance : distance;
        const i32 packet = clamp(24 - absolute, 0, 24) * impact >> 10;
        const i32 y = wave * (18 + packet) + ((rotation(angle * 2 + travel).sin * impact) >> 7);
        path.points[i] = {x, y - 400};
        path.points[63 - i] = {x, y + 400};
    }
    path.key = key;
}

// Keep all traced pieces in one coordinate system, including their recoil.
void mask(Frame& frame, u32 node, i32 x, i32 y, i32 scale, i32 lift, Turn turn,
          u32 shade = 0, Paint paint = keep) {
    x = x * scale >> 10;
    y = y * scale >> 10;
    place(frame, node, 240 * unit + ((x * turn.cos - y * turn.sin) >> 10),
          (123 * unit - lift) + ((x * turn.sin + y * turn.cos) >> 10), scale, shade, paint);
    opxy::draw::turn(frame, node, turn);
}
}

inline void display_reset(DisplayState& state) {
    state.header.reset();
    state.notes.reset();
    state.initialized = state.track = state.serial = state.next = 0;
    state.edit_age = kDuration;
    state.gate = 0;
    for (u32 i = 0; i < 4; ++i) state.ages[i] = kDuration;
    for (u32 i = 0; i < 8; ++i) state.previous[i] = state.visual[i] = 0;
}

inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const bool snapshot = !state.initialized || state.track != input.track;
    const bool held = input.held_notes > 0 && input.held_notes <= 128;
    const i32 elapsed = clamp(input.elapsed, 0, kDuration);
    const NoteEvents events = state.notes.update(input);
    if (snapshot || input.reduced_motion) {
        state.next = 0;
        state.edit_age = kDuration;
        state.gate = held ? unit : 0;
        for (u32 i = 0; i < 4; ++i) state.ages[i] = kDuration;
        for (u32 i = 0; i < 8; ++i) state.visual[i] = input.slots[i] >> 21;
    } else {
        for (u32 i = 0; i < 4; ++i) state.ages[i] = clamp(state.ages[i] + elapsed, 0, kDuration);
        state.edit_age = clamp(state.edit_age + elapsed, 0, kDuration);
        state.gate = follow(state.gate, held ? unit : 0, elapsed);
        if (input.note_serial != state.serial) {
            state.ages[state.next] = 0;
            state.next = (state.next + 1) & 3;
        }
        // Release closes the visor smoothly; it never creates another hit.
        if (events.hold_ended) state.gate = follow(state.gate, 0, elapsed);
        for (u32 i = 0; i < 8; ++i) {
            if (input.slots[i] != state.previous[i]) state.edit_age = 0;
            state.visual[i] = follow(state.visual[i], input.slots[i] >> 21, elapsed);
        }
    }
    state.initialized = 1;
    state.track = input.track;
    state.serial = input.note_serial;
    for (u32 i = 0; i < 8; ++i) state.previous[i] = input.slots[i];
    const i32 age = state.ages[(state.next + 3) & 3];
    const i32 impact = remaining(age), edited = remaining(state.edit_age);
    const u32 mode = shape_index(input.slots[3]);
    const i32 drive = state.visual[2];
    const char* const titles[8] = {"punch", "fall", "drive", "shape", "click", "sub", "color", "phase"};
    const char* const modes[4] = {"round", "tube", "diode", "fold"};
    char values[8][header_capacity] = {};
    for (u32 i = 0; i < 8; ++i) header_amount(values[i], header_capacity, input.slots[i]);
    integer(values[0], (static_cast<u32>(input.slots[0] >> 16) * 48 + 16384) >> 15, "st");
    integer(values[1], (control_table(fall_milliseconds, input.slots[1]) + 500) / 1000, "ms");
    copy(values[3], header_capacity, modes[mode]);
    integer(values[7], (((static_cast<u32>(input.slots[7]) >> 16) * 8) >> 15) * 45);
    page_header(state.header, input, frame, titles, values, 2);

    // Initialize every retained node, including particles hidden this frame.
    for (u32 node = 10; node < Frame::node_count; ++node) hide(frame, node);
    trace(frame.paths[0], 0, 0, age, true);
    trace(frame.paths[1], drive, mode, age, false);
    place(frame, 0, 87 * unit, 120 * unit, unit, impact > 200 ? 10 : 6, stroke);
    place(frame, 1, 393 * unit, 120 * unit, unit, impact > 200 ? 14 : 12, stroke);

    // Four independent impacts survive rapid retriggers. Each is three broken
    // ellipses that counter-rotate and decelerate outward behind the mask.
    for (u32 slot = 0; slot < 4; ++slot) {
        const i32 pulse_age = state.ages[slot];
        if (pulse_age >= kDuration || input.reduced_motion) continue;
        const i32 progress = ease(pulse_age);
        for (u32 layer = 0; layer < 3; ++layer) {
            const u32 node = layer ? 60 + slot * 2 + layer - 1 : 25 + slot;
            const i32 scale = 950 + (progress * static_cast<i32>(110 + layer * 80) >> 10);
            const i32 shade = clamp(10 - pulse_age / 23 - static_cast<i32>(layer) * 2, 1, 10);
            place(frame, node, 240 * unit, 123 * unit, scale, static_cast<u32>(shade), stroke);
            turn(frame, node, rotation((layer & 1 ? -1 : 1) * (progress * 180 >> 10) + static_cast<i32>(layer) * 250));
        }
    }

    const i32 mask_scale = unit + (impact * (68 + state.visual[0] / 32) >> 10);
    const i32 lift = impact * 4;
    const Turn pose = rotation((state.serial & 1 ? 1 : -1) * (impact * 12 >> 10));
    for (u32 node = 46; node < 58; ++node) mask(frame, node, 0, 0, mask_scale, lift, pose);
    mask(frame, 12, 0, 0, mask_scale, lift, pose);
    for (u32 node = 13; node < 18; ++node) mask(frame, node, 0, 0, mask_scale, lift, pose);
    const u32 eye_shade = impact > 320 ? 15 : state.gate > 400 ? 14 : 7;
    for (u32 side = 0; side < 2; ++side) {
        mask(frame, 10 + side, 0, 0, mask_scale, lift, pose, eye_shade, fill_and_stroke);
        if (age > 8 && age < 125 && !input.reduced_motion) {
            const i32 scan = clamp((age - 8) * unit / 117, 0, unit);
            const i32 x = (side ? 1 : -1) * (24 * unit - scan * 9);
            mask(frame, 58 + side, x, 6 * unit, mask_scale, lift, pose, 15, fill_and_stroke);
        }
    }
    // Red ribs charge from the mouth's centre toward its edge.
    for (u32 rib = 0; rib < 6; ++rib) {
        const i32 distance = rib < 3 ? 2 - static_cast<i32>(rib) : static_cast<i32>(rib) - 3;
        const i32 brightness = clamp((drive * 3 / 4 + impact - distance * 130) >> 7, 0, 4);
        if (state.gate || impact || drive > 64)
            mask(frame, 18 + rib, (-14 * unit + static_cast<i32>(rib) * 5734), 53 * unit,
                 mask_scale, lift, pose, static_cast<u32>(11 + brightness), fill_and_stroke);
    }
    if (impact) mask(frame, 24, 0, 57 * unit, mask_scale, lift, pose, impact > 300 ? 14 : 12, fill_and_stroke);

    // Elliptical spark trajectories stay below the header and above the logo.
    if (age < 145 && !input.reduced_motion) {
        const i32 progress = ease(age);
        for (u32 spark = 0; spark < 12; ++spark) {
            const i32 angle = static_cast<i32>(spark) * 300 + (state.serial & 1 ? 70 : -70);
            const Turn direction = rotation(angle);
            const i32 x = (80 + (progress * 26 >> 10)) * direction.cos;
            const i32 y = (47 + (progress * 13 >> 10)) * direction.sin;
            place(frame, 68 + spark, 240 * unit + x, 123 * unit + y,
                  unit - (progress >> 1), spark % 3 ? 12 + static_cast<u32>(impact > 300) : 9, fill_and_stroke);
            turn(frame, 68 + spark, direction);
        }
    }
    if (impact && !input.reduced_motion) {
        const i32 progress = ease(age);
        for (u32 packet = 0; packet < 6; ++packet) {
            const i32 distance = clamp((progress * 45 >> 10) - static_cast<i32>(packet % 3) * 8, 0, 34);
            const i32 side = packet < 3 ? -1 : 1;
            place(frame, 80 + packet, (240 + side * (100 - distance)) * unit,
                  (119 + static_cast<i32>(packet % 3) * 3) * unit, unit,
                  packet < 3 ? 8 : 14, fill_and_stroke);
        }
    }
    for (u32 corner = 0; corner < 4; ++corner) {
        const i32 inset = edited / 170;
        place(frame, 29 + corner, (corner & 1 ? 454 - inset : 26 + inset) * unit,
              (corner & 2 ? 180 - inset : 65 + inset) * unit, unit, edited ? 8 : 3, stroke);
        turn(frame, 29 + corner, rotation(static_cast<i32>(corner) * 900));
    }
    for (u32 bar = 0; bar < 8; ++bar) {
        const i32 level = drive * static_cast<i32>(8 - bar) / 14 + impact * static_cast<i32>(8 - bar) / 12;
        place(frame, 33 + bar, (354 + static_cast<i32>(bar) * 11) * unit, 170 * unit,
              unit + clamp(level, 0, unit), bar < static_cast<u32>(drive / 128 + 1) ? 13 : 3, fill_and_stroke);
    }
    const u32 slots[4] = {0, 1, 2, 5};
    for (u32 dial = 0; dial < 4; ++dial) {
        place(frame, 41 + dial, (40 + static_cast<i32>(dial) * 13) * unit, 169 * unit, unit, 7, stroke);
        turn(frame, 41 + dial, rotation(-1350 + state.visual[slots[dial]] * 2700 / 1023));
    }
    place(frame, 45, 240 * unit, 199 * unit, unit, state.gate ? 14 : 3, fill_and_stroke);
}
}
