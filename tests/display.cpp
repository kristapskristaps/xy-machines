#include "../display.cpp"
#include <cstring>
#include <cstdio>
using namespace opxy::draw;
static bool hidden(const weave::Frame& frame,unsigned node) {return frame.nodes[node].x==hidden_x*unit && frame.nodes[node].y==hidden_y*unit;}
static bool check(const weave::Frame& frame) {
    if(!valid(frame))return false;
    for(unsigned p=0;p<weave::Frame::path_count;++p) {
        long long area=0;
        for(unsigned i=0;i<weave::Frame::point_count;++i) {
            const auto& a=frame.paths[p].points[i];const auto& b=frame.paths[p].points[(i+1)%weave::Frame::point_count];
            area+=static_cast<long long>(a.x)*b.y-static_cast<long long>(b.x)*a.y;
            if(!hidden(frame,p)) {
                const int x=frame.nodes[p].x+a.x,y=frame.nodes[p].y+a.y;
                if(p==0 && (x<95*unit || x>200*unit || y<108*unit || y>164*unit))return false;
                if(p==1 && (x<202*unit || x>274*unit || y<103*unit || y>149*unit))return false;
            }
        }
        if(area<(p==0?100:8)*1024LL*1024)return false;
    }
    return true;
}
int main() {
    weave::DisplayState state={};weave::Frame frame={};Input in={};in.elapsed=34;
    unsigned rng=7;
    for(unsigned test=0;test<3000;++test) {
        for(auto& value:in.slots)value=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        in.held_notes=test%5;in.note_serial=test/11;in.track=test/101;in.note_off_serial=test/7;
        in.elapsed=test%3?34:2147483647;
        weave::display(state,in,frame);
        if(!check(frame)){std::printf("FAIL bounds/area at %u\n",test);return 1;}
    }
    in=Input();in.elapsed=34;in.held_notes=1;in.note_serial=99;
    for(unsigned i=1;i<4;++i)in.slots[i]=1073741824;
    weave::display_reset(state);weave::display(state,in,frame);
    if(state.pulse_age!=weave::settle || frame.nodes[weave::art::piston].y!=70*unit)return 1;
    in.track=1;++in.note_serial;weave::display(state,in,frame);
    if(state.pulse_age!=weave::settle)return 1;
    ++in.note_serial;weave::display(state,in,frame);
    if(state.pulse_age || frame.nodes[weave::art::piston].y<=70*unit || state.energy!=unit)return 1;
    in.note_serial=0xffffffffu;weave::display(state,in,frame);
    in.note_serial=0;weave::display(state,in,frame);if(state.pulse_age)return 1;
    weave::Frame prior=frame;
    for(unsigned i=0;i<4;++i)weave::display(state,in,frame);
    if(!std::memcmp(frame.paths,prior.paths,sizeof frame.paths) ||
       (frame.nodes[weave::art::flywheel].cos==prior.nodes[weave::art::flywheel].cos && frame.nodes[weave::art::flywheel].sin==prior.nodes[weave::art::flywheel].sin))return 1;
    in.held_notes=0;++in.note_off_serial;
    for(unsigned i=0;i<6;++i)weave::display(state,in,frame);
    if(state.energy || !hidden(frame,weave::art::steam_0) || !hidden(frame,weave::art::lamp_glow))return 1;
    prior=frame;
    for(unsigned i=0;i<30;++i)weave::display(state,in,frame);
    if(std::memcmp(&frame,&prior,sizeof frame))return 1;
    // Model changes swap physical cartridges without leaving ghost mechanisms.
    for(unsigned model=0;model<15;++model) {
        in.slots[0]=static_cast<opxy::i32>((2*model+1)*2147483647LL/30);
        for(unsigned i=0;i<7;++i)weave::display(state,in,frame);
        if(!check(frame) || hidden(frame,weave::art::label_shaper+model))return 1;
        for(unsigned other=0;other<15;++other)if(hidden(frame,weave::art::label_shaper+other)!=(other!=model))return 1;
        for(unsigned other=0;other<15;++other)if(hidden(frame,weave::art::model_shaper+other)!=(other!=model))return 1;
        const unsigned count=model==1?2:model==14?6:0;
        for(unsigned i=0;i<6;++i)if(hidden(frame,weave::art::operator_0+i)!=(i>=count))return 1;
    }
    // Static controls still have a physical effect with reduced motion enabled.
    in.reduced_motion=1;weave::display(state,in,frame);
    prior=frame;in.slots[1]=2147483647;weave::display(state,in,frame);
    if(prior.nodes[weave::art::model_sixop].scale==frame.nodes[weave::art::model_sixop].scale)return 1;
    in.slots[2]=0;weave::display(state,in,frame);
    if(hidden(frame,weave::art::cargo_0) || !hidden(frame,weave::art::cargo_1))return 1;
    in.slots[2]=2147483647;weave::display(state,in,frame);
    if(hidden(frame,weave::art::cargo_2))return 1;
    prior=frame;in.slots[3]=2147483647;weave::display(state,in,frame);
    if(prior.nodes[weave::art::gauge].cos==frame.nodes[weave::art::gauge].cos && prior.nodes[weave::art::gauge].sin==frame.nodes[weave::art::gauge].sin)return 1;
    prior=frame;in.slots[4]=2147483647;weave::display(state,in,frame);
    if(prior.nodes[weave::art::valve].cos==frame.nodes[weave::art::valve].cos && prior.nodes[weave::art::valve].sin==frame.nodes[weave::art::valve].sin)return 1;
    in.shown[0]=4;weave::display(state,in,frame);if(state.header.page || std::strcmp(frame.header[0],"100"))return 1;
    in.shown[0]=4;weave::display(state,in,frame);if(std::strcmp(frame.header[0],"100"))return 1;
    in.held_notes=1;weave::display(state,in,frame);prior=frame;
    for(unsigned i=0;i<30;++i){++in.note_serial;weave::display(state,in,frame);}
    if(std::memcmp(&frame,&prior,sizeof frame) || state.phase || state.travel)return 1;
    in.reduced_motion=0;in.elapsed=17;in.slots[0]=0;
    weave::display_reset(state);weave::display(state,in,frame);
    unsigned outlines=0,rotations=0;
    for(unsigned i=0;i<180;++i) {
        prior=frame;weave::display(state,in,frame);
        if(prior.paths[0].key!=frame.paths[0].key)++outlines;
        if(prior.nodes[weave::art::flywheel].cos!=frame.nodes[weave::art::flywheel].cos || prior.nodes[weave::art::flywheel].sin!=frame.nodes[weave::art::flywheel].sin)++rotations;
    }
    if(outlines<45 || outlines>62 || rotations<160)return 1;
    // Both parameter rows stay in place, switch together, and report applicability.
    in.reduced_motion=1;in.slots[0]=2147483647;
    for(unsigned i=0;i<4;++i)in.shown[i]=i+4;
    in.slots[5]=in.slots[6]=in.slots[7]=2147483647;
    weave::display(state,in,frame);
    if(state.header.page!=1 || std::strcmp(frame.header[1],"--") ||
       std::strcmp(frame.header[2],"hold") || std::strcmp(frame.header[3],"127") ||
       std::strcmp(frame.texts[0].chars,"output") || std::strcmp(frame.texts[8].chars,"output"))return 1;
    for(unsigned i=0;i<4;++i)if(hidden(frame,weave::art::bar_0+i))return 1;
    in.slots[0]=static_cast<opxy::i32>(25*2147483647LL/30); // modal
    in.slots[5]=0x20000000;
    weave::display(state,in,frame);
    if(std::strcmp(frame.header[1],"soft") || std::strcmp(frame.header[2],"--") ||
       std::strcmp(frame.header[3],"--"))return 1;
    std::printf("PASS: state %zu, frame %zu, %u actors; native bounds/area, 15 cartridges, operator visibility, notes/track baselines, finite settling, control mapping, idle cache and reduced motion; %u geometry / %u motor updates\n",sizeof(state),sizeof(frame),weave::Frame::node_count,outlines,rotations);
}
