#include "display.h"
namespace acidline {
inline void display_reset(DisplayState& s){
    s.header.reset();s.notes.reset();s.phase=s.bloom=s.serial=s.track=s.initialized=0;
}
inline void display(DisplayState& s,const opxy::draw::Input& in,Frame& frame){
    using namespace opxy::draw;using opxy::i32;using opxy::u32;
    const char* const titles[8]={"Flow","Bite","Burn","Sweep","Length","Shape","Body","Shade"};
    const opxy::u8 pages[2][4]={{0,1,2,3},{4,5,6,7}};
    char values[8][header_capacity]={};
    for(u32 i=0;i<8;++i)header_amount(values[i],header_capacity,in.slots[i]);
    const char* const waves[3]={"Saw","Tri","Square"};
    copy(values[5],header_capacity,waves[(static_cast<u32>(in.slots[5])>>16)*3>>15]);
    page_header(s.header,in,frame,titles,values,3,pages);
    s.notes.update(in);
    const bool changed=!s.initialized || s.track!=in.track;
    if(changed){s.phase=s.bloom=0;s.serial=in.note_on_serial;s.track=in.track;s.initialized=1;}
    else if(s.serial!=in.note_on_serial){s.serial=in.note_on_serial;s.bloom=1024;}
    const u32 elapsed=static_cast<u32>(clamp(in.elapsed,0,102));
    const bool held=in.held_notes>0 && in.held_notes<=128;
    const i32 flow=in.slots[0]>>21,bite=in.slots[1]>>21,burn=in.slots[2]>>21;
    const i32 intensity=(bite+burn)/2;
    if(in.reduced_motion){s.phase=0;s.bloom=0;}
    else if(held || s.bloom)s.phase=(s.phase+elapsed*(2+flow/256))&4095u;
    s.bloom=s.bloom>elapsed*3?s.bloom-elapsed*3:0;
    // Three connected membranes stretch into thin fingers. Geometry is bounded
    // and evaluated only by the display callback, never by the audio renderer.
    for(u32 sheet=0;sheet<3;++sheet){
        auto& path=frame.paths[sheet];
        path.key=1+s.phase+static_cast<u32>(intensity)*4096+s.bloom*4194304u+static_cast<u32>(burn)*8192;
        const i32 depth=34+intensity/24+static_cast<i32>(s.bloom)/32
            +sine(static_cast<i32>(s.phase)*3600/4096+static_cast<i32>(sheet)*1200)/64;
        for(u32 j=0;j<16;++j){
            const i32 y=static_cast<i32>(j)*depth/15;
            const i32 belly=j<11?2:2+sine((static_cast<i32>(j)-11)*1800/4)/128;
            const i32 bend=sine(static_cast<i32>(j)*140+static_cast<i32>(s.phase)*3600/4096
                +static_cast<i32>(sheet)*1200)*(static_cast<i32>(j)+burn/128)/2048;
            path.points[j]={(bend+belly)*unit/2,y*unit/2};
            path.points[31-j]={(bend-belly)*unit/2,y*unit/2};
        }
        place(frame,sheet,(92+static_cast<i32>(sheet)*148)*unit,44*unit,2*unit,15,stroke);
    }
    for(u32 drop=0;drop<24;++drop){
        const u32 node=11+drop;
        if(in.reduced_motion || (!held && !s.bloom) || drop>static_cast<u32>(7+intensity/64)){hide(frame,node);continue;}
        const i32 age=static_cast<i32>((s.phase+drop*613)&4095u);
        const i32 x=30+static_cast<i32>(drop)*137%420+sine(age*3600/4096)/256;
        const i32 y=86+age*age/145000;
        const i32 scale=unit*(2+static_cast<i32>(drop%3))/3;
        place(frame,node,x*unit,y*unit,scale,15,fill_and_stroke);
    }
    for(u32 ring=0;ring<20;++ring){
        const u32 node=35+ring;
        if(in.reduced_motion || (!held && !s.bloom)){hide(frame,node);continue;}
        const i32 age=static_cast<i32>((s.phase+ring*613)&4095u);
        const i32 scale=clamp(unit/4+age*(2+intensity/256)/8,unit/4,2*unit);
        place(frame,node,(30+static_cast<i32>(ring)*137%420)*unit,199*unit,scale,static_cast<u32>(12-age/512),stroke);
    }
}
}
