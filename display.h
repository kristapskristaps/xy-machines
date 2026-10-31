#pragma once
#include <opxy/page_header.hpp>

namespace cyclops {
using Frame = opxy::draw::Frame<1, 64, 16, 16, 16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial;
    opxy::i32 blink;
};
}
