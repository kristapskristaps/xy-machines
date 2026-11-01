#include "display.h"
#include "dsp.h"

namespace cyclops {
inline void display_reset(DisplayState& state) {
    state.header.reset(); state.initialized = 0; state.track = state.serial = 0; state.impulse = 0;
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
    if (baseline || input.reduced_motion) state.impulse = 0;
    else if (state.serial != input.note_serial) state.impulse = unit;
    else {
        const opxy::i32 dt = clamp(input.elapsed, 0, 154);
        state.impulse = dt >= 154 ? 0 : floor_q10(state.impulse * static_cast<opxy::i32>(page_header_detail::remaining(dt) >> 6));
        if (state.impulse < 8) state.impulse = 0;
    }
    state.initialized = 1; state.track = input.track; state.serial = input.note_serial;
    const opxy::i32 cut = dsp::control(input.slots[0]) >> 21;
    const opxy::i32 res = dsp::control(input.slots[1]) >> 21;
    const opxy::i32 drive = dsp::control(input.slots[2]) >> 21;
    const opxy::i32 balance = (dsp::control(input.slots[5]) >> 21) - 512;
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
    const Turn tilt = rotation(balance / 42 + impulse / 64);
    place(frame, 0, 299 * unit + impulse, 177 * unit, unit, 0, keep);
    place(frame, 10, 299 * unit + impulse, 177 * unit, unit, 0, keep);
    place(frame, 11, x, 184 * unit, unit, 0, keep);
    for (unsigned node = 12; node < 24; ++node) {
        place(frame, node, x, y, unit, 0, keep);
        turn(frame, node, tilt);
    }
    place(frame, 1, x + cut * 21, y, unit, 3 + static_cast<unsigned>(impulse * 7 / unit), stroke);
    turn(frame, 1, tilt);
    place(frame, 19, x, y, unit, 10 + static_cast<unsigned>(drive * 3 / unit), fill_and_stroke);
    turn(frame, 19, tilt);
    place(frame, 20, x, y, unit, 12 + static_cast<unsigned>((unit - res) * 2 / unit), fill_and_stroke);
    turn(frame, 20, tilt);
}
}
