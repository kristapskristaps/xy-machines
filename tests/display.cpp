#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
int main() {
    using namespace x0x;
    DisplayState state = {}; Frame frame = {}; opxy::draw::Input input = {};
    display_reset(state);
    for (opxy::u32 model = 0; model < model_count; ++model) {
        input.model = model;
        for (opxy::u32 raw = 0; raw < 128; ++raw) {
            for (opxy::u32 s = 0; s < 8; ++s) input.slots[s] = raw == 127 ? 0x7fffffff : opxy::i32(raw << 24);
            for (opxy::u32 page = 0; page < 2; ++page) {
                for (opxy::u32 i = 0; i < 4; ++i) input.shown[i] = opxy::u8(i + 4 * page);
                input.reduced_motion = 1; input.shift = 0;
                display(state, input, frame);
                require(opxy::draw::valid(frame), "valid two-page drawing");
                if (!page) require(!std::strcmp(frame.header[0], family_name(model, raw)), "style shown as family number");
                for (opxy::u32 i = 0; i < 4; ++i) {
                    char expected[8]; caption(expected, model, i + 4 * page, input.slots[i + 4 * page]);
                    require(!std::strcmp(frame.header[i], expected), "encoder follows active page");
                }
                input.shift = 1; input.choices[1] = 2;
                display(state, input, frame);
                require(opxy::draw::valid(frame), "valid Shift drawing");
                require(!std::strcmp(frame.texts[0].chars, "machine") && !std::strcmp(frame.header[1], "2"), "Shift config from either page");
                require(!frame.header[2][0] && !frame.header[3][0], "unused Shift controls blank");
            }
        }
    }
    input.shift = 0; input.reduced_motion = 0; input.model = kick; input.elapsed = 16;
    for (opxy::u32 tick = 0; tick < 60; ++tick) {
        for (opxy::u32 i = 0; i < 4; ++i) input.shown[i] = opxy::u8(i + (tick >= 10 && tick < 30 ? 4 : 0));
        display(state, input, frame);
        require(opxy::draw::valid(frame), "valid page animation");
    }
    std::puts("PASS: captions, both pages, Shift config and page animation");
}
