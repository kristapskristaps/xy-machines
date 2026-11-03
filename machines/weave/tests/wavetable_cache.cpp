#include "../vendor/macro.cpp"
#include <cmath>
#include <cstdio>

int main() {
    using namespace weave_dsp;
    // Trilinear table construction is checked against independent floating
    // weights, including both mirrored bank joins and all eight source corners.
    opxy::u32 random=897;
    double worst=0;
    for(unsigned test=0;test<4096;++test) {
        opxy::i32 key[3],table[132];
        for(unsigned k=0;k<3;++k){random=random*1664525u+1013904223u;key[k]=random%117440512u;}
        wt_blend_tile<1>(key,table,0,132);
        const int x=key[0]>>24,y=key[1]>>24,z=key[2]>>24;
        const double xf=(key[0]&0xffffff)/16777216.,yf=(key[1]&0xffffff)/16777216.,zf=(key[2]&0xffffff)/16777216.;
        const int z0=z>=4?7-z:z,z1=z+1>=4?6-z:z+1;
        for(unsigned i=0;i<132;++i) {
            double expected=0;
            for(unsigned corner=0;corner<8;++corner) {
                const double weight=(corner&1?xf:1-xf)*(corner&2?yf:1-yf)*(corner&4?zf:1-zf);
                expected+=wt_wave(x+(corner&1),y+((corner>>1)&1),corner&4?z1:z0)[i]*256.*weight;
            }
            const double error=std::fabs(table[i]-expected);if(error>worst)worst=error;
            if(error>7)return 1;
        }
    }
    macro_wavetable state={};wt_init(&state);
    const opxy::u8 parameters[7]={80,64,64,64};opxy::i32 output[32],auxiliary[32];
    for(unsigned test=0;test<10000;++test) {
        const unsigned n=1+test%32;
        opxy::u8 moving[7];for(unsigned k=0;k<7;++k)moving[k]=parameters[k];
        random=random*1664525u+1013904223u;
        moving[1]=random&127;moving[2]=(random>>8)&127;moving[3]=(random>>16)&127;
        const unsigned old_at=state.build_at,old_building=state.building;
        wt_render(&state,moving,4934440u+random%100000000u,output,auxiliary,n);
        if(state.build_at>132 || state.fade>64 || state.active>1)return 1;
        if(old_building && state.build_at>old_at+(n/2?n/2:1))return 1;
        if(!state.valid)return 1;
    }
    std::printf("PASS: 540,672 independently weighted samples (max %.4f Q8 LSB), bounded tiles and arbitrary block lengths\n",worst);
}
