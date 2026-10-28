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
    // Exercise the actual grain renderer against the former four-product pan.
    // Complementary stereo energy is preserved with at most two PCM steps' change.
    Workspace workspace={};
    for(unsigned trial=0;trial<2000;++trial) {
        const opxy::i32 l=static_cast<opxy::i16>(rng=rng*1664525u+1013904223u);
        const opxy::i32 r=static_cast<opxy::i16>(rng=rng*1664525u+1013904223u);
        const unsigned packed=static_cast<opxy::u16>(l)|(static_cast<unsigned>(static_cast<opxy::u16>(r))<<16);
        for(unsigned i=0;i<66;++i)ring[i]=static_cast<opxy::i32>(packed);
        Grain grain={};grain.remaining=64;grain.increment=65536;grain.envelope=0x80000000u;grain.slope=32768;
        grain.left_gain=(rng=rng*1664525u+1013904223u)>>17;
        grain.right_gain=(rng=rng*1664525u+1013904223u)>>17;
        workspace.grains[0]=grain;opxy::i32 left[64]={},right[64]={};
        render_grain<false>(state,workspace,ring,0,left,right);
        const opxy::i32 gl=gain15(l,32767),gr=gain15(r,32767);
        const opxy::i32 old_l=gain15(gl,grain.left_gain)+gain15(gr,32767-grain.right_gain);
        const opxy::i32 old_r=gain15(gr,grain.right_gain)+gain15(gl,32767-grain.left_gain);
        for(unsigned i=0;i<64;++i)if(std::abs(left[i]-old_l)>2||std::abs(right[i]-old_r)>2||left[i]+right[i]!=gl+gr)return 1;
    }
    std::printf("PASS: 100,000 float-reference interpolation checks (max errors %.3f / %.3f PCM steps), five grain windows\n",worst_linear,worst_cubic);
}
