// Frozen exact reference at 97b6a9cc011169159625186d57b5e040e31dfee7.
#pragma once
#include <opxy/audio.hpp>

namespace vader_frozen {
struct Voice {
    opxy::u32 phase, pending;
    opxy::i32 sweep, fade, body, body2, drive, sub, color, boost;
    opxy::u32 noise;
    opxy::i32 click, click_low;
};
// State shared by every voice; empty when voices share nothing.
struct Shared {};
}
