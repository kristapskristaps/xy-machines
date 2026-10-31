#pragma once
#include <opxy/page_header.hpp>

namespace cyclops {
using Frame = opxy::draw::Frame<2, 64, 24, 16, 16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial;
    opxy::i32 impulse;
};
}
