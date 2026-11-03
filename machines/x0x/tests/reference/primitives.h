// Frozen original three-partial waveform helpers; sine and division are unchanged.
#pragma once
namespace x0x { namespace dsp {
static inline i32 legacy_square(u32 phase){return divide_signal<23>(square_partials(phase));}
static inline i32 legacy_square_sixth(u32 phase){return divide_signal<138>(square_partials(phase));}
}}
