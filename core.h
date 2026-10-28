#pragma once
#include <opxy/audio.hpp>

namespace vader {
struct Voice {
    opxy::u32 phase, pending;
    opxy::i32 sweep, fade, body, drive, sub, color;
    opxy::u32 noise;
    opxy::i32 click, click_low;
};
// State shared by every voice; empty when voices share nothing.
struct Shared {};
}
