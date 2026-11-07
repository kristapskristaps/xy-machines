#include "display.h"

namespace acidline {
namespace {
using opxy::i32;
using opxy::u32;
using namespace opxy::draw;

constexpr u32 route_points = Frame::point_count / 2;
constexpr u32 first_header = Frame::path_count;
constexpr u32 first_spark = first_header + page_header_nodes;
constexpr i32 origin_x = 240 * unit;
constexpr i32 origin_y = 130 * unit;

Point plate_point(i32 u, i32 v) {
    return {79 * unit + u + 3 * v / 4, 133 * unit - 7 * u / 32 + 3 * v / 8};
}

void trace(Frame& frame, u32 slot, const Point (&route)[route_points], u32 shade) {
    auto& path = frame.paths[slot];
    bool changed = path.key == 0;
    for (u32 i = 0; i < Frame::point_count; ++i) {
        const bool back = i >= route_points;
        const auto p = route[back ? Frame::point_count - 1 - i : i];
        const auto projected = plate_point(p.x, p.y + (back ? unit : -unit));
        const Point next = {(projected.x - origin_x) / 2, (projected.y - origin_y) / 2};
        changed |= path.points[i].x != next.x || path.points[i].y != next.y;
        path.points[i] = next;
    }
    if (changed && !++path.key)
        ++path.key;
    place(frame, slot, origin_x, origin_y, 2 * unit, shade, fill_and_stroke);
}

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
    page_header(state.header, input, frame, titles, values, first_header);

    const i32 flow = input.slots[0] >> 21;
    const i32 bite = input.slots[1] >> 21;
    const i32 burn = input.slots[2] >> 21;
    const i32 sweep = input.slots[3] >> 21;
    const i32 length = input.slots[4] >> 21;
    const i32 body = input.slots[6] >> 21;
    const i32 shade = input.slots[7] >> 21;
    update_pulse(state, input, length);

    const i32 waveform[3][4] = {{-4, -1, 2, 4}, {-4, 0, 4, 0}, {-4, -4, 4, 4}};
    const i32 knee = 5 * unit - 4 * flow;
    const i32 centre = 34 * unit + 10 * flow - 5 * sweep;
    const i32 depth = 3 * unit + 10 * bite;
    const i32 attack = 10 * sweep * state.pulse / unit;
    Point cut[route_points];
    Point echo[route_points];

    for (u32 i = 0; i < route_points; ++i) {
        const i32 step = static_cast<i32>(i);
        const i32 opens = clamp(2 * (step * unit - knee), 0, unit);
        const i32 tail = clamp((16 - step) * unit / 3, 0, unit);
        const i32 angle = waveform[shape][i % 4];
        const i32 pit = i % 4 == 2 ? 4 * burn : 0;
        const i32 turn = clamp(step - 13, 0, 3) * (2 * unit + 5 * length) / 3;
        const i32 fold = angle * opens * (depth - 2 * shade) / (4 * unit) + pit;
        const i32 struck = angle * opens * attack / (4 * unit);
        const i32 u = (22 + 13 * step) * unit;
        cut[i] = {u, clamp(centre + (fold + struck) * tail / unit + turn, 9 * unit, 68 * unit)};
        echo[i] = {u, centre + 15 * unit + 9 * body + fold * tail / unit * 3 / 5 + turn};
    }
    trace(frame, 0, echo, 1 + static_cast<u32>(shade) / 256);
    trace(frame, 1, cut, 0);

    for (u32 trail = 0; trail < 3; ++trail) {
        const i32 progress = unit - state.pulse - static_cast<i32>(trail) * 64;
        const u32 node = first_spark + trail;
        if (!state.pulse || progress < 0) {
            hide(frame, node);
            continue;
        }
        const i32 along = progress * static_cast<i32>(route_points - 1);
        const u32 edge = static_cast<u32>(along / unit);
        const i32 part = along % unit;
        const auto a = cut[edge], b = cut[edge + 1];
        const auto p = plate_point(a.x + (b.x - a.x) * part / unit, a.y + (b.y - a.y) * part / unit);
        place(frame, node, p.x, p.y, unit, 0, keep);
    }
}
}
