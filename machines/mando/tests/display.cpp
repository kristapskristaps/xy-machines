#include "../display.cpp"
#include <cstdio>
#include <cstring>
namespace {
int failures = 0;
opxy::draw::Point point(const mando::Frame& frame, unsigned node, int x, int y) {
    using namespace opxy::draw;
    const Node& part = frame.nodes[node];
    const int sx = x * part.scale, sy = y * part.scale;
    const Point result = {part.x + floor_q10(sx * part.cos - sy * part.sin),
                          part.y + floor_q10(sx * part.sin + sy * part.cos)};
    return result;
}
int distance(opxy::draw::Point a, opxy::draw::Point b) {
    const int x = a.x - b.x, y = a.y - b.y;
    return (x < 0 ? -x : x) + (y < 0 ? -y : y);
}
void require(bool condition, const char* message) {
    if (!condition) { std::printf("FAIL: %s\n", message); ++failures; }
}
}
int main() {
    mando::DisplayState state = {};
    mando::Frame frame = {};
    opxy::draw::Input input = {};
    input.elapsed = 17; input.track = 1;
    input.slots[0] = 1331439861; input.slots[5] = 1073741824; input.slots[6] = 1073741824;
    mando::display_reset(state);
    mando::display(state, input, frame);
    require(opxy::draw::valid(frame), "initial display frame");
    require(state.impulse == 0, "activation does not fabricate a note");
    const int resting_y = frame.nodes[12].y;
    const int resting_rifle_y = frame.nodes[24].y;
    input.note_serial++;
    mando::display(state, input, frame);
    require(frame.nodes[12].y > resting_y, "note gently nods the helmet");
    require(frame.nodes[12].y - resting_y <= 2 * opxy::draw::unit, "nod remains under two pixels");
    require(frame.nodes[24].y > resting_rifle_y, "note recoils the rifle");
    for (unsigned i = 0; i < 12; ++i) {
        mando::display(state, input, frame);
        require(opxy::draw::valid(frame), "impulse remains a valid frame");
    }
    require(state.impulse == 0, "helmet impulse settles within 0.2 seconds");
    const mando::Frame settled = frame;
    for (unsigned i = 0; i < 100; ++i) mando::display(state, input, frame);
    require(std::memcmp(&settled, &frame, sizeof(frame)) == 0, "idle frame stays still");
    input.reduced_motion = 1; input.note_serial++;
    mando::display(state, input, frame);
    require(state.impulse == 0, "reduced motion suppresses helmet movement");
    input.reduced_motion = 0; input.track++; input.note_serial += 50;
    mando::display(state, input, frame);
    require(state.impulse == 0, "track change establishes a fresh baseline");
    // Compare artwork only, so a changing number in the header cannot satisfy
    // the requirement that every sound parameter has a visual response.
    for (unsigned slot = 0; slot < 8; ++slot) {
        mando::display_reset(state);
        input = opxy::draw::Input(); input.elapsed = 17; input.track = 1;
        mando::display(state, input, frame);
        const mando::Frame low = frame;
        input.slots[slot] = 2147483647;
        for (unsigned i = 0; i < 12; ++i) mando::display(state, input, frame);
        bool changed = std::memcmp(low.paths, frame.paths, sizeof(frame.paths)) != 0;
        for (unsigned node = 0; node < mando::Frame::node_count; ++node) {
            if (node >= 2 && node < 10) continue;
            changed |= std::memcmp(&low.nodes[node], &frame.nodes[node], sizeof(frame.nodes[0])) != 0;
        }
        require(changed, "each parameter changes artwork independently of the header");
        const unsigned landmarks[8] = {10,24,37,24,37,12,12,32};
        const int coordinates[8][2] = {{80,47},{0,-80},{-6,-25},{0,-80},{-6,-25},{0,-63},{0,-63},{0,0}};
        const int minimum_pixels[8] = {20,20,10,24,7,12,26,16};
        const opxy::draw::Point before = point(low, landmarks[slot], coordinates[slot][0], coordinates[slot][1]);
        const opxy::draw::Point after = point(frame, landmarks[slot], coordinates[slot][0], coordinates[slot][1]);
        require(distance(before, after) >= minimum_pixels[slot] * opxy::draw::unit,
                "each parameter moves a visible feature by a substantial pixel distance");
        const mando::Frame end = frame;
        for (unsigned i = 0; i < 100; ++i) mando::display(state, input, frame);
        require(std::memcmp(&end, &frame, sizeof(frame)) == 0, "parameter response settles and stays still");
        input.reduced_motion = 1; input.slots[slot] = 0;
        mando::display(state, input, frame);
        require(std::memcmp(low.paths, frame.paths, sizeof(frame.paths)) == 0, "reduced motion snaps contours");
        for (unsigned node = 0; node < mando::Frame::node_count; ++node) {
            if (node >= 2 && node < 10) continue;
            require(std::memcmp(&low.nodes[node], &frame.nodes[node], sizeof(frame.nodes[0])) == 0,
                    "reduced motion snaps artwork to the resting pose");
        }
    }
    input.reduced_motion = 0;
    for (unsigned corner = 0; corner < 256; ++corner) {
        for (unsigned j = 0; j < 8; ++j) input.slots[j] = corner & (1u << j) ? 2147483647 : 0;
        for (unsigned page = 0; page < 2; ++page) {
            for (unsigned j = 0; j < 4; ++j) input.shown[j] = static_cast<opxy::u8>(j + page * 4);
            input.note_serial++;
            for (unsigned i = 0; i < 16; ++i) {
                mando::display(state, input, frame);
                require(opxy::draw::valid(frame), "all controls and both page transitions are valid");
                require(frame.nodes[1].y > 145 * opxy::draw::unit && frame.nodes[1].paint == opxy::draw::keep,
                        "former visor mark is a neutral rifle sling below the face");
                for (unsigned corner = 0; corner < 4; ++corner) {
                    const opxy::draw::Point edge = point(frame, 12, corner & 1 ? 52 : -52, corner & 2 ? 64 : -64);
                    require(edge.y >= 38 * opxy::draw::unit && edge.y < 220 * opxy::draw::unit,
                            "entire helmet remains below the header and inside the screen");
                }
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
