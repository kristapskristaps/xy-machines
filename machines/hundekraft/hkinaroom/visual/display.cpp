// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkinaroom {
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
  const i32 w = px(35) + k[0] * 65, h = w * (450 + k[1] / 2) / 1024;
  const i32 x0 = px(234) - w / 2, x1 = x0 + w, y0 = px(40) - h / 2, y1 = y0 + h;
  scene.box(x0, y0, x1, y1, BLUE);
  for (i32 i = 1; i < 5; ++i) {
    scene.line(x0 + w * i / 5, y0, x0 + w * i / 5, y1,
               level > 0 ? WHITE : GREY);
    scene.line(x0, y0 + h * i / 5, x1, y0 + h * i / 5, GREY);
  }
  scene.line(px(297), y0, px(297), y1, WHITE);
  for (i32 i = 0; i < 80; ++i) {
    const i32 x = px(16) + i * px(288) / 80;
    const i32 speech = sn(i * 957) * sn(i * 291) / 1024;
    const i32 room = sn(i * (420 + k[1])) / 2;
    const i32 wave = lerp(speech, room, k[2]),
              amp = px(1) + mul(wave < 0 ? -wave : wave, level) * 10;
    scene.line(x, px(90) - amp, x, px(90) + amp, GREY);
  }
  scene.line(px(16) + (t % 4096) * 72, px(75), px(16) + (t % 4096) * 72,
             px(105), RED);
  scene.label(0, px(16), px(15), k[7] ? "REC" : "GEN --", 1536);
  const char *shapes[4] = {"CUBE", "GOLDEN", "BOLT", "LUCIER"};
  scene.label(1, px(16), px(43), shapes[k[1] * 4 / 1024]);
  scene.label(2, px(16), px(59), "ROOM MODES");
  const char *names[4] = {"size", "shape", "erosion", "dry"};
  for (i32 i = 0; i < 4; ++i) {
    scene.label(3 + i, px(16 + i * 75), px(119), names[i]);
    scene.line(px(16 + i * 75), px(133), px(16 + i * 75) + k[i] * 63, px(133),
               i == 0 ? BLUE : WHITE, px(3));
  }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  {
    static const char *const labels[] = {"cube", "golde", "bolt", "lucie"};
    copy(values[1], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[1])) * 4 / 1024]);
  }
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  header_amount(values[4], header_capacity, input.slots[4]);
  header_amount(values[5], header_capacity, input.slots[5]);
  header_amount(values[6], header_capacity, input.slots[6]);
  {
    static const char *const labels[] = {"off", "on"};
    copy(values[7], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[7])) * 2 / 1024]);
  }
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkinaroom
