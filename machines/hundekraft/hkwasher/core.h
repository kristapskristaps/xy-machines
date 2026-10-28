#pragma once
#include <opxy/effect.hpp>
namespace hkwasher {
const opxy::u32 buffer_bytes=8368;
struct State { opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick; };
}
