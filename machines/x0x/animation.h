#pragma once
#include "display.h"
#include "models.h"
#include "drummer_animation.h"

namespace x0x {
const opxy::u32 body_node = 8, moving_node = body_node+model_count;
const opxy::u32 family_node = moving_node+model_count, impact_node = family_node+4;
inline void instrument_animation(Frame& frame, opxy::u32 model, opxy::u32 family,
                                 opxy::i32 age, opxy::i32 duration, bool reduced, opxy::u32 serial) {
    using namespace opxy::draw;
    for (opxy::u32 i = body_node; i < Frame::node_count; ++i) hide(frame,i);
    static const opxy::i32 settles[4] = {144,168,205,184};
    const opxy::i32 time = reduced ? 2048 : family == 1 ? age/24*24 : age;
    const opxy::i32 energy = animation_energy(time,settles[family]);
    const bool active = age < duration;
    const opxy::u32 light = active ? 9 : 1;
    // Printed like a band's emblem: it follows the kick membrane, rather than
    // floating above the kit as a separate, oversized instrument graphic.
    const opxy::i32 logo_scale = 440*kick_skin_scale(model,time,settles[family])/unit;
    place(frame,body_node+model,240*unit,172*unit,logo_scale,light,stroke);
    place(frame,moving_node+model,240*unit,172*unit,logo_scale,light,stroke);
    drummer_animation(frame,model,family,age,reduced,serial);
    if (!energy) return;
    const DrummerHit& target = drummer_hits[model];
    const opxy::i32 spread = 15*unit+8*(unit-energy);
    for (opxy::u32 i = 0; i < 8; ++i) {
        const opxy::i32 degrees = opxy::i32(i)*450+opxy::i32(model%3)*100;
        opxy::i32 px, py, turn_angle;
        if (family == 1) {
            px = (i%2 ? 1 : -1)*(18*unit+4*(unit-energy));
            py = (opxy::i32(i/2)*8-12)*unit; turn_angle = 900;
        } else {
            px = sine(degrees+900)*spread/unit;
            py = sine(degrees)*spread/unit; turn_angle = degrees+900;
        }
        if (family == 0 && i%2) continue;
        if (family == 2 && i%2 == 0) continue;
        place(frame,impact_node+i,target.x*unit+px,target.y*unit+py,600+energy/5,i%2 ? 5 : 6,stroke);
        turn(frame,impact_node+i,rotation(turn_angle));
    }
}
}
