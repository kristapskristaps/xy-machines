#include "display.h"
#include "dsp.h"

namespace cyclops {
inline void display_reset(DisplayState& state) {
    state.header.reset(); state.initialized = 0; state.track = state.serial = 0; state.impulse = 0;
    for (unsigned i = 0; i < 8; ++i) state.pose[i] = 0;
}
namespace {
inline void number(char* out, opxy::u32 n) {
    char reverse[10]; unsigned count = 0;
    do { reverse[count++] = static_cast<char>('0' + n % 10); n /= 10; } while (n);
    unsigned i = 0;
    while (count) out[i++] = reverse[--count];
    out[i] = 0;
}
inline void frequency(char* out, opxy::u32 hz) {
    if (hz < 1000) { number(out, hz); return; }
    const opxy::u32 tenths = (hz + 50) / 100;
    number(out, tenths / 10);
    unsigned n = 0; while (out[n]) ++n;
    out[n++] = '.'; out[n++] = static_cast<char>('0' + tenths % 10); out[n++] = 'k'; out[n] = 0;
}
inline void attached(Frame& frame, unsigned node, opxy::i32 x, opxy::i32 y,
                     opxy::i32 dx, opxy::i32 dy, opxy::draw::Turn parent,
                     opxy::i32 scale = opxy::draw::unit) {
    using namespace opxy::draw;
    place(frame, node, x + floor_q10(dx * parent.cos - dy * parent.sin),
          y + floor_q10(dx * parent.sin + dy * parent.cos), scale, 0, keep);
    turn(frame, node, parent);
}
}
inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const char* const titles[8] = {"cutoff", "reso", "drive", "waves", "detune", "balance", "octave", "keytrack"};
    const char* const wave_names[4] = {"S S", "S P", "P S", "P P"};
    const char* const octaves[3] = {"-12", "0", "+12"};
    char values[8][header_capacity] = {};
    for (unsigned i = 0; i < 8; ++i) header_amount(values[i], header_capacity, input.slots[i]);
    const opxy::u32 cutoff = static_cast<opxy::u32>(dsp::multiply(dsp::control(input.slots[0]), 19005440));
    frequency(values[0], static_cast<opxy::u32>(dsp::interpolate(tables::cutoff_hz, cutoff, 16)));
    copy(values[3], header_capacity, wave_names[dsp::option(input.slots[3], 4)]);
    const unsigned cents = ((static_cast<unsigned>(dsp::control(input.slots[4])) >> 16) * 50 + 16384) >> 15;
    number(values[4], cents);
    unsigned n = 0; while (values[4][n]) ++n; values[4][n++] = 'c'; values[4][n] = 0;
    copy(values[6], header_capacity, octaves[dsp::option(input.slots[6], 3)]);
    page_header(state.header, input, frame, titles, values, 2);

    const bool baseline = !state.initialized || state.track != input.track;
    const opxy::i32 dt = clamp(input.elapsed, 0, 154);
    const opxy::i32 remaining = static_cast<opxy::i32>(page_header_detail::remaining(dt) >> 6);
    for (unsigned i = 0; i < 8; ++i) {
        const opxy::i32 target = i == 6 ? dsp::option(input.slots[i], 3) * 512
                                      : dsp::control(input.slots[i]) >> 21;
        const opxy::i32 difference = state.pose[i] - target;
        state.pose[i] = baseline || input.reduced_motion || dt >= 154 ? target
                       : target + floor_q10(difference * remaining);
        // Snap the subpixel tail so a full-range turn settles within 0.2 s.
        if (state.pose[i] - target >= -4 && state.pose[i] - target <= 4) state.pose[i] = target;
    }
    if (baseline || input.reduced_motion) state.impulse = 0;
    else if (state.serial != input.note_serial) state.impulse = unit;
    else {
        state.impulse = dt >= 154 ? 0 : floor_q10(state.impulse * remaining);
        if (state.impulse < 8) state.impulse = 0;
    }
    state.initialized = 1; state.track = input.track; state.serial = input.note_serial;
    const opxy::i32 cut = state.pose[0], res = state.pose[1], drive = state.pose[2];
    const opxy::i32 detune = state.pose[4], balance = state.pose[5] - 512;
    const opxy::i32 octave = state.pose[6], keytrack = state.pose[7];
    const opxy::i32 impulse = state.impulse;
    // Eight edges, eight samples per edge: closed contours with 64 distinct
    // points, matching the native retained-path topology.
    const opxy::i32 contours[2][8][2] = {
        {{-3,-21},{9,-8},{19,3},{34,18},{45,29},{32,22},{18,9},{5,-5}},
        {{-66,-10},{-26,-10},{-16,-8},{-22,-6},{-36,-6},{-58,-6},{-68,-7},{-70,-8}}
    };
    for (unsigned path = 0; path < 2; ++path) {
        Frame::path_type& contour = frame.paths[path];
        contour.key = path + 1;
        for (unsigned i = 0; i < Frame::point_count; ++i) {
            const unsigned edge = i / 8, next = (edge + 1) % 8, t = i % 8;
            const opxy::i32 divisor = path ? 16 : 8;
            contour.points[i].x = (contours[path][edge][0] * (8 - static_cast<int>(t)) + contours[path][next][0] * static_cast<int>(t)) * unit / divisor;
            contour.points[i].y = (contours[path][edge][1] * (8 - static_cast<int>(t)) + contours[path][next][1] * static_cast<int>(t)) * unit / divisor;
        }
    }
    const opxy::i32 x = 240 * unit;
    const opxy::i32 y = 115 * unit + impulse * 3 / 2;
    const opxy::i32 angle = balance / 17 + impulse / 64;
    const Turn tilt = rotation(angle);
    place(frame, 0, 299 * unit + impulse, 177 * unit, unit, 0, keep);
    place(frame, 10, 299 * unit + impulse, 177 * unit, unit, 0, keep);
    place(frame, 11, x, 184 * unit, unit, 0, keep);
    for (unsigned node = 12; node < 24; ++node) {
        place(frame, node, x, y, unit, 0, keep);
        turn(frame, node, tilt);
    }
    attached(frame, 1, x, y, cut * 45, 0, tilt);
    frame.nodes[1].shade = impulse > 64 ? 14 : 13;
    frame.nodes[1].paint = stroke;
    place(frame, 19, x, y, unit, 10 + static_cast<unsigned>(drive * 3 / unit), fill_and_stroke);
    turn(frame, 19, tilt);
    place(frame, 20, x, y, unit, 12, fill_and_stroke);
    turn(frame, 20, tilt);

    // All rifle layers share one pivot; the scope retains its lens centre
    // while resonance tightens its focus ring.
    const opxy::i32 gun_x = 306 * unit;
    const opxy::i32 gun_y = 155 * unit - (octave - 512) * 10 + impulse * 2;
    const Turn aim = rotation(140 + detune / 10);
    for (unsigned node = 24; node <= 30; ++node) {
        place(frame, node, gun_x, gun_y, unit, 0, keep);
        turn(frame, node, aim);
    }
    attached(frame, 28, gun_x, gun_y, 0, -19 * unit, aim, 640 + drive * 3 / 4);
    attached(frame, 30, gun_x, gun_y, 12 * unit, -44 * unit, aim, 1536 - res);
    const opxy::i32 mast = 640 + keytrack;
    attached(frame, 31, x, y, 53 * unit, -8 * unit, tilt, mast);
    attached(frame, 32, x, y, 53 * unit, -8 * unit - mast * 15, tilt);
    turn(frame, 32, rotation(angle - keytrack * 6 / 10));

    const unsigned waves = dsp::option(input.slots[3], 4);
    for (unsigned osc = 0; osc < 2; ++osc) {
        const unsigned saw = 33 + osc * 2, pulse = saw + 1;
        const bool square = osc ? (waves & 1) != 0 : waves >= 2;
        const unsigned selected = square ? pulse : saw;
        place(frame, selected, (osc ? 258 : 223) * unit, (osc ? 205 : 201) * unit, unit, 0, keep);
        hide(frame, square ? saw : pulse);
    }
}
}
