#pragma once
#include <opxy/draw.hpp>
namespace matter {
using Frame=opxy::draw::Frame<0,0,31,1,8>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 phase,serial,track,initialized;
    opxy::i32 energy;
};
void display_reset(DisplayState&);
void display(DisplayState&,const opxy::draw::Input&,Frame&);
}
