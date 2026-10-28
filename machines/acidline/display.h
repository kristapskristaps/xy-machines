#pragma once
#include <opxy/draw.hpp>
namespace acidline {
using Frame=opxy::draw::Frame<1,64,18,0,16>;
struct DisplayState {opxy::draw::NoteTracker notes;opxy::u32 phase,bloom;};
}
