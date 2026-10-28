#pragma once
#include <opxy/audio.hpp>
#include "sine.hpp"
#include "dsp.hpp"
namespace hksid {
struct alignas(8) Voice { opxy::u32 pending, half; opxy::u32 pitch[128]; opxy::i16 audio[128]; opxy::u32 storage[52]; };
struct Shared {};
}
