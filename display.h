#pragma once
#include <opxy/page_header.hpp>
namespace x0x {
using Frame = opxy::draw::Frame<0, 0, 94, 16, 10>;
struct DisplayState {
    opxy::draw::PageHeader header;
    opxy::u32 initialized, track, note, model, family;
    opxy::i32 age;
    opxy::u32 hit_model, hit_family;
    opxy::i32 hit_duration;
};
}
