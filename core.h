#pragma once
#include <opxy/audio.hpp>
namespace x0x {
struct Voice {
    opxy::u32 model, age, pending, active, seed, phase[6], cursor, fraction;
    opxy::i32 amplitude, transient, pitch_envelope, noise_low, metal_low,
              output_low, dc_input, dc_output;
};
struct Shared {};
void drum_defaults(opxy::u32 key, opxy::u8* model, opxy::u8* controls);
void drum_select(Voice&, Shared&, opxy::u32 owner, opxy::u32 model);
const char* model_name(opxy::u32 model);
const char* parameter_name(opxy::u32 model, opxy::u32 slot);
opxy::u32 parameter_count(opxy::u32 model);
opxy::u32 edit_parameter(opxy::u32 model, opxy::u32 slot, opxy::u32 value, opxy::i32 delta);
bool finished(const Voice&, const Shared&);
}
