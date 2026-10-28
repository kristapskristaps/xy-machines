// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkgranular {
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
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], amount(input.parameters[i]) * 6144 / 1024 - 3072,
                 px(70 + i * 60), px(133));
  // A labelled display study; no borrowed sample/peak pointer is available to
  // draw::Input.
  for (i32 i = 0; i < 88; ++i) {
    const i32 wave = sn(i * 711) * sn(i * 113) / 1024,
              amp = px(1) + (wave < 0 ? -wave : wave) * 28;
    scene.line(px(20) + i * px(280) / 88, px(60) - amp,
               px(20) + i * px(280) / 88, amp + px(60), GREY);
  }
  const i32 x = px(20) + k[0] * 280;
  scene.line(x, px(20), x, px(100), BLUE);
  scene.line(clamp(x - k[2] * 100, px(20), px(300)), px(104),
             clamp(x + k[2] * 100, px(20), px(300)), px(104), WHITE);
  scene.line(x, px(16), clamp(x + px(4) + k[1] * 100, px(20), px(300)), px(16),
             WHITE);
  if (level > 0)
    for (i32 i = 0; i < 4; ++i) {
      const i32 phase = (t * (3 + i) + i * 2048) % 8192;
      const i32 xx = clamp(x + sn(phase) * k[2] / 10, px(20), px(300)),
                h = px(2) + (sn(phase / 2) > 0 ? sn(phase / 2) : 0) * 28;
      scene.line(xx, px(60) - h, xx, px(60) + h, ORANGE, px(1.5));
    }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  header_amount(values[4], header_capacity, input.slots[4]);
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkgranular
