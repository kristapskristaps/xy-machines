#pragma once
#include <opxy/types.hpp>
#include <opxy/q31.hpp>
namespace vader {
inline opxy::u32 shape_index(opxy::i32 value) {
    return ((static_cast<opxy::u32>(value) >> 16) * 4) >> 15;
}
inline opxy::u32 control_table(const opxy::u32* table, opxy::i32 value) {
    const opxy::u32 position = static_cast<opxy::u32>(value) >> 10;
    const opxy::u32 at = position >> 15, mix = position & 32767u;
    const opxy::u32 a = table[at], b = table[at + 1];
    const opxy::i32 fraction = static_cast<opxy::i32>(mix * 65536u);
    return b >= a ? a + static_cast<opxy::u32>(opxy::q31::multiply(static_cast<opxy::i32>(b - a), fraction))
                  : a - static_cast<opxy::u32>(opxy::q31::multiply(static_cast<opxy::i32>(a - b), fraction));
}
}
