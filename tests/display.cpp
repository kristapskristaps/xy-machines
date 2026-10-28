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
    require(state.ages[0] == 184, "activation must not fabricate a note");
    vader::Frame idle = frame;
    for (unsigned i = 0; i < 300; ++i) vader::display(state, input, frame);
    require(!std::memcmp(&idle, &frame, sizeof(frame)), "idle screen is stationary");
    ++input.note_serial;
    input.held_notes = 1;
    vader::display(state, input, frame);
    require(state.ages[0] == 0, "each note triggers an impact");
    require(frame.nodes[10].shade == 15, "held notes illuminate the visor");
    for (unsigned i = 0; i < 14; ++i) vader::display(state, input, frame);
    require(state.ages[0] == 184, "note motion settles in 0.2 s");
    const vader::Frame held = frame;
    for (unsigned i = 0; i < 300; ++i) vader::display(state, input, frame);
    require(!std::memcmp(&held, &frame, sizeof(frame)), "held pose settles without an idle clock");
    ++input.note_serial;
    vader::display(state, input, frame);
    require(frame.nodes[47].scale > 1024, "the traced helmet recoils as a whole");
    for (unsigned i = 0; i < 3; ++i) vader::display(state, input, frame);
    ++input.note_serial;
    vader::display(state, input, frame);
    require(state.ages[1] > 0 && state.ages[2] == 0, "rapid notes preserve independent rings");
    require(frame.nodes[26].x != 640 * 1024 && frame.nodes[27].x != 640 * 1024,
            "two recent impacts remain visible together");
    input.reduced_motion = 1;
    ++input.note_serial;
    vader::display(state, input, frame);
    require(state.ages[0] == 184 && frame.nodes[10].shade == 14, "reduced motion keeps static note feedback");
    require(frame.nodes[47].scale == 1024 && frame.nodes[68].x == 640 * 1024,
            "reduced motion removes recoil and particles");
    input.held_notes = 0;
    vader::display(state, input, frame);
    require(frame.nodes[10].shade == 7, "release extinguishes visor");
    input.reduced_motion = 0;
    input.track = 1;
    ++input.note_serial;
    vader::display(state, input, frame);
    require(state.ages[0] == 184, "track change establishes a baseline");
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
