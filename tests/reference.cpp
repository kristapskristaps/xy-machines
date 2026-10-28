#include "../vendor/macro.cpp"
#include "reference_vectors.h"
#include <cstdio>
int main() {
    weave_dsp::macro_voice voice;
    opxy::u8 p[7]={}; opxy::i16 output[16];
    for (const auto& reference : reference_vectors) {
        const unsigned variant=reference.variant;
        weave_dsp::macro_init(&voice); voice.rng=0x9e3779b9u+1u;
        p[0]=reference.model*8;
        p[1]=variant==1?0:variant==2?127:64;
        p[2]=variant==1?32:variant==2?127:64;
        p[3]=variant==1?16:variant==2?127:64;
        p[4]=variant==3?127:0;
        weave_dsp::macro_trig(&voice);
        opxy::u32 hash=2166136261u;
        for (unsigned block=0;block<2048;++block) {
            if (block==1024) weave_dsp::macro_trig(&voice);
            weave_dsp::macro_render(&voice,p,42852281u,output,16);
            for(auto s:output) {hash^=static_cast<opxy::u16>(s);hash*=16777619u;}
        }
        // The all-maximum 6-op fixture reaches the original exponential overflow.
        // Keep its original hash in reference_vectors.h as provenance; the
        // corrected render is also covered by the independent polynomial sweep.
        const unsigned expected=reference.model==14 && variant==2 ? 3686065058u : reference.hash;
        if (hash!=expected) {
            std::printf("FAIL model %u variant %u: %u != %u\n",reference.model,variant,hash,expected);
            return 1;
        }
    }
    std::puts("PASS: 59 bit-exact upstream renders and one corrected FM exponential fixture");
}
