// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkarc {
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
  // Display model of the original x/v portrait; the SDK exposes no DSP trace.
  for (i32 lamp = 0; lamp < (k[5] > 0 ? 2 : 1); ++lamp) {
    i32 ox = 0, oy = 0;
    for (i32 i = 0; i <= 48; ++i) {
      const i32 a = i * 8192 / 48 + t * (lamp ? 3 : 2), s = sn(a), c = cs(a);
      const i32 nonlinear =
          s * (1024 + k[0]) / (1024 + (s < 0 ? -s : s) * k[0] / 1024);
      const i32 x = px(AG_CX) + nonlinear * (lamp ? 56 : 90),
                y = px(AG_CY) - c * (lamp ? 26 : 39);
      if (i)
        scene.line(ox, oy, x, y, lamp ? GREY : BLUE);
      ox = x;
      oy = y;
    }
  }
  scene.line(px(AG_L_X0), px(AG_CY - 5), px(AG_L_TIP), px(AG_CY - 5), GREY);
  scene.line(px(AG_L_X0), px(AG_CY + 5), px(AG_L_TIP), px(AG_CY + 5), GREY);
  scene.line(px(AG_R_TIP), px(AG_CY - 5), px(AG_R_X0), px(AG_CY - 5), GREY);
  scene.line(px(AG_R_TIP), px(AG_CY + 5), px(AG_R_X0), px(AG_CY + 5), GREY);
  if (level > 8) {
    i32 ox = px(AG_L_TIP + 1), oy = px(AG_CY);
    for (i32 i = 1; i <= 6; ++i) {
      const i32 x = px(AG_L_TIP + 1) + i * px(AG_R_TIP - AG_L_TIP - 2) / 6;
      const i32 y = px(AG_CY) - mul(sn(i * 4096 / 6), level) * 9 +
                    mul(sn(t * 143 + i * 1919), k[2]) * 3;
      scene.line(ox, oy, x, y, WHITE, px(2));
      ox = x;
      oy = y;
    }
    if (!reduced && state.age < 250)
      for (i32 i = 0; i < 6 * k[3] / 1024; ++i) {
        const i32 a = 4608 + i * 491;
        scene.line(px(160) + cs(a) * 14, px(61) + sn(a) * 14,
                   px(160) + cs(a) * (18 + level / 32),
                   px(61) + sn(a) * (18 + level / 32), ORANGE);
      }
  }
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], amount(input.parameters[i]) * 6144 / 1024 - 3072,
                 px(ag_dial_x[i]), px(AG_DIAL_Y));

  scene.finish();
  static const char *const titles[8] = {
      "Current", "Carbon", "Hiss", "Strike", "Ratio", "Coupling", "Bright", ""};
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  header_amount(values[4], header_capacity, input.slots[4]);
  header_amount(values[5], header_capacity, input.slots[5]);
  header_amount(values[6], header_capacity, input.slots[6]);
  static const opxy::u8 pageMap[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 3}};
  page_header(state.header, input, frame, titles, values, 160, pageMap);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkarc
