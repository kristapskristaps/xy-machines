#include "display.h"

namespace acidline {
namespace {
using opxy::i32;
using opxy::u32;
using namespace opxy::draw;

enum StageNode : u32 {
    inlet = page_header_nodes,
    upper_valve,
    lower_valve,
    saw_glyph,
    triangle_glyph,
    square_glyph,
    reservoir,
    heater,
    droplet,
    outlet,
};

void update_pulse(DisplayState& state, const Input& input, i32 length) {
    const bool same_track = state.initialized && state.track == input.track;
    if (!same_track || input.reduced_motion) {
        state.pulse = 0;
    } else if (state.note_serial != input.note_on_serial) {
        state.pulse = unit;
    } else {
        const bool held = input.held_notes != 0 && input.held_notes <= 128;
        const i32 decay = clamp(input.elapsed, 0, 256) * (10 * unit - 7 * length) / unit;
        state.pulse = clamp(state.pulse - decay, held ? unit / 4 : 0, unit);
    }
    state.track = input.track;
    state.note_serial = input.note_on_serial;
    state.initialized = 1;
}
}

inline void display_reset(DisplayState& state) {
    state.header.reset();
    state.initialized = 0;
    state.pulse = 0;
}

inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    const char* const titles[8] = {"Flow", "Bite", "Burn", "Sweep", "Length", "Shape", "Body", "Shade"};
    const char* const waves[3] = {"Saw", "Tri", "Square"};
    const u32 shape = (static_cast<u32>(input.slots[5]) >> 16) * 3 >> 15;
    char values[8][header_capacity] = {};
    for (u32 slot = 0; slot < 8; ++slot)
        header_amount(values[slot], header_capacity, input.slots[slot]);
    copy(values[5], header_capacity, waves[shape]);
    page_header(state.header, input, frame, titles, values, 0);

    const i32 flow = input.slots[0] >> 21;
    const i32 bite = input.slots[1] >> 21;
    const i32 burn = input.slots[2] >> 21;
    const i32 sweep_amount = input.slots[3] >> 21;
    const i32 length = input.slots[4] >> 21;
    const i32 body = input.slots[6] >> 21;
    const i32 shade_amount = input.slots[7] >> 21;
    update_pulse(state, input, length);

    place(frame, inlet, 76 * unit + 44 * flow, 127 * unit, 3 * unit / 4 + 3 * length / 4, 0, stroke);

    const i32 valve_gap = 18 * unit - 11 * bite;
    place(frame, upper_valve, 170 * unit, 127 * unit - valve_gap, unit, 1, fill_and_stroke);
    place(frame, lower_valve, 170 * unit, 127 * unit + valve_gap, unit, 1, fill_and_stroke);
    for (u32 glyph = saw_glyph; glyph <= square_glyph; ++glyph) {
        if (glyph == saw_glyph + shape)
            place(frame, glyph, 170 * unit, 171 * unit, unit, 0, keep);
        else
            hide(frame, glyph);
    }

    place(frame, reservoir, 243 * unit, 128 * unit, 3 * unit / 4 + body, 2, stroke);
    place(frame, heater, 243 * unit, 128 * unit, unit / 2 + burn, 2, fill_and_stroke);

    const i32 excursion = state.pulse * (8 + 12 * sweep_amount / unit);
    place(frame, droplet, 307 * unit + 42 * sweep_amount, 127 * unit - excursion, unit, 3, fill_and_stroke);
    place(frame, outlet, 394 * unit, 127 * unit, unit, 4 + static_cast<u32>(shade_amount) / 128, fill_and_stroke);
    turn(frame, outlet, rotation(-280 + 560 * shade_amount / unit));
}
}
