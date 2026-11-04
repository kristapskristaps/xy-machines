#include "display.h"
#include <opxy/page_header.hpp>
namespace weave_analog {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 settle=180;
i32 ease(i32 age) {
    const i32 left=unit-clamp(age,0,settle)*unit/settle;
    return unit-left*left/unit*left/unit;
}
void ribbon(Frame& f,unsigned p,i32 amplitude,i32 cycles,i32 phase,i32 skew=0) {
    auto& path=f.paths[p];
    for(unsigned i=0;i<16;++i) {
        const i32 x=-54*unit+static_cast<i32>(i)*108*unit/15;
        const i32 y=floor_q10(sine(static_cast<i32>(i)*3600*cycles/15+phase)*amplitude)+x*skew/32768;
        path.points[i]={x,y-768};path.points[31-i]={x,y+768};
    }
    u32 hash=2166136261u;
    for(const auto& point:path.points)hash=((hash^static_cast<u32>(point.x))*16777619u^static_cast<u32>(point.y))*16777619u;
    path.key=hash?hash:1;
}

}
void display_reset(DisplayState& state) {state=DisplayState();}
void display(DisplayState& s,const opxy::draw::Input& in,Frame& f) {
    using namespace opxy::draw;
    const bool snapshot=!s.initialized || s.track!=in.track;
    const bool held=in.held_notes>0 && in.held_notes<=128;
    const bool hit=!snapshot && in.track<8 && s.serial!=in.note_serial;
    s.notes.update(in);
    const i32 dt=clamp(in.elapsed,0,102);
    i32 target[4];for(unsigned i=0;i<4;++i)target[i]=in.slots[i]<0?0:in.slots[i]>>16;
    if(snapshot || in.reduced_motion) {
        s.phase=0;s.pulse=settle;s.knob_age=settle;s.energy=held?unit:0;
        for(unsigned i=0;i<4;++i)s.values[i]=s.from[i]=s.target[i]=target[i];
    } else {
        bool changed=false;for(unsigned i=0;i<4;++i)changed=changed || target[i]!=s.target[i];
        if(changed){for(unsigned i=0;i<4;++i){s.from[i]=s.values[i];s.target[i]=target[i];}s.knob_age=0;}
        s.knob_age=clamp(s.knob_age+dt,0,settle);
        const i32 mix=ease(s.knob_age);
        for(unsigned i=0;i<4;++i)s.values[i]=s.from[i]+(s.target[i]-s.from[i])*mix/unit;
        s.pulse=hit?0:clamp(s.pulse+dt,0,settle);
        s.energy=held?unit:clamp(s.energy-dt*unit/settle,0,unit);
        if(held)s.phase=(s.phase+static_cast<u32>(dt)*2u)%3600u;
    }
    for(unsigned i=0;i<Frame::node_count;++i)hide(f,i);
    for(unsigned i=0;i<4;++i)header_amount(f.header[i],header_capacity,in.slots[i]);
    const i32 a=s.values[0],b=s.values[1],c=s.values[2],d=s.values[3];
    const i32 phase=static_cast<i32>(s.phase);
    const i32 pulse=unit-ease(s.pulse),energy=s.energy;

    ribbon(f,0,(8+b/2048)*unit,1+a/10923,phase);
    ribbon(f,1,(8+b/2048)*unit,1+a/10923,phase+c/12);
    ribbon(f,2,(4+energy/160)*unit,2,phase);
    place(f,0,132*unit,126*unit,unit,4,stroke);
    place(f,1,348*unit,126*unit,unit,4,stroke);
    place(f,2,240*unit,183*unit,unit,5,stroke);
    for(unsigned i=0;i<2;++i) {
        place(f,3+i,(i?348:132)*unit,(73*unit+pulse*4),unit,4,stroke);
        turn(f,3+i,rotation(i?-phase-c/20:phase));
    }
    place(f,5,(206+c/500)*unit,75*unit,unit,5,fill_and_stroke);
    place(f,6,240*unit,(153+d/2048)*unit,unit,held?5:2,fill_and_stroke);

    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
