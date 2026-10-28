#pragma once
#include <opxy/draw.hpp>
namespace subvista {
using Frame = opxy::draw::Frame<4,64,30,0,16>;
struct DisplayState { opxy::draw::NoteTracker notes; opxy::u32 phase, bloom; };
}
