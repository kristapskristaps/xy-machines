#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void require(bool value,const char* message){if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
    static_assert(sizeof(bitflip::DisplayState)==36,"declared display storage");
    bitflip::DisplayState state={};bitflip::Frame frame={};opxy::draw::Input input={};
    input.elapsed=17;input.slots[1]=input.slots[6]=input.slots[7]=2147483647;input.slots[3]=1073741824;
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i);
    bitflip::display_reset(state);bitflip::display(state,input,frame);
    require(opxy::draw::valid(frame),"valid initial display");
    require(state.age==154,"activation establishes a note baseline");
    require(!std::strcmp(frame.header[0],"PLS")&&!std::strcmp(frame.header[3],"+0"),"main header has model and neutral sweep");
    const auto idle=frame;for(unsigned i=0;i<100;++i)bitflip::display(state,input,frame);
    require(!std::memcmp(&idle,&frame,sizeof(frame)),"idle screen stays still");
    ++input.note_serial;input.held_notes=1;bitflip::display(state,input,frame);
    require(state.age==0&&frame.nodes[9].y<126*1024,"notes bounce the pixel character");
    for(unsigned i=0;i<10;++i)bitflip::display(state,input,frame);
    require(state.age==154,"motion settles within 0.15 seconds");
    const auto held=frame;for(unsigned i=0;i<100;++i)bitflip::display(state,input,frame);
    require(!std::memcmp(&held,&frame,sizeof(frame)),"held notes do not start an animation clock");
    input.reduced_motion=1;++input.note_serial;bitflip::display(state,input,frame);
    require(state.age==154&&frame.nodes[11].x==640*1024&&frame.nodes[9].shade==6,"reduced motion suppresses bounce and sparkles");
    input.reduced_motion=0;++input.track;++input.note_serial;bitflip::display(state,input,frame);
    require(state.age==154,"track changes establish a fresh baseline");
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i+4);
    input.reduced_motion=1;bitflip::display(state,input,frame);
    require(!std::strcmp(frame.header[2],"16")&&!std::strcmp(frame.header[3],"44.1k"),"DAC captions have real bit depth and sample rate");
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i);
    input.slots[3]=2147483647;bitflip::display(state,input,frame);
    require(!std::strcmp(frame.header[3],"+24"),"full Sweep displays its actual range");
    unsigned random=7139;
    for(unsigned tick=0;tick<4096;++tick){
        for(auto& value:input.slots)value=static_cast<int>((random=random*1664525u+1013904223u)>>1);
        for(unsigned col=0;col<4;++col)input.shown[col]=static_cast<opxy::u8>(col+(tick%2)*4);
        input.elapsed=tick%257;input.reduced_motion=tick%17==0;if(tick%7==0)++input.note_serial;
        bitflip::display(state,input,frame);require(opxy::draw::valid(frame),"all model/control/page/note frames remain bounded");
        long long area=0;for(unsigned i=0;i<64;++i){const auto& a=frame.paths[0].points[i];const auto& b=frame.paths[0].points[(i+1)%64];area+=static_cast<long long>(a.x)*b.y-static_cast<long long>(a.y)*b.x;}
        require(area!=0,"native waveform is a nondegenerate ribbon");
    }
    std::puts("PASS: real-unit captions, both pages, stationary poses, note response, track changes, reduced motion and 4,096 valid ribbon frames");
}
