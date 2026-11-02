#include "../vendor/macro.cpp"
#include <cstdio>
int main(){
    unsigned rng=71;
    for(unsigned divisor=32768;divisor<=65535;++divisor){
        if(weave_dsp::divide_modal_seed(divisor)!=1073741824u/divisor){
            std::printf("FAIL Modal seed / %u\n",divisor);return 1;
        }
    }
    const int edges[]={-2147483647-1,-2147483647,-65537,-33,-32,-31,-17,-16,-15,-1,0,1,15,16,17,31,32,33,65537,2147483647};
    for(int frames=1;frames<=64;++frames){
        for(auto value:edges)if(weave_dsp::divide_block(value,frames)!=value/frames){std::printf("FAIL block %d / %d\n",value,frames);return 1;}
        for(unsigned i=0;i<10000;++i){
            const int value=static_cast<int>(rng=rng*1664525u+1013904223u);
            if(weave_dsp::divide_block(value,frames)!=value/frames){std::printf("FAIL block %d / %d\n",value,frames);return 1;}
        }
    }
    for(unsigned divisor=16384;divisor<=2097152;++divisor){
        const unsigned cases[]={0,divisor-1,divisor,0x7fffffffu,0x80000000u,0xffffffffu,rng=rng*1664525u+1013904223u};
        for(auto numerator:cases)if(weave_dsp::divide_audio(numerator,divisor)!=numerator/divisor){std::printf("FAIL %u / %u\n",numerator,divisor);return 1;}
    }
    for(unsigned i=0;i<1000000;++i){
        const unsigned divisor=16384+(rng=rng*1664525u+1013904223u)%(2097152-16384+1);
        const unsigned numerator=rng=rng*1664525u+1013904223u;
        if(weave_dsp::divide_audio(numerator,divisor)!=numerator/divisor)return 1;
    }
    std::puts("PASS: every Modal seed divisor, signed block ramps for lengths 1..64, audio division across every divisor and randomized numerators");
}
