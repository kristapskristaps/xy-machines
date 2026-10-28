// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkpalme {
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
                 px(pg_dial[i][0]), px(pg_dial[i][1]));
  if (k[4] > 20) {
    scene.circle(px(160), px(73), px(22), GREY);
    scene.circle(px(160), px(73), px(30), GREY);
    scene.circle(px(160), px(73), px(7), WHITE);
  }
  for (i32 i = 0; i < PG_NS; ++i) {
    i32 ox = pg_bot[i][0], oy = pg_bot[i][1];
    for (i32 j = 1; j <= 7; ++j) {
      const i32 window = sn(j * 4096 / 7),
                displacement =
                    mul(mul(sn(t * (3 + i) + j * 8192 / 7), window), level) * 4;
      const i32 x = pg_bot[i][0] + (pg_top[i][0] - pg_bot[i][0]) * j / 7 +
                    mul(pg_nrm[i][0], displacement);
      const i32 y = pg_bot[i][1] + (pg_top[i][1] - pg_bot[i][1]) * j / 7 +
                    mul(pg_nrm[i][1], displacement);
      scene.line(ox, oy, x, y, WHITE);
      ox = x;
      oy = y;
    }
  }
  for (i32 side = 0; side < 2; ++side) {
    const i32 i0 = side ? 11 : 0, hx = pg_dmp[i0][0], hy = pg_dmp[i0][1],
              angle = k[7] ? (side ? 700 : -700) : 0;
    i32 ox = hx, oy = hy;
    for (i32 j = 1; j < 6; ++j) {
      const i32 i = side ? 11 - j : j, dx = pg_dmp[i][0] - hx,
                dy = pg_dmp[i][1] - hy;
      const i32 x = hx + mul(dx, cs(angle)) - mul(dy, sn(angle)),
                y = hy + mul(dx, sn(angle)) + mul(dy, cs(angle));
      scene.line(ox, oy, x, y, k[7] ? RED : GREY, px(2));
      ox = x;
      oy = y;
    }
  }
  scene.circle(px(160), px(147), px(2) + level * 4, WHITE);

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
} // namespace hkpalme
