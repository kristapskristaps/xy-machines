#include "../dsp.h"
#include <cstdio>
#include <initializer_list>

static int magnitude(int n) { return n<0?-n:n; }
static bool row(const opxy::i16* table) {
    const unsigned fractions[]={0,1,2,127,1023,4095,8191,16383,16384,24575,32765,32766,32767};
    for(unsigned i=0;i<512;++i) {
        const int difference=table[i+1]-table[i];
        if(difference < -32768 || difference > 32767)return false;
        for(unsigned f:fractions) {
            const unsigned phase=(i<<23)|(f<<8)|255u;
            const int expected=table[i]+((difference*static_cast<int>(f))>>15);
            if(westfold::dsp::read(table,phase)!=expected)return false;
        }
    }
    return true;
}
int main() {
    if(!row(westfold::tables::sine))return 1;
    int maximum_slope=0,maximum_mip=0;
    for(unsigned band=0;band<8;++band)for(unsigned fold=0;fold<17;++fold) {
        const auto* points=westfold::tables::waves[band][fold];
        if(!row(points))return 1;
        for(unsigned i=0;i<513;++i) {
            if(i<512 && magnitude(points[i+1]-points[i])>maximum_slope)maximum_slope=magnitude(points[i+1]-points[i]);
            if(band<7 && magnitude(westfold::tables::waves[band+1][fold][i]-points[i])>maximum_mip)maximum_mip=magnitude(westfold::tables::waves[band+1][fold][i]-points[i]);
        }
    }
    std::printf("PASS: all 137 adjacent lookup rows, adjacent mip bound%d and phase slope%d; exact signed16 lookup products\n",maximum_mip,maximum_slope);
}
