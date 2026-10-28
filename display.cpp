#include "display.h"
namespace matter {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
u32 amount(i32 x) {return x<0?0u:static_cast<u32>(x)>>16;}
u32 root(u32 x) {
    u32 result=0,bit=1u<<30;
    while(bit>x)bit>>=2;
    while(bit) {
        if(x>=result+bit){x-=result+bit;result=(result>>1)+bit;}
        else result>>=1;
        bit>>=2;
    }
    return result;
}
void edge(Frame& frame,unsigned node,const Point& a,const Point& b,unsigned shade) {
    const i32 dx=b.x-a.x,dy=b.y-a.y;
    const u32 length=root(static_cast<u32>(dx*dx)+static_cast<u32>(dy*dy));
    place(frame,node,a.x,a.y,static_cast<i32>(length/32),shade,stroke);
    const Turn direction={dx*unit/static_cast<i32>(length),dy*unit/static_cast<i32>(length)};
    turn(frame,node,direction);
}
}
void display_reset(DisplayState& state) {state=DisplayState();}
void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    state.notes.update(input);
    const i32 elapsed=clamp(input.elapsed,0,102);
    const bool held=input.held_notes<=128 && input.held_notes;
    if(state.initialized && state.track==input.track && state.serial!=input.note_serial)state.energy=1024;
    if(held && state.energy<640)state.energy=640;
    if(!held)state.energy=clamp(state.energy-elapsed,0,1024);
    if(!input.reduced_motion)state.phase=(state.phase+static_cast<u32>(elapsed*3))%3600u;
    const i32 phase=input.reduced_motion?0:static_cast<i32>(state.phase);
    const i32 energy=input.reduced_motion?0:state.energy;
    const i32 geometry=static_cast<i32>(amount(input.slots[0]));
    const i32 brightness=static_cast<i32>(amount(input.slots[1]));
    const i32 damping=static_cast<i32>(amount(input.slots[2]));
    const i32 position=static_cast<i32>(amount(input.slots[3]));
    const unsigned model=amount(input.slots[4])*6u>>15;
    Point points[49];
    for(unsigned row=0;row<7;++row)for(unsigned col=0;col<7;++col) {
        const i32 x=static_cast<i32>(col)-3,y=static_cast<i32>(row)-3;
        const i32 radius=x*x+y*y;
        const i32 wave=sine(radius*(100+geometry/128)-phase);
        const i32 taper=(4-(x<0?-x:x))*(4-(y<0?-y:y));
        const i32 z=wave*(3+brightness/4096)*energy/1024*taper/16;
        points[row*7+col].x=240*unit+(x-y)*24*1250;
        points[row*7+col].y=119*unit+(x+y)*24*360-z;
    }
    unsigned node=0;
    for(unsigned row=0;row<7;++row)for(unsigned col=0;col<6;++col)
        edge(frame,node++,points[row*7+col],points[row*7+col+1],held?4:3);
    for(unsigned col=0;col<7;++col)for(unsigned row=0;row<6;++row)
        edge(frame,node++,points[row*7+col],points[(row+1)*7+col],held?5:3);
    for(unsigned i=0;i<49;++i) {
        const bool pickup=i==(static_cast<unsigned>(position)*49u>>15);
        place(frame,84+i,points[i].x,points[i].y,pickup?1700:700,pickup?7:(held?6:4),fill_and_stroke);
    }
    for(unsigned i=0;i<4;++i) {
        const i32 x=i&1?438:42,y=i&2?180:63;
        place(frame,133+i,x*unit,y*unit,unit,2+static_cast<unsigned>(damping>>14),stroke);
    }
    for(unsigned i=0;i<6;++i) {
        if(i==model)place(frame,137+i,334*unit,198*unit,unit,5,stroke);
        else hide(frame,137+i);
    }
    hide(frame,held?143:144);
    place(frame,held?144:143,405*unit,35*unit,768,held?7:3,stroke);
    if(input.shown[0]==4) {
        const char* const code[6]={"MALT","PLUK","RAIN","BOW","BLOW","SAMP"};
        header(frame,0,code[model]);
    }
    state.initialized=1;state.track=input.track;state.serial=input.note_serial;
}
}
