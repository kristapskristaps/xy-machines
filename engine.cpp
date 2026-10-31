#include "core.h"
#include "models.h"
#include "dsp/voice.h"
namespace x0x {
inline void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    voice = Voice(); voice.seed = owner * 2654435761u + 1u;
    if (!voice.seed) voice.seed = 1;
}
inline void start(Voice& voice, Shared&, opxy::u32) {
    const opxy::u32 model = voice.model, seed = voice.seed;
    voice = Voice(); voice.model = model; voice.seed = seed;
    voice.pending = voice.active = 1;
}
inline void destroy(Voice& voice, Shared&, opxy::u32) { voice.active = 0; }
inline void drum_defaults(opxy::u32 key, opxy::u8* model, opxy::u8* controls) { default_pad(key, model, controls); }
inline void drum_select(Voice& voice, Shared&, opxy::u32, opxy::u32 model) { voice.model = model < model_count ? model : kick; }
inline const char* model_name(opxy::u32 model) { return short_name(model); }
inline const char* parameter_name(opxy::u32 model, opxy::u32 slot) { return control_name(model, slot); }
inline opxy::u32 parameter_count(opxy::u32 model) { return control_count(model); }
inline opxy::u32 edit_parameter(opxy::u32 model, opxy::u32 slot, opxy::u32 value, opxy::i32 delta) {
    return stepped_value(model, slot, value, delta);
}
inline bool finished(const Voice& voice, const Shared&) { return voice.age && !voice.active; }
inline void render(Voice& voice, Shared&, opxy::u32, opxy::Output output, opxy::Parameters parameters, opxy::Pitch) {
    dsp::render(voice, output, parameters);
}
}
