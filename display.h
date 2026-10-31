#pragma once
#include <opxy/page_header.hpp>
namespace x0x {
using Frame = opxy::draw::Frame<0, 0, 8, 16, 2>;
struct DisplayState { opxy::draw::PageHeader header; };
}
