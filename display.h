#pragma once
#include <opxy/page_header.hpp>

namespace vader {
// Matches resources.display in definition.json: paths, points, nodes, texts, palette.
using Frame = opxy::draw::Frame<2, 64, 86, 16, 16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::draw::NoteTracker notes;
    opxy::u32 initialized, track, serial;
    opxy::u32 next;
    opxy::i32 ages[4], edit_age, gate;
    opxy::i32 previous[8];
    opxy::i32 visual[8];
};
}
