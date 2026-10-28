#include "../engine.cpp"
#include <cstdio>
int main() {
    weave::Voice voice={};weave::Shared shared={};
    const opxy::i32 p[8]={200000000,0,0,1073741824,0,0,0,0};
    opxy::u32 pitch[64];for(auto& x:pitch)x=42852281u;
    opxy::i32 out[64];weave::initialize(voice,shared,1);weave::start(voice,shared,1);
    unsigned crossings=0;opxy::i32 previous=0;
    for(unsigned block=0;block<1500;++block) {
        weave::render(voice,shared,1,out,p,pitch);
        if(block>=100)for(auto x:out) {if(previous<=0&&x>0)++crossings;previous=x;}
    }
    const double hz=crossings*44100.0/(1400*64);
    std::printf("Resampled carrier: %.3f Hz\n",hz);
    return hz>439&&hz<441?0:1;
}
