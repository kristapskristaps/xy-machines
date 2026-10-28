// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkreverb {
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

  i32 k[8] = {};
  for (unsigned i = 0; i < 8; ++i)
    k[i] = amount(input.slots[i]);
  Scene<Frame> scene(frame);
  const unsigned arms[6] = {G_karm1, G_karm2, G_karm3,
                            G_karm4, G_karm5, G_karm6};
  for (i32 i = 0; i < 6; ++i)
    scene.show(arms[i], i < 1 + (k[0] * 5 + 512) / 1024);
  scene.show(G_kfire, k[1] > 102);
  scene.tint(G_kskull, k[2] > 512 ? WHITE : GREY);
  const unsigned mix[10] = {G_kmix0, G_kmix1, G_kmix2, G_kmix3, G_kmix4,
                            G_kmix5, G_kmix6, G_kmix7, G_kmix8, G_kmix9};
  for (i32 i = 0; i < 10; ++i)
    scene.show(mix[i], i >= 10 - (k[3] * 10 + 512) / 1024);
  scene.rotate(G_ktime, k[0] * 4096 / 1024, KALI_TIME_X, KALI_TIME_Y);
  scene.rotate(G_kdiff, k[2] * 4096 / 1024, KALI_DIFF_X, KALI_DIFF_Y);
  scene.rotate(G_ktone, k[1] * 4096 / 1024, KALI_TONE_X, KALI_TONE_Y);

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
} // namespace hkreverb
