#pragma once
#include <opxy/draw.hpp>
namespace weave {
using Frame = opxy::draw::Frame<4, 64, 89, 0, 8>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 phase, serial, track, initialized;
};
void display_reset(DisplayState&);
void display(DisplayState&, const opxy::draw::Input&, Frame&);
}
