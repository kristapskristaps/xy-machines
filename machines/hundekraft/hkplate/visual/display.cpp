// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkplate {
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
  const i32 max = 30 + k[0] * 60 / 1024;
  for (i32 i = 0; i < 6; ++i) {
    const i32 r = (i * 15 + t / 22) % 90;
    if (r < 2 || r > max)
      continue;
    scene.box(px(clamp(132 - r, 105, 215)),
              px(clamp(68 - r * 13 / 16, 23, 113)),
              px(clamp(132 + r, 105, 215)),
              px(clamp(68 + r * 13 / 16, 23, 113)), level ? BLUE : GREY);
  }
  const i32 left = 214 - (1024 - k[1]) * 64 / 1024;
  if (left < 212) {
    scene.box(px(left), px(28), px(212), px(108), GREY);
    for (i32 x = left - 72; x < 212; x += 8) {
      i32 a = x, b = x + 80, ya = 108, yb = 28;
      if (a < left) {
        ya -= left - a;
        a = left;
      }
      if (b > 212) {
        yb += b - 212;
        b = 212;
      }
      if (a < b)
        scene.line(px(a), px(ya), px(b), px(yb), GREY);
    }
  }
  i32 ox = px(84), oy = px(68);
  for (i32 x = 86; x <= 128; x += 2) {
    const i32 window = clamp(x < 88 ? x - 84 : x > 124 ? 128 - x : 4, 0, 4);
    const i32 y = px(68) + sn((x - 84) * (120 + k[2])) * window;
    scene.line(ox, oy, px(x), y, WHITE);
    ox = px(x);
    oy = y;
  }
  const char *labels[4] = {"decay", "tone", "pre", "dry"};
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
} // namespace hkplate
