#pragma once
#include <opxy/effect.hpp>
namespace hkpalme {
const opxy::u32 buffer_bytes=360368;
struct State { opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick; };
}
