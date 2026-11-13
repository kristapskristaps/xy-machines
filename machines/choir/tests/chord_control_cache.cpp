#include "../vendor/macro.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
static void require(bool ok,const char* text) {
    if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}
}
int main() {
    using namespace choir_dsp;
    uint32_t seed=38741u;unsigned reused=0;
    const int blocks[]={1,3,8,12,16,32,64};
    const unsigned rates[]={44100u,48000u};
    for(unsigned rate:rates) {
        macro_chord cached,reference;chord_init(&cached);chord_init(&reference);
        uint8_t p[7]={64,0,0,0,0,0,0};uint32_t inc=1234567;
        for(unsigned block=0;block<7000;++block) {
            if(block%151==0) {
                for(unsigned control=1;control<=3;++control) {
                    seed=seed*1664525u+1013904223u;p[control]=static_cast<uint8_t>(seed>>25);
                }
                seed=seed*1664525u+1013904223u;inc=seed%1200000000u;
            }
            const int n=blocks[block%7];int32_t out[64],aux[64],expected[64],root[64];
            const int32_t old_morph=cached.prepared_morph,old_timbre=cached.prepared_timbre;
            const bool valid=cached.prepared_valid;
            chord_render(&cached,p,inc,out,aux,n,rate);
            reference.prepared_valid=0;reference.smooth_n=0;chord_render(&reference,p,inc,expected,root,n,rate);
            require(!std::memcmp(out,expected,n*sizeof(int32_t)),"cached full chord remains bit exact");
            require(!std::memcmp(aux,root,n*sizeof(int32_t)),"cached root AUX remains bit exact");
            require(!std::memcmp(&cached,&reference,offsetof(macro_chord,prepared_inc)),"phases, ramps, smoothing and chord selection remain exact");
            if(valid && old_morph==cached.prepared_morph && old_timbre==cached.prepared_timbre)++reused;
        }
    }
    require(reused>1000,"held controls reuse prepared voicings");
    std::printf("PASS: 14000 variable-block chord comparisons; %u held preparations reused\n",reused);
}
