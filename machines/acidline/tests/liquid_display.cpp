#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool pass,const char* why){if(!pass){std::puts(why);std::exit(1);}}
static acidline::Frame scene(unsigned slot,opxy::i32 value){
    acidline::DisplayState state={};acidline::Frame frame={};opxy::draw::Input in={};
    for(auto& parameter:in.slots)parameter=0x40000000;
    in.slots[slot]=value;in.elapsed=34;in.held_notes=1;
    for(unsigned tick=0;tick<90;++tick){
        in.note_serial=tick?1:0;
        acidline::display(state,in,frame);
        require(opxy::draw::valid(frame),"FAIL liquid display exceeds native frame limits");
    }
    return frame;
}
int main(){
    for(unsigned slot=0;slot<8;++slot){
        const auto low=scene(slot,0),high=scene(slot,0x7fffffff);
        // Exclude header geometry and text: the actual surface,
        // particles or highlights must change, rather than just a label.
        require(std::memcmp(low.nodes+8,high.nodes+8,sizeof(opxy::draw::Node)*(acidline::Frame::node_count-8))!=0,
            "FAIL a sound parameter has no visible effect on liquid geometry or shading");
        std::printf("PASS parameter %u changes the liquid\n",slot);
    }
    std::puts("PASS all eight visual mappings across 1440 active frames");
}
