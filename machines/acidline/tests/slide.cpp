#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
static void require(bool ok,const char* why){if(!ok){std::puts(why);std::exit(1);}}
int main(){
    acidline::Voice voice={};acidline::Shared shared={};
    acidline::initialize(voice,shared,1);acidline::start(voice,shared,1);
    opxy::i32 controls[8]={0x20000000,0x50000000,0x20000000,0x60000000,0x50000000,0,0,0},output[64];
    opxy::u32 pitch[64],phase=0;
    for(unsigned block=0;block<64;++block){
        const auto before=voice.envelope;
        for(unsigned i=0;i<64;++i){
            // A tied phrase changes pitch without sending a new gate/start.
            pitch[i]=block<16?7000000:block<32?7000000+(block-16)*300000+i*4687:11800000;
            phase+=pitch[i];
        }
        acidline::render(voice,shared,1,output,controls,pitch);
        require(voice.envelope<=before,"FAIL tied pitch change retriggers filter envelope");
        require(voice.phase==phase,"FAIL glide applied twice or supplied ramp altered");
    }
    const auto released=voice.envelope;
    acidline::start(voice,shared,1);
    require(voice.envelope>released,"FAIL detached note does not retrigger filter envelope");
    std::puts("PASS tied pitch ramps retain filter contour; detached starts retrigger. Stock event delivery still needs hardware validation.");
}
