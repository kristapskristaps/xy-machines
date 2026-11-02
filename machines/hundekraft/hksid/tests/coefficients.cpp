#include "../dsp.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void require(bool ok, const char* message) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

// Always recompute from the original formulas, without relying on cache logic.
static void reference(hksid::port::Dsp& dsp, hksid::port::sid_state& s,
                      unsigned inc, const short* q) {
    const bool first = !s.configured;
    s.configured = 1; s.cached_inc = inc;
    for (unsigned i=0;i<4;++i) s.cached_q[i]=q[i];
    s.mode=dsp.sid_wave_mode(q[0]);s.secondary=q[1]>>7;
    s.width=2048+((int)q[1]*61440>>15);s.cutoff=dsp.cutoff_target(q[2]);
    if(first)s.cutoff_live=s.cutoff;
    s.resonance=hksid::port::cRSID_Resonances8580[(unsigned)q[3]>>11];
    unsigned root=inc>>4;if(root>0x06000000u)root=0x06000000u;
    s.step[0]=root;
    if(s.mode>=4){
        const unsigned ratio=256u+((unsigned)q[1]*768u>>15);
        s.step[1]=(root>>8)*ratio;s.step[2]=root;
    }else{s.step[1]=root*2;s.step[2]=root+(root>>1);}
    for(unsigned i=0;i<3;++i){
        unsigned step=s.step[i];if(step>0x06000000u)step=0x06000000u;
        s.step[i]=step;s.pulse_steep[i]=step>=4096?0x0fffffffu/step:65535;
        unsigned steep=(step>>4)/288;if(!steep)steep=1;
        s.saw_steep[i]=steep;s.saw_recip[i]=(1u<<20)/steep;
    }
}

int main() {
    hksid::port::Dsp dsp={};
    for(unsigned n=0;n<=0x00600000u;++n)
        require(dsp.saw_slope(n<<4)==n/288,"complete bounded slope quotient range");
    hksid::port::sid_state expected={},actual={};
    const unsigned pitches[]={0,65535,65536,10713070,42852281,0x60000000u,0xffffffffu};
    unsigned random=7139;
    for(unsigned trial=0;trial<30000;++trial){
        if(trial%97==0){dsp.sid_reset(&expected);dsp.sid_reset(&actual);}
        // Keep the pitch steady for most transitions to exercise selective updates.
        const unsigned inc=pitches[(trial/31)%7];
        short q[4]={static_cast<short>((trial/7%6)*512+256),
                    static_cast<short>((trial/9*137)&32767),
                    static_cast<short>((random=random*1664525u+1013904223u)>>17),
                    static_cast<short>((random=random*1664525u+1013904223u)>>17)};
        reference(dsp,expected,inc,q);dsp.sid_configure(&actual,inc,q);
        require(!std::memcmp(&expected,&actual,sizeof(expected)),"complete configured state");
        // Evolve the filter and oscillators between configurations, including
        // smoothing history that a filter-only update must leave intact.
        const short a=dsp.sid_sample(&expected),b=dsp.sid_sample(&actual);
        require(a==b && !std::memcmp(&expected,&actual,sizeof(expected)),"sample and evolved state");
    }
    std::puts("PASS: 6,291,457 exact slope quotients and 30,000 independent state transitions");
}
