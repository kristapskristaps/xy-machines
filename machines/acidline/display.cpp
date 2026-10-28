#include "display.h"
namespace acidline {
inline void display_reset(DisplayState& s){s.notes.reset();s.phase=s.bloom=0;}
inline void display(DisplayState& s,const opxy::draw::Input& in,Frame& frame){
    using namespace opxy::draw;using opxy::i32;using opxy::u32;
    const NoteEvents events=s.notes.update(in);
    const u32 elapsed=static_cast<u32>(clamp(in.elapsed,0,102));
    if(!in.reduced_motion)s.phase=(s.phase+elapsed)&4095u;
    if(events.hold_started)s.bloom=1024;
    s.bloom=s.bloom>elapsed*4?s.bloom-elapsed*4:0;
    const i32 drive=in.slots[2]>>21,wave=in.slots[5]>>21,cutoff=in.slots[0]>>21;
    frame.paths[0].key=1+s.phase+static_cast<u32>(drive+wave+cutoff)*8192;
    for(u32 i=0;i<64;++i){
        const i32 j=static_cast<i32>(i<32?i:63-i);
        const i32 phase=(j*7200/31+(in.reduced_motion?0:static_cast<i32>(s.phase)*3600/4096))%3600;
        const i32 saw=(phase-1800)*20/1800,pulse=phase<1800?-19:19;
        const i32 raw=saw+(pulse-saw)*wave/1024;
        const i32 clean=sine(phase)*17/1024+(raw-sine(phase)*17/1024)*cutoff/1024;
        const i32 clipped=clamp(clean*(1024+drive*7)/1024,-23,23);
        frame.paths[0].points[i].x=(-62+j*124/31)*unit;
        frame.paths[0].points[i].y=(clipped+(i<32?-1:1))*unit;
    }
    place(frame,0,240*unit,134*unit,2*unit,15,stroke);
    for(u32 i=0;i<16;++i)
        place(frame,1+i,(33+static_cast<i32>(i)*26)*unit,185*unit,unit,
              static_cast<i32>(i)*64<=drive?15:3,fill_and_stroke);
    place(frame,17,443*unit,52*unit,unit,in.held_notes?15:4,fill_and_stroke);
}
}
