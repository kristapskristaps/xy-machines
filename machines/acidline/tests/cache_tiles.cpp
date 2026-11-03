#include "../engine.cpp"
#include "direct_reference/engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
static void require(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){
    using namespace acidline;using opxy::i32;using opxy::u32;
    require(shape_tile_points==16,"explicit cache work budget");
    Voice v={};Shared shared;initialize(v,shared,1);
    for(auto& point:v.shape_curve)point=0x5a5a;
    const i32 resonance=9174,drive=11468;
    unsigned calls=0;
    while(!v.shape_ready){
        const unsigned before=v.shape_progress;prepare_shape(v,resonance,drive);++calls;
        require(v.shape_progress>before && v.shape_progress-before<=16,"each callback prepares at most16 points");
        require(v.shape_ready==(v.shape_progress==1025),"no partial cache is published");
        for(unsigned i=0;i<1025;++i){
            if(i<v.shape_progress){
                const int body=acidline_direct::interpolate(acidline_direct::tables::body,resonance)*65536;
                int expected=acidline_direct::distort(acidline_direct::limit(opxy::q31::multiply(int(i*64)-32768,body)*6,32768),drive,acidline_direct::distortion_gain(drive));
                if(expected>32767)expected=32767;
                require(v.shape_curve[i]==expected,"completed entries equal the frozen direct transfer");
            }else require(v.shape_curve[i]==0x5a5a,"unbuilt entries remain untouched");
        }
    }
    require(calls==65,"1025 points complete in exactly65 bounded tiles");
    Voice complete=v;prepare_shape(v,resonance,drive);require(!std::memcmp(&v,&complete,sizeof(v)),"a complete matching cache performs no rebuild");
    prepare_shape(v,resonance,drive+1);require(v.shape_progress==16&&!v.shape_ready&&!v.shape_transition,"Drive invalidates and restarts only one tile");
    // Poison every unfinished point: reading any partial table changes audio.
    // The first64 complete callbacks must exactly match the frozen direct path.
    initialize(v,shared,1);for(auto& point:v.shape_curve)point=0x5a5a;
    acidline_direct::Voice direct={};acidline_direct::Shared ds;acidline_direct::initialize(direct,ds,1);
    i32 p[8]={0x40000000,0x48000000,0x58000000,0x50000000,0x40000000,0,0x30000000,0x20000000};
    i32 actual[64],expected[64];u32 pitch[64];for(auto& value:pitch)value=10713067;
    for(unsigned block=0;block<64;++block){
        render(v,shared,1,actual,p,pitch);acidline_direct::render(direct,ds,1,expected,p,pitch);
        require(!v.shape_ready&&v.shape_progress==(block+1)*16,"render prepares exactly one unfinished tile");
        require(!std::memcmp(actual,expected,sizeof(actual)),"cold audio remains exactly direct until completion");
        require(!std::memcmp(&v,&direct,sizeof(direct)),"cold phase/filter/output state remains exactly direct");
    }
    render(v,shared,1,actual,p,pitch);acidline_direct::render(direct,ds,1,expected,p,pitch);
    require(v.shape_ready&&v.shape_transition&&v.shape_progress==1025,"completed cache enters a single transition block");
    require(std::abs(double(actual[0])-expected[0])<=16*32768.,"transition begins within the bounded first crossfade step");
    const unsigned progress=v.shape_progress;p[6]^=0x18000000;p[7]^=0x20000000;
    render(v,shared,1,actual,p,pitch);
    require(v.shape_ready&&v.shape_progress==progress,"Sub and Damp never invalidate the shaping cache");
    start(v,shared,1);require(v.shape_ready&&v.shape_progress==progress,"retrigger retains a matching per-voice cache");
    initialize(v,shared,1);require(!v.shape_ready&&!v.shape_progress&&!v.shape_transition,"initialization resets all generation state");
    acidline_direct::initialize(direct,ds,1);
    for(unsigned block=0;block<128;++block){
        p[1]=block&1?0x10000000:0x70000000;p[2]=block&1?0x60000000:0x20000000;
        render(v,shared,1,actual,p,pitch);acidline_direct::render(direct,ds,1,expected,p,pitch);
        require(!v.shape_ready&&v.shape_progress==(block==0?16:0),"moving shaping controls invalidate without wasted generation work");
        require(!std::memcmp(actual,expected,sizeof(actual)),"automated controls render immediately through the direct path");
    }
    Voice a={},b={},other={};initialize(a,shared,1);initialize(b,shared,1);initialize(other,shared,2);i32 discard[64];
    for(unsigned block=0;block<100;++block){
        render(a,shared,1,actual,p,pitch);i32 q[8];std::memcpy(q,p,sizeof(q));q[1]^=0x38000000;render(other,shared,2,discard,q,pitch);render(b,shared,1,expected,p,pitch);
        require(!std::memcmp(actual,expected,sizeof(actual))&&!std::memcmp(&a,&b,sizeof(a)),"interleaved voices keep independent cache generation and audio");
    }
    std::puts("PASS:16-point cache work bound, exact65-tile transfer, no partial reads, direct cold/automated audio, transition, invalidation, resets and voice isolation");
}
