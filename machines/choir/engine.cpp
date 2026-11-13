#include "core.h"
#include "vendor/macro.cpp"

namespace choir {
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}


}

__attribute__((noinline)) static void render_chord_native(Voice& voice,const opxy::u8* controls,
                                                       opxy::Pitch pitch,opxy::Output output) {
    if(pitch[0]==pitch[32] && choir_dsp::macro_render_chord_try64(&voice.oscillator,controls,pitch[0],output))return;
    else {
        choir_dsp::macro_render_chord_q31(&voice.oscillator,controls,pitch[0],output,32);
        choir_dsp::macro_render_chord_q31(&voice.oscillator,controls,pitch[32],output+32,32);
    }
}

void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    choir_dsp::macro_init(&voice.oscillator);
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
        choir_dsp::macro_trig(&voice.oscillator);
    }
    render_chord_native(voice,controls,pitch,output);
}
}
