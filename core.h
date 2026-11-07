#pragma once
#include <opxy/effect.hpp>

namespace lustre {
const opxy::u32 kHistory = 32768;
constexpr opxy::u32 kFrames = opxy::block_frames/2;
constexpr opxy::u32 kMirror = kFrames-1;
constexpr opxy::u32 kRoomLengths[] = {1499,1877,2381,3011};
constexpr opxy::u32 kDiffuserLengths[] = {113,173,241,337};

constexpr opxy::u32 delay_offset(const opxy::u32* lengths, unsigned line) {
    return line ? lengths[line-1]+kMirror+delay_offset(lengths,line-1) : 0;
}

// Stereo history followed by delay lines whose first samples are mirrored
// at the end, so every 32-sample read and write is contiguous.
constexpr opxy::u32 kRoomWords = delay_offset(kRoomLengths,4);
constexpr opxy::u32 kBufferWords = 2*kHistory+kRoomWords+delay_offset(kDiffuserLengths,4);

struct Grain {
    opxy::u32 position, phase, increment, remaining;
    opxy::i32 speed, pan;
};
struct State {
    Grain grains[6];
    opxy::u32 write, written, reverb_written;
    opxy::u32 room_position[4], diffuser_position[4];
    opxy::u32 countdown, next, random;
    opxy::i32 input[2], antialias[2], output[2];
    opxy::i32 damping[4], tone_memory[2], dc[2];
    opxy::i32 controls[8];
    opxy::u32 started;
};
void initialize(State&);
void process(State&, opxy::Buffer, opxy::Channel, opxy::Channel, opxy::Parameters);
}
