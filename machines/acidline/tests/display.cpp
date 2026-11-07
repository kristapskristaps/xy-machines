#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
using acidline::DisplayState;
using acidline::Frame;
using opxy::draw::Input;
using opxy::draw::page_header_nodes;

void require(bool pass, const char* why) {
    if (!pass) {
        std::puts(why);
        std::exit(1);
    }
}

Input midpoint() {
    Input input = {};
    for (auto& parameter : input.slots)
        parameter = 0x40000000;
    for (opxy::u8 column = 0; column < 4; ++column)
        input.shown[column] = column;
    input.elapsed = 34;
    return input;
}

unsigned changes(const Frame& before, const Frame& after) {
    unsigned count = 0;
    for (unsigned node = page_header_nodes; node < Frame::node_count; ++node)
        count += std::memcmp(&before.nodes[node], &after.nodes[node], sizeof(before.nodes[node])) != 0;
    return count;
}

void render(DisplayState& state, const Input& input, Frame& frame) {
    acidline::display(state, input, frame);
    require(opxy::draw::valid(frame), "FAIL display exceeds native frame limits");
}
}

int main() {
    for (unsigned slot = 0; slot < 8; ++slot) {
        Input input = midpoint();
        if (slot >= 4) {
            for (auto& shown : input.shown)
                shown += 4;
        }
        DisplayState state = {};
        Frame middle = {}, low = {}, high = {};
        render(state, input, middle);
        input.slots[slot] = 0;
        render(state, input, low);
        input.slots[slot] = 0x7fffffff;
        render(state, input, high);
        require(changes(middle, low) > 0 && changes(middle, high) > 0,
                "FAIL a control does not move the stage on the next frame at rest");
    }

    DisplayState state = {};
    Input input = midpoint();
    Frame idle = {}, previous = {}, current = {};
    render(state, input, idle);
    input.note_on_serial++;
    input.held_notes = 1;
    render(state, input, current);
    require(changes(idle, current) == 1, "FAIL note attack should move just the droplet");

    for (unsigned tick = 0; tick < 32; ++tick) {
        previous = current;
        render(state, input, current);
        require(changes(previous, current) <= 1, "FAIL held note redraws more than one stage node");
    }
    require(changes(idle, current) == 1 && changes(previous, current) == 0,
            "FAIL held droplet should settle above the channel");
    input.note_on_serial++;
    previous = current;
    render(state, input, current);
    require(changes(previous, current) == 1, "FAIL retrigger should lift the droplet immediately");

    input.held_notes = 0;
    input.note_off_serial++;
    for (unsigned tick = 0; tick < 32; ++tick) {
        previous = current;
        render(state, input, current);
        require(changes(previous, current) <= 1, "FAIL release redraws more than one stage node");
    }
    require(changes(idle, current) == 0, "FAIL release does not settle at rest");

    input.reduced_motion = 1;
    input.note_on_serial++;
    input.held_notes = 1;
    render(state, input, current);
    require(changes(idle, current) == 0, "FAIL reduced motion animates a note");
    input.reduced_motion = 0;
    input.track++;
    input.note_on_serial = 0;
    input.held_notes = 0;
    render(state, input, current);
    require(changes(idle, current) == 0, "FAIL track change fabricates a note");
    acidline::display_reset(state);
    input.note_on_serial = 9;
    render(state, input, current);
    require(changes(idle, current) == 0, "FAIL reset fabricates a note");

    const opxy::i32 values[] = {0, 0x40000000, 0x7fffffff};
    for (unsigned tick = 0; tick < 600; ++tick) {
        const bool alternate = (tick / 60) % 2;
        input.reduced_motion = tick > 400;
        input.held_notes = (tick / 30) % 2;
        input.note_on_serial += tick % 60 == 30;
        for (unsigned slot = 0; slot < 8; ++slot)
            input.slots[slot] = values[(tick / 10 + slot) % 3];
        for (opxy::u8 column = 0; column < 4; ++column)
            input.shown[column] = column + (alternate ? 4 : 0);
        render(state, input, current);
        require(state.header.page == alternate, "FAIL header does not follow the control page");
    }

    std::puts("PASS eight immediate control mappings; one-node note motion and stable idle");
}
