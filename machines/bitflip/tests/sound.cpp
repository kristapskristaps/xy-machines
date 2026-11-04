#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
using opxy::i32;using opxy::u32;
void require(bool value,const char* message){if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
i32 q(double value){return static_cast<i32>(value*2147483647.0+.5);}
void defaults(i32* p){const double values[8]={0,1,0,.5,0,.4,1,1};for(unsigned i=0;i<8;++i)p[i]=q(values[i]);}
std::vector<i32> sound(const i32* p,double hz,unsigned blocks=690){
    bitflip::Voice voice={};bitflip::Shared shared={};u32 pitch[64];
    for(auto& value:pitch)value=static_cast<u32>(hz/44100*4294967296.0);
    std::vector<i32> output(blocks*64);bitflip::start(voice,shared,1);
    for(unsigned b=0;b<blocks;++b)bitflip::render(voice,shared,1,output.data()+b*64,p,pitch);
    return output;
}
double partial(const std::vector<i32>& data,double hz){
    double real=0,imag=0;for(unsigned i=0;i<22050;++i){const double angle=i*hz*6.283185307179586/44100;
        real+=data[11025+i]*std::cos(angle);imag+=data[11025+i]*std::sin(angle);}
    return real*real+imag*imag;
}
void check(const std::vector<i32>& data){
    double energy=0,mean=0;for(i32 value:data){require(value>=-268435456&&value<=268435456,"declared output headroom");energy+=double(value)*value;mean+=value;}
    require(energy/data.size()>1e9,"model is audible");
    require(std::abs(mean/data.size())<5000000,"DC is removed");
}
}
int main(){
    i32 p[8];defaults(p);auto pulse=sound(p,220);check(pulse);
    require(partial(pulse,220)>partial(pulse,218)*100,"pulse follows the played pitch");
    require(partial(pulse,660)>partial(pulse,440)*100,"square has its expected odd harmonic character");
    for(unsigned model=0;model<8;++model){
        defaults(p);p[0]=q((model+.5)/8);p[1]=q(.55);p[2]=q(.6);
        for(double hz:{55.0,220.0,880.0,3520.0})check(sound(p,hz));
        require(sound(p,220)==sound(p,220),"restruck notes are deterministic");
    }
    defaults(p);p[5]=0;auto slow=sound(p,220);p[5]=q(1);
    require(slow==sound(p,220),"Arp Speed has no effect with Chord off");
    p[4]=q(2.5/8);require(slow!=sound(p,220),"major chord arpeggiation changes the note");
    defaults(p);auto full=sound(p,220);p[6]=q(.15);
    require(full!=sound(p,220),"Bits changes the DAC sound");
    defaults(p);p[7]=q(.2);require(full!=sound(p,220),"Clock changes the held-sample sound");
    bitflip::Voice voice={};bitflip::Shared shared={};u32 pitch[64];i32 out[64];
    for(auto& value:pitch)value=static_cast<u32>(55.0/44100*4294967296.0);
    u32 phases[3];
    for(unsigned sweep=0;sweep<3;++sweep){
        defaults(p);p[3]=q(sweep*.5);bitflip::start(voice,shared,1);
        bitflip::render(voice,shared,1,out,p,pitch);phases[sweep]=voice.phase;
        for(unsigned b=0;b<689;++b)bitflip::render(voice,shared,1,out,p,pitch);
        require(voice.sweep_envelope==0,"pitch sweep settles fully");
        const u32 before=voice.phase;bitflip::render(voice,shared,1,out,p,pitch);
        require(voice.phase-before==pitch[0]*64u,"sweep returns exactly to the played note");
    }
    require(phases[0]<phases[1]&&phases[1]<phases[2],"Sweep starts below or above the key as labelled");
    defaults(p);p[4]=q(2.5/8);p[5]=0;bitflip::start(voice,shared,1);
    for(unsigned b=0;b<1000;++b)bitflip::render(voice,shared,1,out,p,pitch);
    const unsigned long long cycles=static_cast<unsigned long long>(bitflip::tables::arp_rate[0])*64000;
    require(voice.arp_step==((cycles>>32)&3)&&voice.arp_phase==static_cast<u32>(cycles),"arpeggio advances at its stated free-running rate");
    defaults(p);p[2]=q(1);bitflip::start(voice,shared,1);
    bitflip::render(voice,shared,1,out,p,pitch);p[2]=0;
    for(unsigned b=0;b<32;++b)bitflip::render(voice,shared,1,out,p,pitch);
    require(voice.edge==0,"Edge settles exactly to zero and restores its bypass");
    unsigned random=7139;
    for(unsigned model=0;model<8;++model)for(unsigned mask=0;mask<128;++mask){
        p[0]=q((model+.5)/8);for(unsigned i=1;i<8;++i)p[i]=mask&(1u<<(i-1))?q(1):0;
        bitflip::start(voice,shared,model+1);
        for(unsigned block=0;block<16;++block){
            for(auto& value:pitch)value=(random=random*1664525u+1013904223u);
            u32 original_pitch[64];i32 original_parameters[8];std::memcpy(original_pitch,pitch,sizeof(pitch));std::memcpy(original_parameters,p,sizeof(p));
            bitflip::render(voice,shared,model+1,out,p,pitch);
            require(!std::memcmp(original_pitch,pitch,sizeof(pitch))&&!std::memcmp(original_parameters,p,sizeof(p)),"input buffers remain borrowed and read-only");
            for(i32 value:out)require(value>=-268435456&&value<=268435456,"all model/control corners are bounded");
        }
    }
    defaults(p);bitflip::start(voice,shared,1);
    for(unsigned block=0;block<4096;++block){
        if(block%83==0)bitflip::start(voice,shared,1);
        for(auto& value:p)value=static_cast<i32>(random=random*1664525u+1013904223u);
        for(auto& value:pitch)value=(random=random*1664525u+1013904223u);
        bitflip::render(voice,shared,1,out,p,pitch);
        for(i32 value:out)require(value>=-268435456&&value<=268435456,"random signed controls and pitch stay bounded");
    }
    std::puts("PASS: eight models, tuning, harmonics, DAC, arpeggio, sweep, deterministic notes, 1,024 corners, ownership and random automation");
}
