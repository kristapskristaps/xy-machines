#include "display.h"
#include "dsp.h"

namespace mando {
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
                                      : i == 3 ? dsp::option(input.slots[i], 4) * 341
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
    const opxy::i32 fold[8][2] = {{12,6},{26,17},{42,27},{57,39},{61,42},{50,38},{35,27},{20,14}};
    frame.paths[0].key = 1;
    for (unsigned i = 0; i < Frame::point_count; ++i) {
        const unsigned edge = i / 8, next = (edge + 1) % 8;
        const opxy::i32 t = i % 8;
        frame.paths[0].points[i].x = (fold[edge][0] * (8 - t) + fold[next][0] * t) * unit / 8;
        frame.paths[0].points[i].y = (fold[edge][1] * (8 - t) + fold[next][1] * t) * unit / 8;
    }
    const opxy::i32 cape_scale = unit + cut / 3;
    const Turn wind = rotation(-10 - cut / 10 + impulse / 32);
    const unsigned cape_nodes[] = {0, 10, 48, 49};
    for (unsigned i = 0; i < 4; ++i) {
        place(frame, cape_nodes[i], 270 * unit, 148 * unit, cape_scale, 0, keep);
        turn(frame, cape_nodes[i], wind);
    }
    place(frame, 11, 240 * unit, 184 * unit, unit, 0, keep);
    for (unsigned node = 40; node <= 47; ++node) place(frame, node, 0, 0, unit, 0, keep);

    const opxy::i32 x = 240 * unit, y = 115 * unit + impulse * 3 / 2;
    const opxy::i32 angle = balance / 7 + impulse / 64;
    const Turn tilt = rotation(angle);
    for (unsigned node = 12; node < 24; ++node) {
        place(frame, node, x, y, unit, 0, keep);
        turn(frame, node, tilt);
    }
    for (unsigned node = 50; node <= 51; ++node) {
        place(frame, node, x, y, unit, 0, keep);
        turn(frame, node, tilt);
    }
    // Illumination follows the head turn. The visor remains empty black glass.
    frame.nodes[13].paint = frame.nodes[14].paint = fill_and_stroke;
    frame.nodes[13].shade = 10 - static_cast<unsigned>((balance + 512) * 5 / unit);
    frame.nodes[14].shade = 4 + static_cast<unsigned>((balance + 512) * 5 / unit);
    frame.nodes[19].paint = fill_and_stroke;
    frame.nodes[19].shade = 10 + static_cast<unsigned>(impulse * 2 / unit);

    const opxy::i32 stance = state.pose[3];
    const opxy::i32 gun_x = 304 * unit + stance * 14;
    const opxy::i32 gun_y = 165 * unit - stance * 10 + impulse * 6;
    const opxy::i32 aim_angle = 60 + res * 230 / unit + stance * 230 / unit;
    const Turn aim = rotation(aim_angle);
    const opxy::i32 stock_x = gun_x - 5 * aim.sin, stock_y = gun_y + 5 * aim.cos;
    const opxy::i32 dx = 290 * unit - stock_x, dy = 178 * unit - stock_y;
    const Point sling[8] = {{-unit,0},{unit,0},{dx/3+unit,dy/3},{dx+unit,dy},
                            {dx,dy+unit},{dx-unit,dy},{dx/3-unit,dy/3},{-unit,2*unit}};
    opxy::u32 key = (2166136261u ^ static_cast<opxy::u32>(dx)) * 16777619u;
    key = (key ^ static_cast<opxy::u32>(dy)) * 16777619u;
    frame.paths[1].key = key ? key : 1;
    for (unsigned i = 0; i < Frame::point_count; ++i) {
        const unsigned edge = i / 8, next = (edge + 1) % 8;
        const opxy::i32 t = i % 8;
        frame.paths[1].points[i].x = (sling[edge].x * (8 - t) + sling[next].x * t) / 8;
        frame.paths[1].points[i].y = (sling[edge].y * (8 - t) + sling[next].y * t) / 8;
    }
    place(frame, 1, stock_x, stock_y, unit, 0, keep);
    for (unsigned node = 24; node <= 30; ++node) {
        place(frame, node, gun_x, gun_y, unit, 0, keep);
        turn(frame, node, aim);
    }
    attached(frame, 28, gun_x, gun_y, 0, -19 * unit, aim, 640 + drive + impulse / 4);
    attached(frame, 30, gun_x, gun_y, 12 * unit, -44 * unit, aim, 1280 - res / 2);
    attached(frame, 39, gun_x, gun_y, 0, -70 * unit, aim, unit + drive * 31 / 32);
    attached(frame, 54, gun_x, gun_y, 0, -80 * unit - drive * 14, aim);
    for (unsigned side = 0; side < 2; ++side) {
        const opxy::i32 sign = side ? 1 : -1;
        const unsigned prong = 37 + side, bevel = 52 + side;
        attached(frame, prong, gun_x, gun_y, sign * detune * 6, -80 * unit - drive * 14, aim);
        turn(frame, prong, rotation(aim_angle + sign * detune * 100 / unit));
        frame.nodes[bevel] = frame.nodes[prong];
    }
    const opxy::i32 mast = 640 + keytrack * 4 / 3;
    attached(frame, 31, x, y, 53 * unit, -8 * unit, tilt, mast);
    attached(frame, 32, x, y, 53 * unit, -8 * unit - mast * 15, tilt);
    turn(frame, 32, rotation(angle - keytrack * 900 / unit));

    const unsigned waves = dsp::option(input.slots[3], 4);
    for (unsigned osc = 0; osc < 2; ++osc) {
        const unsigned saw = 33 + osc * 2, pulse = saw + 1;
        const bool square = osc ? (waves & 1) != 0 : waves >= 2;
        place(frame, square ? pulse : saw, (osc ? 260 : 226) * unit,
              (osc ? 205 : 201) * unit, 1536, 0, keep);
        hide(frame, square ? saw : pulse);
    }
    // Three framing distances share a fixed lower anchor. Every portrait
    // component inherits this transform, including armour, scope and muzzle.
    const opxy::i32 zoom = 860 + octave * 180 / unit;
    for (unsigned node = 0; node < Frame::node_count; ++node) {
        if ((node >= 2 && node < 10) || frame.nodes[node].x == hidden_x * unit) continue;
        Node& part = frame.nodes[node];
        part.x = 240 * unit + floor_q10((part.x - 240 * unit) * zoom);
        part.y = 220 * unit + floor_q10((part.y - 220 * unit) * zoom);
        part.scale = floor_q10(part.scale * zoom);
    }
}
}
