// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkmbira {
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
  const unsigned primary[3] = {G_l0, G_l1, G_l2},
                 secondary[3] = {G_p0, G_p1, G_p2};
  for (unsigned i = 0; i < 3; ++i) {
    scene.show(primary[i], input.shown[i] < 4);
    scene.show(secondary[i], input.shown[i] >= 4);
  }
  const i32 row0[4] = {25768, 59846, 93923, 128000},
            row1[4] = {36761, 70838, 104915, 138993};
  for (i32 i = 0; i < 4; ++i) {
    const i32 slot = input.shown[i] < 8 ? input.shown[i] : i,
              n = (k[slot] * 12 + 512) / 1024;
    for (i32 j = 0; j < n; ++j)
      scene.line(248503 + j * 6991, row0[i], 248503 + j * 6991, row1[i],
                 i == 0 ? BLUE : WHITE, px(1.4));
  }
  if (level > 8) {
    for (i32 i = 0; i < 2; ++i)
      scene.arc(152867, 95022, px(4 + i * 3) + level * 3 + sn(t * 90), 6656,
                9216, WHITE, 6);
    scene.arc(141874, 106015, px(6) + (t % 340) * 60, 3072, 6144, GREY, 8);
    for (i32 c = 0; c < 2; ++c)
      for (i32 j = 0; j < k[3] * 6 / 1024; ++j) {
        const i32 a = j * 1365 + t * 49;
        scene.line((c ? 142974 : 71521) + cs(a) * 11,
                   (c ? 31265 : 48853) + sn(a) * 11,
                   (c ? 142974 : 71521) + cs(a) * 14,
                   (c ? 31265 : 48853) + sn(a) * 14, RED);
      }
  }

  scene.finish();
  static const char *const titles[8] = {"Strike", "Tine",   "Box", "Buzz",
                                        "Ring",   "Double", "Wah", ""};
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  header_amount(values[4], header_capacity, input.slots[4]);
  header_amount(values[5], header_capacity, input.slots[5]);
  header_amount(values[6], header_capacity, input.slots[6]);
  static const opxy::u8 pageMap[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 3}};
  page_header(state.header, input, frame, titles, values, 128, pageMap);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkmbira
