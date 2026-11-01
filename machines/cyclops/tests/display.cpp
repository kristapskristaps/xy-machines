#include "../display.cpp"
#include <cstdio>
#include <cstring>
namespace {
int failures = 0;
void require(bool condition, const char* message) {
    if (!condition) { std::printf("FAIL: %s\n", message); ++failures; }
}
}
int main() {
    cyclops::DisplayState state = {};
    cyclops::Frame frame = {};
    opxy::draw::Input input = {};
    input.elapsed = 17; input.track = 1;
    input.slots[0] = 1331439861; input.slots[5] = 1073741824; input.slots[6] = 1073741824;
    cyclops::display_reset(state);
    cyclops::display(state, input, frame);
    require(opxy::draw::valid(frame), "initial display frame");
    require(state.impulse == 0, "activation does not fabricate a note");
    const int resting_y = frame.nodes[12].y;
    const unsigned resting_glint = frame.nodes[1].shade;
    input.note_serial++;
    cyclops::display(state, input, frame);
    require(frame.nodes[12].y > resting_y, "note gently nods the helmet");
    require(frame.nodes[12].y - resting_y <= 2 * opxy::draw::unit, "nod remains under two pixels");
    require(frame.nodes[1].shade > resting_glint, "note lights the visor reflection");
    for (unsigned i = 0; i < 12; ++i) {
        cyclops::display(state, input, frame);
        require(opxy::draw::valid(frame), "impulse remains a valid frame");
    }
    require(state.impulse == 0, "helmet impulse settles within 0.2 seconds");
    const cyclops::Frame settled = frame;
    for (unsigned i = 0; i < 100; ++i) cyclops::display(state, input, frame);
    require(std::memcmp(&settled, &frame, sizeof(frame)) == 0, "idle frame stays still");
    input.reduced_motion = 1; input.note_serial++;
    cyclops::display(state, input, frame);
    require(state.impulse == 0, "reduced motion suppresses helmet movement");
    input.reduced_motion = 0; input.track++; input.note_serial += 50;
    cyclops::display(state, input, frame);
    require(state.impulse == 0, "track change establishes a fresh baseline");
    for (unsigned corner = 0; corner < 256; ++corner) {
        for (unsigned j = 0; j < 8; ++j) input.slots[j] = corner & (1u << j) ? 2147483647 : 0;
        for (unsigned page = 0; page < 2; ++page) {
            for (unsigned j = 0; j < 4; ++j) input.shown[j] = static_cast<opxy::u8>(j + page * 4);
            input.note_serial++;
            for (unsigned i = 0; i < 16; ++i) {
                cyclops::display(state, input, frame);
                require(opxy::draw::valid(frame), "all controls and both page transitions are valid");
                for (unsigned node = 13; node < 24; ++node)
                    require(frame.nodes[node].x == frame.nodes[12].x &&
                            frame.nodes[node].y == frame.nodes[12].y &&
                            frame.nodes[node].cos == frame.nodes[12].cos &&
                            frame.nodes[node].sin == frame.nodes[12].sin,
                            "helmet facets stay attached across controls and notes");
            }
        }
    }
    std::printf("%s: helmet response, idle, reduced motion, track baselines, controls and pages\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
