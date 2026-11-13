#pragma once
#include <opxy/draw.hpp>
namespace crease {
using Frame=opxy::draw::Frame<3,32,24,0,8>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 initialized,track,serial,phase;
    opxy::i32 pulse,knob_age,energy,values[4],from[4],target[4];
};
void display_reset(DisplayState&);
void display(DisplayState&,const opxy::draw::Input&,Frame&);
}
