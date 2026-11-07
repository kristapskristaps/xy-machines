#pragma once
#include <opxy/page_header.hpp>

namespace acidline {
using Frame = opxy::draw::Frame<2, 34, 13, 16, 5>;

struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::i32 pulse;
    opxy::u32 note_serial, track, initialized;
};
}
