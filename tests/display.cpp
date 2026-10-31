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
                if (!page && family_count(model) > 1) require(!std::strcmp(frame.header[0], family_name(model, raw)), "style shown as family number");
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
    display_reset(state); input.note_serial = 9; input.track = 0;
    display(state, input, frame);
    require(frame.nodes[9].x == opxy::draw::hidden_x * 1024, "entering a track does not invent a hit");
    ++input.note_serial; display(state, input, frame);
    require(frame.nodes[9].x == 350 * 1024 && frame.nodes[9].shade == 1, "a real note starts a visible hit");
    const opxy::i32 initial = frame.nodes[9].scale;
    display(state, input, frame);
    require(frame.nodes[9].scale < initial && opxy::draw::valid(frame), "hit decays between notes");
    input.reduced_motion = 1; ++input.note_serial; display(state, input, frame);
    require(frame.nodes[9].scale == 1024 && frame.nodes[10].scale == 1024, "reduced motion keeps static hit feedback");
    input.track = 1; display(state, input, frame);
    require(frame.nodes[9].x == opxy::draw::hidden_x * 1024, "track changes clear a previous hit");
    input.elapsed = 0x7fffffff;
    for (unsigned i = 0; i < 30; ++i) display(state, input, frame);
    require(opxy::draw::valid(frame) && frame.nodes[9].x == opxy::draw::hidden_x * 1024, "late frames settle safely");
    std::puts("PASS: captions, both pages, Shift config, page and hit animation");
}
