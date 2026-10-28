#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

int main() {
    vader::DisplayState state = {};
    vader::Frame frame = {};
    opxy::draw::Input input = {};
    input.elapsed = 17;
    input.slots[1] = 1073741824;
    input.slots[6] = 2147483647;
    for (unsigned i = 0; i < 4; ++i) input.shown[i] = static_cast<opxy::u8>(i);
    vader::display_reset(state);
    input.note_serial = 25;
    vader::display(state, input, frame);
    require(opxy::draw::valid(frame), "valid initial frame");
    require(!state.impact, "activation must not fabricate a note");
    vader::Frame idle = frame;
    for (unsigned i = 0; i < 300; ++i) vader::display(state, input, frame);
    require(!std::memcmp(&idle, &frame, sizeof(frame)), "idle screen is stationary");
    ++input.note_serial;
    input.held_notes = 1;
    vader::display(state, input, frame);
    require(state.impact == 1024, "each note triggers an impact");
    require(frame.nodes[10].shade == 15, "held notes illuminate the visor");
    for (unsigned i = 0; i < 14; ++i) vader::display(state, input, frame);
    require(!state.impact, "note motion settles in 0.2 s");
    input.reduced_motion = 1;
    ++input.note_serial;
    vader::display(state, input, frame);
    require(!state.impact && frame.nodes[10].shade == 15, "reduced motion keeps static note feedback");
    input.held_notes = 0;
    vader::display(state, input, frame);
    require(frame.nodes[10].shade == 12, "release extinguishes visor");
    input.reduced_motion = 0;
    input.track = 1;
    ++input.note_serial;
    vader::display(state, input, frame);
    require(!state.impact, "track change establishes a baseline");
    unsigned seed = 871;
    for (unsigned f = 0; f < 3000; ++f) {
        seed = seed * 1664525u + 1013904223u;
        input.slots[f % 8] = static_cast<opxy::i32>(seed & 0x7fffffffu);
        if (f % 7 == 0) ++input.note_serial;
        for (unsigned j = 0; j < 4; ++j) input.shown[j] = static_cast<opxy::u8>(j + (f % 2) * 4);
        input.elapsed = f % 29;
        vader::display(state, input, frame);
        require(opxy::draw::valid(frame), "valid control and note animation frame");
    }
    std::puts("PASS: idle, note impacts, release, reduced motion, track changes and 3000 animated frames.");
}
