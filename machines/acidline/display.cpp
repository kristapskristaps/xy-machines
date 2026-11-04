#include "display.h"
namespace acidline {
namespace {
using opxy::i32;using opxy::u32;
using namespace opxy::draw;
i32 center(i32 x,i32 phase,i32 energy){
    return 132+sine(x*9-phase)*(12+energy/64)/1024+sine(x*19+phase/2)*5/1024;
}
i32 radius(i32 x,i32 phase,i32 energy){
    return 12+(sine(x*15-phase*2)+1024)*(14+energy/96)/2048;
}
}
inline void display_reset(DisplayState& s){
    s.header.reset();s.notes.reset();s.phase=s.bloom=s.serial=s.track=s.initialized=0;
}
inline void display(DisplayState& s,const opxy::draw::Input& in,Frame& frame){
    const char* const titles[8]={"Flow","Bite","Burn","Sweep","Length","Shape","Body","Shade"};
    const opxy::u8 pages[2][4]={{0,1,2,3},{4,5,6,7}};
    char values[8][header_capacity]={};
    for(u32 i=0;i<8;++i)header_amount(values[i],header_capacity,in.slots[i]);
    const char* const waves[3]={"Saw","Tri","Square"};
    copy(values[5],header_capacity,waves[(static_cast<u32>(in.slots[5])>>16)*3>>15]);
    page_header(s.header,in,frame,titles,values,4,pages);
    s.notes.update(in);
    const bool changed=!s.initialized || s.track!=in.track;
    if(changed){s.phase=s.bloom=0;s.serial=in.note_on_serial;s.track=in.track;s.initialized=1;}
    else if(s.serial!=in.note_on_serial){s.serial=in.note_on_serial;s.bloom=1024;}
    const u32 elapsed=static_cast<u32>(clamp(in.elapsed,0,102));
    const bool held=in.held_notes>0 && in.held_notes<=128;
    const i32 flow=in.slots[0]>>21,bite=in.slots[1]>>21,burn=in.slots[2]>>21;
    if(in.reduced_motion){s.phase=0;s.bloom=0;}
    else if(held || s.bloom)s.phase=(s.phase+elapsed*(1+flow/256))&65535u;
    s.bloom=s.bloom>elapsed*3?s.bloom-elapsed*3:0;
    const i32 phase=static_cast<i32>(s.phase)*3600/8192;
    const i32 energy=(bite+burn+static_cast<i32>(s.bloom))/3;
    // Closed, thin ribbons carry the two fluid silhouette edges.
    // Two halves keep every mutable point within the public 64-pixel extent.
    for(u32 layer=0;layer<4;++layer){
        auto& path=frame.paths[layer];
        path.key=1+s.phase+static_cast<u32>(energy)*65536u;
        const i32 half=static_cast<i32>(layer%2),side=layer<2?-1:1;
        for(u32 j=0;j<32;++j){
            const i32 x=half*240+static_cast<i32>(j)*240/31;
            const i32 y=center(x,phase,energy),r=radius(x,phase,energy);
            const i32 fold=side*r+sine(x*27-phase)*r/6144;
            path.points[j]={(x-half*240-120)*unit/2,(y-132+fold)*unit/2};
            path.points[63-j]={(x-half*240-120)*unit/2,(y-132+fold)*unit/2+unit/4};
        }
        place(frame,layer,(120+half*240)*unit,132*unit,2*unit,11,stroke);
    }
    // Rotating cross sections form a translucent, continuously folding mesh.
    for(u32 rib=0;rib<64;++rib){
        const i32 x=static_cast<i32>(rib)*480/63;
        const i32 y=center(x,phase,energy),r=radius(x,phase,energy);
        const i32 twist=x*15-phase*2;
        const i32 depth=sine(twist);
        const i32 scale=clamp(r*unit/24,unit/3,2*unit);
        place(frame,12+rib,x*unit,y*unit,scale,static_cast<u32>(rib%9==0?12:3+(depth+1024)*5/2048),stroke);
        turn(frame,12+rib,rotation(sine(x*9-phase+900)*(18+burn/96)/64));
    }
    // Dense beads travel in six clusters through the core of the membrane.
    for(u32 bead=0;bead<36;++bead){
        const i32 group=static_cast<i32>(bead/6),member=static_cast<i32>(bead%6);
        const i32 orbit=member*600+phase*2+group*431;
        const i32 x=24+group*78+sine(orbit)*9/1024;
        const i32 depth=sine(orbit+900);
        const i32 y=center(x,phase,energy)+sine(orbit)*(6+energy/96)/1024;
        const i32 scale=unit/2+(depth+1024)*(unit+energy/2)/2048;
        place(frame,76+bead,x*unit,y*unit,scale,static_cast<u32>(7+(depth+1024)*8/2048),fill_and_stroke);
    }
    for(u32 bubble=0;bubble<24;++bubble){
        const i32 age=static_cast<i32>((s.phase+bubble*2731)&8191u);
        const i32 x=12+static_cast<i32>(bubble)*137%456;
        const i32 side=bubble%2?1:-1;
        const i32 spread=18+age/160;
        const i32 y=clamp(center(x,phase,energy)+side*spread,47,212);
        const i32 scale=unit/3+static_cast<i32>(bubble%4)*unit/4;
        place(frame,112+bubble,x*unit,y*unit,scale,static_cast<u32>(3+(bubble*7)%8),stroke);
    }
}
}
