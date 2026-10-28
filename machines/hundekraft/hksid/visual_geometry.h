// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hksid {
namespace art {

struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[4] = {{{-1, -1, -1, -1}, -1, 0, 113316, 73298},
                                 {{-1, -1, -1, -1}, -1, 0, 113500, 71813},
                                 {{-1, -1, -1, -1}, -1, 0, 124764, 74045},
                                 {{-1, -1, -1, -1}, -1, 0, 148603, 72837}};
static const Variant kVariants[1] = {{0, 1024, 1}};

} // namespace art
} // namespace hksid
