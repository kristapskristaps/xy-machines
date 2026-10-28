// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hksnowglobe {
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
  const unsigned snow[3] = {G_snow0, G_snow1, G_snow2};
  if (moving && !k[4])
    state.memory[0] = (state.memory[0] + dt * (2 + level / 160)) % 8192;
  const i32 spin = reduced ? 0 : state.memory[0];
  for (i32 i = 0; i < 3; ++i) {
    scene.show(snow[i], i < 1 + (k[0] * 2 + 512) / 1024);
    scene.rotate(snow[i], (i & 1 ? -1 : 1) * spin * (3 - i) / 2, px(160),
                 px(68.91));
  }
  scene.scale(G_shimmer, 563 + k[1] * 614 / 1024 + mul(sn(t * 42), level) / 20,
              px(186.27), px(105.03));
  scene.scale(G_halo, 870 + k[2] * 358 / 1024, px(160), px(68.91));
  scene.rotate(G_key, -k[3] * 1365 / 1024 + (k[4] ? t * 2 : 0), px(204),
               px(121.19));
  scene.move(G_smoke, 0, level > 12 ? -(t * 7) % px(3) : 0);

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  {
    static const char *const labels[] = {"off", "on"};
    copy(values[4], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[4])) * 2 / 1024]);
  }
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hksnowglobe
