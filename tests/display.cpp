#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
static unsigned part_node(x0x::BeaverPart part) {
    for (unsigned i = 0; i < x0x::beaver_layers; ++i)
        if (x0x::beaver_poses[i].part == part) return x0x::beaver_node+i;
    std::abort();
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
    require(frame.nodes[body_node+kick].shade == 1, "entering a track does not invent a hit");
    ++input.note_serial; display(state, input, frame);
    require(frame.nodes[body_node+kick].x == 240 * 1024 && frame.nodes[body_node+kick].shade == 9, "a real note starts a visible hit on the bass drum");
    const opxy::i32 initial = frame.nodes[moving_node+kick].scale;
    display(state, input, frame);
    require(frame.nodes[moving_node+kick].scale < initial && opxy::draw::valid(frame), "hit decays between notes");
    input.reduced_motion = 1; ++input.note_serial; display(state, input, frame);
    require(frame.nodes[body_node+kick].scale == 440 && frame.nodes[moving_node+kick].scale == 440, "reduced motion keeps static hit feedback");
    input.track = 1; display(state, input, frame);
    require(frame.nodes[body_node+kick].shade == 1, "track changes clear a previous hit");
    input.elapsed = 0x7fffffff;
    for (unsigned i = 0; i < 30; ++i) display(state, input, frame);
    require(opxy::draw::valid(frame) && frame.nodes[body_node+kick].shade == 1, "late frames settle safely");
    unsigned hashes[47], count = 0;
    for (unsigned model = 0; model < model_count; ++model) for (unsigned family = 0; family < 4; ++family) {
        if (!(family_masks[model] & (1u << family))) continue;
        input = {}; input.model = model; input.elapsed = 16;
        const unsigned raw[8] = {family_raw(model,family),64,48,64,0,64,64,0};
        for (unsigned s = 0; s < 8; ++s) input.slots[s] = opxy::i32(raw[s]<<24);
        display_reset(state); display(state,input,frame);
        require(frame.nodes[body_node+model].shade == 1,"all types have a still resting graphic");
        ++input.note_serial; display(state,input,frame);
        require(frame.nodes[body_node+model].shade == 9,"all 47 variants respond to a trigger");
        const Frame hit = frame;
        const unsigned hands = playing_hands(model,input.note_serial);
        for (unsigned hand = 0; hand < 2; ++hand) if (hands & (1u<<hand)) {
            const auto& stick = frame.nodes[part_node(hand ? beaver_right_stick : beaver_left_stick)];
            const int tx = hand ? 27 : -20, ty = -34;
            const int x = stick.x+tx*stick.cos-ty*stick.sin;
            const int y = stick.y+tx*stick.sin+ty*stick.cos;
            require(std::abs(x-beaver_hits[model].x*1024)<3*1024 && std::abs(y-beaver_hits[model].y*1024)<3*1024,
                    "the active stick contacts the playing surface for every model/style");
        }
        unsigned hash = 2166136261u;
        for (unsigned tick = 0; tick < 100; ++tick) {
            require(opxy::draw::valid(frame),"valid full animation for every type and family");
            require(frame.nodes[body_node+model].scale <= 500 && frame.nodes[body_node+model].y == 172*1024,
                    "the band emblem stays inside the bass-drum head");
            for (unsigned hand = 0; hand < 2; ++hand) {
                const auto& arm = frame.nodes[part_node(hand ? beaver_right_arm : beaver_left_arm)];
                const auto& stick = frame.nodes[part_node(hand ? beaver_right_stick : beaver_left_stick)];
                const int hx = hand ? 20 : -15, hy = hand ? -20 : -26;
                require(stick.x == arm.x+hx*arm.cos-hy*arm.sin && stick.y == arm.y+hx*arm.sin+hy*arm.cos,
                        "each rebounding stick remains attached to its paw");
            }
            if (tick < 12) for (unsigned node = body_node; node < Frame::node_count; ++node) {
                const auto& n = frame.nodes[node];
                const opxy::i32 fields[] = {n.x,n.y,n.scale,n.cos,n.sin,opxy::i32(n.shade)};
                for (auto field : fields) hash = (hash ^ unsigned(field))*16777619u;
            }
            display(state,input,frame);
        }
        for (unsigned previous = 0; previous < count; ++previous)
            require(hash != hashes[previous],"each type/style combination has its own animation");
        hashes[count++] = hash;
        const Frame rest = frame;
        display(state,input,frame);
        require(!std::memcmp(rest.nodes+body_node,frame.nodes+body_node,(Frame::node_count-body_node)*sizeof(frame.nodes[0])),"idle artwork stays still");
        ++input.note_serial; display(state,input,frame);
        require(!std::memcmp(hit.nodes+body_node,frame.nodes+body_node,(beaver_node-body_node)*sizeof(frame.nodes[0])),"retrigger restarts the emblem and impact animation");
        // Different note counters, including uint32 wrap, are all hit events.
        input.note_serial = 0xffffffffu; display(state,input,frame);
        input.note_serial = 0; display(state,input,frame);
        require(frame.nodes[body_node+model].shade == 9,"wrapped serial triggers every model");
        input.reduced_motion = 1; ++input.note_serial; display(state,input,frame);
        for (unsigned part = 0; part < 2; ++part) {
            const unsigned node = (part ? moving_node : body_node)+model;
            require(frame.nodes[node].scale == 440 && frame.nodes[node].cos == 1024 && !frame.nodes[node].sin,
                    "reduced motion has static geometry for every model");
        }
        for (unsigned layer = 0; layer < beaver_layers; ++layer) {
            const auto& n = frame.nodes[beaver_node+layer]; const auto& pose = beaver_poses[layer];
            require(n.x == pose.x*1024 && n.y == pose.y*1024 && n.scale == 1024 && n.cos == 1024 && !n.sin,
                    "reduced motion returns the entire drummer to a still pose");
        }
        ++input.track; display(state,input,frame);
        require(frame.nodes[body_node+model].shade == 1,"track entry never fabricates a trigger");
        input.reduced_motion = 0; ++input.note_serial; display(state,input,frame);
        input.model = (model+1)%model_count; display(state,input,frame);
        require(frame.nodes[body_node+input.model].shade == 1,"browsing a type does not borrow a previous hit");
        input.slots[style] = opxy::i32(unsigned(family_raw(input.model,(family+1)%4))<<24);
        display(state,input,frame);
        require(frame.nodes[body_node+input.model].shade == 1,"browsing a family stays still");
    }
    require(count == 47,"complete animated sound inventory");
    std::puts("PASS: captions, both pages, Shift config, all 47 distinct type/style trigger animations, retrigger, rest and reduced motion");
}
