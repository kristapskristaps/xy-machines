// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkfather {
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
  const i32 position = k[1] * 12;
  scene.move(G_gate_person, position, 0);
  const i32 scale = 1626 - k[0] * 1234 / 1024;
  scene.scale(G_face, scale, px(156), px(94));
  scene.move(G_face, 0, -px(12));
  const i32 gate = level * 14;
  if (k[1]) {
    scene.line(px(104) + position - gate, px(108), px(104) + position - gate,
               px(137), WHITE, px(2));
    scene.line(px(104) + position + gate, px(108), px(104) + position + gate,
               px(137), WHITE, px(2));
  }
  char value[8] = {};
  header_amount(value, sizeof(value), input.slots[0]);
  scene.label(0, px(14), px(11), "distance");
  scene.label(1, px(14), px(32), value, 1536);
  scene.label(2, px(14), px(126), "gate");
  scene.label(3, px(260), px(126), "color");
  scene.label(4, px(284), px(11), "dry");
  scene.line(px(302), px(36), px(302), px(82), GREY, px(5));
  scene.line(px(302), px(82) - k[3] * 46, px(302), px(82), RED, px(5));

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
} // namespace hkfather
