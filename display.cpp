#include "display.h"
namespace weave {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 settle = 184;
const char* const codes[15] = {
    "SHAP", "FM", "NOIS", "DUST", "KICK", "SNAR", "HAT", "GRAN",
    "CHRD", "SWRM", "WAVE", "ANLG", "MODL", "STRG", "6OP"
};
const char* const names[15] = {
    "shaper", "2-op fm", "noise", "dust", "kick", "snare", "hihat", "grain",
    "chords", "swarm", "waves", "analog", "modal", "string", "6-op fm"
};
struct Profile { i32 turns, folds, flat, lift; };
const Profile profiles[15] = {
    {2,3,260,680}, {2,5,230,620}, {1,7,280,620}, {1,9,210,800},
    {1,2,280,300}, {1,4,250,600}, {1,8,180,450}, {2,3,300,760},
    {2,5,280,720}, {1,6,250,800}, {3,4,230,650}, {2,3,190,550},
    {1,3,300,780}, {1,2,130,380}, {3,5,210,760}
};
i32 amount(i32 word) { return word < 0 ? 0 : word >> 16; }
i32 ease(i32 age) {
    const i32 remaining = unit - clamp(age, 0, settle) * unit / settle;
    return unit - remaining * remaining / unit * remaining / unit;
}
u32 hash_word(u32 hash, u32 word) { return (hash ^ word) * 16777619u; }
struct Basis { i32 radius, tube, cp, sp, cq, sq, flat, lift; };
Basis basis(unsigned model, const DisplayState& state, unsigned point) {
    const Profile& profile = profiles[model];
    const i32 angle = static_cast<i32>(point) * 3600 / 64;
    const i32 orbit = profile.turns * angle + state.draw_phase / 3 + state.draw_controls[0] * 720 / 255;
    const i32 twist = profile.folds * angle + state.draw_phase
        + state.draw_controls[3] * 1800 / 255;
    Basis result;
    result.radius = 40 * unit + state.draw_controls[0] * (8 * unit) / 255;
    result.tube = 5 * unit + state.draw_controls[1] * (7 * unit) / 255 + state.draw_energy * (3 * unit) / 32;
    result.cp = sine(orbit + 900); result.sp = sine(orbit);
    result.cq = sine(twist + 900); result.sq = sine(twist);
    result.flat = clamp(profile.flat + (state.draw_controls[2] - 128) / 2, 120, 340);
    result.lift = clamp(profile.lift + state.draw_controls[2] / 2, 250, 900);
    return result;
}
Point strand(const Basis& b, unsigned index) {
    const i32 cosine[4] = {b.cq, -b.sq, -b.cq, b.sq};
    const i32 sine_wave[4] = {b.sq, b.cq, -b.sq, -b.cq};
    const i32 radius = b.radius + floor_q10(b.tube * cosine[index]);
    const Point point = {
        floor_q10(radius * b.cp),
        floor_q10(floor_q10(radius * b.sp) * b.flat)
            + floor_q10(floor_q10(b.tube * sine_wave[index]) * b.lift)
    };
    return point;
}
void geometry(const DisplayState& state, Frame& frame) {
    u32 key = hash_word(2166136261u, state.model);
    key = hash_word(key, state.draw_blend == unit ? state.model : state.previous_model);
    key = hash_word(key, static_cast<u32>(state.draw_phase));
    key = hash_word(key, static_cast<u32>(state.draw_energy));
    key = hash_word(key, static_cast<u32>(state.draw_blend));
    for (auto value : state.draw_controls) key = hash_word(key, static_cast<u32>(value));
    if (!key) key = 1;
    bool unchanged = true;
    for (const auto& path : frame.paths) if (path.key != key) unchanged = false;
    if (unchanged) return;
    // Four strands share their trigonometry. Only a model transition needs a
    // second profile; settled/idle geometry is retained by both renderers.
    for (unsigned i = 0; i < Frame::point_count; ++i) {
        const Basis next = basis(state.model, state, i);
        const Basis previous = state.draw_blend == unit ? next : basis(state.previous_model, state, i);
        for (unsigned path = 0; path < Frame::path_count; ++path) {
            const Point a = strand(previous, path), b = strand(next, path);
            frame.paths[path].points[i].x = a.x + (b.x - a.x) * state.draw_blend / unit;
            frame.paths[path].points[i].y = a.y + (b.y - a.y) * state.draw_blend / unit;
        }
    }
    for (auto& path : frame.paths) path.key = key;
}
}
void display_reset(DisplayState& state) { state = DisplayState(); }
void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const bool snapshot = !state.initialized || state.track != input.track;
    const auto events = state.notes.update(input);
    const bool held = input.held_notes <= 128 && input.held_notes != 0;
    const bool hit = !snapshot && input.track < 8 && state.serial != input.note_serial;
    const unsigned selected = static_cast<unsigned>(amount(input.slots[0])) * 15u >> 15;
    const i32 elapsed = clamp(input.elapsed, 0, 102);
    const bool changed_model = !snapshot && selected != state.model;
    const bool was_settling = state.control_age < settle || state.model_age < settle;
    i32 target[4];
    for (unsigned i = 0; i < 4; ++i) target[i] = amount(input.slots[i + 1]);
    if (snapshot || input.reduced_motion) {
        state.phase = state.travel = 0;
        state.model = state.previous_model = selected;
        state.model_age = state.control_age = state.pulse_age = state.quiet_age = settle;
        state.energy = held && !input.reduced_motion ? 640 : 0;
        for (unsigned i = 0; i < 4; ++i)
            state.controls[i] = state.from[i] = state.target[i] = target[i];
    } else {
        bool changed = false;
        for (unsigned i = 0; i < 4; ++i) if (target[i] != state.target[i]) changed = true;
        if (changed) {
            state.control_age = 0;
            for (unsigned i = 0; i < 4; ++i) {
                state.from[i] = state.controls[i]; state.target[i] = target[i];
            }
        }
        state.control_age = clamp(state.control_age + elapsed, 0, settle);
        const i32 blend = ease(state.control_age);
        for (unsigned i = 0; i < 4; ++i)
            state.controls[i] = state.from[i] + (state.target[i] - state.from[i]) * blend / unit;
        if (changed_model) {
            state.previous_model = state.model; state.model = selected; state.model_age = 0;
        }
        state.model_age = clamp(state.model_age + elapsed, 0, settle);
        if (hit) { state.energy = unit; state.pulse_age = 0; state.quiet_age = 0; }
        else {
            state.pulse_age = clamp(state.pulse_age + elapsed, 0, settle);
            const i32 target_energy = held ? 640 : 0;
            state.energy += (target_energy - state.energy) * clamp(elapsed * 12, 0, unit) / unit;
        }
        if (events.hold_ended) state.quiet_age = 0;
        if (held) state.quiet_age = 0;
        else state.quiet_age = clamp(state.quiet_age + elapsed, 0, settle);
        if (!held && state.quiet_age == settle) state.energy = 0;
        state.phase = (state.phase + static_cast<u32>(elapsed * (3 + state.controls[1] / 8192) * state.energy / (unit * 4))) % 3600u;
        state.travel = (state.travel + static_cast<u32>(elapsed * (7 + state.controls[1] / 8192) * state.energy / unit)) % 3600u;
    }
    state.sample_age += elapsed;
    // Twenty geometry updates per second; note starts and model changes respond
    // immediately. The last settling frame bypasses the cadence limit.
    const bool finished = state.control_age == settle && state.model_age == settle;
    if (snapshot || input.reduced_motion || hit || changed_model || state.sample_age >= 51 ||
        (finished && (was_settling || state.energy == 0))) {
        state.sample_age %= 51;
        for (unsigned i = 0; i < 4; ++i) state.draw_controls[i] = state.controls[i] >> 7;
        state.draw_phase = static_cast<i32>(state.phase / 10 * 10);
        state.draw_energy = state.energy >> 5;
        state.draw_blend = ease(state.model_age);
    }
    geometry(state, frame);
    for (unsigned path = 0; path < 4; ++path)
        place(frame, path, 240 * unit, 113 * unit, 2 * unit,
              path == 3 ? (held || state.energy > 320 ? 7 : 6) : 2 + path, stroke);
    for (unsigned path = 0; path < 4; ++path) for (unsigned tail = 0; tail < 3; ++tail) {
        const unsigned node = 4 + path * 3 + tail;
        if (!state.energy || input.reduced_motion) { hide(frame, node); continue; }
        const u32 cursor = (state.travel + path * 230u + 3600u - tail * 110u) % 3600u * 64u * unit / 3600u;
        const unsigned at = cursor >> 10, next = (at + 1) & 63u;
        const i32 fraction = static_cast<i32>(cursor & 1023u);
        const Point& a = frame.paths[path].points[at];
        const Point& b = frame.paths[path].points[next];
        const i32 x = a.x + (b.x - a.x) * fraction / unit;
        const i32 y = a.y + (b.y - a.y) * fraction / unit;
        place(frame, node, 240 * unit + x * 2, 113 * unit + y * 2,
              tail ? 640 - static_cast<i32>(tail) * 160 : 896 + state.energy / 3,
              tail ? (path == 3 ? 6 : 2) : (path == 3 ? 8 : 5), fill_and_stroke);
    }
    if (state.pulse_age < settle && !input.reduced_motion)
        place(frame, 16, 240 * unit, 113 * unit, 128 + ease(state.pulse_age) * 1650 / unit,
              state.pulse_age < 40 ? 8 : state.pulse_age < 100 ? 7 : 6, stroke);
    else hide(frame, 16);
    place(frame, 17, (241 + static_cast<i32>(selected) * 14) * unit, 204 * unit, unit, 7, fill_and_stroke);
    if (held) place(frame, 18, 452 * unit, 37 * unit, unit, 7, fill_and_stroke);
    else hide(frame, 18);
    char number[8] = {static_cast<char>('0' + (selected + 1) / 10), static_cast<char>('0' + (selected + 1) % 10), ' ', '/', ' ', '1', '5', 0};
    text(frame, 0, 24 * unit, 40 * unit, unit, number);
    text(frame, 1, 113 * unit, 207 * unit, unit, names[selected]);
    for (unsigned i = 0; i < 4; ++i) header(frame, i, "");
    if (input.shown[0] == 0) header(frame, 0, codes[selected]);
    state.initialized = 1; state.track = input.track; state.serial = input.note_serial;
}
}
