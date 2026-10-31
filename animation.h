#pragma once
#include "display.h"
#include "models.h"

namespace x0x {
// Artwork: one body and one moving part for each drum; four family frames and
// eight impact marks. Shared by the native renderer and browser audition.
const opxy::u32 body_node = 8, moving_node = body_node + model_count;
const opxy::u32 family_node = moving_node + model_count, impact_node = family_node + 4;
inline void instrument_animation(Frame& frame, opxy::u32 model, opxy::u32 family,
                                 opxy::i32 age, opxy::i32 duration, bool reduced) {
    using namespace opxy::draw;
    for (opxy::u32 i = body_node; i < Frame::node_count; ++i) hide(frame,i);
    const bool active = age < duration;
    static const opxy::i32 settle[4] = {144,168,205,184};
    opxy::i32 time = age;
    if (family == 1) time = time/24*24;
    const opxy::i32 left = clamp(unit-time*unit/settle[family],0,unit);
    opxy::i32 motion = left*left/unit;
    motion = reduced ? 0 : motion*left/unit;
    const opxy::i32 wobble = sine(clamp(age,0,2048)*(family == 3 ? 65 : 45))*motion/unit;
    const opxy::u32 light = active ? (age < 80 ? 1 : 4) : 3;
    const opxy::i32 x = 350*unit, y = 124*unit;
    opxy::i32 body_scale = unit, part_scale = unit, dx = 0, dy = 0, angle = 0, body_angle = 0;
    switch (model) {
    case kick:
        body_scale += motion/14; part_scale += motion/5; dy = 3*motion;
        break;
    case snare:
        dx = 3*wobble; dy = 2*motion; angle = wobble/16;
        break;
    case clap:
        part_scale += motion/3; dy = -8*motion; angle = -motion/8;
        break;
    case rim:
        dy = -10*motion; angle = motion/3;
        break;
    case low_tom: case mid_tom: case high_tom:
        dy = (5-opxy::i32(model-low_tom))*motion;
        part_scale += motion/(7+model-low_tom); body_scale += motion/24;
        break;
    case closed_hat:
        dy = 4*motion; angle = wobble/24;
        break;
    case open_hat:
        dy = -12*motion; angle = wobble/14;
        break;
    case cymbal:
        angle = wobble/5; body_angle = -wobble/12; part_scale += motion/16;
        break;
    case ride:
        dy = -7*motion; angle = wobble/9; body_angle = -wobble/24;
        break;
    case cowbell:
        angle = motion/4; body_angle = -wobble/18; dy = 3*motion;
        break;
    case claves:
        angle = -motion/3; body_angle = motion/5; dx = 6*motion;
        break;
    case maracas:
        dx = 8*wobble; angle = wobble/3; body_angle = -wobble/8;
        break;
    case low_conga: case mid_conga: case high_conga:
        dy = (5-opxy::i32(model-low_conga))*motion; part_scale += motion/12;
        break;
    case tambourine:
        angle = wobble/4; part_scale += motion/6; dx = 3*wobble;
        break;
    default: break;
    }
    // Family character affects the instrument as well as its impact marks.
    if (family == 0) angle += motion/24;
    if (family == 1) { dx = dx/(2*unit)*(2*unit); dy = dy/(2*unit)*(2*unit); }
    if (family == 2) body_scale += motion/24;
    if (family == 3) dy -= 3*motion;
    place(frame,body_node+model,x,y,body_scale,light,stroke);
    turn(frame,body_node+model,rotation(body_angle));
    place(frame,moving_node+model,x+dx,y+dy,part_scale,light,stroke);
    turn(frame,moving_node+model,rotation(angle));
    place(frame,family_node+family,x,y,unit,active ? 4 : 2,stroke);
    if (!motion) return;
    const opxy::i32 spread = 48*unit + 15*(unit-motion);
    for (opxy::u32 i = 0; i < 8; ++i) {
        const opxy::i32 degrees = opxy::i32(i)*450 + opxy::i32(model%3)*100;
        opxy::i32 px, py, turn_angle;
        if (family == 1) {
            // The sampled family breaks into a small, stepped rectangular grid.
            px = (i%2 ? 1 : -1)*(48*unit+10*(unit-motion));
            py = (opxy::i32(i/2)*22-33)*unit;
            turn_angle = 900;
        } else {
            px = sine(degrees+900)*spread/unit;
            py = sine(degrees)*spread/unit;
            turn_angle = degrees+900;
        }
        if (family == 0 && i%2) continue;
        if (family == 2 && i%2 == 0) continue;
        place(frame,impact_node+i,x+px,y+py,unit+motion/3,age < 70 ? 1 : 3,stroke);
        turn(frame,impact_node+i,rotation(turn_angle));
    }
}
}
