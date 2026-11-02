#pragma once
#include <opxy/audio.hpp>
#include "sine.hpp"
#include "dsp.hpp"
namespace hkarc {
struct alignas(8) Voice {
    opxy::u32 pending, half;
    opxy::u32 pitch[128];
    opxy::i16 audio[128];
    opxy::u32 storage[(sizeof(port::Dsp) + sizeof(opxy::u32) - 1) / sizeof(opxy::u32)];
};
struct Shared {};
}
