#pragma once
#include <opxy/types.hpp>
namespace x0x { namespace dsp {
struct MetalFilterState { opxy::i32 x1, x2, y1, y2; };
struct MetalState {
    MetalFilterState filters[5];
    opxy::i32 fast, medium, air_low;
    opxy::u32 initialized;
};
}}
