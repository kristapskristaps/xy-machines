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
inline void key(Frame& f,unsigned p) {
    u32 hash=2166136261u;
    for(const auto& point:f.paths[p].points) {
        hash=(hash^static_cast<u32>(point.x))*16777619u;
        hash=(hash^static_cast<u32>(point.y))*16777619u;
    }
    f.paths[p].key=hash?hash:1;
}
inline void orbit(Frame& f,unsigned p,i32 lobes,i32 phase,i32 depth) {
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 radius=51*unit+floor_q10(sine(angle*lobes+phase)*depth);
        f.paths[p].points[i]={floor_q10(sine(angle+900)*radius),floor_q10(sine(angle)*radius)};
    }
    key(f,p);
}
inline void plane(Frame& f,unsigned p,i32 height,i32 shear,i32 tilt) {
    const opxy::draw::Point corners[4]={{-56*unit+shear,-height},{56*unit+shear,-height+tilt},
                            {56*unit-shear,height},{-56*unit-shear,height-tilt}};
    for(unsigned edge=0;edge<4;++edge)for(unsigned i=0;i<8;++i) {
        const auto& a=corners[edge];const auto& b=corners[(edge+1)%4];
        f.paths[p].points[edge*8+i]={a.x+(b.x-a.x)*static_cast<i32>(i)/8,
                                    a.y+(b.y-a.y)*static_cast<i32>(i)/8};
    }
    key(f,p);
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

    const i32 left=132*unit+pulse*9,right=348*unit-pulse*9;
    for(unsigned p=0;p<3;++p) {
        orbit(f,p,2+a/8192,phase*(p==1?-1:1)+c/12+(p?900:0),(1+b/5000)*unit);
        place(f,p,p==0?left:p==1?right:240*unit,123*unit,p==2?1050:1450,p==2?5:4,stroke);
    }
    place(f,3,left,123*unit,1450,1,fill_and_stroke);
    place(f,4,right,123*unit,1450,2,fill_and_stroke);
    for(unsigned i=0;i<8;++i) {
        const bool second=i>=4;
        place(f,5+i,second?right:left,123*unit,1350,i%2?3:4,fill_and_stroke);
        turn(f,5+i,rotation(static_cast<i32>(i%4)*450+(second?-phase-c/14:phase)));
    }
    place(f,13,(218+d/750)*unit,123*unit,1100+energy/5,5,fill_and_stroke);
    turn(f,13,rotation(phase+a/30));

    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
