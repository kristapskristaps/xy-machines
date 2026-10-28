#pragma once
#include <opxy/audio.hpp>
#include "sine.hpp"
#include "dsp.hpp"
namespace hktaffy {
struct alignas(8) Voice { opxy::u32 pending, half; opxy::u32 pitch[128]; opxy::i16 audio[128]; opxy::u32 storage[26]; };
struct Shared {};
}
