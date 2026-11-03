#include "../engine.cpp"
#include <cstdio>

int main() {
    const unsigned fractions[]={0,1,127,16383,16384,32766,32767};
    for(unsigned mode=0;mode<4;++mode)for(unsigned level=0;level<9;++level)
        for(unsigned band=0;band<6;++band)for(unsigned index=0;index<512;++index) {
            const auto* original=vader::waves[mode][level][band];
            const auto* packed=vader::packed::waves[mode][level][band];
            for(unsigned fraction:fractions) {
                const opxy::u32 phase=(index<<23)|(fraction<<8)|255u;
                const opxy::i32 expected=vader::lookup(original,phase);
                const auto position=vader::packed_position(phase);
                const opxy::i32 actual=vader::packed_lookup(packed,position.index,position.fraction);
                if(actual!=expected) {
                    std::printf("FAIL: packed interpolation %u/%u/%u index%u fraction%u\n",
                                mode,level,band,index,fraction);return 1;
                }
            }
        }
    std::puts("PASS: exact packed Q31 interpolation against original samples in all216slots and512phases");
}
