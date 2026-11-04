#pragma once
#include <opxy/page_header.hpp>
#include "scene.h"

namespace bitflip {
using Frame = opxy::draw::Frame<0, 0, scene::node_count, 16, 8>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, serial, travel, collected, coins;
    opxy::i32 jump_age, activity, pop_age;
};
}
