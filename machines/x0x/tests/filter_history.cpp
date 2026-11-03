#include "../engine.cpp"
#include <cstdio>
#include <cstring>

int main() {
    using namespace x0x::dsp;
    const MetalCoefficients coefficients[]={low_band,high_band,low_cut,hat_cut,air_cut};
    u32 rng=7139;
    for(const auto& c:coefficients) {
        for(i32 x=-32767;x<=32767;++x) {
            if(metal_product(x,-c.b0)!=-metal_product(x,c.b0)) {
                std::puts("FAIL: truncating metallic products are not odd");return 1;
            }
        }
        for(u32 frames=0;frames<=opxy::block_frames;++frames)for(unsigned alias=0;alias<2;++alias)
            for(unsigned variant=0;variant<16;++variant) {
                MetalState actual={},expected={};
                i32 initial[4];
                for(auto& x:initial) {
                    rng=rng*1664525u+1013904223u;
                    x=variant==0?32767:variant==1?-32767:i32(rng%65535u)-32767;
                }
                actual.filters[0]={initial[0],initial[1],initial[2],initial[3]};
                expected=actual;
                i32 source[64],input[64],output[64],reference[64];
                for(unsigned i=0;i<64;++i) {
                    rng=rng*1664525u+1013904223u;
                    source[i]=variant<2?((i&1)?-32767:32767):i32(rng%65535u)-32767;
                    input[i]=source[i];output[i]=reference[i]=alias?source[i]:0x12345678;
                }
                for(u32 i=0;i<frames;++i)
                    reference[i]=metal_filter(expected.filters[0],c,source[i]);
                metal_filter_block<0>(actual,c,alias?output:input,output,frames);
                if(std::memcmp(output,reference,sizeof(output)) || std::memcmp(&actual,&expected,sizeof(actual)) ||
                   std::memcmp(input,source,sizeof(input))) {
                    std::printf("FAIL: weighted metallic history frames %u alias %u variant %u\n",frames,alias,variant);
                    return 1;
                }
            }
    }
    std::puts("PASS: weighted metallic history equals scalar per-product filtering for every partial block, aliasing, clamped extremes and random state");
}
