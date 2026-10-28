#include "../fx.cpp"
#include <cstdio>
#include <cstring>
#include <cmath>

static opxy::i32 memory[282624/4];
static vapour::State state;
static opxy::i32 left[64],right[64];
static opxy::u32 phase;
static void input() {
    for(unsigned i=0;i<64;++i) {
        phase+=42852281u;
        left[i]=static_cast<opxy::i32>(std::sin(phase*(6.283185307179586/4294967296.0))*268435456.0);
        right[i]=left[i];
    }
}
int main() {
    opxy::i32 p[8]={0,966367642,1073741824,429496730,1073741824,0,0,2147483647};
    opxy::Buffer buffer={memory,sizeof(memory)/4};
    std::memset(memory,0x7f,sizeof memory);
    vapour::initialize(state);
    // Old scratch contents must never be heard, including with Freeze engaged.
    p[6]=2147483647;
    for(unsigned b=0;b<30;++b) {
        for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
        vapour::process(state,buffer,left,right,p);
        for(unsigned i=0;i<64;++i) if(left[i]||right[i]) {std::puts("FAIL stale scratch");return 1;}
    }
    vapour::initialize(state); p[6]=0;
    double energy=0;
    for(unsigned b=0;b<1500;++b) {
        input();vapour::process(state,buffer,left,right,p);
        for(unsigned i=0;i<64;++i) {energy+=double(left[i])*left[i]; if(left[i]<-536870912||left[i]>536870912)return 1;}
    }
    if(energy<1e12) {std::puts("FAIL silent grains");return 1;}
    // Freeze must preserve every recorded sample while continuing to render.
    static opxy::i32 frozen[vapour::ring_frames];
    std::memcpy(frozen,memory,sizeof frozen); p[6]=2147483647;
    for(unsigned b=0;b<100;++b) {input();vapour::process(state,buffer,left,right,p);}
    if(std::memcmp(frozen,memory,sizeof frozen)) {std::puts("FAIL freeze changed buffer");return 1;}
    // Silent fixture on a dirty buffer after reset, at every density and texture endpoint.
    for(unsigned variant=0;variant<8;++variant) {
        vapour::initialize(state);p[3]=variant&1?2147483647:0;p[4]=variant&2?2147483647:0;p[5]=variant&4?2147483647:0;p[6]=0;
        for(unsigned b=0;b<100;++b) {
            for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
            vapour::process(state,buffer,left,right,p);
            for(unsigned i=0;i<64;++i) if(left[i]||right[i]) {std::puts("FAIL reset leak");return 1;}
        }
    }
    opxy::Buffer empty={nullptr,0};input();vapour::process(state,empty,left,right,p);
    for(unsigned i=0;i<64;++i)if(left[i]||right[i])return 1;
    std::puts("PASS: audible stereo grains, freeze, dirty scratch, reset and missing buffer");
}
