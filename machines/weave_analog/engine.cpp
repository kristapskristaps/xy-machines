#include "core.h"
#include "vendor/macro.cpp"
#include "resample.h"

namespace weave_analog {
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}

inline void advance_source(Voice& voice,const opxy::u8* controls,const opxy::u32* pitch,opxy::u32& read,opxy::u32& write) {
    if(read==32) {
        const opxy::u32 input=*pitch;
        const opxy::u32 increment=(input/160u)*147u+((input%160u)*147u)/160u;
        weave_analog_dsp::macro_render(&voice.oscillator,controls,increment,voice.cache,32);
        read=0;
    }
    const opxy::i16 sample=voice.cache[read++];
    voice.history[write]=voice.history[write+24]=sample;
    if(++write==24)write=0;
}
}



void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    weave_analog_dsp::macro_init(&voice.oscillator);
    voice.oscillator.rng = 0x9e3779b9u * owner + 1u;
    voice.pending = 1;
    voice.cache_read=32;voice.history_write=voice.resample_phase=0;
    for(auto& sample:voice.history)sample=0;
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
        weave_analog_dsp::macro_trig(&voice.oscillator);
        voice.cache_read=32;
    }
    // Keep the source DSP at its original 48 kHz so all time constants and
    // key-scaling tables retain their meaning. A rational polyphase filter
    // supplies the SDK's 44.1 kHz stream without software floating point.
    // Source DSP only observes oscillator/cache, so retain SRC indices in
    // callback locals and publish their identical endpoints once per block.
    opxy::u32 read=voice.cache_read,write=voice.history_write,phase=voice.resample_phase;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        // A phase in 0..146 consumes one source sample, or occasionally two.
        // Fold the first subtraction into the increment and test only the second.
        phase+=13;
        advance_source(voice,controls,pitch+i,read,write);
        if(phase>=147) {
            phase-=147;
            advance_source(voice,controls,pitch+i,read,write);
        }
        const opxy::i16* history=voice.history+write+23;
        const opxy::i32 sample=resample_sum(history,phase)>>15;
        output[i]=(sample<-32768?-32768:sample>32767?32767:sample)*16384;
    }
    voice.cache_read=read;voice.history_write=write;voice.resample_phase=phase;
}
}
