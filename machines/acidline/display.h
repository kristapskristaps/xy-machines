#pragma once
#include <opxy/page_header.hpp>

namespace acidline {
using Frame = opxy::draw::Frame<0, 0, 20, 16, 1>;

struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::i32 pulse;
    opxy::u32 note_serial, track, initialized;
};
}
