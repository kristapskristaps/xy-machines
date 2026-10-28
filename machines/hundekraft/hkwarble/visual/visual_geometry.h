// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkwarble {
namespace art {

struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[14] = {{{-1, -1, -1, -1}, -1, 0, 163840, 59392},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 74752},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 59392},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 75776},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 59392},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 78848},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 75776},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 59904},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 59392},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 60416},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 63488},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 93184},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 136704},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 136192}};
static const Variant kVariants[1] = {{0, 1024, 1}};

} // namespace art
} // namespace hkwarble
