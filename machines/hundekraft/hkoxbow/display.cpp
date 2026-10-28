// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkoxbow {
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
  const i32 sx = 901 + k[0] * 246 / 1024,
            droop = k[0] < 371 ? -(371 - k[0]) / 4 : 0;
  scene.axis(G_hornl, sx);
  scene.axis(G_hornr, sx);
  scene.rotate(G_hornl, droop, 141149, 36508);
  scene.rotate(G_hornr, -droop, 210761, 39200);
  scene.move(G_birdl, (1024 - k[3]) * 9, (1024 - k[3]) * 4);
  scene.move(G_birdr, -(1024 - k[3]) * 10, (1024 - k[3]) * 4);
  scene.rotate(G_bell, mul(sn(t * 7), k[1]) * 600 / 1024 * level / 1024, 148612,
               88996);
  const i32 env = k[2] > 512 ? k[2] - 512 : 512 - k[2];
  scene.scale(G_breath, 512 + env * 600 / 512 + mul(sn(t * 3), level) / 10,
              224799, 78237);
  const unsigned stars[6] = {G_s0, G_s1, G_s2, G_s3, G_s4, G_s5};
  for (unsigned i = 0; i < 6; ++i)
    scene.show(stars[i], input.held_notes <= 128 && i < input.held_notes);

  scene.finish();
  static const char *const titles[8] = {
      "Cutoff", "Reso", "Envelope", "Detune", "Wave", "Filter", "Drive", ""};
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  {
    static const char *const labels[] = {"saw",  "pulse", "both",
                                         "sync", "xmod",  "fifth"};
    copy(values[4], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[4])) * 6 / 1024]);
  }
  {
    static const char *const labels[] = {"lp12",  "hp12", "bp12",
                                         "notch", "lp24", "sem"};
    copy(values[5], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[5])) * 6 / 1024]);
  }
  header_amount(values[6], header_capacity, input.slots[6]);
  static const opxy::u8 pageMap[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 3}};
  page_header(state.header, input, frame, titles, values, 44, pageMap);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkoxbow
