#pragma once
#include <opxy/draw.hpp>
#include "art.h"
namespace weave {
using Frame = opxy::draw::Frame<2, 32, art::node_count, 2, 9>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 initialized, track, serial, model, phase, travel;
    opxy::i32 controls[4], from[4], target[4];
    opxy::i32 control_age, swap_age, pulse_age, quiet_age, energy, sample_age;
    opxy::i32 draw_controls[4], draw_phase, draw_energy;
};
void display_reset(DisplayState&);
void display(DisplayState&, const opxy::draw::Input&, Frame&);
}
