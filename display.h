#pragma once
#include <opxy/page_header.hpp>
namespace x0x {
using Frame = opxy::draw::Frame<0, 0, 56, 16, 5>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, note, model, family;
    opxy::i32 age;
};
}
