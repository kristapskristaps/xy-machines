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
    const opxy::i32 gaze = (dsp::control(input.slots[5]) >> 21) - 512;
    // A note wakes the iris and lifts the lids; repeated notes never black out
    // the eye. Controls establish the resting pose, with no idle animation.
    const opxy::i32 impulse = state.impulse;
    const opxy::i32 radius_y = 22 * unit + cut * 8 - res * 3 + impulse;
    for (unsigned path = 0; path < 2; ++path) {
        Frame::path_type& eye = frame.paths[path];
        eye.key = static_cast<opxy::u32>(radius_y * 2 + path + 1);
        for (unsigned i = 0; i < Frame::point_count; ++i) {
            const opxy::i32 angle = static_cast<opxy::i32>(i * 3600 / Frame::point_count);
            eye.points[i].x = (path ? 57 : 61) * sine(angle + 900);
            eye.points[i].y = floor_q10((radius_y - (path ? 3 * unit : 0)) * sine(angle));
        }
    }
    const opxy::i32 x = 240 * unit + gaze * 13;
    const opxy::i32 y = 123 * unit - impulse / 2;
    const opxy::i32 iris = 695 + cut / 7 - res / 12 + impulse / 12;
    place(frame, 0, 240 * unit, 123 * unit, 1280, 9, stroke);
    place(frame, 1, 240 * unit, 123 * unit, 1280, 3, stroke);
    place(frame, 10, x, y, iris, 8 + static_cast<unsigned>(drive * 4 / unit), fill_and_stroke);
    place(frame, 11, x, y, iris, 1, fill_and_stroke);
    place(frame, 12, x, y, iris, 6 + static_cast<unsigned>(drive * 5 / unit), fill_and_stroke);
    place(frame, 13, x, y, iris, 12, stroke);
    turn(frame, 13, rotation(35 + cut / 5 + impulse / 9));
    place(frame, 14, x, y, iris, 2, fill_and_stroke);
    place(frame, 15, x, y, floor_q10(iris * (unit - res / 4)), 0, fill_and_stroke);
    place(frame, 16, x, y, iris, 15, fill_and_stroke);
    place(frame, 17, x, y, iris, 11, fill_and_stroke);
    place(frame, 18, x, y, iris, 10, stroke);
    place(frame, 19, x, y, iris, 6, stroke);
    place(frame, 20, x, y, iris - 48 + impulse / 32, 3 + static_cast<unsigned>(impulse * 5 / unit), stroke);
    const opxy::i32 brow = res - impulse;
    place(frame, 21, 240 * unit, 87 * unit + brow * 2, unit, 0, keep);
    place(frame, 22, 240 * unit, 87 * unit + brow * 2, unit, 0, keep);
    const opxy::i32 tilt = (dsp::control(input.slots[4]) >> 24) - 64;
    turn(frame, 21, rotation(tilt));
    turn(frame, 22, rotation(-tilt));
    place(frame, 23, 240 * unit, 178 * unit + impulse, unit, 0, keep);
}
}
