#include "display.h"
#include "control_tables.h"

namespace bitflip {
namespace {
using namespace opxy::draw;
const char* const model_names[8] = {"PLS","TRI","TWIN","WAVE","FM","MET","NOIS","XOR"};
const char* const chord_names[8] = {"off","oct","major","minor","sus","maj7","min7","power"};
const unsigned shades[8] = {3,5,3,4,7,7,6,4};
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

// All scenery fits within 64 pixels of its origin, so wrapping off-screen never pops a visible tile.
opxy::i32 world_x(unsigned at,unsigned offset) {
    const unsigned position=(at+640-offset)%640;
    return position>576?static_cast<opxy::i32>(position)-640:static_cast<opxy::i32>(position);
}
}
inline void display_reset(DisplayState& state) {
    state.header.reset();state.initialized=0;
}
inline void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    const unsigned model=amount(input.slots[0])>>12, chord=amount(input.slots[4])>>12;
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
    page_header(state.header,input,frame,titles,values,0);
    const bool baseline=!state.initialized || state.track!=input.track;
    const bool note=!baseline && state.serial!=input.note_serial;
    const bool held=input.held_notes>0 && input.held_notes<=128;
    const opxy::i32 dt=baseline?0:clamp(input.elapsed,0,102);
    if(baseline) {
        state.travel=state.collected=state.coins=0;
        state.jump_age=512;state.pop_age=154;state.activity=0;
    }
    state.initialized=1;state.track=input.track;state.serial=input.note_serial;
    if(input.reduced_motion) {
        state.jump_age=512;state.pop_age=154;state.activity=0;
    } else {
        state.jump_age=clamp(state.jump_age+dt,0,512);
        state.pop_age=clamp(state.pop_age+dt,0,154);
        if(note)state.jump_age=0;
        state.activity=held || note ? unit : clamp(state.activity-dt*7,0,unit);
        const unsigned old_lap=(state.travel>>10)/640;
        const unsigned speed=64+(amount(input.slots[5])>>12)*12;
        state.travel=(state.travel+static_cast<unsigned>(dt)*speed*static_cast<unsigned>(state.activity)/unit)%(5120u*unit);
        if(old_lap!=(state.travel>>10)/640)state.collected=0;
    }
    const unsigned travel=state.travel>>10;
    const opxy::i32 distance=static_cast<opxy::i32>((travel+290)%320)-160;
    const opxy::i32 square=distance*distance;
    const opxy::i32 automatic=square<3136?48*(3136-square)/3136:0;
    const opxy::i32 support=distance>-24 && distance<24?28:0;
    const opxy::i32 hop=36*state.jump_age*(512-state.jump_age)/65536;
    const opxy::i32 running_jump=automatic*state.activity/unit;
    const opxy::i32 jump=input.reduced_motion?support:(running_jump>support+hop?running_jump:support+hop);
    const unsigned pose=input.reduced_motion?0:jump>support+2?5:state.activity?1+(state.travel/(8*unit))%4:0;
    const opxy::i32 bob=pose==2 || pose==4?2:0;
    const opxy::i32 runner_y=184-jump-bob;
    const unsigned shade=shades[model];
    // Every retained node gets a complete pose; only visible cels are placed.
    for(unsigned i=8;i<Frame::node_count;++i)hide(frame,i);
    for(unsigned i=0;i<5;++i)
        place(frame,scene::hills+i,world_x(i*128,(travel/8)%640)*unit,184*unit,unit,1,keep);
    for(unsigned i=0;i<4;++i)
        place(frame,scene::clouds+i,world_x(40+i*160,(travel/16)%320)*unit,static_cast<opxy::i32>(80+(i%2)*16)*unit,unit,2,keep);
    for(unsigned i=0;i<16;++i)
        place(frame,scene::ground+i,(16+static_cast<opxy::i32>(i)*32-static_cast<opxy::i32>(travel%32))*unit,184*unit,unit,2,keep);
    for(unsigned i=0;i<2;++i) {
        place(frame,scene::obstacles+i,world_x(300+i*320,travel%640)*unit,184*unit,unit,shade,keep);
        place(frame,scene::platforms+i,world_x(140+i*320,travel%640)*unit,88*unit,unit,5,keep);
    }
    const unsigned coin_x[6]={252,284,316,572,604,636};
    for(unsigned i=0;i<6;++i) {
        if(state.collected&(1u<<i))continue;
        const opxy::i32 x=world_x(coin_x[i],travel%640), y=i%3==1?110:126;
        if(!input.reduced_motion && (state.activity || state.jump_age<512) &&
           x>96 && x<124 && y>=runner_y-44 && y<=runner_y+4) {
            state.collected|=1u<<i;state.coins=(state.coins+1)%100;state.pop_age=0;
        } else place(frame,scene::coins+i*4+(travel/8)%4,x*unit,y*unit,unit,5,keep);
    }
    const unsigned body_shades[5]={held?6u:shade,5,0,7,4};
    for(unsigned i=0;i<5;++i)
        place(frame,scene::body+i,110*unit,runner_y*unit,unit,body_shades[i],fill_and_stroke);
    for(unsigned i=0;i<3;++i)
        place(frame,scene::limbs+pose*3+i,110*unit,runner_y*unit,unit,i==0?4:i==1?5:6,keep);
    if(state.pop_age<154 && !input.reduced_motion) {
        const opxy::i32 spread=4+state.pop_age/8;
        for(unsigned i=0;i<4;++i)
            place(frame,scene::sparks+i,(110+(i&1?spread:-spread))*unit,
                  (runner_y-25-(i&2?spread:-spread))*unit,unit,5,keep);
    }
    place(frame,scene::digits+state.coins/10,432*unit,46*unit,unit,5,keep);
    place(frame,scene::digits+10+state.coins%10,444*unit,46*unit,unit,5,keep);
    for(unsigned i=0;i<8;++i)
        place(frame,scene::models+i,static_cast<opxy::i32>(280+i*12)*unit,51*unit,unit,i==model?shade:1,fill_and_stroke);
}
}
