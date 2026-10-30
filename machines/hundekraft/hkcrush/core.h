#pragma once
#include <opxy/effect.hpp>
namespace hkcrush {
const opxy::u32 buffer_bytes=136;
struct State { opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick, lift, gain_check; };
}
