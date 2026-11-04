#include "display.h"
namespace acidline {
namespace {
using opxy::i32;using opxy::u32;
using namespace opxy::draw;
struct Liquid {
    i32 phase,bite,burn,sweep,length,shape,body,shade,pulse;
    i32 angle(i32 x) const {return x*(7+bite/256)-phase;}
    i32 center_q10(i32 x) const {
        const i32 a=angle(x),wave=sine(a);
        const i32 folded=shape==2?wave*wave/1024*(wave<0?-1:1):wave;
        return 132*unit+folded*(8+sweep/64+pulse/128)+sine(a*2+phase/2)*(shape==1?12:4);
    }
    i32 center(i32 x) const {return center_q10(x)/unit;}
    i32 radius_q10(i32 x) const {
        const i32 fold=sine(x*(10+bite/128-length/256)-phase*2);
        return (16+body/96)*unit+(fold+1024)*(9+bite/128+pulse/128)/2;
    }
    i32 radius(i32 x) const {return radius_q10(x)/unit;}
    i32 twist(i32 x) const {return sine(angle(x)+900)*(650+bite/2)/1024;}

};
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
    const i32 shape=static_cast<i32>((static_cast<u32>(in.slots[5])>>16)*3>>15);
    copy(values[5],header_capacity,waves[shape]);
    page_header(s.header,in,frame,titles,values,0,pages);
    s.notes.update(in);
    const bool changed=!s.initialized || s.track!=in.track;
    if(changed){s.phase=s.bloom=0;s.serial=in.note_on_serial;s.track=in.track;s.initialized=1;}
    else if(s.serial!=in.note_on_serial){s.serial=in.note_on_serial;s.bloom=1024;}
    const u32 elapsed=static_cast<u32>(clamp(in.elapsed,0,102));
    const bool held=in.held_notes>0 && in.held_notes<=128;
    const i32 flow=in.slots[0]>>21,length=in.slots[4]>>21;
    if(in.reduced_motion){s.phase=0;s.bloom=0;}
    else if(held || s.bloom)s.phase=(s.phase+elapsed*(1+flow/256))&65535u;
    const u32 fade=elapsed*static_cast<u32>(5-length/256);
    s.bloom=s.bloom>fade?s.bloom-fade:0;
    const Liquid liquid={static_cast<i32>(s.phase)*3600/8192,in.slots[1]>>21,in.slots[2]>>21,
        in.slots[3]>>21,length,shape,in.slots[6]>>21,in.slots[7]>>21,static_cast<i32>(s.bloom)};
    // Continuous cubic surfaces carry eight nested layers of diffuse and
    // specular reflection. Layers paint back-to-front across all six folds.
    const i32 base_light[8]={2,3,4,6,8,11,5,14};
    for(u32 fold=0;fold<6;++fold){
        const i32 x=40+static_cast<i32>(fold)*80;
        const i32 r=liquid.radius_q10(x),twist=liquid.twist(x);
        const i32 scale=clamp(unit/2+r/64,3*unit/4,3*unit/2);
        const i32 slope=clamp((liquid.center_q10(x+2)-liquid.center_q10(x-2))*143/unit,-350,350);
        const i32 depth=sine(twist+900);
        for(u32 layer=0;layer<8;++layer){
            const i32 reflection=layer>=4?(depth+1024)*(2+liquid.burn/256)/2048:0;
            const i32 shade=clamp(base_light[layer]+reflection-liquid.shade/256,1,15);
            const i32 shear=layer>=4?sine(twist+static_cast<i32>(layer)*150)/2:0;
            const u32 node=8+layer*6+fold;
            place(frame,node,x*unit,liquid.center_q10(x)+shear,scale,static_cast<u32>(shade),fill_and_stroke);
            turn(frame,node,rotation(slope+sine(liquid.phase+static_cast<i32>(fold)*600)*liquid.sweep/8192));
        }
    }
    // Small pearl cores have a separate moving specular point, rather than
    // outlined particle rings. Their volume follows Body.
    for(u32 bead=0;bead<24;++bead){
        const i32 group=static_cast<i32>(bead/4),member=static_cast<i32>(bead%4);
        const i32 orbit=member*900+liquid.phase*2+group*431;
        const i32 x=24+group*78+sine(orbit)*10/1024;
        const i32 depth=sine(orbit+900);
        const i32 y=liquid.center(x)+sine(orbit)*(4+liquid.body/128)/1024;
        const i32 scale=unit/2+(depth+1024)*(unit/2+liquid.body/2)/2048;
        place(frame,56+bead,x*unit,y*unit,scale,static_cast<u32>(5+(depth+1024)*5/2048),fill_and_stroke);
        place(frame,80+bead,x*unit-scale,y*unit-scale,scale/2,14,fill_and_stroke);
    }
    for(u32 bubble=0;bubble<12;++bubble){
        const i32 age=static_cast<i32>((s.phase+bubble*2731)&8191u);
        const i32 x=12+static_cast<i32>(bubble)*137%456;
        const i32 spread=22+age*(1+length/256)/400;
        const i32 y=clamp(liquid.center(x)+(bubble%2?spread:-spread),47,212);
        place(frame,104+bubble,x*unit,y*unit,unit/2,static_cast<u32>(5+bubble%5),stroke);
    }
}
}
