#pragma once
#include <opxy/effect.hpp>
namespace hkplate {
const opxy::u32 buffer_bytes=176544;
struct State { opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick; };
}
