#include "display.h"
namespace subvista {
inline void display_reset(DisplayState& state) { state.notes.reset(); state.phase=state.bloom=0; }
inline void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    using opxy::i32;
    using opxy::u32;
    const NoteEvents events=state.notes.update(input);
    const i32 elapsed=clamp(input.elapsed,0,102);
    if (!input.reduced_motion) state.phase=(state.phase+static_cast<u32>(elapsed)*2)&4095u;
    if (events.hold_started || events.note_released) state.bloom=1024;
    if (!input.reduced_motion) state.bloom=state.bloom>static_cast<u32>(elapsed)*3?state.bloom-elapsed*3:0;
    else state.bloom=0;
    const i32 cutoff=input.slots[0]>>21, reso=input.slots[1]>>21, shape=input.slots[2]>>21;
    const i32 radius=17+cutoff*27/1024+static_cast<i32>(state.bloom)*6/1024;
    for (u32 p=0;p<Frame::path_count;++p) {
        frame.paths[p].key=1+state.phase+static_cast<u32>(cutoff+reso+shape)*8192u;
        for (u32 i=0;i<64;++i) {
            Point& point=frame.paths[p].points[i];
            if (p==2) {
                const Turn angle=rotation(static_cast<i32>(i)*3600/64);
                point.x=angle.cos*radius;
                point.y=angle.sin*radius;
            } else {
                const i32 j=static_cast<i32>(i<32?i:63-i);
                const i32 x=-60+j*120/31;
                i32 y=0;
                if (p==0 || p==1) {
                    const i32 phase=(j*7200/31+static_cast<i32>(state.phase)*3600/4096)%3600;
                    const i32 saw=(phase-1800)*26/1800;
                    const i32 pulse=phase<1800?-20:20;
                    const i32 raw=saw+(pulse-saw)*shape/1024;
                    const i32 sinewave=sine(phase)*22/1024;
                    const i32 opened=cutoff*cutoff/1024;
                    y=p==0?raw:sinewave+(raw-sinewave)*opened/1024;
                    if (p==1) y=y*(24+reso/80)/24;
                } else {
                    const i32 knee=-45+cutoff*90/1024;
                    const i32 delta=x-knee;
                    y=delta<0?-9:clamp(-9+delta*3, -9, 22);
                    if (delta>-12 && delta<8) y-=reso*(12-(delta<0?-delta:delta))/1024;
                }
                point.x=x*unit;
                point.y=(y+(i<32?-1:1))*unit;
            }
        }
    }
    for (u32 i=0;i<Frame::node_count;++i) hide(frame,i);
    place(frame,0,99*unit,112*unit,unit,11,stroke);
    place(frame,1,381*unit,112*unit,unit,15,stroke);
    place(frame,2,240*unit,112*unit,unit,15,stroke);
    place(frame,3,240*unit,181*unit,1536,9,stroke);
    for (u32 i=0;i<8;++i) {
        const i32 angle=static_cast<i32>(i)*450+cutoff/3;
        const Turn t=rotation(angle);
        place(frame,4+i,240*unit+t.cos*(radius+5),112*unit+t.sin*(radius+5),unit,7,stroke);
        turn(frame,4+i,rotation(angle+450));
    }
    for (u32 i=0;i<12;++i) {
        const i32 progress=(static_cast<i32>(state.phase)+static_cast<i32>(i)*341)&4095;
        const i32 x=30+progress*420/4096;
        const i32 wave=sine(progress*7200/4096)*7/1024;
        const bool blocked=x>240 && static_cast<i32>(i%5)*9>radius-8;
        if (!blocked) place(frame,12+i,x*unit,(112+wave+static_cast<i32>(i%3)*9-9)*unit,
                            input.held_notes && !input.reduced_motion?unit:512,x<240?7:12,fill_and_stroke);
    }
    place(frame,24,(173+cutoff*135/1024)*unit,208*unit,unit,15,fill_and_stroke);
    for (u32 i=0;i<4;++i) place(frame,25+i,(207+static_cast<i32>(i)*22)*unit,52*unit,unit,
                               input.slots[i]>0x40000000?13:5,fill_and_stroke);
    place(frame,29,444*unit,42*unit,unit,input.held_notes?15:4,fill_and_stroke);
}
}
