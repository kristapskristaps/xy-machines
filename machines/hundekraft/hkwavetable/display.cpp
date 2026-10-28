// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "dsp.hpp"
#include "sine.hpp"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkwavetable {
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
  scene.move(G_active, 0, px(400));
  scene.move(G_dots, (t * 3) % px(4), 0);
  scene.rotate(G_gear0, t * (2 + k[1] / 128) + k[1] * 2, px(91.59), px(113.46));
  scene.rotate(G_gear1, -2 * (t * (2 + k[1] / 128) + k[1] * 2), px(139.07),
               px(116.26));
  scene.move(G_press, 0,
             (1024 - k[2]) * 10 + mul(sn(t * 7) + 1024, level) * 3 / 4);
  scene.axis(G_coil, 1024 - k[3] * 461 / 1024);
  port::Dsp wave;
  for (i32 i = 0; i < 24; ++i) {
    const opxy::u32 a = static_cast<opxy::u32>(i) * 178956970u,
                    b = static_cast<opxy::u32>(i + 1) * 178956970u;
    const i32 ya =
        wave.wavetable_shape(input.slots[0] >> 16, a) * px(3) / 30000;
    const i32 yb =
        wave.wavetable_shape(input.slots[0] >> 16, b) * px(3) / 30000;
    scene.line(px(33.16) + i * px(25.23) / 24, px(33.35) - ya,
               px(33.16) + (i + 1) * px(25.23) / 24, px(33.35) - yb, BLUE,
               px(1.8));
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
} // namespace hkwavetable
