// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkcrush {
namespace art {

struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[8] = {{{-1, -1, -1, -1}, -1, 0, 163840, 75776},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 79360},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 76288},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 65536},
                                 {{-1, -1, -1, -1}, -1, 0, 163840, 143360}};
static const Variant kVariants[1] = {{0, 1024, 1}};

} // namespace art
} // namespace hkcrush
