// Dirty-buffer recovery, send isolation, dry routing and output guards.
#include "../fx.cpp"
#include <cstdio>
using namespace hkinaroom;
struct Memory { opxy::u32 before; alignas(8) opxy::i32 words[89532]; opxy::u32 after; };
static Memory a,b;
int main() {
    a.before=b.before=0x12345678u;a.after=b.after=0x87654321u;
    for(unsigned i=0;i<89532;++i) {a.words[i]=0x42424242;b.words[i]=-12424242;}
    State first={},second={};initialize(first);initialize(second);
    opxy::i32 p[8]={1894048201,1879048191,851994000,2147483647,530857800,393228000,0,0}, other[8]={1894048201,1879048191,851994000,2147483647,530857800,393228000,0,0};other[3]=0;
    opxy::Transport clock={1200,0,true};
    bool audible=false;
    for(unsigned block=0;block<4000;++block) {
        clock.ticks96=block*64*192/44100;
        if(block==200) p[7]=other[7]=0x7fffffff;
        opxy::i32 l[64],r[64],l2[64],r2[64];
        for(unsigned i=0;i<64;++i) {
            const unsigned t=block*64+i;
            l[i]=l2[i]=static_cast<opxy::i32>((t*100003u)&0x3fffffffu)-0x20000000;
            r[i]=r2[i]=static_cast<opxy::i32>((t*73333u)&0x3fffffffu)-0x20000000;
        }
        process(first,opxy::Buffer{a.words,89532},clock,l,r,p);
        process(second,opxy::Buffer{b.words,89532},clock,l2,r2,other);
        if(a.before!=0x12345678u||b.before!=0x12345678u||a.after!=0x87654321u||b.after!=0x87654321u) return 20;
        for(unsigned i=0;i<64;++i) {
            if(l[i]!=l2[i]||r[i]!=r2[i]) {std::puts("dirty memory or Dry leaked into wet output");return 21;}
            audible=audible||l[i]!=0||r[i]!=0;
        }
    }
    if(!audible) {std::puts("default effect never produced output");return 22;}
    initialize(first);
    for(unsigned block=0;block<200;++block) {
        opxy::i32 l[64]={},r[64]={};
        process(first,opxy::Buffer{a.words,89532},clock,l,r,p);
        for(unsigned i=0;i<64;++i) if(l[i]||r[i]) {std::puts("old effect tail survived reset");return 23;}
    }
    return 0;
}
