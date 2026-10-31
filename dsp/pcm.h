#pragma once
#include <opxy/types.hpp>
namespace x0x { namespace pcm {
struct Sample { const opxy::u8* data; opxy::u32 frames, rate; };
const opxy::u32 sample_count = 19;
extern const Sample samples[sample_count];
extern const opxy::i16 decode[256];
}}
