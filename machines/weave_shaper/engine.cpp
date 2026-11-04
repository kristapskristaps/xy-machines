#include "core.h"
#include "vendor/macro.cpp"

namespace weave_shaper {
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}


}



void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    weave_shaper_dsp::macro_init(&voice.oscillator);
    voice.oscillator.rng = 0x9e3779b9u * owner + 1u;
    voice.pending = 1;
}
void start(Voice& voice, Shared&, opxy::u32) { voice.pending = 1; }
void destroy(Voice& voice, Shared&, opxy::u32) { voice.pending = 0; }

void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
            opxy::Parameters parameters, opxy::Pitch pitch) {
    const bool trigger = voice.pending != 0;
    voice.pending = 0;
    opxy::u8 controls[7] = {};
    controls[0] = 0;
    for (unsigned i = 1; i < 4; ++i)
        controls[i] = static_cast<opxy::u8>(normalized(parameters[i-1]) >> 8);
    controls[4] = static_cast<opxy::u8>(normalized(parameters[3]) >> 8);
    if (trigger) {
        weave_shaper_dsp::macro_trig(&voice.oscillator);
    }
    for(unsigned offset=0;offset<opxy::block_frames;offset+=32) {
        weave_shaper_dsp::macro_render_device32(&voice.oscillator,controls,pitch[offset],voice.cache);
        for(unsigned i=0;i<32;++i)output[offset+i]=static_cast<opxy::i32>(voice.cache[i])*16384;
    }
}
}
