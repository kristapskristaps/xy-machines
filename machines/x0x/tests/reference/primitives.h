// Frozen original three-partial waveform helpers; reference_sine and division are unchanged.
#pragma once
#include "audio_primitives.h"
namespace x0x { namespace dsp {
static inline i32 legacy_square(u32 phase){return reference_divide_signal<23>(reference_square_partials(phase));}
static inline i32 legacy_square_sixth(u32 phase){return reference_divide_signal<138>(reference_square_partials(phase));}
}}
