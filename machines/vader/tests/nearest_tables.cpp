#include "../engine.cpp"
#include <cstdio>

int main() {
    const unsigned fractions[]={0,1,8191,8192,16383,16384,24575,24576,32767};
    for(unsigned mode=0;mode<4;++mode)for(unsigned level=0;level<9;++level)
        for(unsigned band=0;band<6;++band) {
            const auto* original=vader::waves[mode][level][band];
            const auto* dense=vader::nearest::waves[mode][level][band];
            for(unsigned point=0;point<1024;++point) {
                const unsigned phase=point<<22;
                if(dense[vader::nearest_index(phase)]!=vader::lookup(original,phase))return 1;
            }
            for(unsigned index=0;index<512;++index)for(unsigned fraction:fractions) {
                const unsigned phase=(index<<23)|(fraction<<8)|255u;
                const int before=vader::lookup(original,phase),after=dense[vader::nearest_index(phase)];
                const int difference=original[index+1]-original[index];
                const int bound=((difference<0?-difference:difference)+3)/4+1;
                const int error=after-before;
                if(error>bound || error<-bound)return 1;
            }
        }
    std::puts("PASS: all216 original banks preserved at1024gridpoints; bounded nearest-phase error and quarter reflection");
}
