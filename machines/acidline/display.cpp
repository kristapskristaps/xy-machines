#include "display.h"
namespace acidline {
inline void display_reset(DisplayState& s){
    s.header.reset();s.notes.reset();s.phase=s.bloom=s.serial=s.track=s.initialized=0;
}
inline void display(DisplayState& s,const opxy::draw::Input& in,Frame& frame){
    using namespace opxy::draw;using opxy::i32;using opxy::u32;
    const char* const titles[8]={"Cutoff","Reso","Distort","Env Amt","Decay","Wave","",""};
    const opxy::u8 pages[2][4]={{0,1,2,3},{4,5,2,3}};
    char values[8][header_capacity]={};
    for(u32 i=0;i<6;++i)header_amount(values[i],header_capacity,in.slots[i]);
    page_header(s.header,in,frame,titles,values,3,pages);
    s.notes.update(in);
    const bool changed=!s.initialized || s.track!=in.track;
    if(changed){s.phase=s.bloom=0;s.serial=in.note_on_serial;s.track=in.track;s.initialized=1;}
    else if(s.serial!=in.note_on_serial){s.serial=in.note_on_serial;s.bloom=1024;}
    const u32 elapsed=static_cast<u32>(clamp(in.elapsed,0,102));
    const bool held=in.held_notes>0 && in.held_notes<=128;
    if(in.reduced_motion){s.phase=0;s.bloom=0;}
    else if(held || s.bloom)s.phase=(s.phase+elapsed)&4095u;
    s.bloom=s.bloom>elapsed*3?s.bloom-elapsed*3:0;
    const i32 cutoff=in.slots[0]>>21,reso=in.slots[1]>>21,drive=in.slots[2]>>21;
    const i32 levels[3]={-18+cutoff*24/1024,-14+reso*22/1024,-18+drive*24/1024};
    for(u32 vial=0;vial<3;++vial){
        const i32 center=120+static_cast<i32>(vial)*120;
        const i32 phase=static_cast<i32>(s.phase)*3600/4096;
        const i32 amplitude=1+(held?reso/256:0)+static_cast<i32>(s.bloom)/256;
        frame.paths[vial].key=1+static_cast<u32>(levels[vial]+32)+s.phase*128+static_cast<u32>(amplitude)*524288;
        for(u32 i=0;i<16;++i){
            const i32 angle=static_cast<i32>(i)*3600/15+phase+static_cast<i32>(vial)*1100;
            frame.paths[vial].points[i]={(-26+static_cast<i32>(i)*52/15)*unit,
                levels[vial]*unit+sine(angle)*amplitude};
            const Turn bend=rotation(static_cast<i32>(i)*1800/15);
            frame.paths[vial].points[16+i]={bend.cos*26,24*unit+bend.sin*26};
        }
        place(frame,vial,center*unit,140*unit,unit,12+vial,stroke);
        place(frame,32+vial,center*unit,(140+levels[vial])*unit,unit,0,fill_and_stroke);
        for(u32 bubble=0;bubble<6;++bubble){
            const u32 node=11+vial*6+bubble;
            if(!held && !s.bloom){hide(frame,node);continue;}
            const i32 cycle=static_cast<i32>((s.phase*(2+vial)+bubble*659+vial*271)&4095u);
            const i32 top=levels[vial]+146;
            const i32 y=180-cycle*(180-top)/4096;
            const i32 x=center-16+static_cast<i32>(bubble%3)*16+sine(cycle*3600/4096)/384;
            place(frame,node,x*unit,y*unit,unit,10,stroke);
        }
        if(!s.bloom || in.reduced_motion)hide(frame,29+vial);
        else place(frame,29+vial,center*unit,(70-static_cast<i32>(1024-s.bloom)/128)*unit,unit,12+vial,stroke);
    }
}
}
