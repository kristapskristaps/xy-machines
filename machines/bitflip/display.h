#pragma once
#include <opxy/page_header.hpp>

namespace bitflip {
using Frame = opxy::draw::Frame<1, 64, 28, 16, 8>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial;
    opxy::i32 age;
};
}
