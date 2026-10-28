// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkfather {
namespace art {
const unsigned G_gate_person = 0;
const unsigned G_face = 1;
struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[7] = {{{-1, -1, -1, -1}, -1, 0, 163840, 71680},
                                 {{0, -1, -1, -1}, -1, 0, 106496, 84480},
                                 {{-1, -1, -1, -1}, -1, 0, 37376, 91648},
                                 {{-1, -1, -1, -1}, -1, 0, 37888, 98304},
                                 {{1, -1, -1, -1}, -1, 0, 159744, 65536},
                                 {{1, -1, -1, -1}, -1, 0, 159744, 65024},
                                 {{1, -1, -1, -1}, -1, 0, 159744, 94464}};
static const Variant kVariants[2] = {{0, 1024, 1}, {0, 1024, 1}};

} // namespace art
} // namespace hkfather
