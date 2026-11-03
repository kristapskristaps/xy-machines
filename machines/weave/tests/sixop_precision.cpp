#include "../vendor/macro.cpp"
#include <cmath>
#include <cstdio>

int main() {
    using namespace weave_dsp;
    static_assert(32768LL*16384<2147483647LL,"Signed16 sine/amplitude product headroom");
    double maximum_slope=0;
    for(unsigned i=0;i<512;++i) {
        const double slope=std::fabs(double(dx7_sine[i+1])-dx7_sine[i]);
        if(slope>maximum_slope)maximum_slope=slope;
    }
    // Half a lookup cell, one Q15 sine unit (including positive-peak clipping),
    // half a Q12 amplitude unit, and the final integer shift.
    const double bound=(maximum_slope/(2*(1u<<(dx_sine_phase_bits-9)))+32768)*.25+16384*.5+1;
    if(bound>70000)return 1;
    opxy::u32 random=179;
    double worst=0,energy=0,error_energy=0,mean=0;
    for(unsigned i=0;i<1048576;++i) {
        random=random*1664525u+1013904223u;
        const unsigned phase=random;
        const opxy::i32 amplitude=static_cast<opxy::i32>((random*2654435761u)&0x1fffffffu);
        const unsigned index=phase>>23;
        const double fraction=(phase&0x7fffffu)/8388608.;
        const double sine=dx7_sine[index]+(dx7_sine[index+1]-dx7_sine[index])*fraction;
        const double expected=sine*amplitude/2147483648.;
        const double actual=dx_sine_product(phase,amplitude),error=actual-expected;
        if(std::fabs(error)>worst)worst=std::fabs(error);
        if(std::fabs(error)>bound)return 1;
        energy+=expected*expected;error_energy+=error*error;mean+=error;
    }
    const double snr=10*std::log10(energy/error_energy);
    if(snr<77 || std::fabs(mean/1048576)>64)return 1;
    std::printf("PASS: 1,048,576 independent operator products, max %.2f Q26 LSB, SNR %.2f dB, mean %.3f LSB\n",worst,snr,mean/1048576);
}
