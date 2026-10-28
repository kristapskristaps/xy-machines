// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkstutter {
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
  const i32 divs[6] = {1, 2, 4, 8, 16, 32}, div = divs[k[0] * 6 / 1024],
            slice = t * div / 512;
  scene.rotate(G_reel, (slice % div) * 8192 / div, SX_REEL_X, SX_REEL_Y);
  const i32 xs[4] = {SX_RING0_X, SX_RING2_X, SX_RING1_X, SX_RING3_X},
            ys[4] = {SX_RING0_Y, SX_RING2_Y, SX_RING1_Y, SX_RING3_Y};
  for (i32 i = 0; i < 4; ++i) {
    const i32 value = amount(input.parameters[i]),
              a = 3072 + value * 6144 / 1024;
    scene.arc(xs[i], ys[i], SX_RING_R, 3072, a, i == 0 ? BLUE : WHITE, 8);
    scene.circle(xs[i], ys[i], SX_RING_R * 3 / 4, GREY);
    scene.line(xs[i], ys[i], xs[i] + mul(cs(a), SX_RING_R * 5 / 8),
               ys[i] + mul(sn(a), SX_RING_R * 5 / 8), WHITE);
  }
  const i32 mode = k[1] * 4 / 1024, cw = (SX_PIC_X1 - SX_PIC_X0) / 16,
            top = SX_PIC_Y0 + px(3), bot = SX_PIC_Y1 - px(2);
  for (i32 i = 0; i < 16; ++i) {
    const i32 x = SX_PIC_X0 + cw * i + cw / 2;
    const bool repeat = ((i + slice) * 7919) % 1024 < k[3];
    if (!repeat)
      scene.line(x, bot - px(3), x, bot, GREY);
    else if (mode == 3)
      for (i32 y = top; y < bot; y += px(7))
        scene.line(x, y, x, y + px(3), WHITE);
    else
      scene.line(x + (mode == 2 ? px(.8) : 0),
                 top + (mode == 1 ? (i & 3) * px(5) : 0),
                 x - (mode == 2 ? px(.8) : 0), bot, i == 15 ? WHITE : GREY);
  }
  if (k[5])
    scene.circle(SX_RING3_X, SX_RING3_Y, SX_RING_R / 2, BLUE);

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  {
    static const char *const labels[] = {"2", "3", "4", "6", "8", "16"};
    copy(values[4], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[4])) * 6 / 1024]);
  }
  {
    static const char *const labels[] = {"off", "on"};
    copy(values[5], header_capacity,
         labels[static_cast<unsigned>(amount(input.slots[5])) * 2 / 1024]);
  }
  header_amount(values[6], header_capacity, input.slots[6]);
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkstutter
