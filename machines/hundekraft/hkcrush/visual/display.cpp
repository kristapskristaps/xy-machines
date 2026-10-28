// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "../dsp.hpp"
#include "../sine.hpp"
#include "visual_geometry.h"
#include "visual_runtime.h"

namespace hkcrush {
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
  port::Dsp curve;
  const i32 bits = curve.crush_bits(input.slots[0] >> 16),
            drive = k[2] * 24 / 1024;
  const opxy::u32 rate =
      static_cast<opxy::u32>(
          curve.crush_exp2((((input.slots[1] >> 16) - 32767) * 14336) >> 15)) *
          44100u >>
      16;
  scene.line(px(106), px(21 + drive), px(214), px(21 + drive), GREY);
  scene.line(px(106), px(108 - drive), px(214), px(108 - drive), GREY);
  i32 ox = px(110), oy = px(68), qx = ox, qy = oy;
  const i32 spacing = clamp(rate ? 32768 / static_cast<i32>(rate) : 20, 2, 20),
            quant = bits < 10 ? 1024 >> bits : 1;
  for (i32 x = 110; x <= 212; x += 2) {
    const i32 s = sn(t * 10 + x * 375), y = px(68) - s * 30;
    if (x > 110)
      scene.line(ox, oy, px(x), y, GREY);
    ox = px(x);
    oy = y;
    if ((x - 110) % spacing < 2) {
      i32 sample = clamp(s * 30, -px(40 - drive), px(40 - drive));
      sample = sample / quant * quant;
      const i32 yy = px(68) - sample;
      scene.line(qx, qy, px(x), qy, BLUE, px(1.4));
      scene.line(px(x), qy, px(x), yy, BLUE, px(1.4));
      qx = px(x);
      qy = yy;
    }
  }
  const char *labels[4] = {"bits", "rate", "drive", "dry"};
  for (unsigned i = 0; i < 4; ++i) {
    const i32 x = px(i < 2 ? 14 : 246), y = px((i & 1) ? 80 : 4);
    char value[8] = {};
    if (i == 0)
      number(value, bits);
    else if (i == 1)
      khz(value, rate);
    else if (i == 2)
      number(value, drive);
    else
      header_amount(value, sizeof(value), input.slots[i]);
    scene.label(i * 2, x, y, labels[i]);
    scene.label(i * 2 + 1, x, y + px(20), value, 1536);
  }

  scene.finish();
  char values[8][header_capacity] = {};
  header_amount(values[0], header_capacity, input.slots[0]);
  header_amount(values[1], header_capacity, input.slots[1]);
  header_amount(values[2], header_capacity, input.slots[2]);
  header_amount(values[3], header_capacity, input.slots[3]);
  number(values[0], bits);
  khz(values[1], rate);
  number(values[2], drive);
  for (unsigned i = 0; i < 4; ++i)
    copy(frame.header[i], header_capacity,
         values[input.shown[i] < 8 ? input.shown[i] : i]);
  state.memory[31] = static_cast<opxy::i32>(scene.overflow);
  state.held = held;
}
} // namespace hkcrush
