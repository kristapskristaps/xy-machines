#include "core.h"
#include "vendor/macro.cpp"

namespace atlas {
namespace {
unsigned normalized(opxy::i32 word) { return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 24; }
} // namespace
void initialize(Voice &voice, Shared &, opxy::u32 owner) {
    atlas_dsp::macro_init(&voice.oscillator);
    voice.oscillator.rng = 0x9e3779b9u * owner + 1u;
    voice.pending = 1;
}
void start(Voice &voice, Shared &, opxy::u32) { voice.pending = 1; }
void destroy(Voice &voice, Shared &, opxy::u32) { voice.pending = 0; }
void render(Voice &voice, Shared &, opxy::u32, opxy::Output output, opxy::Parameters p,
            opxy::Pitch pitch) {
    opxy::u8 controls[7] = {};
    for (unsigned i = 0; i < 4; ++i)
        controls[i + 1] = static_cast<opxy::u8>(normalized(p[i]));
    if (voice.pending) {
        atlas_dsp::macro_trig(&voice.oscillator);
        voice.pending = 0;
    }
    // Render at the device rate. Integrated wavetables retain their own
    // differentiator/low-pass reconstruction; no second sample-rate filter.
    opxy::i16 samples[32];
    for (unsigned offset = 0; offset < 64; offset += 32) {
        atlas_dsp::macro_render_e<44100, atlas_dsp::MACRO_WAVETABLE, 32>(
            &voice.oscillator, controls, pitch[offset], samples, 32);
        for (unsigned i = 0; i < 32; ++i)
            output[offset + i] = static_cast<opxy::i32>(samples[i]) * 16384;
    }
}
} // namespace atlas
