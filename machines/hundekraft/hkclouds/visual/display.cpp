// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkclouds {
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
  const i32 level = state.level;

  i32 k[8] = {};
  for (unsigned i = 0; i < 8; ++i)
    k[i] = amount(input.slots[i]);
  Scene<Frame> scene(frame);
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], amount(input.parameters[i]) * 6144 / 1024 - 3072,
                 px(cs_dial_x[i]), px(CS_DIAL_Y));
  const i32 x = px(20) + k[0] * 280, y = px(58);
  scene.line(px(20), y, px(300), y, k[7] ? BLUE : GREY);
  scene.line(px(20), y - px(7), px(20), y + px(7), k[7] ? GREY : RED, px(1.8));
  scene.line(x, y - px(16), x, y - px(10), BLUE, px(1.4));
  scene.line(x - px(6), y - px(16), x, y - px(16), BLUE, px(1.4));
  if (moving && !k[7])
    state.memory[0] = (state.memory[0] + dt * (1 + k[2] / 90)) % 65536;
  for (i32 i = 0; i < 4 + k[2] * 12 / 1024 && level > 4; ++i) {
    const i32 p = (state.memory[0] + i * 4096) % 65536;
    const i32 xx = px(20) + p * 280 / 64,
              h = px(2) +
                  mul(sn(p / 8) < 0 ? -sn(p / 8) : sn(p / 8), level) * 14;
    scene.line(xx, y - h, xx, y + h, WHITE, px(1.4));
  }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  header_amount(values[4], header_capacity, input.slots[4]);
  header_amount(values[5], header_capacity, input.slots[5]);
  header_amount(values[6], header_capacity, input.slots[6]);
  {
    static const char *const labels[] = {"off", "on"};
    copy(values[7], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[7])) * 2 / 1024]);
  }
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkclouds
