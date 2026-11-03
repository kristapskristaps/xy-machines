#pragma once
#include <opxy/audio.hpp>

namespace westfold_reference {
struct Voice {
    opxy::u32 phase, mod_phase, slope_phase, initialized;
    opxy::i32 controls[8];
    opxy::i32 envelope, gate, low1, low2, dc_input, dc_output;
};
struct Shared {};
}
