#include "../engine.cpp"
#include "../display.cpp"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <initializer_list>

namespace {
using opxy::i32; using opxy::u32;
struct Energy { double rms, difference; };
Energy measure(i32 cutoff,i32 resonance,i32 shape,i32 contour,u32 increment=42852281u) {
    subvista::Voice v={};subvista::Shared shared={};
    subvista::initialize(v,shared,1);subvista::start(v,shared,1);
    const i32 p[8]={cutoff,resonance,shape,contour,0,0,0,0};
    u32 pitch[64];i32 out[64];for (unsigned i=0;i<64;++i)pitch[i]=increment;
    double square=0,difference=0,previous=0;unsigned count=0;
    for (unsigned b=0;b<250;++b) {
        subvista::render(v,shared,1,out,p,pitch);
        for (unsigned i=0;i<64;++i) {
            if(out[i]<-1073741824 || out[i]>1073741824) {std::puts("FAIL output peak");std::exit(1);}
            const double x=out[i]/2147483648.0;
            if(b>80){square+=x*x;difference+=(x-previous)*(x-previous);++count;}
            previous=x;
        }
    }
    const Energy result={std::sqrt(square/count),std::sqrt(difference/count)};return result;
}
}
int main() {
    const i32 maximum=0x7fffffff;
    const Energy dark=measure(0x10000000,0,0,0),bright=measure(0x70000000,0,0,0);
    if (!(bright.rms>dark.rms*2 && bright.difference>dark.difference*3)) {
        std::printf("FAIL low-pass attenuation: dark %f/%f, bright %f/%f\n",dark.rms,dark.difference,bright.rms,bright.difference);return 1;
    }
    const i32 values[]={0,0x40000000,maximum};
    for(unsigned c=0;c<3;++c)for(unsigned r=0;r<3;++r)for(unsigned s=0;s<3;++s) {
        const Energy e=measure(values[c],values[r],values[s],maximum,42852281u);
        if(!(e.rms>0.00001 && std::isfinite(e.rms))){std::puts("FAIL silent/non-finite setting");return 1;}
    }
    for(u32 pitch: {u32(796254),u32(171798692),u32(1790000000)})measure(maximum,maximum,maximum,maximum,pitch);
    subvista::Voice a={},b={};subvista::Shared shared={};
    i32 outa[64],outb[64],parameters[8]={0x40000000,0x40000000,0x20000000,0x60000000};
    u32 pitch[64];for(unsigned i=0;i<64;++i)pitch[i]=42852281u;
    subvista::initialize(a,shared,1);subvista::render(a,shared,1,outa,parameters,pitch);
    subvista::start(a,shared,1);subvista::initialize(b,shared,2);
    subvista::render(a,shared,1,outa,parameters,pitch);subvista::render(b,shared,2,outb,parameters,pitch);
    if(std::memcmp(outa,outb,sizeof(outa))){std::puts("FAIL retrigger does not reset the voice");return 1;}
    subvista::DisplayState display={};subvista::Frame frame={};opxy::draw::Input input={};
    subvista::display_reset(display);
    for(unsigned i=0;i<800;++i) {
        input.elapsed=17;input.track=i/200;input.held_notes=(i/50)%2;input.note_serial=i/50;
        input.note_off_serial=i/100;input.reduced_motion=i>600;
        for(unsigned n=0;n<4;++n)input.slots[n]=input.parameters[n]=values[(i/10+n)%3];
        subvista::display(display,input,frame);
        if(!opxy::draw::valid(frame)){std::printf("FAIL invalid display frame %u\n",i);return 1;}
    }
    const u32 stopped_phase=display.phase;subvista::display(display,input,frame);
    if(display.phase!=stopped_phase){std::puts("FAIL reduced-motion animation");return 1;}
    std::printf("PASS: filter attenuation, 27 control combinations, pitch extremes, retrigger and 800 display frames. RMS dark %.4f, bright %.4f\n",dark.rms,bright.rms);
    return 0;
}
