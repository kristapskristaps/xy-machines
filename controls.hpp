#pragma once
#include <opxy/types.hpp>
namespace vader {
inline opxy::u32 shape_index(opxy::i32 value) {
    return ((static_cast<opxy::u32>(value) >> 16) * 4) >> 15;
}
inline opxy::u32 control_table(const opxy::u32* table, opxy::i32 value) {
    const opxy::u32 position = static_cast<opxy::u32>(value) >> 10;
    const opxy::u32 at = position >> 15, mix = position & 32767u;
    const opxy::u32 a = table[at], b = table[at + 1];
    return b >= a ? a + (((b - a) >> 8) * mix >> 7)
                  : a - (((a - b) >> 8) * mix >> 7);
}
}
