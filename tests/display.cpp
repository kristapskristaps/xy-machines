#include "../display.cpp"
#include <cstring>
#include <cstdio>

static bool check(const weave::Frame& frame) {
    if (!opxy::draw::valid(frame)) return false;
    for (const auto& path : frame.paths) {
        long long area = 0;
        for (unsigned i = 0; i < weave::Frame::point_count; ++i) {
            const auto& a = path.points[i];
            const auto& b = path.points[(i + 1) % weave::Frame::point_count];
            area += static_cast<long long>(a.x) * b.y - static_cast<long long>(b.x) * a.y;
            const int x = 240 * 1024 + a.x * 2, y = 113 * 1024 + a.y * 2;
            if (x < 110 * 1024 || x > 370 * 1024 || y < 25 * 1024 || y > 190 * 1024) return false;
        }
        if (area < 1024LL * 1024 * 40) return false;
    }
    return true;
}
int main() {
    weave::DisplayState state={}; weave::Frame frame={};
    opxy::draw::Input input={}; input.elapsed=34;
    weave::display_reset(state);
    unsigned rng=7;
    for (unsigned test=0; test<3000; ++test) {
        for (auto& value:input.slots) value=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        for (unsigned i=0; i<4; ++i) input.parameters[i]=input.slots[i];
        input.held_notes=test%5; input.note_serial=test/11; input.track=test/101;
        input.note_off_serial=test/7;
        input.elapsed=test%3?34:2147483647;
        weave::display(state,input,frame);
        if (!check(frame)) {std::printf("FAIL: bounds/area at frame %u\n",test);return 1;}
    }
    // Entering or changing tracks establishes a baseline, including a held note.
    input=opxy::draw::Input(); input.elapsed=34; input.held_notes=1; input.note_serial=99;
    for (unsigned i=1; i<4; ++i) input.slots[i]=1073741824;
    weave::display_reset(state); weave::display(state,input,frame);
    if (frame.nodes[16].x!=opxy::draw::hidden_x*1024 || state.pulse_age!=weave::settle) return 1;
    input.track=1; ++input.note_serial; weave::display(state,input,frame);
    if (state.pulse_age!=weave::settle) return 1;
    // A new note pulses immediately; a serial wrap is also a new event.
    ++input.note_serial; weave::display(state,input,frame);
    if (state.pulse_age || frame.nodes[16].x!=240*1024 || state.energy!=1024) return 1;
    input.note_serial=0xffffffffu; weave::display(state,input,frame);
    input.note_serial=0; weave::display(state,input,frame); if (state.pulse_age) return 1;
    // Held animation actually changes geometry and carrier positions.
    weave::Frame prior=frame;
    for (unsigned i=0; i<4; ++i) weave::display(state,input,frame);
    if (!std::memcmp(frame.paths,prior.paths,sizeof frame.paths) ||
        (frame.nodes[4].x==prior.nodes[4].x && frame.nodes[4].y==prior.nodes[4].y)) return 1;
    // Release settles in 0.2 s and then leaves every frame/cache key unchanged.
    input.held_notes=0; ++input.note_off_serial;
    for (unsigned i=0; i<6; ++i) weave::display(state,input,frame);
    if (state.energy || frame.nodes[4].x!=opxy::draw::hidden_x*1024) return 1;
    prior=frame;
    for (unsigned i=0; i<30; ++i) weave::display(state,input,frame);
    if (std::memcmp(&frame,&prior,sizeof frame)) return 1;
    // All fifteen models have different settled poses and stay inside the canvas.
    opxy::draw::Point poses[15][64]={};
    for (unsigned model=0; model<15; ++model) {
        input.slots[0]=static_cast<opxy::i32>((2*model+1)*2147483647LL/30);
        for (unsigned i=0; i<7; ++i) weave::display(state,input,frame);
        if (!check(frame) || std::strcmp(frame.texts[1].chars,weave::names[model])) return 1;
        std::memcpy(poses[model],frame.paths[0].points,sizeof poses[model]);
        for (unsigned previous=0; previous<model; ++previous)
            if (!std::memcmp(poses[previous],poses[model],sizeof poses[model])) return 1;
    }
    // Each sound control reshapes the loom; alternate Output keeps the stock header.
    for (unsigned control=1; control<5; ++control) {
        prior=frame;
        input.slots[control]=2147483647;
        for (unsigned i=0; i<7; ++i) weave::display(state,input,frame);
        if (!std::memcmp(prior.paths[0].points,frame.paths[0].points,sizeof poses[0]) || !check(frame)) return 1;
    }
    input.shown[0]=4; weave::display(state,input,frame); if (frame.header[0][0]) return 1;
    input.shown[0]=0; weave::display(state,input,frame); if (std::strcmp(frame.header[0],"6OP")) return 1;
    // Reduced motion has a static held pose and no pulse, travel, or serial flicker.
    input.reduced_motion=1; input.held_notes=1;
    weave::display(state,input,frame); prior=frame;
    for (unsigned i=0; i<30; ++i) {++input.note_serial;weave::display(state,input,frame);}
    if (std::memcmp(&frame,&prior,sizeof frame) || state.phase || state.travel) return 1;
    // A 60 Hz host gets about 20 geometry updates per second, with smooth carriers.
    input.reduced_motion=0; input.elapsed=17; input.slots[0]=0;
    weave::display_reset(state); weave::display(state,input,frame);
    unsigned outlines=0,carriers=0;
    for (unsigned i=0; i<180; ++i) {
        prior=frame; weave::display(state,input,frame);
        if (prior.paths[0].key!=frame.paths[0].key) ++outlines;
        if (prior.nodes[4].x!=frame.nodes[4].x || prior.nodes[4].y!=frame.nodes[4].y) ++carriers;
    }
    if (outlines<45 || outlines>62 || carriers<160) return 1;
    std::printf("PASS: %zu-byte state; bounds/area, 15 distinct poses, note/track baselines, pulses, finite release, idle cache, controls and reduced motion; %u/180 geometry updates, %u carrier updates\n",sizeof(state),outlines,carriers);
}
