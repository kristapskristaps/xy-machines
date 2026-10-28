// Retained drawing adapter for the Hundekraft artwork. Display-thread code
// only.
#pragma once
#include <opxy/draw.hpp>
#include <opxy/page_header.hpp>

namespace hkstutter {
namespace art {
using namespace opxy;
using namespace opxy::draw;
const i32 S = 1126, X = 64 * unit, Y = 42 * unit;
template <class T> inline i32 px(T n) { return static_cast<i32>(n) * 1024; }
constexpr i32 px(double n) { return static_cast<i32>(n * 1024); }
inline i32 sn(i32 a) { return sine((a % 8192) * 3600 / 8192); }
inline i32 cs(i32 a) { return sn(a + 2048); }
inline i32 mul(i32 a, i32 b) { return floor_q10(a * b); }
inline i32 amount(i32 q) { return q < 0 ? 0 : q >> 21; }
inline i32 lerp(i32 a, i32 b, i32 f) { return a + mul(b - a, f); }
inline void number(char *out, u32 n) {
  char reverse[10];
  u32 count = 0;
  do {
    reverse[count++] = static_cast<char>('0' + n % 10);
    n /= 10;
  } while (n && count < 7);
  u32 i = 0;
  while (count)
    out[i++] = reverse[--count];
  out[i] = 0;
}
inline void khz(char *out, u32 hz) {
  const u32 tenths = (hz + 50) / 100;
  number(out, tenths / 10);
  u32 i = 0;
  while (out[i])
    ++i;
  out[i++] = '.';
  out[i++] = static_cast<char>('0' + tenths % 10);
  out[i++] = 'k';
  out[i] = 0;
}
inline u32 root(u32 n) {
  u32 out = 0, bit = 1u << 30;
  while (bit > n)
    bit >>= 2;
  while (bit) {
    if (n >= out + bit) {
      n -= out + bit;
      out = (out >> 1) + bit;
    } else
      out >>= 1;
    bit >>= 2;
  }
  return out;
}
struct Matrix {
  i32 a, b, c, d, x, y;
};
inline Matrix identity() {
  const Matrix m = {unit, 0, 0, unit, 0, 0};
  return m;
}
inline Matrix compose(const Matrix &p, const Matrix &q) {
  const Matrix m = {
      mul(p.a, q.a) + mul(p.c, q.b),       mul(p.b, q.a) + mul(p.d, q.b),
      mul(p.a, q.c) + mul(p.c, q.d),       mul(p.b, q.c) + mul(p.d, q.d),
      mul(p.a, q.x) + mul(p.c, q.y) + p.x, mul(p.b, q.x) + mul(p.d, q.y) + p.y};
  return m;
}
enum Ink { WHITE, GREY, BLUE, RED, ORANGE };
// Width donors compensate for the scale of retained line/circle geometry.
inline u32 ink(Ink color, i32 width, i32 scale) {
  const i32 wanted = width * unit / (scale > 0 ? scale : 1);
  if (color == WHITE)
    return wanted < 1536    ? 0
           : wanted < 3072  ? 1
           : wanted < 6144  ? 2
           : wanted < 12288 ? 3
                            : 4;
  if (color == GREY)
    return wanted < 2560 ? 5 : 6;
  const u32 first = color == BLUE ? 7 : color == RED ? 10 : 13;
  return first + (wanted < 2560 ? 0 : wanted < 10240 ? 1 : 2);
}
struct Group {
  Matrix matrix;
  i32 visible, variant, tint;
};

template <class F> struct Scene {
  F &frame;
  Group groups[1];
  u32 lines, circles, overflow;
  Scene(F &f) : frame(f), lines(0), circles(0), overflow(0) {
    for (u32 i = 0; i < 1; ++i) {
      groups[i].matrix = identity();
      groups[i].visible = 1;
      groups[i].variant = 0;
      groups[i].tint = -1;
    }
    for (u32 i = 0; i < F::node_count; ++i)
      hide(frame, i);
    for (u32 i = 0; i < F::text_count; ++i)
      text(frame, i, 640 * unit, 320 * unit, unit, "");
    for (u32 i = 0; i < 4; ++i)
      frame.header[i][0] = 0;
  }
  void show(u32 g, bool yes) {
    if (g < 1)
      groups[g].visible = yes;
  }
  void tint(u32 g, Ink color) {
    if (g < 1)
      groups[g].tint = ink(color, unit, unit);
  }
  void move(u32 g, i32 x, i32 y) {
    if (g < 1) {
      groups[g].matrix.x += x;
      groups[g].matrix.y += y;
    }
  }
  void rotate(u32 g, i32 angle, i32 x, i32 y) {
    if (g >= 1)
      return;
    const i32 c = cs(angle), s = sn(angle);
    const Matrix m = {
        c, s, -s, c, x - mul(c, x) + mul(s, y), y - mul(s, x) - mul(c, y)};
    groups[g].matrix = compose(m, groups[g].matrix);
  }
  void scale(u32 g, i32 size, i32 x, i32 y) {
    if (g >= 1)
      return;
    size = clamp(size, 64, 1800);
    const Matrix m = {size, 0, 0, size, x - mul(size, x), y - mul(size, y)};
    groups[g].matrix = compose(m, groups[g].matrix);
  }
  void axis(u32 g, i32 value) {
    if (g >= 1)
      return;
    const Variant &v = kVariants[g];
    if (v.count > 1)
      groups[g].variant =
          clamp(((value - v.low) * (v.count - 1) + (v.high - v.low) / 2) /
                    (v.high - v.low),
                0, v.count - 1);
  }
  void finish() {
    for (u32 n = 0; n < 11; ++n) {
      const Piece &p = kPieces[n];
      Matrix m = identity();
      bool visible = true;
      i32 tint = -1;
      for (u32 j = 0; j < 4 && p.parents[j] >= 0; ++j) {
        const Group &g = groups[p.parents[j]];
        visible = visible && g.visible;
        if (g.tint >= 0)
          tint = g.tint;
        m = compose(m, g.matrix);
      }
      if (p.variantGroup >= 0 && groups[p.variantGroup].variant != p.variant)
        visible = false;
      if (!visible)
        continue;
      const i32 sc =
          static_cast<i32>(root(static_cast<u32>(m.a * m.a + m.b * m.b)));
      if (sc < 1)
        continue;
      const i32 x = X + mul(S, m.x + mul(m.a, p.cx) + mul(m.c, p.cy));
      const i32 y = Y + mul(S, m.y + mul(m.b, p.cx) + mul(m.d, p.cy));
      if (x < node_left * unit || x > node_right * unit ||
          y < node_top * unit || y > node_bottom * unit)
        continue;
      place(frame, n, x, y, clamp(mul(S, sc), 1, 2048),
            tint < 0 ? 0 : static_cast<u32>(tint), tint < 0 ? keep : stroke);
      frame.nodes[n].cos = clamp(m.a * unit / sc, -unit, unit);
      frame.nodes[n].sin = clamp(m.b * unit / sc, -unit, unit);
    }
  }
  void segment(i32 x0, i32 y0, i32 x1, i32 y1, Ink color = WHITE,
               i32 width = unit) {
    x0 = X + mul(S, x0);
    x1 = X + mul(S, x1);
    y0 = Y + mul(S, y0);
    y1 = Y + mul(S, y1);
    const i32 dx = (x1 - x0) / 2, dy = (y1 - y0) / 2;
    const i32 length =
        static_cast<i32>(root(static_cast<u32>(dx * dx + dy * dy))) * 2;
    if (length < 256)
      return;
    if (lines >= 160) {
      ++overflow;
      return;
    }
    const i32 sc = clamp(length / 32, 1, 2048);
    const u32 n = 11 + lines++;
    place(frame, n, x0, y0, sc, ink(color, mul(S, width), sc), stroke);
    frame.nodes[n].cos = clamp((x1 - x0) * unit / length, -unit, unit);
    frame.nodes[n].sin = clamp((y1 - y0) * unit / length, -unit, unit);
  }
  void line(i32 x0, i32 y0, i32 x1, i32 y1, Ink color = WHITE,
            i32 width = unit) {
    const i32 dx = x1 - x0, dy = y1 - y0, ax = dx < 0 ? -dx : dx,
              ay = dy < 0 ? -dy : dy;
    const i32 pieces = (ax > ay ? ax : ay) / px(52) + 1;
    for (i32 i = 0; i < pieces; ++i)
      segment(x0 + dx * i / pieces, y0 + dy * i / pieces,
              x0 + dx * (i + 1) / pieces, y0 + dy * (i + 1) / pieces, color,
              width);
  }
  void circle(i32 x, i32 y, i32 radius, Ink color = WHITE, bool fill = false,
              i32 width = unit) {
    if (radius <= 0)
      return;
    if (circles >= 8) {
      ++overflow;
      return;
    }
    const i32 sc = clamp(mul(S, radius) / 16, 1, 2048);
    const u32 n = 11 + 160 + circles++;
    place(frame, n, X + mul(S, x), Y + mul(S, y), sc,
          ink(color, mul(S, width), sc), fill ? fill_and_stroke : stroke);
  }
  void arc(i32 x, i32 y, i32 radius, i32 a0, i32 a1, Ink color = WHITE,
           i32 steps = 10, i32 width = unit) {
    i32 ox = x + mul(radius, cs(a0)), oy = y + mul(radius, sn(a0));
    for (i32 i = 1; i <= steps; ++i) {
      const i32 a = a0 + (a1 - a0) * i / steps;
      const i32 xx = x + mul(radius, cs(a)), yy = y + mul(radius, sn(a));
      line(ox, oy, xx, yy, color, width);
      ox = xx;
      oy = yy;
    }
  }
  void box(i32 x, i32 y, i32 xx, i32 yy, Ink color = WHITE, i32 width = unit) {
    line(x, y, xx, y, color, width);
    line(xx, y, xx, yy, color, width);
    line(xx, yy, x, yy, color, width);
    line(x, yy, x, y, color, width);
  }
  void label(u32 slot, i32 x, i32 y, const char *value, i32 size = unit) {
    text(frame, slot, X + mul(S, x), Y + mul(S, y), size, value);
  }
};
} // namespace art
} // namespace hkstutter
