// Quiet-send precision, level changes, reset, and scratch-boundary regression.
#include "../fx.cpp"
#include <cmath>
#include <cstdio>
using namespace hkhail;
static const unsigned words=90780;
struct Memory { opxy::i32 before[8]; alignas(8) opxy::i32 data[words]; opxy::i32 after[8]; };
static Memory memory;
int main() {
    const opxy::i32 levels[]={268435456,1048576,8192,512};
    double reference=0;
    for(opxy::i32 peak:levels) {
        State state={};initialize(state);
        for(unsigned i=0;i<words;++i) memory.data[i]=0x42424242;
        for(unsigned i=0;i<8;++i) memory.before[i]=memory.after[i]=0x12345678;
        opxy::i32 p[8]={644218880,1288437760,1073741824,644218880,1073741824,2147418112,536870912,0};
        opxy::Transport transport={};transport.bpm_x10=1200;
        double square=0;unsigned samples=0;
        for(unsigned block=0;block<4000;++block) {
            opxy::i32 l[64],r[64];
            for(unsigned i=0;i<64;++i) {
                const double phase=(block*64+i)*6.283185307179586*220/44100;
                l[i]=static_cast<opxy::i32>(peak*std::sin(phase));
                r[i]=static_cast<opxy::i32>(peak*std::sin(phase*1.5));
            }
            
            process(state,opxy::Buffer{memory.data,words},l,r,p);
            if(block>2000) for(unsigned i=0;i<64;++i) {
                square+=double(l[i])*l[i]+double(r[i])*r[i];samples+=2;
            }
        }
        const double gain=std::sqrt(square/samples)/peak;
        if(reference==0) reference=gain;
        if(gain<0.005||gain<reference*0.9||gain>reference*1.1) {
            std::printf("Send-level response changed: peak=%ld gain=%.6f reference=%.6f\n",
                static_cast<long>(peak),gain,reference);return 1;
        }
        // A full-level transient must not wrap either output or corrupt the
        // state. Follow it with silence, then reset into the same dirty buffer.
        for(unsigned block=0;block<1500;++block) {
            opxy::i32 l[64]={},r[64]={};
            if(block==0) {l[63]=2147483647;r[63]=-2147483647;}
            process(state,opxy::Buffer{memory.data,words},l,r,p);
        }
        initialize(state);
        (void)transport;
        for(unsigned block=0;block<200;++block) {
            opxy::i32 l[64]={},r[64]={};
            process(state,opxy::Buffer{memory.data,words},l,r,p);
            for(unsigned i=0;i<64;++i) if(l[i]||r[i]) {
                std::puts("Old tail survived dirty-buffer reset");return 2;
            }
        }
        for(unsigned i=0;i<8;++i) if(memory.before[i]!=0x12345678||memory.after[i]!=0x12345678) {
            std::puts("Effect wrote outside its declared scratch buffer");return 3;
        }
    }
    return 0;
}
