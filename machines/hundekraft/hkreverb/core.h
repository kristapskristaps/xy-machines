#pragma once
#include <opxy/effect.hpp>
namespace hkreverb {
const opxy::u32 buffer_bytes=353616;
struct State { opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick; };
}
