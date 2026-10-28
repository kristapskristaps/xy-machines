// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkeva {
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
  const unsigned masks[4] = {G_m0, G_m1, G_m2, G_m3};
  for (unsigned i = 0; i < 4; ++i) {
    const i32 n = (k[i] * 16 + 512) / 1024;
    scene.move(masks[i], n >= 16 ? px(200) : n * 5796, 0);
  }
  scene.move(G_cross, k[1] * 20152 / 1024, 0);
  const i32 breath =
      mul(k[2], 128 + (sn(t * 2) > 0 ? sn(t * 2) * 896 / 1024 : 0));
  scene.axis(G_fog, clamp(breath, 20, 1024));
  scene.show(G_strip,
             k[3] > 0 && !(moving && k[3] > 128 && (t / 64) % 13 == 0));

  scene.finish();
  static const char *const titles[8] = {"Pressure", "Vowel", "Oxygen", "Com",
                                        "Octave",   "",      "",       ""};
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  {
    static const char *const labels[] = {"low", "mid", "high"};
    copy(values[4], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[4])) * 3 / 1024]);
  }
  static const opxy::u8 pageMap[2][4] = {{0, 1, 2, 3}, {0, 1, 2, 4}};
  page_header(state.header, input, frame, titles, values, 48, pageMap);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkeva
