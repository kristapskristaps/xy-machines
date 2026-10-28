#pragma once
#include <opxy/draw.hpp>
#include <opxy/page_header.hpp>
namespace hkclouds {
using Frame = opxy::draw::Frame<0, 0, 79, 0, 16>;
struct DisplayState {
  opxy::draw::PageHeader header;
  opxy::u32 initialized, track, serial, released, seed;
  opxy::i32 time, age, level, held, memory[32];
};
} // namespace hkclouds
