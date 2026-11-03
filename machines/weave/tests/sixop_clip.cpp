#include "../vendor/macro.cpp"
#include <cstdio>

int main() {
    using namespace weave_dsp;
    int minimum_error=0,maximum_error=0;
    for(uint32_t magnitude=0;magnitude<=(3u<<24);++magnitude) {
        const int32_t sample=static_cast<int32_t>(magnitude);
        const int32_t squared=fmac1(defined_shift(sample,3),defined_shift(sample,3));
        const uint32_t numerator=static_cast<uint32_t>((27<<23)+squared)<<3;
        const uint32_t divisor=static_cast<uint32_t>((27<<23)+9*squared)>>13;
        const uint32_t before=numerator/divisor;
        const uint32_t after=sixop_clip_ratio(magnitude,numerator,divisor);
        if(before!=after){std::printf("FAIL: exact SixOp quotient at %u: %u!=%u\n",magnitude,before,after);return 1;}
        const unsigned index=magnitude>>16;
        const int fraction=static_cast<int>((magnitude&65535u)>>1);
        const int delta=sixop_clip_seed[index+1]-sixop_clip_seed[index];
        if(delta<-32768 || delta>32767)return 1;
        const int estimate=sixop_clip_seed[index]+1+(delta*fraction>>15);
        const int error=estimate-static_cast<int>(before);
        if(error<minimum_error)minimum_error=error;
        if(error>maximum_error)maximum_error=error;
        if(error<-3 || error>2 || static_cast<unsigned long long>(estimate)*divisor>0xffffffffull)return 1;
        // Preserve original final signed rounding and int16 endpoint behavior.
        const int old_coefficient=before>=65536?2147483647:defined_shift(static_cast<int>(before),15);
        const int new_coefficient=after>=65536?2147483647:defined_shift(static_cast<int>(after),15);
        for(int sign=-1;sign<=1;sign+=2) {
            const int old_q24=fmac1(sign*sample,old_coefficient);
            const int new_q24=fmac1(sign*sample,new_coefficient);
            if(static_cast<int16_t>((old_q24+256)>>9)!=static_cast<int16_t>((new_q24+256)>>9))return 1;
        }
    }
    // Beyond |3| the surrounding original clamp is unchanged; the exact
    // negative endpoint remains one sample count different from that clamp.
    const int s=3<<24,x2=fmac1(defined_shift(s,3),defined_shift(s,3));
    const unsigned ratio=(static_cast<unsigned>((27<<23)+x2)<<3)/(static_cast<unsigned>((27<<23)+9*x2)>>13);
    const int coefficient=defined_shift(static_cast<int>(ratio),15);
    if(static_cast<int16_t>((fmac1(s,coefficient)+256)>>9)!=-32768 ||
       static_cast<int16_t>((fmac1(-s,coefficient)+256)>>9)!=-32767)return 1;
    std::printf("PASS: all 50331649 magnitudes, both signed integer output curves, exact quotient; seed error [%d,%d], table %u bytes\n",minimum_error,maximum_error,unsigned(sizeof(sixop_clip_seed)));
}
