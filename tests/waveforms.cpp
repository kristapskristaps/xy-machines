#include "../engine.cpp"
#include "waveforms.h"
#include <cstdio>
#include <cstdlib>

int main() {
    using namespace x0x;
    unsigned count = 0;
    for (unsigned model = 0; model < model_count; ++model) {
        for (unsigned family = 0; family < 4; ++family) {
            if (!(family_masks[model] & (1u << family))) continue;
            unsigned long long hash = 14695981039346656037ull;
            Voice voice; Shared shared; opxy::i32 output[64], parameters[8];
            opxy::u32 pitch[64] = {};
            initialize(voice, shared, 1);
            for (unsigned slot = 0; slot < 8; ++slot) {
                for (unsigned value = 0; value < 128; ++value) {
                    const unsigned raw[8] = {family_raw(model,family),64,100,64,0,64,64,0};
                    for (unsigned i = 0; i < 8; ++i) parameters[i] = opxy::i32(raw[i] << 24);
                    parameters[slot] = opxy::i32(value << 24);
                    start(voice, shared, 1); drum_select(voice, shared, 1, model);
                    for (unsigned block = 0; block < 8; ++block) {
                        // Exercise cache hits, every control's invalidation,
                        // model changes during a note, and live style changes.
                        if (block == 3) parameters[(slot+1)%8] = opxy::i32(((value+31)%128) << 24);
                        if (block == 5) drum_select(voice, shared, 1, (model+1)%model_count);
                        render(voice, shared, 1, output, parameters, pitch);
                        for (auto sample : output) hash = (hash ^ opxy::u32(sample)) * 1099511628211ull;
                    }
                }
            }
            if (hash != waveform_reference[count]) {
                std::fprintf(stderr,"FAIL: reference waveform changed: model %u family %u\n",model,family);
                return 1;
            }
            ++count;
        }
    }
    if (count != 47) return 1;
    std::puts("PASS: 47 reference fingerprints, all control values, cache hits, live edits and model changes");
}
