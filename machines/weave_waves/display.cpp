#include "display.h"
#include <opxy/page_header.hpp>
namespace weave_waves {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 settle=180;
i32 ease(i32 age) {
    const i32 left=unit-clamp(age,0,settle)*unit/settle;
    return unit-left*left/unit*left/unit;
}
void oval(Frame& f,unsigned p,i32 radius,i32 phase,i32 depth) {
    auto& path=f.paths[p];u32 hash=2166136261u;
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 r=radius+floor_q10(sine(angle*3+phase)*depth);
        path.points[i]={floor_q10(sine(angle+900)*r),floor_q10(sine(angle)*r)/2};
        hash=(hash^static_cast<u32>(path.points[i].x))*16777619u;
        hash=(hash^static_cast<u32>(path.points[i].y))*16777619u;
    }
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
    const i32 pulse=unit-ease(s.pulse);

    for(unsigned i=0;i<3;++i) {
        oval(f,i,(50-static_cast<i32>(i)*10)*unit,phase+static_cast<i32>(i)*420,(2+c/4096)*unit);
        place(f,i,(218+static_cast<i32>(i)*12)*unit,(119-static_cast<i32>(i)*10)*unit,unit,i==0?5:3,stroke);
        turn(f,i,rotation(-180+a/160));
    }
    for(unsigned i=0;i<4;++i) {
        place(f,3+i,(162+static_cast<i32>(i)*35)*unit,(145-static_cast<i32>(i)*16)*unit+pulse*3,unit,i==static_cast<unsigned>(a)/8192u?4:2,stroke);
        turn(f,3+i,rotation(-180));
    }
    place(f,7,(170+b/260)*unit,(143-c/450)*unit,unit,5,fill_and_stroke);
    place(f,8,(184+d/350)*unit,185*unit,unit,4,stroke);

    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
