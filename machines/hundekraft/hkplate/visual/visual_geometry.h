// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkplate {
namespace art {

struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[12] = {{{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 200192, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 199168, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 196608, 69632},
                                  {{-1, -1, -1, -1}, -1, 0, 196608, 69632}};
static const Variant kVariants[1] = {{0, 1024, 1}};

} // namespace art
} // namespace hkplate
