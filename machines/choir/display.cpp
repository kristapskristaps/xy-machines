#include "display.h"
#include <opxy/page_header.hpp>
namespace choir {
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
inline void ribbon(Frame& f,unsigned p,i32 base,i32 phase,i32 amplitude,bool folded) {
    for(unsigned i=0;i<32;++i) {
        const i32 column=static_cast<i32>(i<16?i:31-i);
        const i32 wave=folded?((column%6<3?static_cast<i32>(column%6):6-static_cast<i32>(column%6))*2*unit/3-unit):sine(column*220+phase);
        const i32 bend=folded ? floor_q10(sine(phase+column*120)*6*unit) : 0;
        const i32 y=base+floor_q10(wave*amplitude)+bend+(i<16?-5:5)*unit;
        f.paths[p].points[i]={(-62*unit+column*124*unit/15)/2,(y-123*unit)/2};
    }
    key(f,p);
}
inline void capsule(Frame& f,unsigned p,i32 width,i32 height,i32 phase,i32 depth) {
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 x=floor_q10(sine(angle+900)*width);
        const i32 y=floor_q10(sine(angle)*height);
        f.paths[p].points[i]={(x+floor_q10(sine(angle*2+phase)*depth))/2,y/2};
    }
    key(f,p);
}
inline void arch(Frame& f,unsigned p,i32 height,i32 lean) {
    for(unsigned i=0;i<32;++i) {
        i32 x,y;
        if(i<16) {const i32 angle=1800-static_cast<i32>(i)*1800/15;
            x=floor_q10(sine(angle+900)*27*unit);y=-height+27*unit-floor_q10(sine(angle)*27*unit);}
        else if(i<21) {x=27*unit;y=-height+27*unit+static_cast<i32>(i-15)*(2*height-27*unit)/5;}
        else if(i<27) {x=27*unit-static_cast<i32>(i-20)*54*unit/6;y=height;}
        else {x=-27*unit;y=height-static_cast<i32>(i-26)*(2*height-27*unit)/5;}
        f.paths[p].points[i]={(x+floor_q10(sine(y/20)*lean))/2,y/2};
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

    // Five nested apertures breathe independently, like a harmonic sculpture.
    for(unsigned i=0;i<5;++i) {
        const i32 voice=static_cast<i32>(i),order=(voice+b/6554)%5;
        const i32 breathe=floor_q10(sine(phase+order*720)*(energy*7));
        const i32 root=i==2?0:d/1800;
        const i32 height=(61+a/3277+order*2-root)*unit+breathe+pulse*4;
        const i32 centre=(74+voice*83)*unit+(voice-2)*(b-16384)/5;
        if(i<3) {arch(f,i,height,(c/4000)*unit);place(f,i,centre,123*unit,2*unit,4,stroke);}
        else place(f,3+i-3,centre,123*unit,height/72,4,stroke);
        for(unsigned ring=0;ring<3;++ring) {
            const unsigned node=5+i*3+ring;
            place(f,node,centre,123*unit,clamp(800-static_cast<i32>(ring)*170+breathe/26+a/160-root*12,140,1080),4,stroke);
        }
    }
    (void)a;(void)d;

    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
