#pragma once
#include <opxy/page_header.hpp>

namespace mando {
using Frame = opxy::draw::Frame<2, 64, 55, 16, 16>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial;
    opxy::i32 impulse;
    opxy::i32 pose[8];
};
}
