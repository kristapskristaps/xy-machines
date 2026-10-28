#include "display.h"
namespace weave {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 settle = 184;
const char* const codes[15] = {"SHAP","FM","NOIS","DUST","KICK","SNAR","HAT","GRAN","CHRD","SWRM","WAVE","ANLG","MODL","STRG","6OP"};
const char* const names[15] = {"shaper","2-op fm","noise","dust","kick","snare","hihat","grain","chords","swarm","waves","analog","modal","string","6-op fm"};
i32 amount(i32 word) { return word < 0 ? 0 : word >> 16; }
i32 ease(i32 age) {
    const i32 remaining = unit - clamp(age,0,settle)*unit/settle;
    return unit-remaining*remaining/unit*remaining/unit;
}
u32 hash_word(u32 hash,u32 word) { return (hash^word)*16777619u; }
void geometry(const DisplayState& state,Frame& frame) {
    u32 key=hash_word(2166136261u,state.model);
    key=hash_word(key,static_cast<u32>(state.draw_phase));
    key=hash_word(key,static_cast<u32>(state.draw_energy));
    for(auto value:state.draw_controls)key=hash_word(key,static_cast<u32>(value));
    if(!key)key=1;
    if(frame.paths[0].key==key && frame.paths[1].key==key)return;
    // The belt wraps the actual wheels. The red bed is a thin closed ribbon,
    // so its resting shape retains area in the native SVG renderer.
    for(unsigned i=0;i<32;++i) {
        const bool left=i<16;
        const i32 step=static_cast<i32>(left?i:i-16);
        const i32 angle=left?620+step*2060/15:2680+step*1540/15;
        const i32 ripple=sine(state.draw_phase*3+angle)*state.draw_energy/64;
        const i32 radius=(left?24:10)*unit+ripple;
        frame.paths[0].points[i].x=(left?-24:36)*unit+floor_q10(radius*sine(angle+900));
        frame.paths[0].points[i].y=(left?6:-10)*unit+floor_q10(radius*sine(angle));
    }
    const i32 waves=1+state.draw_controls[0]*3/256+static_cast<i32>(state.model%3);
    const i32 amplitude=state.draw_energy*64*(1+state.draw_controls[2]/128);
    for(unsigned i=0;i<16;++i) {
        const i32 at=static_cast<i32>(i),envelope=sine(at*1800/15);
        const i32 wave=floor_q10(sine(at*1800*waves/15+state.draw_phase*3)*amplitude);
        const i32 y=floor_q10(wave*envelope),x=-25*unit+at*50*unit/15;
        frame.paths[1].points[i]={x,y-307};
        frame.paths[1].points[31-i]={x,y+307};
    }
    frame.paths[0].key=frame.paths[1].key=key;
}
}
void display_reset(DisplayState& state) { state=DisplayState(); }
void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {
    using namespace opxy::draw;
    const bool snapshot=!state.initialized || state.track!=input.track;
    const auto events=state.notes.update(input);
    const bool held=input.held_notes<=128 && input.held_notes!=0;
    const bool hit=!snapshot && input.track<8 && state.serial!=input.note_serial;
    const unsigned selected=static_cast<unsigned>(amount(input.slots[0]))*15u>>15;
    const bool changed_model=!snapshot && selected!=state.model;
    const i32 elapsed=clamp(input.elapsed,0,102);
    const bool was_settling=state.control_age<settle || state.swap_age<settle;
    i32 target[4];for(unsigned i=0;i<4;++i)target[i]=amount(input.slots[i+1]);
    if(snapshot || input.reduced_motion) {
        state.phase=state.travel=0;state.model=selected;
        state.control_age=state.swap_age=state.pulse_age=state.quiet_age=settle;
        state.energy=held && !input.reduced_motion?640:0;
        for(unsigned i=0;i<4;++i)state.controls[i]=state.from[i]=state.target[i]=target[i];
    } else {
        bool changed=false;
        for(unsigned i=0;i<4;++i)if(target[i]!=state.target[i])changed=true;
        if(changed) {
            state.control_age=0;
            for(unsigned i=0;i<4;++i){state.from[i]=state.controls[i];state.target[i]=target[i];}
        }
        state.control_age=clamp(state.control_age+elapsed,0,settle);
        const i32 blend=ease(state.control_age);
        for(unsigned i=0;i<4;++i)state.controls[i]=state.from[i]+(state.target[i]-state.from[i])*blend/unit;
        if(changed_model){state.model=selected;state.swap_age=0;}
        state.swap_age=clamp(state.swap_age+elapsed,0,settle);
        if(hit){state.energy=unit;state.pulse_age=0;state.quiet_age=0;}
        else {
            state.pulse_age=clamp(state.pulse_age+elapsed,0,settle);
            const i32 target_energy=held?640:0;
            state.energy+=(target_energy-state.energy)*clamp(elapsed*12,0,unit)/unit;
        }
        if(events.hold_ended)state.quiet_age=0;
        if(held)state.quiet_age=0;else state.quiet_age=clamp(state.quiet_age+elapsed,0,settle);
        if(!held && state.quiet_age==settle)state.energy=0;
        state.phase=(state.phase+static_cast<u32>(elapsed*(4+state.controls[1]/8192)*state.energy/(unit*2)))%3600u;
        state.travel=(state.travel+static_cast<u32>(elapsed*(5+state.controls[1]/8192)*state.energy/unit))%3600u;
    }
    state.sample_age+=elapsed;
    const bool finished=state.control_age==settle && state.swap_age==settle;
    if(snapshot || input.reduced_motion || hit || changed_model || state.sample_age>=51 ||
       (finished && (was_settling || !state.energy))) {
        state.sample_age%=51;
        for(unsigned i=0;i<4;++i)state.draw_controls[i]=state.controls[i]>>7;
        state.draw_phase=static_cast<i32>(state.phase/20*20);state.draw_energy=state.energy>>5;
    }
    geometry(state,frame);
    for(unsigned i=0;i<Frame::node_count;++i)hide(frame,i);
    const i32 energy=input.reduced_motion?0:state.energy;
    const i32 phase=input.reduced_motion?0:static_cast<i32>(state.phase);
    const i32 travel=input.reduced_motion?0:static_cast<i32>(state.travel);
    const i32 impact=input.reduced_motion?0:unit-ease(state.pulse_age);
    const i32 harmonic=state.controls[0],timbre=state.controls[1],morph=state.controls[2],output=state.controls[3];
    const unsigned red=held || energy>320?7:6;
    place(frame,art::belt,146*unit,131*unit,unit,red,stroke);
    const bool operators=selected==1 || selected==14;
    if(!operators && selected!=9)
        place(frame,art::bed,238*unit,(selected==13?126:141)*unit,unit,red,stroke);
    place(frame,art::flywheel,122*unit,137*unit,unit,4,stroke);
    turn(frame,art::flywheel,rotation(phase*(1+harmonic/8192)));
    place(frame,art::pulley,182*unit,121*unit,unit,red,stroke);
    turn(frame,art::pulley,rotation(-phase*(2+harmonic/8192)));
    place(frame,art::piston,245*unit,70*unit+impact*(4+morph/5462),unit,red,fill_and_stroke);
    place(frame,art::gauge,346*unit,83*unit,unit,7,stroke);
    turn(frame,art::gauge,rotation(-950+morph*1700/32767+energy/5));
    place(frame,art::valve,389*unit,125*unit,unit,7,stroke);
    turn(frame,art::valve,rotation(output*2700/32767));
    const i32 swing=sine(phase)*energy/(unit*2);
    place(frame,art::lamp,173*unit+swing,66*unit,unit,4,keep);
    turn(frame,art::lamp,rotation(swing/12));
    if(held || energy) {
        place(frame,art::lamp_glow,173*unit+swing,66*unit,unit,held?2:1,fill_and_stroke);
        turn(frame,art::lamp_glow,rotation(swing/12));
    }
    place(frame,art::vent,412*unit,98*unit,unit,4,keep);
    turn(frame,art::vent,rotation(-phase*3));
    const i32 tooth=travel*8*unit/3600;
    place(frame,art::conveyor,364*unit+tooth,163*unit-tooth/3,unit,3,stroke);
    for(unsigned i=0;i<3;++i) {
        if(i>=1+static_cast<unsigned>(timbre)/10923u)continue;
        const i32 at=(travel+static_cast<i32>(i)*1200)%3600;
        const bool fine=selected==2 || selected==3 || selected==7 || selected==9;
        place(frame,art::cargo_0+i,326*unit+at*74*unit/3600,166*unit-at*23*unit/3600,
              fine?640:unit,i?4:red,fill_and_stroke);
    }
    if(energy && timbre>4096)for(unsigned i=0;i<3;++i) {
        const i32 age=(travel+static_cast<i32>(i)*1200)%3600;
        place(frame,art::steam_0+i,270*unit+sine(age+static_cast<i32>(i)*700)*4,
              53*unit-age*20*unit/3600,384+age*700/3600,age<1200?4:age<2400?3:2,stroke);
    }
    place(frame,art::indicator,279*unit,97*unit,unit,held?7:2,fill_and_stroke);
    const i32 bob=sine(phase*2)*energy/(unit*2);
    place(frame,art::caretaker,84*unit,151*unit-bob,unit,4,stroke);
    place(frame,art::visor,84*unit,151*unit-bob,unit,impact>400?8:red,fill_and_stroke);
    place(frame,art::wrench,98*unit,147*unit-bob,unit,4,keep);
    turn(frame,art::wrench,rotation(-220+sine(phase*2)*energy/(unit*4)));
    // Each model has a physical cartridge; only the selected one is visible.
    const i32 arrival=(unit-ease(state.swap_age))*5;
    i32 shift=0,tilt=0,scale=896+harmonic*256/32767;
    if(selected==0 || (selected>=4 && selected<=6))shift=-impact*(selected==4?5:3);
    if(selected==2 || selected==3)tilt=sine(phase*3)*energy/(unit*10);
    if(selected==7)shift=sine(phase*2)*energy/unit;
    if(selected==8)scale+=energy/16;
    if(selected==10)shift=sine(phase)*energy/unit;
    if(selected==11 || selected==12 || selected==13)tilt=sine(phase*2)*energy/(unit*14);
    place(frame,art::model_shaper+selected,238*unit+arrival,125*unit+shift,scale,4,stroke);
    turn(frame,art::model_shaper+selected,rotation(selected==9?phase*2:tilt));
    if(operators) {
        const unsigned count=selected==1?2:6;
        for(unsigned i=0;i<count;++i) {
            const i32 x=selected==1?(i?12:-12):(static_cast<i32>(i%3)-1)*18;
            const i32 y=selected==1?0:(i<3?-8:8);
            place(frame,art::operator_0+i,238*unit+x*scale+arrival,125*unit+y*scale,scale,red,stroke);
            turn(frame,art::operator_0+i,rotation(phase*static_cast<i32>(i+1)*(1+harmonic/10923)*(i&1?-1:1)));
        }
    }
    text(frame,0,211*unit,162*unit,unit,names[selected]);
    char number[3]={static_cast<char>('0'+(selected+1)/10),static_cast<char>('0'+(selected+1)%10),0};
    text(frame,1,266*unit,98*unit,unit,number);
    for(unsigned i=0;i<4;++i)header(frame,i,"");
    if(input.shown[0]==0)header(frame,0,codes[selected]);
    state.initialized=1;state.track=input.track;state.serial=input.note_serial;
}
}
