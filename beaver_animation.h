#pragma once
#include "beaver_pose.h"
#include "beaver_hits.h"
#include "models.h"
namespace x0x {
inline opxy::i32 animation_energy(opxy::i32 time, opxy::i32 settle) {
    using namespace opxy::draw;
    const opxy::i32 left = clamp(unit-time*unit/settle,0,unit);
    return left*left/unit*left/unit;
}
inline opxy::i32 animation_spring(opxy::i32 time, opxy::i32 settle, opxy::i32 rate) {
    using namespace opxy::draw;
    return animation_energy(time,settle)*sine(900+time*rate)/unit;
}
inline opxy::u32 playing_hands(opxy::u32 model, opxy::u32 serial) {
    const opxy::u32 hands = beaver_hits[model].hands;
    return hands == 3 && model != clap ? (serial%2 ? 1 : 2) : hands;
}
inline opxy::i32 kick_skin_scale(opxy::u32 model, opxy::i32 time, opxy::i32 settle) {
    return opxy::draw::unit+(model == kick ? animation_spring(time,settle,28)/28 : 0);
}
inline void beaver_animation(Frame& frame, opxy::u32 model, opxy::u32 family,
                              opxy::i32 age, bool reduced, opxy::u32 serial) {
    using namespace opxy::draw;
    static_assert(beaver_node+beaver_layers == Frame::node_count,"Beaver artwork budget");
    static const opxy::i32 settles[4] = {144,168,205,184};
    const opxy::i32 settle = settles[family];
    const opxy::i32 time = reduced ? 2048 : family == 1 ? age/24*24 : age;
    const opxy::i32 energy = animation_energy(time,settle);
    const opxy::i32 arm_spring = animation_spring(time,settle,16);
    const opxy::i32 wrist_spring = animation_spring(time,settle-24,25);
    const opxy::i32 ring = energy*sine(time*(family == 3 ? 55 : 42))/unit;
    const opxy::i32 nod = energy*sine(time*17)/unit;
    const BeaverHit& hit = beaver_hits[model];
    const opxy::u32 hands = playing_hands(model,serial);
    opxy::i32 arms[2] = {}, wrists[2] = {};
    for (opxy::u32 hand = 0; hand < 2; ++hand) {
        if (hands & (1u<<hand)) {
            arms[hand] = hit.arm[hand]*arm_spring/unit;
            wrists[hand] = hit.stick[hand]*wrist_spring/unit;
        } else {
            arms[hand] = (hand ? -1 : 1)*nod/32;
            wrists[hand] = arms[hand];
        }
        if (family == 1) { arms[hand] = arms[hand]/20*20; wrists[hand] = wrists[hand]/20*20; }
    }
    for (opxy::u32 i = 0; i < beaver_layers; ++i) {
        const BeaverPose& pose = beaver_poses[i];
        opxy::i32 x = pose.x*unit, y = pose.y*unit, angle = 0, scale = unit;
        switch (pose.part) {
        case beaver_tail: angle = -nod/12; break;
        case beaver_body: y += nod; break;
        case beaver_head: y += 2*nod; angle = (serial%2 ? -1 : 1)*nod/16; break;
        case beaver_left_arm: angle = arms[0]; break;
        case beaver_right_arm: angle = arms[1]; break;
        case beaver_left_stick: case beaver_right_stick: {
            // Forward kinematics keeps each independently rebounding stick
            // attached to its paw throughout the wrist and shoulder motion.
            const opxy::u32 side = pose.part == beaver_right_stick ? 1 : 0;
            const Turn arm = rotation(arms[side]);
            const opxy::i32 hx = side ? 20 : -15, hy = side ? -20 : -26;
            x = (side ? 279 : 202)*unit+hx*arm.cos-hy*arm.sin;
            y = (side ? 124 : 120)*unit+hx*arm.sin+hy*arm.cos;
            angle = wrists[side];
            break;
        }
        case beaver_cymbal: if (model == cymbal || model == ride) angle = ring/5; break;
        case beaver_hat_upper:
            if (model == closed_hat || model == open_hat) { y += (model == open_hat ? -5 : 3)*arm_spring; angle = ring/32; }
            break;
        case beaver_snare: if (model == snare || model == clap || model == rim) { y += ring; angle = ring/48; } break;
        case beaver_skin_left:
            if (model == low_tom || model == mid_tom || (model >= low_conga && model <= high_conga)) scale += ring/32;
            break;
        case beaver_skin_right: if (model == high_tom || model == cowbell) scale += ring/32; break;
        case beaver_bass_skin: scale = kick_skin_scale(model,time,settle); break;
        case beaver_left_foot: if (model == kick) { y += 2*energy; angle = -energy/20; } break;
        case beaver_right_foot: if (model == closed_hat || model == open_hat) y += 2*energy; break;
        case beaver_pedal: if (model == kick) { y -= 2*energy; angle = energy/12; } break;
        default: break;
        }
        place(frame,beaver_node+i,x,y,scale,0,keep);
        turn(frame,beaver_node+i,rotation(angle));
    }
}
}
