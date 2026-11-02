#include "../engine.cpp"
#include <cstdio>

static bool exact(opxy::i32 a,opxy::i32 b) {
    const opxy::i64 sum=static_cast<opxy::i64>(a)+b;
    const opxy::i64 maximum=2147483647,minimum=-maximum-1;
    return subvista::add(a,b)==(sum>maximum?maximum:sum<minimum?minimum:sum);
}
int main() {
    for(unsigned band=0;band<8;++band)for(unsigned index=0;index<512;++index) {
        const opxy::i32 slope=(subvista::tables::saw[band][index+1]-subvista::tables::saw[band][index])/32768;
        if(subvista::tables::slopes[band][index]!=slope)return 1;
        for(unsigned fraction=0;fraction<32768;fraction+=127) {
            const opxy::u32 phase=(index<<23)|(fraction<<8)|255u;
            if(subvista::wave(phase,band)!=subvista::tables::saw[band][index]+slope*static_cast<opxy::i32>(fraction))return 1;
        }
    }
    const opxy::i32 values[]={-2147483647-1,-2147483647,-1610612736,-1073741824,-1,0,1,1073741824,1610612736,2147483646,2147483647};
    for(auto a:values)for(auto b:values)if(!exact(a,b))return 1;
    for(auto a:values)for(opxy::i32 delta=-4096;delta<=4096;++delta) {
        if(!exact(a,delta)||!exact(a,2147479551+delta)||!exact(a,-2147479552+delta))return 1;
    }
    opxy::u32 random=7139;
    for(unsigned i=0;i<1000000;++i) {
        const opxy::i32 a=static_cast<opxy::i32>(random=random*1664525u+1013904223u);
        const opxy::i32 b=static_cast<opxy::i32>(random=random*1664525u+1013904223u);
        if(!exact(a,b))return 1;
    }
    std::puts("PASS: exact wave slopes/interpolation and saturating sums at boundaries and one million random pairs");
}
