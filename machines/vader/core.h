#pragma once
#include <opxy/audio.hpp>

namespace vader {
struct Voice {
    opxy::u32 phase, pending;
    opxy::i32 sweep, fade, body, body2, drive, sub, color, boost;
    opxy::u32 noise;
    opxy::i32 click, click_low;
    opxy::u32 cache_key[4], cache_status;
    opxy::i16 cached_wave[513];
};
// State shared by every voice; empty when voices share nothing.
struct Shared {};
}
