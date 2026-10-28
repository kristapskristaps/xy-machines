// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkwasher {
inline void display_reset(DisplayState &s) {
  s.header.reset();
  s.initialized = s.track = s.serial = s.released = 0;
  s.seed = 4711;
  s.time = 0;
  s.age = 60000;
  s.level = s.held = 0;
  for (unsigned i = 0; i < 32; ++i)
    s.memory[i] = 0;
}
inline void display(DisplayState &state, const opxy::draw::Input &input,
                    Frame &frame) {
  using namespace art;
  const bool fresh = !state.initialized || state.track != input.track;
  const i32 dt = clamp(input.elapsed, 0, 102);
  const bool reduced = input.reduced_motion != 0;
  const bool held = input.held_notes > 0 && input.held_notes <= 128;
  const bool strike = !fresh && state.serial != input.note_on_serial;
  if (fresh) {
    display_reset(state);
    state.track = input.track;
  }
  state.initialized = 1;
  state.serial = input.note_on_serial;
  state.released = input.note_off_serial;
  if (strike)
    state.age = 0;
  state.age = clamp(state.age + dt, 0, 60000);
  const i32 target = held ? 1024
                     : !reduced && state.age < 1200
                         ? 1024 - state.age * 1024 / 1200
                         : 0;
  state.level = reduced ? target
                        : state.level + (target - state.level) *
                                            clamp(dt * 12, 0, 1024) / 1024;
  if (state.level < 2)
    state.level = 0;
  const bool moving = !reduced && (held || state.level > 0);
  if (moving)
    state.time = (state.time + dt) % 61440;
  const i32 t = reduced ? 0 : state.time;
  const i32 level = state.level;

  i32 k[8] = {};
  for (unsigned i = 0; i < 8; ++i)
    k[i] = amount(input.slots[i]);
  Scene<Frame> scene(frame);
  scene.move(G_water, 0, px(400));
  const i32 phase = t * (1 + k[0] / 100), swing = 182 + k[1] * 501 / 1024;
  scene.rotate(G_socks, mul(sn(phase), swing), px(100), px(76));
  const unsigned bubbles[9] = {G_bub0, G_bub1, G_bub2, G_bub3, G_bub4,
                               G_bub5, G_bub6, G_bub7, G_bub8};
  for (i32 i = 0; i < 9; ++i) {
    scene.show(bubbles[i], i < 2 + k[2] * 7 / 1024);
    scene.move(bubbles[i], 0, -mul(sn(t * 13 + i * 1500) + 1024, level));
  }
  scene.move(G_sock, -px(45) + k[3] * 60,
             -mul(sn(t * 35) < 0 ? -sn(t * 35) : sn(t * 35), level) * 2);
  for (i32 i = 0; i < 2; ++i)
    scene.arc(px(100), px(76), px(69), phase + i * 4096,
              phase + i * 4096 + 569 + k[0] * 1707 / 1024, BLUE, 12, px(2));
  i32 ox = 0, oy = 0;
  bool previous = false;
  for (i32 i = 0; i <= 48; ++i) {
    const i32 x = px(49) + i * px(102) / 48,
              y = px(106) -
                  sn(i * 8192 * 23 / 480 + phase) * (2 + k[1] * 7 / 1024);
    const i32 dx = (x - px(100)) / 64, dy = (y - px(76)) / 64;
    const bool inside = dx * dx + dy * dy < 784 * 784;
    if (i && inside && previous)
      scene.line(ox, oy, x, y, WHITE, px(2));
    ox = x;
    oy = y;
    previous = inside;
  }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkwasher
