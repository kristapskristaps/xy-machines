#include "../vendor/granular.h"
#include "../fx.cpp"
#include <cmath>
#include <cstdio>
static opxy::i32 ring[vapour::ring_frames];
int main() {
    using namespace vapour;
    using namespace vapour::granular;
    State state={};state.written=ring_frames;
    opxy::u32 rng=1;
    for(auto& packed:ring)packed=static_cast<opxy::i32>(rng=rng*1664525u+1013904223u);
    double worst_linear=0,worst_cubic=0;
    for(unsigned trial=0;trial<50000;++trial) {
        const opxy::u32 phase=(rng=rng*1664525u+1013904223u);
        const unsigned index=phase>>16; const double t=(phase&65535u)/65536.0;
        for(unsigned channel=0;channel<2;++channel) {
            const double xm1=sample(state,ring,index,channel),x0=sample(state,ring,index+1,channel);
            const double x1=sample(state,ring,index+2,channel),x2=sample(state,ring,index+3,channel);
            const double linear=xm1+(x0-xm1)*t;
            const double c=(x1-xm1)*.5,v=x0-x1,w=c+v,a=w+v+(x2-x0)*.5,b=w+a;
            double cubic=((a*t-b)*t+c)*t+x0;
            cubic=cubic<-32768?-32768:cubic>32767?32767:cubic;
            worst_linear=std::fmax(worst_linear,std::fabs(read(state,ring,phase,channel,false)-linear));
            worst_cubic=std::fmax(worst_cubic,std::fabs(read(state,ring,phase,channel,true)-cubic));
        }
    }
    if(worst_linear>3||worst_cubic>10) {std::printf("FAIL interpolation: %f %f\n",worst_linear,worst_cubic);return 1;}
    const unsigned shapes[]={0u,8192u,16384u,24576u,32767u};
    for(unsigned shape: shapes) {
        Grain grain={};grain.remaining=4096;grain.envelope_increment=0xffffffffu/4096u;
        grain.smoothness=shape>=16384u?(shape-16384u)*2u:0;
        grain.slope=shape>=16384u?32768:static_cast<opxy::i32>(536870912u/(shape+328u));
        for(unsigned i=0;i<4096;++i) {
            const double phase=2.0*i/4096.0;
            double expected=phase>=1?2-phase:phase;
            if(shape>=16384u)expected+=(shape/32768.0-.5)*2*((.5-.5*std::cos(expected*3.141592653589793))-expected);
            else expected=std::fmin(1.0,expected*.5/(shape/32768.0+.01));
            const double actual=envelope(grain)/32768.0;
            if(std::fabs(actual-expected)>.003) {std::puts("FAIL grain window");return 1;}
        }
    }
    std::printf("PASS: 100,000 float-reference interpolation checks (max errors %.3f / %.3f PCM steps), five grain windows\n",worst_linear,worst_cubic);
}
