// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hktape {
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
  const unsigned spokes[9] = {G_s0, G_s1, G_s2, G_s3, G_s4,
                              G_s5, G_s6, G_s7, G_s8};
  const i32 speed = 2 + (1024 - k[2]) / 120, rot = t * speed;
  for (unsigned i = 0; i < 9; ++i)
    scene.rotate(spokes[i], rot * tp_roll[i][2], tp_roll[i][0], tp_roll[i][1]);
  scene.rotate(G_pinch, -rot, px(TP_CAPSTAN_X), px(TP_TOP_Y - 6));
  const unsigned bars[4] = {G_bar0, G_bar1, G_bar2, G_bar3};
  for (unsigned i = 0; i < 4; ++i)
    scene.rotate(bars[i], k[i] * 6144 / 1024 - 3072, px(tp_dial[i][0]),
                 px(tp_dial[i][1]));
  const i32 head[3] = {TP_ERASE_X, TP_REC_X, TP_PLAY_X};
  for (i32 i = 0; i < 3; ++i)
    scene.box(px(head[i] - 6), px(TP_HEAD_TOP), px(head[i] + 6), px(TP_TOP_Y),
              i == 0   ? BLUE
              : i == 1 ? RED
                       : WHITE,
              px(1.2));
  scene.circle(px(TP_LED_X), px(TP_LED_Y), px(3), level > 8 ? ORANGE : GREY,
               true);
  // Moving white registration marks follow the original tape's polygonal route.
  for (i32 d = (t * speed / 16) % 44; d < TP_LOOP_LEN; d += 44) {
    i32 part = 0, left = d;
    while (part < TP_N - 1 && left >= tp_len[part])
      left -= tp_len[part++];
    const i32 next = (part + 1) % TP_N;
    const i32 x = px(tp_pt[part][0]) + (tp_pt[next][0] - tp_pt[part][0]) *
                                           left * 1024 / tp_len[part];
    const i32 y = px(tp_pt[part][1]) + (tp_pt[next][1] - tp_pt[part][1]) *
                                           left * 1024 / tp_len[part];
    scene.line(x, y, x + px(2), y, GREY, px(2));
  }
  scene.line(px(TP_LED_X), px(25), px(TP_LED_X), px(TP_LANE_Y), GREY);
  scene.line(px(TP_PLAY_X), px(TP_LANE_Y), px(TP_SUM_X), px(TP_LANE_Y), WHITE);

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
} // namespace hktape
