// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkfmbell {
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
  scene.rotate(G_clapper, mul(sn(t * 20), level) * 250 / 1024, px(160), px(46));
  const i32 n = 1 + k[1] * 3 / 1024, h = 180 + level * 620 / 1024;
  for (i32 i = 0; i < n; ++i) {
    scene.arc(px(160), px(78), px(56 + i * 9), -h, h, WHITE, 6);
    scene.arc(px(160), px(78), px(56 + i * 9), 4096 - h, 4096 + h, WHITE, 6);
  }
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], k[i] * 6144 / 1024 - 3072, px(i % 2 ? 284 : 36),
                 px(i < 2 ? 32 : 128));

  scene.finish();
  char values[8][header_capacity] = {};
  {
    static const char *const labels[] = {
        "0.5", "1.0", "1.5", "2.0", "2.5", "3.0", "3.5", "4.0",
        "4.5", "5.0", "5.5", "6.0", "6.5", "7.0", "7.5", "8.0"};
    copy(values[0], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[0])) * 16 / 1024]);
  }
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkfmbell
