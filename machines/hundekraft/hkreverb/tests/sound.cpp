// Dirty-buffer recovery, send isolation, dry routing and output guards.
#include "../fx.cpp"
#include <cstdio>
using namespace hkreverb;
struct Memory { opxy::u32 before; alignas(8) opxy::i32 words[88404]; opxy::u32 after; };
static Memory a,b;
int main() {
    a.before=b.before=0x12345678u;a.after=b.after=0x87654321u;
    for(unsigned i=0;i<88404;++i) {a.words[i]=0x42424242;b.words[i]=-12424242;}
    State first={},second={};initialize(first);initialize(second);
    opxy::i32 p[8]={1048608000,1310760001,1073774593,2147483647,0,0,0,0}, other[8]={1048608000,1310760001,1073774593,2147483647,0,0,0,0};other[3]=0;
    
    bool audible=false;
    for(unsigned block=0;block<4000;++block) {
        
        
        opxy::i32 l[64],r[64],l2[64],r2[64];
        for(unsigned i=0;i<64;++i) {
            const unsigned t=block*64+i;
            l[i]=l2[i]=static_cast<opxy::i32>((t*100003u)&0x3fffffffu)-0x20000000;
            r[i]=r2[i]=static_cast<opxy::i32>((t*73333u)&0x3fffffffu)-0x20000000;
        }
        process(first,opxy::Buffer{a.words,88404},l,r,p);
        process(second,opxy::Buffer{b.words,88404},l2,r2,other);
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
        process(first,opxy::Buffer{a.words,88404},l,r,p);
        for(unsigned i=0;i<64;++i) if(l[i]||r[i]) {std::puts("old effect tail survived reset");return 23;}
    }
    return 0;
}
