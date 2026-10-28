// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hksupersaw {
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
  const i32 running = level;
  const i32 shake = mul(sn(t * 117), mul(k[3], running)) * 3 / 2;
  scene.move(G_saw, shake / 2, shake);
  scene.rotate(G_sprk, -t * 36, px(SS_SPRK_X), px(SS_SPRK_Y));
  for (i32 run = 0; run < 3; ++run) {
    if (run == 0 && k[0] < 8)
      continue;
    if (run == 2 && k[2] < 8)
      continue;
    const i32 period = run == 2 ? 24 : 12,
              top = run == 2 ? SS_BAR_BOT : SS_BAR_TOP, dir = run == 2 ? 1 : -1;
    const i32 speed = run == 0 ? 5 + k[0] / 160 : run == 2 ? -2 : 5;
    const i32 offset = (t * speed / 32) % (period * 1024);
    for (i32 n = -1; n < 16; ++n) {
      i32 x = px(SS_CHAIN_X0 + n * period) - offset, xx = x + px(period);
      const i32 lo = px(SS_CHAIN_X0), hi = px(SS_CHAIN_X1);
      if (xx < lo || x > hi)
        continue;
      const i32 rise = px(period) * (1024 - k[1]) / 1024,
                fall = px(period) - px(period / 2) * k[1] / 1024;
      const i32 h = run == 2 ? k[2] * 5 : px(5), y = px(top) + shake;
      if (x >= lo && x + rise <= hi)
        scene.line(x, y, x + rise, y + dir * h, run == 0 ? GREY : WHITE);
      if (x + rise >= lo && x + fall <= hi)
        scene.line(x + rise, y + dir * h, x + fall, y + dir * h, WHITE);
      if (x + fall >= lo && x + fall <= hi)
        scene.line(x + fall, y + dir * h, x + fall, y, run == 0 ? GREY : WHITE);
    }
  }
  if (k[2] > 16 && level > 4)
    for (i32 i = 0; i < 3; ++i) {
      const i32 p = (t * 4 + i * 1024) % 3072;
      scene.circle(px(SS_EXH_X) - p * 22 / 3, px(SS_EXH_Y) - p * 34 / 3,
                   px(1.5) + p * (1 + k[2] / 210) / 3, GREY);
    }
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], k[i] * 6144 / 1024 - 3072, px(ss_dial_x[i]),
                 px(SS_DIAL_Y));

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
} // namespace hksupersaw
