#include "../engine.cpp"
#include <cstdio>
#include <cmath>
static matter::Voice voice;
static matter::Shared shared;
int main() {
    opxy::i32 p[8]={429496730,1073741824,644245094,644245094,0,1073741824,1288490188,2147483647};
    opxy::i32 output[64];opxy::u32 pitch[64];for(auto& x:pitch)x=42852281u;
    for(unsigned model=0;model<6;++model) {
        p[4]=static_cast<opxy::i32>((model*32768u+1200u)/6u*65536u);
        matter::initialize(voice,shared,1);matter::start(voice,shared,1);
        double energy=0;
        for(unsigned block=0;block<690;++block) {
            matter::render(voice,shared,1,output,p,pitch);
            for(auto x:output) {if(x<-536870912||x>536870912)return 1;energy+=double(x)*x;}
        }
        if(energy<1e12) {std::printf("FAIL: model %u silent\n",model);return 1;}
        std::printf("model %u RMS %.6f\n",model,std::sqrt(energy/44160)/2147483648.0);
    }
    opxy::u32 rng=21;
    for(unsigned block=0;block<1500;++block) {
        for(unsigned i=0;i<8;++i)p[i]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        for(auto& x:pitch)x=100000u+(rng%300000000u);
        if(!(block%17))matter::start(voice,shared,1);
        matter::render(voice,shared,1,output,p,pitch);
        for(auto x:output)if(x<-536870912||x>536870912)return 1;
    }
    matter::destroy(voice,shared,1);
    std::puts("PASS: six exciter families, bounds, retriggers and moving controls");
}
