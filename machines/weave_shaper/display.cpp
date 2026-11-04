#include "display.h"
#include <opxy/page_header.hpp>
namespace weave_shaper {
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

    const i32 height=(42+b/5000)*unit;
    for(unsigned p=0;p<3;++p) {
        const i32 step=static_cast<i32>(p);
        plane(f,p,height,(c/7000-2)*unit,(a/3200-5)*unit);
        place(f,p,(112+step*128)*unit,123*unit,1380+pulse/18,p==1?5:(p==0?2:4),p==1?stroke:fill_and_stroke);
        turn(f,p,rotation((step-1)*(40+c/400)+sine(phase)*energy/(unit*30)));
    }
    for(unsigned i=0;i<8;++i) {
        const i32 at=static_cast<i32>(i);
        place(f,12+i,(43+at*56)*unit,123*unit,1300,(i==static_cast<unsigned>(d)/4096u)?5:3,stroke);
        turn(f,12+i,rotation(i%2?120+a/180:-120-a/180));
    }

    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
