#include "../display.cpp"
#include <cstring>
#include <cstdio>
int main() {
    weave::DisplayState state={};weave::Frame frame={};
    opxy::draw::Input input={};input.elapsed=34;
    weave::display_reset(state);
    unsigned rng=7;
    for(unsigned test=0;test<3000;++test) {
        for(auto& value:input.slots)value=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        for(unsigned i=0;i<4;++i)input.parameters[i]=input.slots[i];
        input.held_notes=test%5;input.note_serial=test/11;input.track=test/101;
        input.elapsed=test%3?34:2147483647;
        weave::display(state,input,frame);
        for(const auto& node:frame.nodes) {
            if(node.scale<=0 || node.shade>=weave::Frame::palette_count)return 1;
            if(node.x< -65536 || node.x>655360 || node.y< -65536 || node.y>327680)return 1;
        }
        for(unsigned path=0;path<weave::Frame::path_count;++path)
            for(const auto& point:frame.paths[path].points)
                if(point.x< -65536 || point.x>65536 || point.y< -65536 || point.y>65536)return 1;
    }
    input.reduced_motion=1;input.held_notes=0;input.elapsed=34;
    weave::Frame next={};frame=weave::Frame();
    weave::display(state,input,frame);
    input.note_serial+=1;weave::display(state,input,next);
    if(std::memcmp(&frame,&next,sizeof frame))return 1;
    std::puts("PASS: display bounds, control motion and reduced-motion stability");
}
