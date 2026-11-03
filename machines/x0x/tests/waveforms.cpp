#include "../engine.cpp"
#include "waveforms.h"
#include "reference/voice.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main() {
    using namespace x0x;
    unsigned count = 0;double minimum_rms=2,maximum_rms=0,maximum_error=0;
    for (unsigned model = 0; model < model_count; ++model) {
        for (unsigned family = 0; family < 4; ++family) {
            if (!(family_masks[model] & (1u << family))) continue;
            unsigned long long hash = 14695981039346656037ull;
            double old_energy=0,new_energy=0,error_energy=0;
            Voice voice,legacy; Shared shared; opxy::i32 output[64],reference[64],parameters[8];
            opxy::u32 pitch[64] = {};
            initialize(voice, shared, 1);initialize(legacy,shared,1);
            for (unsigned slot = 0; slot < 8; ++slot) {
                for (unsigned value = 0; value < 128; ++value) {
                    const unsigned raw[8] = {family_raw(model,family),64,100,64,0,64,64,0};
                    for (unsigned i = 0; i < 8; ++i) parameters[i] = opxy::i32(raw[i] << 24);
                    parameters[slot] = opxy::i32(value << 24);
                    start(voice, shared, 1); drum_select(voice, shared, 1, model);
                    start(legacy,shared,1);drum_select(legacy,shared,1,model);
                    for (unsigned block = 0; block < 8; ++block) {
                        // Exercise cache hits, every control's invalidation,
                        // model changes during a note, and live style changes.
                        if (block == 3) parameters[(slot+1)%8] = opxy::i32(((value+31)%128) << 24);
                        if (block == 5){drum_select(voice, shared, 1, (model+1)%model_count);drum_select(legacy,shared,1,(model+1)%model_count);}
                        render(voice, shared, 1, output, parameters, pitch);
                        dsp::legacy_render(legacy,reference,parameters);
                        for(unsigned i=0;i<64;++i){
                            hash=(hash^opxy::u32(reference[i]))*1099511628211ull;
                            const double a=reference[i]/2147483648.,b=output[i]/2147483648.;
                            old_energy+=a*a;new_energy+=b*b;error_energy+=(a-b)*(a-b);
                        }
                    }
                }
            }
            if (hash != waveform_reference[count]) {
                std::fprintf(stderr,"FAIL: reference waveform changed: model %u family %u\n",model,family);
                return 1;
            }
            const double rms=std::sqrt(new_energy/old_energy),error=std::sqrt(error_energy/old_energy);
            if(!std::isfinite(rms)||rms<.98||rms>1.02||error>.03){
                std::fprintf(stderr,"FAIL reference sound tolerance model %u family %u: RMS%.6f error%.6f\n",model,family,rms,error);return 1;
            }
            if(rms<minimum_rms)minimum_rms=rms;if(rms>maximum_rms)maximum_rms=rms;if(error>maximum_error)maximum_error=error;
            ++count;
        }
    }
    if (count != 47) return 1;
    std::printf("Sound ratios RMS %.9f..%.9f; worst relative RMS difference %.9f\n",minimum_rms,maximum_rms,maximum_error);
    std::puts("PASS: 47 frozen legacy fingerprints and quantitative new/legacy comparisons, all controls, cache hits, edits and model changes");
}
