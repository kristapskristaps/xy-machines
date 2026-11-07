#include "display.h"

namespace acidline {
namespace {
using opxy::i32;
using opxy::u32;
using namespace opxy::draw;

constexpr u32 folds_per_wave = 4;

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
    const char* const waves[3] = {"saw", "tri", "sqr"};
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

    // Each waveform repeats at four scales. Keep the other two sets parked off screen.
    const u32 first = page_header_nodes + shape * folds_per_wave;
    for (u32 node = page_header_nodes; node < Frame::node_count; ++node) {
        if (node < first || node >= first + folds_per_wave)
            hide(frame, node);
    }

    place(frame, first, 170 * unit + 36 * flow, 139 * unit, 1152 + length / 4, 0, keep);
    place(frame, first + 1, 247 * unit - 16 * bite, 137 * unit - 18 * bite, 896, 0, keep);
    place(frame, first + 2, 278 * unit, 131 * unit, 576 + body / 4, 0, keep);
    turn(frame, first + 2, rotation(-90 + 270 * burn / unit));

    const i32 excursion = state.pulse * (9 + 12 * sweep_amount / unit);
    place(frame, first + 3, 314 * unit + 40 * sweep_amount, 127 * unit - excursion,
          576 - shade_amount / 4, 0, keep);
}
}
