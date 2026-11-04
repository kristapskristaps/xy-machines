#pragma once
#include <opxy/page_header.hpp>
namespace acidline {
using Frame=opxy::draw::Frame<0,0,116,16,16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::draw::NoteTracker notes;
    opxy::u32 phase,bloom,serial,track,initialized;
};
}
