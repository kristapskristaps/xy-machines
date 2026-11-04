#include "../display.cpp"
#include <cstdio>
#include <cstring>
int main() {
    westfold::DisplayState state = {};
    westfold::Frame frame = {};
    opxy::draw::Input in = {};
    westfold::display_reset(state);
    const double defaults[8] = {.24,.44,.68,.62,1.0/7,.05,.32,.12};
    for (unsigned i=0;i<8;++i) in.slots[i]=static_cast<opxy::i32>(defaults[i]*2147483647.0);
    for (unsigned i=0;i<4;++i) {in.parameters[i]=in.slots[i];in.shown[i]=i;}
    in.elapsed=17;
    westfold::display(state,in,frame);
    if (!opxy::draw::valid(frame) || state.gate) return 1;
    in.note_serial=1;in.held_notes=1;
    westfold::display(state,in,frame);
    if (state.gate<800 || !opxy::draw::valid(frame)) return 2;
    in.track=1;
    westfold::display(state,in,frame);
    if (state.gate) return 3;
    in.reduced_motion=1;
    westfold::display(state,in,frame);
    auto previous=frame.paths[2];
    for (unsigned i=0;i<64;++i)westfold::display(state,in,frame);
    if (std::memcmp(previous.points,frame.paths[2].points,sizeof(previous.points))) return 4;
    in.held_notes=0;in.note_off_serial=1;
    westfold::display(state,in,frame);
    if (state.gate) return 5;
    in.shown[0]=0;in.parameters[0]=2147483647;
    westfold::display(state,in,frame);
    if(std::strcmp(frame.header[0],"!!"))return 7;
    opxy::u32 seed=1;
    for(unsigned f=0;f<4096;++f){
        for(unsigned i=0;i<8;++i){seed=1664525u*seed+1013904223u;in.slots[i]=static_cast<opxy::i32>(seed);}
        for(unsigned i=0;i<4;++i){in.shown[i]=(f+i)%8;in.parameters[i]=in.slots[in.shown[i]];}
        in.elapsed=f%103;in.reduced_motion=f%11==0;in.held_notes=f%4;
        if(f%9==0)++in.note_serial;
        if(f%13==0)++in.note_off_serial;
        if(f%97==0)in.track=(in.track+1)%8;
        westfold::display(state,in,frame);
        if(!opxy::draw::valid(frame)){std::printf("invalid frame %u\n",f);return 6;}
    }
    std::puts("PASS complete drawing bounds, note response, track changes, alternate headers and reduced motion");
    return 0;
}
