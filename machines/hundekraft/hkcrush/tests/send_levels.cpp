#include "../fx.cpp"
#include <cstdio>
using namespace hkcrush;
static opxy::i32 memory[64];
int main() {
    State state={}; initialize(state);
    opxy::i32 p[8]={2147483647,2147483647,0,0};
    const opxy::i32 levels[]={512,8192,1048576,268435456};
    for(opxy::i32 level:levels) {
        for(unsigned block=0;block<4;++block) {
            opxy::i32 l[64],r[64];
            for(unsigned i=0;i<64;++i) {l[i]=(i&1)?level:-level;r[i]=-l[i];}
            process(state,opxy::Buffer{memory,64},l,r,p);
            if(!block) continue;
            for(unsigned i=0;i<64;++i) {
                const opxy::i32 expected=(i&1)?level:-level;
                const opxy::i32 tolerance=level/4096+2;
                if(l[i]<expected-tolerance||l[i]>expected+tolerance||
                   r[i]<-expected-tolerance||r[i]>-expected+tolerance) {
                    std::puts("Transparent Crush lost quiet-send precision or changed gain"); return 1;
                }
            }
        }
    }
    initialize(state);
    p[1]=0;
    for(unsigned block=0;block<3;++block) {
        opxy::i32 l[64],r[64];
        for(unsigned i=0;i<64;++i) {l[i]=512;r[i]=-512;}
        if(block==2) {l[63]=8192;r[63]=-8192;}
        process(state,opxy::Buffer{memory,64},l,r,p);
        if(block==2) for(unsigned i=0;i<64;++i) {
            if(l[i]<511||l[i]>513||r[i]<-513||r[i]>-511) {
                std::printf("Working-level change jumped the held sample: %u %ld %ld\n",i,static_cast<long>(l[i]),static_cast<long>(r[i]));return 2;
            }
        }
    }
    return 0;
}
