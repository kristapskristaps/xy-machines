#include "display.h"
#include "vessels.h"
namespace acidline {
inline void display_reset(DisplayState& s){
    s.header.reset();s.notes.reset();s.phase=s.bloom=s.serial=s.track=s.initialized=0;
}
inline void display(DisplayState& s,const opxy::draw::Input& in,Frame& frame){
    using namespace opxy::draw;using opxy::i32;using opxy::u32;
    const char* const titles[8]={"Cutoff","Reso","Distort","Env Amt","Decay","Wave","Sub","Damp"};
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
        const i32 surface=140+levels[vial],width=vessel_width(vial,surface);
        for(u32 i=0;i<16;++i){
            const i32 angle=static_cast<i32>(i)*3600/15+phase+static_cast<i32>(vial)*1100;
            const i32 taper=sine(static_cast<i32>(i)*1800/15);
            frame.paths[vial].points[i]={(-width+static_cast<i32>(i)*width*2/15)*unit,
                levels[vial]*unit+sine(angle)*amplitude*taper/1024};
            if(vial==2){
                const Turn bend=rotation(static_cast<i32>(i)*1800/15);
                frame.paths[vial].points[16+i]={bend.cos*14,36*unit+bend.sin*14};
            }else if(i<7 || i>8){
                const i32 step=static_cast<i32>(i<7?i:15-i);
                const i32 bottom=vial==0?184:188;
                const i32 y=surface+3+(bottom-surface-3)*step/6;
                const i32 side=vessel_width(vial,y);
                frame.paths[vial].points[16+i]={(i<7?side:-side)*unit,(y-140)*unit};
            }else{
                frame.paths[vial].points[16+i]={(i==7?1:-1)*(vial==0?35:1)*unit,(vial==0?48:50)*unit};
            }
        }
        place(frame,vial,center*unit,140*unit,unit,12+vial,stroke);
        place(frame,32+vial,center*unit,(140+levels[vial])*unit,unit,0,fill_and_stroke);
        for(u32 bubble=0;bubble<6;++bubble){
            const u32 node=11+vial*6+bubble;
            if(!held && !s.bloom){hide(frame,node);continue;}
            const i32 cycle=static_cast<i32>((s.phase*(2+vial)+bubble*659+vial*271)&4095u);
            const i32 top=levels[vial]+146;
            const i32 y=180-cycle*(180-top)/4096;
            const i32 spread=clamp(vessel_width(vial,y)-6,2,28);
            const i32 x=center+(static_cast<i32>(bubble%3)-1)*spread+sine(cycle*3600/4096)/512;
            place(frame,node,x*unit,y*unit,unit,10,stroke);
        }
        if(!s.bloom || in.reduced_motion)hide(frame,29+vial);
        else place(frame,29+vial,center*unit,(70-static_cast<i32>(1024-s.bloom)/128)*unit,unit,12+vial,stroke);
    }
    // Travel along glass pipes in screen space; no audio state is consulted.
    const i32 routes[6][5][2]={
        {{44,149},{76,149},{76,66},{120,66},{120,77}},
        {{120,77},{120,64},{240,64},{240,77},{240,77}},
        {{240,77},{240,50},{360,50},{360,77},{360,77}},
        {{360,77},{400,77},{400,132},{438,132},{438,132}},
        {{438,166},{404,166},{404,202},{240,202},{240,194}},
        {{120,194},{120,202},{74,202},{74,170},{44,170}},
    };
    const u32 count[6]={5,4,4,4,5,5};
    for(u32 pipe=0;pipe<6;++pipe){
        i32 length=0;
        for(u32 part=1;part<count[pipe];++part){
            const i32 dx=routes[pipe][part][0]-routes[pipe][part-1][0],dy=routes[pipe][part][1]-routes[pipe][part-1][1];
            length+=(dx<0?-dx:dx)+(dy<0?-dy:dy);
        }
        for(u32 particle=0;particle<2;++particle){
            const u32 node=35+pipe*2+particle;
            if((!held && !s.bloom) || in.reduced_motion){hide(frame,node);continue;}
            i32 distance=static_cast<i32>((s.phase*(2+static_cast<u32>(cutoff)/256)+particle*2048+pipe*349)&4095u)*length/4096;
            for(u32 part=1;part<count[pipe];++part){
                const i32 dx=routes[pipe][part][0]-routes[pipe][part-1][0],dy=routes[pipe][part][1]-routes[pipe][part-1][1];
                const i32 span=(dx<0?-dx:dx)+(dy<0?-dy:dy);
                if(distance<=span){
                    place(frame,node,(routes[pipe][part-1][0]+dx*distance/span)*unit,
                        (routes[pipe][part-1][1]+dy*distance/span)*unit,unit,pipe>=2 && pipe<=4?13:12,fill_and_stroke);break;
                }
                distance-=span;
            }
        }
    }
    const i32 spin=static_cast<i32>(s.phase)*3600/4096;
    place(frame,47,184*unit,64*unit,unit,12,stroke);turn(frame,47,rotation(spin+reso));
    place(frame,48,300*unit,50*unit,unit,13,stroke);turn(frame,48,rotation(-spin+drive));
    place(frame,49,44*unit,141*unit,unit,15,fill_and_stroke);turn(frame,49,rotation(-700+cutoff*1400/1024+static_cast<i32>(s.bloom)/8));
    place(frame,50,438*unit,151*unit,unit,15,fill_and_stroke);turn(frame,50,rotation(-700+reso*1400/1024+static_cast<i32>(s.bloom)/8));
    for(u32 drop=0;drop<4;++drop){
        if(!s.bloom || in.reduced_motion){hide(frame,51+drop);continue;}
        const i32 age=1024-static_cast<i32>(s.bloom);
        place(frame,51+drop,(drop<2?120:360)*unit+(drop%2?1:-1)*age*10,
            75*unit-age*16+age*age/96,unit,drop<2?12:13,fill_and_stroke);
    }

}
}
