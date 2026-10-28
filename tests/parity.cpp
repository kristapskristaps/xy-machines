#include "../audition/bridge.cpp"
#include <cstdio>
int main() {
    opxy::i32* l=halo_left();opxy::i32* r=halo_right();opxy::i32* p=halo_parameters();
    opxy::u32 phase=0,rng=21,hash=2166136261u;
    for(unsigned block=0;block<2400;++block) {
        if(!(block%73))for(unsigned i=0;i<8;++i)p[i]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        if(block==1200)halo_reset();
        for(unsigned i=0;i<64;++i){phase+=23192823u;l[i]=static_cast<opxy::i32>(phase)/4;r[i]=-l[i];}
        halo_process(block%3?1200:600,block,true);
        for(unsigned i=0;i<64;++i){hash=(hash^static_cast<opxy::u32>(l[i]))*16777619u;hash=(hash^static_cast<opxy::u32>(r[i]))*16777619u;}
    }
    std::printf("%u\n",hash);
}
