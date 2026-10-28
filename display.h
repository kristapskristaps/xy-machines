#pragma once
#include <opxy/draw.hpp>

namespace westfold {
using Frame = opxy::draw::Frame<4, 64, 61, 0, 16>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 track, serial, initialized, phase, age, gate, geometry;
};
}
