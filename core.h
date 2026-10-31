#pragma once
#include <opxy/audio.hpp>
namespace x0x {
// The controls prototype deliberately has no drum DSP yet.
struct Voice { opxy::u32 model; };
struct Shared {};
void drum_defaults(opxy::u32 key, opxy::u8* model, opxy::u8* controls);
void drum_select(Voice&, Shared&, opxy::u32 owner, opxy::u32 model);
const char* model_name(opxy::u32 model);
const char* parameter_name(opxy::u32 model, opxy::u32 slot);
opxy::u32 parameter_count(opxy::u32 model);
}
