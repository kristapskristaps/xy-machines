#include "display.h"
namespace matter {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
u32 amount(i32 x) {return x<0?0u:static_cast<u32>(x)>>16;}
}
void display_reset(DisplayState& state) {state=DisplayState();}
void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    state.notes.update(input);
    const i32 elapsed=clamp(input.elapsed,0,102);
    const bool held=input.held_notes<=128 && input.held_notes;
    if(!state.initialized || state.track!=input.track)state.energy=0;
    if(state.initialized && state.track==input.track && state.serial!=input.note_serial)state.energy=1024;
    if(held && state.energy<640)state.energy=640;
    if(!held)state.energy=clamp(state.energy-elapsed*6,0,1024);
    if(!input.reduced_motion && state.energy)state.phase=(state.phase+static_cast<u32>(elapsed*3))%3600u;
    const i32 phase=input.reduced_motion?0:static_cast<i32>(state.phase);
    const i32 energy=input.reduced_motion?0:state.energy;
    const i32 geometry=static_cast<i32>(amount(input.slots[0]));
    const i32 brightness=static_cast<i32>(amount(input.slots[1]));
    const i32 damping=static_cast<i32>(amount(input.slots[2]));
    const u32 position=amount(input.slots[3]);
    const unsigned model=amount(input.slots[4])*6u>>15;
    for(unsigned row=0;row<5;++row)for(unsigned col=0;col<5;++col) {
        const i32 x=static_cast<i32>(col)-2,y=static_cast<i32>(row)-2;
        const i32 wave=energy?sine((x*x+y*y)*(110+geometry/128)-phase):0;
        const i32 taper=(3-(x<0?-x:x))*(3-(y<0?-y:y));
        const i32 z=wave*(2+brightness/4096)*energy/1024*taper/9;
        const bool pickup=row*5+col==(position*25u>>15);
        place(frame,row*5+col,240*unit+(x-y)*45*unit,120*unit+(x+y)*16*unit-z,
              pickup?1700:held?1000:700,pickup?7:held?6:4,fill_and_stroke);
    }
    for(unsigned i=0;i<4;++i)
        place(frame,25+i,(i&1?438:42)*unit,(i&2?180:63)*unit,unit,2+static_cast<unsigned>(damping>>14),stroke);
    hide(frame,held?29:30);
    place(frame,held?30:29,407*unit,35*unit,768,held?7:3,stroke);
    const char* const names[6]={"mallet","pluck","rain","bow","blow","sample"};
    text(frame,0,370*unit,201*unit,unit,names[model]);
    for(unsigned i=0;i<4;++i)header(frame,i,"");
    if(input.shown[0]==4)header(frame,0,names[model]);
    state.initialized=1;state.track=input.track;state.serial=input.note_serial;
}
}
