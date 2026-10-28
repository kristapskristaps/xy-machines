// Sound, buffer ownership and interleaved voice isolation checks.
#include "../engine.cpp"
#include <cstdio>
using namespace hksid;
static Voice target, other;
static Shared shared;
static opxy::i32 reference[256][64];
struct Guarded { opxy::u32 before; opxy::i32 samples[64]; opxy::u32 after; };
int main() {
    opxy::i32 parameters[8]={536870912,1073774593,2106129169,536887296,0,0,0,0};
    const opxy::i32 unchanged[8]={536870912,1073774593,2106129169,536887296,0,0,0,0};
    opxy::u32 pitch[64]; for(unsigned i=0;i<64;++i) pitch[i]=25480391u;
    bool audible=false;
    for(unsigned pass=0;pass<2;++pass) {
        initialize(target,shared,1);start(target,shared,1);
        initialize(other,shared,2);start(other,shared,2);
        for(unsigned block=0;block<256;++block) {
            Guarded out={0xdeadbeefu,{},0xabcdef01u};
            render(target,shared,1,out.samples,parameters,pitch);
            if(out.before!=0xdeadbeefu || out.after!=0xabcdef01u) return 10;
            for(unsigned i=0;i<64;++i) {
                if(out.samples[i]<-1073741824 || out.samples[i]>1073741824) return 11;
                audible=audible || out.samples[i]!=0;
                if(pass==0) reference[block][i]=out.samples[i];
                else if(reference[block][i]!=out.samples[i]) {std::puts("voice state leaked between instances");return 12;}
                if(pitch[i]!=25480391u) return 13;
            }
            if(pass) {
                opxy::i32 alternate[8];for(unsigned i=0;i<8;++i) alternate[i]=i&1?0:0x7fffffff;
                render(other,shared,2,out.samples,alternate,pitch);
            }
        }
        destroy(target,shared,1);destroy(other,shared,2);
    }
    for(unsigned i=0;i<8;++i) if(parameters[i]!=unchanged[i]) return 14;
    if(!audible) {std::puts("default patch is silent");return 15;}
    return 0;
}
