#include "display.h"
#include "models.h"
#include "animation.h"
namespace x0x {
inline void number(char* out, opxy::u32 value) {
    char reverse[10]; opxy::u32 n = 0;
    do { reverse[n++] = char('0' + value % 10); value /= 10; } while (value);
    for (opxy::u32 i = 0; i < n; ++i) out[i] = reverse[n - i - 1];
    out[n] = 0;
}
inline void caption(char* out, opxy::u32 model, opxy::u32 slot, opxy::i32 value) {
    using namespace opxy::draw;
    if (!control_name(model, slot)[0]) { copy(out, header_capacity, ""); return; }
    const opxy::u32 raw = raw_control(value);
    if (slot == style) { copy(out, header_capacity, family_name(model, raw)); return; }
    if (slot == attack) {
        number(out,raw);
        opxy::u32 at = 0; while (out[at]) ++at;
        out[at++] = 'm'; out[at++] = 's'; out[at] = 0;
        return;
    }
    if (slot == tune) {
        const opxy::i32 cents = tune_cents(raw);
        char text[8]; text[0] = cents < 0 ? '-' : '+';
        const opxy::u32 tenths = opxy::u32(cents < 0 ? -cents : cents) / 10;
        number(text + 1, tenths / 10);
        opxy::u32 at = 1; while (text[at]) ++at;
        text[at++] = '.'; text[at++] = char('0' + tenths % 10); text[at] = 0;
        copy(out, header_capacity, text);
        return;
    }
    if (slot == decay) {
        const opxy::u32 ms = decay_ms(model, raw);
        if (ms >= 1000) {
            const opxy::u32 tenths = (ms + 50) / 100;
            number(out, tenths / 10);
            opxy::u32 at = 0; while (out[at]) ++at;
            out[at++] = '.'; out[at++] = char('0' + tenths % 10); out[at++] = 's'; out[at] = 0;
        } else {
            number(out, ms);
            opxy::u32 at = 0; while (out[at]) ++at;
            out[at++] = 'm'; out[at++] = 's'; out[at] = 0;
        }
        return;
    }
    header_amount(out, header_capacity, value);
}
inline void display_reset(DisplayState& state) { state.header.reset(); state.initialized = 0; }
inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const opxy::u32 model = input.model < model_count ? input.model : kick;
    const char* titles[8]; char values[8][header_capacity];
    for (opxy::u32 i = 0; i < 8; ++i) {
        titles[i] = control_name(model, i);
        caption(values[i], model, i, input.slots[i]);
    }
    Input projected = input;
    if (input.shift) {
        for (opxy::u32 i = 0; i < 8; ++i) { titles[i] = ""; values[i][0] = 0; }
        titles[0] = "machine"; titles[1] = "choke";
        copy(values[0], header_capacity, short_name(model));
        static const char* const chokes[] = {"NONE", "1", "2", "3"};
        copy(values[1], header_capacity, chokes[input.choices[1] < 4 ? input.choices[1] : 0]);
        for (opxy::u32 i = 0; i < 4; ++i) projected.shown[i] = opxy::u8(i);
        projected.reduced_motion = 1;
    }
    page_header(state.header, projected, frame, titles, values, 0);
    const opxy::u32 family = family_index(model,raw_control(input.slots[style]));
    const bool hit = state.initialized && state.track == input.track && state.note != input.note_serial;
    if (!state.initialized || state.track != input.track || (!hit && (state.model != model || state.family != family))) {
        state.age = 2048;
        state.hit_model = model; state.hit_family = family;
        state.hit_duration = clamp(opxy::i32(decay_ms(model,raw_control(input.slots[decay]))*1024/1000),128,1536);
    } else if (hit) {
        state.age = 0;
        const bool known = input.trigger_key >= 53 && input.trigger_key < 77 && input.trigger_model < model_count;
        state.hit_model = known ? input.trigger_model : model;
        const opxy::i32* values = known ? input.trigger_controls : input.slots;
        state.hit_family = family_index(state.hit_model,raw_control(values[style]));
        state.hit_duration = clamp(opxy::i32(decay_ms(state.hit_model,raw_control(values[decay]))*1024/1000),128,1536);
    } else {
        state.age = clamp(state.age + clamp(input.elapsed, 0, 103), 0, 2048);
    }
    state.initialized = 1; state.track = input.track; state.note = input.note_serial;
    state.model = model; state.family = family;
    instrument_animation(frame,state.hit_model,state.hit_family,state.age,state.hit_duration,input.reduced_motion != 0,input.note_serial);
}
}
