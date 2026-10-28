// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "dsp.hpp"
#include "sine.hpp"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hktaffy {
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
  scene.move(G_ribbon, 0, px(400));
  scene.rotate(G_roll0, t * (4 + k[1] / 80), px(62.67), px(72.21));
  scene.rotate(G_roll1, -t * (4 + k[1] / 80), px(257.33), px(72.21));
  scene.axis(G_spring, 358 + k[3] * 870 / 1024 + mul(sn(t * 18), level) / 16);
  scene.show(G_spark, !reduced && level > 280 && k[3] > 256 && state.age < 180);
  scene.rotate(G_crank, k[2] * 6827 / 1024 - 910, px(257.33), px(74.48));
  i32 oldx = px(72.11), oldy = px(58.2);
  port::Dsp wave;
  for (i32 i = 0; i <= 112; ++i) {
    const i32 x = px(77.78) + i * px(158.77) / 112;
    const i32 sample =
        wave.taffy_wave_at(input.slots[0] >> 16, input.slots[1] >> 16,
                           input.slots[3] >> 16, i * 65536 / 112);
    const i32 y = px(80.76) - sample * 15 / 32;
    scene.line(oldx, oldy, x, y, BLUE, px(2));
    oldx = x;
    oldy = y;
  }
  scene.line(oldx, oldy, px(248.26), px(58.2), BLUE, px(2));

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
} // namespace hktaffy
