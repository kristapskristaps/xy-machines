#include "../engine.cpp"
#include "reference_v2.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

static unsigned seed = 1234567;
static unsigned random_word() { seed = seed * 1664525u + 1013904223u; return seed; }
static unsigned long long samples = 0;
static void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
static void check_mip(unsigned increment) {
    unsigned a, b; int x, y;
    vader_reference::mip(increment, a, x); vader::mip(increment, b, y);
    if (a != b || x != y) {
        std::fprintf(stderr,"mip %u: %u/%d != %u/%d\n",increment,a,x,b,y); std::exit(1);
    }
}
static void compare(unsigned scenario, unsigned blocks) {
    vader_reference::Voice a = {}; vader_reference::Shared sa = {};
    vader::Voice b = {}; vader::Shared sb = {};
    unsigned pitch[64]; int p[8] = {}, x[64], y[64];
    vader_reference::start(a,sa,scenario); vader::start(b,sb,scenario);
    const unsigned pitches[] = {1,1u<<16,1u<<22,1u<<24,1u<<26,233u<<21,0x70000000u,0xffffffffu};
    for (unsigned i=0;i<8;++i) p[i] = scenario & (1u<<i) ? 2147483647 : 0;
    p[3] = static_cast<int>((scenario % 4) * 536870912u);
    for (unsigned block=0; block<blocks; ++block) {
        if (scenario >= 256 && scenario < 288 && block % 31 == 0) {
            for (unsigned i=0;i<8;++i) p[i] = random_word() >> 1;
            // Exercise exact table boundaries and transitions to/from silence.
            if (block % 93 == 0) p[2] = 0;
            if (block % 155 == 0) p[0] = 0;
            if (block % 217 == 0) p[2] = 2147483647;
        }
        if (scenario >= 272 && scenario < 288 && block % 73 == 0) {
            vader_reference::start(a,sa,scenario+block); vader::start(b,sb,scenario+block);
        }
        for (unsigned i=0;i<64;++i) {
            pitch[i] = scenario >= 280 ? random_word() : pitches[scenario % 8];
            if (scenario >= 264 && scenario < 280)
                pitch[i] += (block * 64 + i) * 7919u; // glide over every sample
        }
        vader_reference::render(a,sa,scenario,x,p,pitch);
        vader::render(b,sb,scenario,y,p,pitch);
        if (std::memcmp(x,y,sizeof(x))) {
            for(unsigned i=0;i<64;++i) if(x[i]!=y[i]) {
                std::fprintf(stderr,"PCM scenario %u block %u sample %u: %d != %d\n",scenario,block,i,x[i],y[i]);
                std::exit(1);
            }
        }
        samples += 64;
        require(a.phase==b.phase && a.sweep==b.sweep && a.body==b.body && a.body2==b.body2,
                "phase, live envelope and filter state match");
    }
}
int main() {
    const unsigned ends[] = {1889785610u,629928536u,269969372u,125985707u,60960826u,29996596u};
    for(unsigned end:ends) for(int delta=-4;delta<=4;++delta) {
        check_mip(end+delta); check_mip(end/2+delta);
    }
    for(unsigned i=0;i<2000000;++i) check_mip(random_word());
    for(unsigned mode=0;mode<4;++mode) for(unsigned band=0;band<6;++band)
        require(!std::memcmp(vader_reference::waves[0][0][0],vader::waves[mode][0][band],513*sizeof(short)),
                "zero-drive banks exactly match the clean sine");
    const unsigned blends[]={0u,1u,32767u,32768u};
    for(unsigned mode=0;mode<4;++mode) for(unsigned band=0;band<6;++band)
        for(unsigned level=0;level<8;++level) for(unsigned mix:blends)
            for(int delta=-1;delta<=1;++delta) for(unsigned i=0;i<16;++i) {
                const unsigned drive=level*32768u+delta;
                if(drive>262136u) continue;
                const unsigned phase=random_word();
                require(vader_reference::shaped(mode,drive,band,mix,phase)==
                        vader::shaped(mode,drive,band,mix,phase),
                        "exact drive levels and mip interpolation boundaries");
                require(vader_reference::shaped(mode,drive,band,mix,phase)==
                        vader::Shaper(mode,drive,band,mix).read(phase),
                        "exact cached waveform addresses and interpolation");
            }
    for(unsigned scenario=0;scenario<288;++scenario) compare(scenario,scenario<256?3000:5000);
    const int clicks[]={1,262143,262144,262145,2147483647};
    for(int click:clicks) {
        vader::Voice a={}; vader_reference::Voice b={};
        vader::Shared sa={}; vader_reference::Shared sb={};
        int p[8]={0,0,0,0,click,0,2147483647,2147483647}, x[64], y[64];
        unsigned pitch[64]; for(unsigned& v:pitch) v=1u<<22;
        vader::start(a,sa,7); vader_reference::start(b,sb,7);
        for(unsigned block=0;block<250;++block) {
            // Editing Click during a note must not restart its latched impulse.
            if(block==30) p[4]=2147483647;
            vader::render(a,sa,7,x,p,pitch); vader_reference::render(b,sb,7,y,p,pitch);
            require(!std::memcmp(x,y,sizeof(x)),"click cutoff and edits after the impulse");
            samples+=64;
        }
    }
    std::printf("PASS: 2,000,108 exact mip selections; %llu bit-identical PCM samples across corners, long tails, automation, retriggers and glides.\n",samples);
    return 0;
}
