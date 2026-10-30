#include "../engine.cpp"
#include "../display.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace {
using opxy::i32;using opxy::u32;
struct Metrics {double energy,edge;};
Metrics run(i32 cutoff,i32 resonance,i32 drive,i32 env,i32 decay,i32 wave){
    acidline::Voice v={};acidline::Shared shared={};
    acidline::initialize(v,shared,1);acidline::start(v,shared,1);
    const i32 p[8]={cutoff,resonance,drive,env,decay,wave,0,0};
    i32 out[64];u32 pitch[64];for(unsigned i=0;i<64;++i)pitch[i]=7142047;
    double energy=0,edge=0,previous=0;
    for(unsigned b=0;b<240;++b){
        acidline::render(v,shared,1,out,p,pitch);
        for(unsigned i=0;i<64;++i){
            if(out[i]<-1073741824 || out[i]>1073741824){std::puts("FAIL output peak");std::exit(1);}
            const double x=out[i]/2147483648.0;
            if(b>60){energy+=x*x;edge+=(x-previous)*(x-previous);}previous=x;
        }
    }
    const Metrics result={energy,edge};return result;
}
}
int main(){
    const i32 values[]={0,0x40000000,0x7fffffff};
    for(unsigned c=0;c<3;++c)for(unsigned r=0;r<3;++r)for(unsigned d=0;d<3;++d)for(unsigned w=0;w<3;++w){
        const Metrics m=run(values[c],values[r],values[d],0x60000000,0x50000000,values[w]);
        if(!std::isfinite(m.energy)||m.energy<.000001){std::puts("FAIL silent/invalid control combination");return 1;}
    }
    const Metrics dark=run(0x08000000,0,0,0,0,0),bright=run(0x70000000,0,0,0,0,0);
    if(bright.edge<dark.edge*3){std::puts("FAIL low-pass does not remove harmonics");return 1;}
    const Metrics clean=run(0x40000000,0x50000000,0,0,0,0),dirty=run(0x40000000,0x50000000,0x70000000,0,0,0);
    if(dirty.edge/dirty.energy<clean.edge/clean.energy*1.2){std::puts("FAIL distortion does not add harmonics");return 1;}
    acidline::Voice a={},b={};acidline::Shared shared={};i32 out[64],first[64];u32 pitch[64];
    i32 params[8]={0x50000000,0x70000000,0x60000000,0x60000000,0x40000000,0,0,0};
    acidline::initialize(a,shared,1);acidline::start(a,shared,1);
    u32 phase=0;
    for(unsigned block=0;block<20;++block){
        for(unsigned i=0;i<64;++i){pitch[i]=7000001u+block*90000u+i*731u;phase+=pitch[i];}
        acidline::render(a,shared,1,out,params,pitch);
        if(a.phase!=phase){std::puts("FAIL supplied stock pitch ramp was altered");return 1;}
    }
    // Starting or destroying another track's voice must not steal or mute this one.
    acidline::Voice baseline=a;
    acidline::render(baseline,shared,1,first,params,pitch);
    acidline::initialize(b,shared,2);acidline::start(b,shared,2);acidline::destroy(b,shared,2);
    acidline::render(a,shared,1,out,params,pitch);
    if(std::memcmp(out,first,sizeof(out))){std::puts("FAIL voices interfere across tracks");return 1;}
    acidline::DisplayState state={};acidline::Frame frame={};opxy::draw::Input input={};
    for(unsigned i=0;i<600;++i){
        input.elapsed=17;input.held_notes=(i/30)%2;input.reduced_motion=i>400;
        for(unsigned j=0;j<8;++j)input.slots[j]=values[(i/10+j)%3];
        acidline::display(state,input,frame);
        if(!opxy::draw::valid(frame)){std::puts("FAIL display frame");return 1;}
    }
    acidline::display_reset(state);input={};input.elapsed=17;
    acidline::display(state,input,frame);
    acidline::Frame idle=frame;
    for(unsigned i=0;i<30;++i)acidline::display(state,input,frame);
    if(std::memcmp(&idle,&frame,sizeof(frame))){std::puts("FAIL idle display moves");return 1;}
    input.note_on_serial=1;input.held_notes=1;acidline::display(state,input,frame);
    if(!state.bloom){std::puts("FAIL note does not trigger chemical reaction");return 1;}
    const opxy::draw::Node packet=frame.nodes[35];
    acidline::display(state,input,frame);
    if(packet.x==frame.nodes[35].x && packet.y==frame.nodes[35].y){std::puts("FAIL pipe fluid does not travel");return 1;}
    input.reduced_motion=true;acidline::display(state,input,frame);
    if(state.phase || state.bloom){std::puts("FAIL reduced-motion reaction keeps moving");return 1;}
    input.shown[0]=4;input.shown[1]=5;acidline::display(state,input,frame);
    if(state.header.page!=1){std::puts("FAIL second control page not selected");return 1;}
    std::printf("PASS: 81 control combinations, filter, distortion, exact stock pitch ramps, independent voices, 600 display frames. Distortion edge ratio %.2f\n",(dirty.edge/dirty.energy)/(clean.edge/clean.energy));
    return 0;
}
