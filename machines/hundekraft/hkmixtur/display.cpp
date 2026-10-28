// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkmixtur {
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
  for (i32 f = 0; f < 2; ++f) {
    i32 ox = 0, oy = 0;
    const i32 peak = px(f ? 202 : 158) + k[4 + f] * (f ? 105 : 135);
    for (i32 x = 150; x <= 310; x += 4) {
      const i32 delta = (px(x) - peak) / 128, q = 24 + k[6] / 5,
                z = delta * q / 128;
      const i32 y = px(116) - px(f ? 38 : 54) * 256 / (256 + (z * z / 256));
      if (x > 150)
        scene.line(ox, oy, px(x), y, GREY);
      ox = px(x);
      oy = y;
    }
  }
  for (i32 i = 0; i < 4; ++i) {
    const i32 n = k[i] * 25 / 1024;
    if (!n)
      continue;
    const i32 y = px(9) + (n - 1) * 4608, length = px(284) / n;
    const Ink col = i == 0 ? BLUE : i == 3 ? ORANGE : WHITE;
    scene.line(px(24), y, px(24) + length, y, col, px(1.4));
    if (level > 4) {
      const i32 tick = t / 143, pos = tick % n;
      scene.circle(px(24) + length - length * pos / n, y, px(1.8), col, true);
    }
  }

  scene.finish();
  static const char *const titles[8] = {"Divider1", "Divider2", "Divider3",
                                        "Divider4", "Formant1", "Formant2",
                                        "Reso",     ""};
  char values[8][header_capacity] = {};
  {
    static const char *const labels[] = {
        "off", "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",
        "9",   "10", "11", "12", "13", "14", "15", "16", "17",
        "18",  "19", "20", "21", "22", "23", "24"};
    copy(values[0], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[0])) * 25 / 1024]);
  }
  {
    static const char *const labels[] = {
        "off", "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",
        "9",   "10", "11", "12", "13", "14", "15", "16", "17",
        "18",  "19", "20", "21", "22", "23", "24"};
    copy(values[1], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[1])) * 25 / 1024]);
  }
  {
    static const char *const labels[] = {
        "off", "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",
        "9",   "10", "11", "12", "13", "14", "15", "16", "17",
        "18",  "19", "20", "21", "22", "23", "24"};
    copy(values[2], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[2])) * 25 / 1024]);
  }
  {
    static const char *const labels[] = {
        "off", "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",
        "9",   "10", "11", "12", "13", "14", "15", "16", "17",
        "18",  "19", "20", "21", "22", "23", "24"};
    copy(values[3], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[3])) * 25 / 1024]);
  }
  header_amount(values[4], header_capacity, input.slots[4]);
  header_amount(values[5], header_capacity, input.slots[5]);
  header_amount(values[6], header_capacity, input.slots[6]);
  static const opxy::u8 pageMap[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 3}};
  page_header(state.header, input, frame, titles, values, 155, pageMap);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkmixtur
