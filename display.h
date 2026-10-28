#pragma once
#include <opxy/page_header.hpp>

namespace vader {
// Matches resources.display in definition.json: paths, points, nodes, texts, palette.
using Frame = opxy::draw::Frame<2, 64, 46, 16, 16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial;
    opxy::i32 impact, edit;
    opxy::i32 previous[8];
};
}
