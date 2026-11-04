#include "../vendor/macro.cpp"
#include <cstdio>
#include <cstdlib>
static void require(bool ok,const char* text) {
    if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}
}
static int32_t original_gain(int32_t x) {
    x=x>65535?65535:x< -65535? -65535:x;
    x=(x*26214)>>15;
    return x>32767?32767:x< -32768? -32768:x;
}
int main() {
    using namespace weave_chords_dsp;
    uint32_t random=188419u;unsigned cases=0;int maximum=0;
    const int32_t edges[]={-2147483647,-65536,-40961,-32768,-1,0,1,32767,40960,65535,2147483647};
    for(unsigned blend=1;blend<17;++blend)for(int32_t input=-65536;input<=65536;++input) {
        random=random*1664525u+1013904223u;
        const int32_t other=(input%11==0)?edges[(random>>16)%11]:static_cast<int32_t>(random);
        const int32_t weight=static_cast<int32_t>(blend*1927),x=original_gain(input),y=original_gain(other);
        const int32_t expected=x+(((y-x)*weight)>>15);
        const int32_t actual=chord_output_blend(input,other,static_cast<uint16_t>(weight));
        const int delta=std::abs(actual-expected);if(delta>maximum)maximum=delta;
        require(actual>=-32768 && actual<=32767,"narrowed output blend cannot overflow signed16");
        require(delta<=1,"narrowed half-difference changes at most one output bit");++cases;
    }
    std::printf("PASS: %u output blend comparisons across rails, maximum error %d Q15 bit\n",cases,maximum);
}
