#include "display.h"
#include "visual_tables.h"
#include "control_tables.h"

namespace bitflip {
namespace {
using namespace opxy::draw;
const char* const model_names[8] = {"PLS","TRI","TWIN","WAVE","FM","MET","NOIS","XOR"};
const char* const chord_names[8] = {"off","oct","major","minor","sus","maj7","min7","power"};
const unsigned shades[8] = {3,5,3,4,7,7,6,4};
const unsigned steps[8][4] = {{0,0,0,0},{0,12,0,12},{0,4,7,12},{0,3,7,12},
    {0,5,7,12},{0,4,7,11},{0,3,7,10},{0,7,12,19}};
inline unsigned amount(opxy::i32 value) { return value < 0 ? 0u : static_cast<unsigned>(value) >> 16; }
void number(char* out, unsigned value) {
    char digits[8]; unsigned n=0;
    do { digits[n++]=static_cast<char>('0'+value%10); value/=10; } while(value && n<7);
    for(unsigned i=0;i<n;++i) out[i]=digits[n-i-1];
    out[n]=0;
}
void rate(char* out, unsigned tenths, const char* suffix) {
    number(out,tenths/10); unsigned at=0;while(out[at])++at;
    out[at++]='.';out[at++]=static_cast<char>('0'+tenths%10);
    for(unsigned i=0;suffix[i] && at<7;++i)out[at++]=suffix[i];
    out[at]=0;
}

}
inline void display_reset(DisplayState& state) { state.header.reset();state.initialized=0;state.age=154; }
inline void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    const unsigned model=amount(input.slots[0])>>12, chord=amount(input.slots[4])>>12;
    const unsigned color=amount(input.slots[1]),edge=amount(input.slots[2]);
    const char* const titles[8]={"Model","Color","Edge","Sweep","Chord","Speed","Bits","Clock"};
    char values[8][header_capacity]={};
    copy(values[0],header_capacity,model_names[model]);
    header_amount(values[1],header_capacity,input.slots[1]);
    header_amount(values[2],header_capacity,input.slots[2]);
    const opxy::i32 semitones=static_cast<opxy::i32>((amount(input.slots[3])*48+16384)>>15)-24;
    values[3][0]=semitones<0?'-':'+';number(values[3]+1,static_cast<unsigned>(semitones<0?-semitones:semitones));
    copy(values[4],header_capacity,chord_names[chord]);
    rate(values[5],(static_cast<unsigned>(tables::arp_rate[amount(input.slots[5])>>12])+4869)/9739,"Hz");
    number(values[6],2+(amount(input.slots[6])*15>>15));
    rate(values[7],(static_cast<unsigned>(tables::clock[amount(input.slots[7])>>12])*441+32768)>>16,"k");
    page_header(state.header,input,frame,titles,values,1);
    if(!state.initialized || state.track!=input.track || input.reduced_motion)state.age=154;
    else if(state.serial!=input.note_serial)state.age=0;
    else state.age=clamp(state.age+clamp(input.elapsed,0,154),0,154);
    state.initialized=1;state.track=input.track;state.serial=input.note_serial;
    const opxy::i32 remaining=154-state.age;
    const opxy::i32 bounce=remaining*remaining*7/(154*154);
    const unsigned shade=shades[model];
    place(frame,9,87*unit,(126-bounce)*unit,unit,input.held_notes>0&&input.held_notes<=128?6:shade,fill_and_stroke);
    place(frame,10,87*unit,(126-bounce)*unit,unit,0,fill_and_stroke);
    if(remaining && !input.reduced_motion)place(frame,11,124*unit,(107-bounce*2)*unit,unit,5,fill_and_stroke);
    else hide(frame,11);
    frame.paths[0].key=1;
    const unsigned family=model==1?1:model==3?2:model==7?3:0, at=color>>12,fraction=(color&4095)*8;
    for(unsigned point=0;point<64;++point){
        const unsigned i=point<32?point*2:(63-point)*2;
        opxy::i32 value=visual::waves[family][at][i]+((visual::waves[family][at+1][i]-visual::waves[family][at][i])*static_cast<opxy::i32>(fraction)>>15);
        if(model==4 || model==5){
            const unsigned harmonic=1+((color*7+16384)>>15);
            const opxy::i32 position=static_cast<opxy::i32>(i);
            const opxy::i32 triangle=position<16?position*2:position<48?64-position*2:position*2-128;
            const unsigned p=(i*harmonic)&63u;
            const opxy::i32 other=static_cast<opxy::i32>(p<32?p*2:128-p*2)-32;
            value=triangle*640+(other*static_cast<opxy::i32>(edge)>>6);
        }else if(model==6){
            unsigned random=i*2654435761u+color*7919u;random^=random>>13;
            value=static_cast<opxy::i32>(random&32767u)-16384;
        }
        frame.paths[0].points[point]={(-62+static_cast<opxy::i32>(i)*2)*unit,-clamp(value*unit/1400,-27*unit,27*unit)+(point<32?-unit/2:unit/2)};
    }
    place(frame,0,248*unit,120*unit,unit,shade,stroke);
    for(unsigned i=0;i<8;++i){
        place(frame,12+i,static_cast<opxy::i32>(357+i%2*46)*unit,static_cast<opxy::i32>(81+i/2*26)*unit,unit,i==model?shade:1,stroke);
        const opxy::i32 height=chord?5+static_cast<opxy::i32>(steps[chord][i&3])/2:3;
        place(frame,20+i,static_cast<opxy::i32>(183+i*21)*unit,180*unit,height*unit/12,chord?shade:1,fill_and_stroke);
    }
}
}
