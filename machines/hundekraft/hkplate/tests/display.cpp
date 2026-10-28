#include "../visual/display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool yes, const char *message) {
  if (!yes) {
    std::fprintf(stderr, "hkplate: %s\n", message);
    std::exit(1);
  }
}
int main() {
  using namespace hkplate;
  using namespace opxy;
  using namespace opxy::draw;
  static_assert(sizeof(DisplayState) == 184, "Display ABI size");
  DisplayState s;
  display_reset(s);
  hkplate::Frame f = {};
  Input in = {};
  in.elapsed = 17;
  in.held_notes = 0;
  for (unsigned i = 0; i < 4; ++i)
    in.shown[i] = i;
  display(s, in, f);
  require(valid(f), "initial frame valid");
  require(s.age > 1000 && s.level == 0,
          "activation must not fabricate a strike");
  const hkplate::Frame idle = f;
  for (unsigned i = 0; i < 240; ++i)
    display(s, in, f);
  require(!std::memcmp(&idle, &f, sizeof(f)), "idle must stay still");
  unsigned seed = 1234567;
  for (unsigned n = 0; n < 1800; ++n) {
    for (unsigned i = 0; i < 8; ++i) {
      seed = seed * 1664525u + 1013904223u;
      in.slots[i] = n % 3 == 0   ? 0
                    : n % 3 == 1 ? 0x7fffffff
                                 : static_cast<i32>(seed & 0x7fffffffu);
    }
    for (unsigned i = 0; i < 4; ++i) {
      in.shown[i] = i + (n & 1 ? 4 : 0);
      in.parameters[i] = in.slots[in.shown[i]];
    }
    in.elapsed = n % 31 == 0 ? -1 : n % 37 == 0 ? 0x7fffffff : 17;
    in.note_serial = n / 9;
    in.held_notes = n % 17;
    in.note_off_serial = n / 13;
    in.track = n / 300;
    in.reduced_motion = n % 7 == 0;
    display(s, in, f);
    if (!valid(f))
      for (unsigned j = 0; j < f.node_count; ++j) {
        const Node &p = f.nodes[j];
        if (!placement(p.x, p.y, p.scale) || !rotation_valid(p.cos, p.sin) ||
            p.shade >= 16)
          std::fprintf(stderr,
                       "frame %u node %u: %d %d scale %d turn %d %d ink %u\n",
                       n, j, p.x, p.y, p.scale, p.cos, p.sin, p.shade);
      }
    require(valid(f),
            "control corners and note activity must emit valid frames");
    require(s.memory[31] == 0, "scene exceeded its retained geometry budget");
  }
  in.reduced_motion = 1;
  in.elapsed = 17;
  in.held_notes = 1;
  in.note_serial++;
  display(s, in, f);
  const hkplate::Frame reduced = f;
  for (unsigned i = 0; i < 240; ++i)
    display(s, in, f);
  require(!std::memcmp(&reduced, &f, sizeof(f)),
          "reduced motion must hold a stable pose");
  in.track++;
  display(s, in, f);
  require(s.age > 1000, "track changes must establish a fresh baseline");
  std::puts("PASS: hkplate drawing bounds, budgets, note baseline, idle and "
            "reduced motion");
}
