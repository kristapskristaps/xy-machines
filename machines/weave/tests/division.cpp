#include "../vendor/macro.cpp"
#include <cstdio>
int main(){
    unsigned rng=71;
    for(unsigned divisor=16384;divisor<=2097152;++divisor){
        const unsigned cases[]={0,divisor-1,divisor,0x7fffffffu,0x80000000u,0xffffffffu,rng=rng*1664525u+1013904223u};
        for(auto numerator:cases)if(weave_dsp::divide_audio(numerator,divisor)!=numerator/divisor){std::printf("FAIL %u / %u\n",numerator,divisor);return 1;}
    }
    for(unsigned i=0;i<1000000;++i){
        const unsigned divisor=16384+(rng=rng*1664525u+1013904223u)%(2097152-16384+1);
        const unsigned numerator=rng=rng*1664525u+1013904223u;
        if(weave_dsp::divide_audio(numerator,divisor)!=numerator/divisor)return 1;
    }
    std::puts("PASS: exact audio division across every divisor and randomized numerators");
}
