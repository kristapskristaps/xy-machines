#include "core.h"
#include "models.h"
namespace x0x {
inline void initialize(Voice& voice, Shared&, opxy::u32) { voice.model = kick; }
inline void start(Voice&, Shared&, opxy::u32) {}
inline void destroy(Voice&, Shared&, opxy::u32) {}
inline void drum_defaults(opxy::u32 key, opxy::u8* model, opxy::u8* controls) { default_pad(key, model, controls); }
inline void drum_select(Voice& voice, Shared&, opxy::u32, opxy::u32 model) { voice.model = model < model_count ? model : kick; }
inline const char* model_name(opxy::u32 model) { return short_name(model); }
inline const char* parameter_name(opxy::u32 model, opxy::u32 slot) { return control_name(model, slot); }
inline opxy::u32 parameter_count(opxy::u32 model) { return control_count(model); }
inline void render(Voice&, Shared&, opxy::u32, opxy::Output output, opxy::Parameters, opxy::Pitch) {
    // The first milestone exercises controls through the real drum adapter.
    for (opxy::u32 i = 0; i < opxy::block_frames; ++i) output[i] = 0;
}
}
