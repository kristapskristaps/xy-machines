// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkwarble {
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

  i32 k[8] = {};
  for (unsigned i = 0; i < 8; ++i)
    k[i] = amount(input.slots[i]);
  Scene<Frame> scene(frame);
  const i32 depth = mul(k[1], k[1]),
            rot = t * 14 + mul(sn(t * (1 + k[0] / 90)), depth) * 4;
  for (i32 c = 0; c < 2; ++c) {
    const i32 x = px(c ? 188 : 132), y = px(49);
    scene.circle(x, y, px(c ? 12 : 17), GREY);
    scene.circle(x, y, px(7), BLUE);
    for (i32 n = 0; n < 6; ++n) {
      const i32 a = rot + n * 8192 / 6;
      scene.line(x + cs(a) * 3, y + sn(a) * 3, x + cs(a) * 6, y + sn(a) * 6,
                 WHITE);
    }
  }
  i32 ox = px(118), oy = px(124);
  for (i32 x = 120; x <= 202; x += 2) {
    const i32 window = clamp(x < 126 ? x - 118 : x > 194 ? 202 - x : 8, 0, 8);
    const i32 y = px(124) + mul(sn(t * (1 + k[0] / 90) +
                                   (x - 118) * 8192 / (64 - k[0] * 52 / 1024)),
                                depth) *
                                window;
    scene.line(ox, oy, px(x), y, WHITE);
    ox = px(x);
    oy = y;
  }
  const char *labels[4] = {"rate", "depth", "tone", "dry"};
  for (unsigned i = 0; i < 4; ++i) {
    const i32 x = px(i < 2 ? 14 : 246), y = px((i & 1) ? 80 : 4);
    char value[8] = {};
    header_amount(value, sizeof(value), input.slots[i]);
    scene.label(i * 2, x, y, labels[i]);
    scene.label(i * 2 + 1, x, y + px(20), value, 1536);
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
} // namespace hkwarble
