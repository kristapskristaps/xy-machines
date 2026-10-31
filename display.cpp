#include "display.h"
#include "models.h"
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
    if (slot == decay) { number(out, decay_ms(raw)); return; }
    header_amount(out, header_capacity, value);
}
inline void display_reset(DisplayState& state) { state.header.reset(); }
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
}
}
