#include "core.h"
#include "vendor/macro.cpp"
#include "resample.h"

namespace weave {
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}
unsigned model(opxy::i32 word) { return normalized(word) * 15u >> 15; }
}

void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    weave_dsp::macro_init(&voice.oscillator);
    voice.oscillator.rng = 0x9e3779b9u * owner + 1u;
    voice.pending = 1;
    voice.model = 0;
    voice.cache_read=32;voice.history_write=voice.resample_phase=0;
    for(auto& sample:voice.history)sample=0;
}
void start(Voice& voice, Shared&, opxy::u32) { voice.pending = 1; }
void destroy(Voice& voice, Shared&, opxy::u32) { voice.pending = 0; }

void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
            opxy::Parameters parameters, opxy::Pitch pitch) {
    const unsigned selected = model(parameters[0]);
    const bool trigger = voice.pending || selected != voice.model;
    voice.model = selected;
    voice.pending = 0;
    opxy::u8 controls[7] = {};
    controls[0] = static_cast<opxy::u8>(selected * 8u);
    for (unsigned i = 1; i < 4; ++i)
        controls[i] = static_cast<opxy::u8>(normalized(parameters[i]) >> 8);
    controls[4] = static_cast<opxy::u8>(normalized(parameters[4]) >> 8);
    controls[5] = static_cast<opxy::u8>(normalized(parameters[5]) * 4u >> 15);
    controls[6] = static_cast<opxy::u8>(normalized(parameters[6]) >> 14);
    if (trigger) {
        // Zero keeps the owner-specific evolving stream used by older sounds.
        // Fixed seeds restart the random sequence, without clearing resonator tails.
        const unsigned seed = normalized(parameters[7]) >> 8;
        if (seed) voice.oscillator.rng = 0x9e3779b9u * seed + 1u;
        weave_dsp::macro_trig(&voice.oscillator);
        voice.cache_read=32;
    }
    // Keep the source DSP at its original 48 kHz so all time constants and
    // key-scaling tables retain their meaning. A rational polyphase filter
    // supplies the SDK's 44.1 kHz stream without software floating point.
    for(unsigned i=0;i<opxy::block_frames;++i) {
        voice.resample_phase+=160;
        while(voice.resample_phase>=147) {
            voice.resample_phase-=147;
            if(voice.cache_read==32) {
                const opxy::u32 input=pitch[i];
                const opxy::u32 increment=(input/160u)*147u+((input%160u)*147u)/160u;
                weave_dsp::macro_render(&voice.oscillator,controls,increment,voice.cache,32);
                voice.cache_read=0;
            }
            const opxy::i16 sample=voice.cache[voice.cache_read++];
            voice.history[voice.history_write]=voice.history[voice.history_write+24]=sample;
            if(++voice.history_write==24)voice.history_write=0;
        }
        opxy::i32 accumulated=0;
        const opxy::i16* history=voice.history+voice.history_write+23;
        const opxy::i16* coefficients=resample_coefficients[voice.resample_phase];
        for(unsigned tap=0;tap<24;++tap) {
            accumulated+=static_cast<opxy::i32>(history[-static_cast<int>(tap)])*coefficients[tap];
        }
        const opxy::i32 sample=accumulated>>15;
        output[i]=(sample<-32768?-32768:sample>32767?32767:sample)*16384;
    }
}
}
