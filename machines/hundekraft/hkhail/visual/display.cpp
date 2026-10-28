// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkhail {
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
  const i32 level = state.level;

  i32 k[8] = {};
  for (unsigned i = 0; i < 8; ++i)
    k[i] = amount(input.slots[i]);
  Scene<Frame> scene(frame);
  const unsigned primary[3] = {G_primary0, G_primary1, G_primary2},
                 secondary[3] = {G_secondary0, G_secondary1, G_secondary2};
  for (unsigned i = 0; i < 3; ++i) {
    scene.show(primary[i], input.shown[i] < 4);
    scene.show(secondary[i], input.shown[i] >= 4);
  }
  if (moving && !k[7])
    state.memory[0] = (state.memory[0] + dt * (1 + k[3] / 160)) % 65536;
  const i32 phase = reduced ? 0 : state.memory[0];
  const i32 count = 4 + (k[3] > 512 ? k[3] - 512 : 512 - k[3]) * 16 / 512;
  for (i32 i = 0; i < count && level > 4; ++i) {
    const i32 u = (phase + i * 65536 / count) % 65536;
    const i32 at = u * (BD_THREAD_N - 1) / 65536,
              f = u * (BD_THREAD_N - 1) % 65536;
    const i32 x = BD_THREAD[at][0] +
                  (BD_THREAD[at + 1][0] - BD_THREAD[at][0]) * f / 65536;
    const i32 y = BD_THREAD[at][1] +
                  (BD_THREAD[at + 1][1] - BD_THREAD[at][1]) * f / 65536;
    const i32 len = px(4) + k[1] * 10;
    scene.line(x - len / 2, y, x + len / 2, y, WHITE, px(2));
    scene.arc(x, y - px(5), px(3), 4096, 8192, GREY, 4);
  }
  const i32 u = k[0] * (BD_THREAD_N - 1),
            at = clamp(u / 1024, 0, BD_THREAD_N - 2), f = u % 1024;
  scene.circle(lerp(BD_THREAD[at][0], BD_THREAD[at + 1][0], f),
               lerp(BD_THREAD[at][1], BD_THREAD[at + 1][1], f), px(5), BLUE);
  for (i32 i = 0; i < count / 2; ++i)
    scene.circle(BD_PILE[i][0], BD_PILE[i][1], px(2), i == 0 ? ORANGE : GREY,
                 true);
  const i32 row[4] = {BD_RING0_Y, BD_RING1_Y, BD_RING2_Y, BD_RING3_Y};
  for (i32 i = 0; i < 4; ++i) {
    const i32 value = amount(input.parameters[i]);
    scene.arc(BD_RING_X, row[i], BD_RING_R, 3072, 3072 + value * 6144 / 1024,
              i == 0 ? BLUE : WHITE, 8);
    scene.circle(BD_RING_X, row[i], BD_RING_R * 3 / 4, GREY);
    const i32 a = 3072 + value * 6144 / 1024;
    scene.line(BD_RING_X, row[i], BD_RING_X + mul(cs(a), BD_RING_R * 5 / 8),
               row[i] + mul(sn(a), BD_RING_R * 5 / 8), WHITE);
  }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
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
} // namespace hkhail
