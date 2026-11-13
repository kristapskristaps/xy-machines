#include "../engine.cpp"
#include "reference/voice.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
int main() {
    using namespace x0x;
    const unsigned count = 2048;
    for (unsigned model = closed_hat; model <= cymbal; ++model) {
        Voice a, b, legacy; Shared shared;
        initialize(a,shared,1); initialize(b,shared,2);
        start(a,shared,1); start(b,shared,2);
        drum_select(a,shared,1,model); drum_select(b,shared,2,model);legacy=a;
        opxy::i32 parameters[8] = {0x50000000,0x40000000,0x30000000,0x40000000,0,0x40000000,0x40000000,0};
        opxy::i32 out[64], other[64],reference[64]; opxy::u32 pitch[64] = {};
        double samples[count],old_samples[count];
        for (unsigned frame = 0; frame < count; frame += 64) {
            render(a,shared,1,out,parameters,pitch); render(b,shared,2,other,parameters,pitch);dsp::legacy_render(legacy,reference,parameters);
            for (unsigned i = 0; i < 64; ++i) {
                require(out[i] == other[i],"808 metallic source must not contain the noise generator");
                old_samples[frame+i]=reference[i]/2147483648.0*(.5-.5*std::cos(2*3.141592653589793*(frame+i)/(count-1)));
                samples[frame+i] = out[i]/2147483648.0 * (.5-.5*std::cos(2*3.141592653589793*(frame+i)/(count-1)));
            }
        }
        // The circuit's upper bands must dominate, rather than the original
        // approximation's sparse pitched partials below 2 kHz. DFT is host-only.
        double energy = 0, low = 0, high = 0, centroid = 0,old_energy=0,old_centroid=0,old_high=0;
        for (unsigned bin = 1; bin <= count/2; ++bin) {
            double real = 0, imaginary = 0,old_real=0,old_imaginary=0;
            const double frequency = 44100.0*bin/count;
            for (unsigned i = 0; i < count; ++i) {
                const double angle = 2*3.141592653589793*bin*i/count;
                real += samples[i]*std::cos(angle); imaginary += samples[i]*std::sin(angle);
                old_real+=old_samples[i]*std::cos(angle);old_imaginary+=old_samples[i]*std::sin(angle);
            }
            const double power = real*real+imaginary*imaginary,old_power=old_real*old_real+old_imaginary*old_imaginary;
            old_energy+=old_power;old_centroid+=old_power*frequency;if(frequency>6000)old_high+=old_power;
            energy += power; centroid += power*frequency;
            if (frequency < 2000) low += power;
            if (frequency > 6000) high += power;
        }
        const double centroid_ratio=(centroid/energy)/(old_centroid/old_energy);
        const double high_ratio=(high/energy)/(old_high/old_energy);
        require(centroid_ratio>.8 && centroid_ratio<1.25 && high_ratio>.7 && high_ratio<1.43,"default stem spectral balance stays close to frozen live synthesis");
        std::printf("New/legacy spectral centroid%.6f, upper-band proportion%.6f\n",centroid_ratio,high_ratio);
        require(energy > .0001,"808 metallic output has useful energy");
        require(low/energy < .15,"808 metallic fundamental leakage is suppressed");
        require(high/energy > .4 && centroid/energy > 5000,"808 metallic output retains the upper overtone bands");
        std::printf("808 %s: %.1f%% below 2 kHz, %.1f%% above 6 kHz, centroid %.0f Hz\n",
                    short_name(model),100*low/energy,100*high/energy,centroid/energy);
    }
    std::puts("PASS: 808 hat/cymbal upper-band spectrum and noise-free rectangular source");
}
