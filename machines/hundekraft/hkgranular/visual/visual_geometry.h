// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkgranular {
namespace art {
const unsigned G_base0 = 0;
const unsigned G_base1 = 1;
const unsigned G_base2 = 2;
const unsigned G_base3 = 3;
const unsigned G_bar0 = 4;
const unsigned G_bar1 = 5;
const unsigned G_bar2 = 6;
const unsigned G_bar3 = 7;
struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[10] = {{{-1, -1, -1, -1}, -1, 0, 163840, 61440},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 134963},
                                  {{0, -1, -1, -1}, -1, 0, 71680, 136192},
                                  {{1, -1, -1, -1}, -1, 0, 133120, 136192},
                                  {{2, -1, -1, -1}, -1, 0, 194560, 136192},
                                  {{3, -1, -1, -1}, -1, 0, 256000, 136192},
                                  {{4, -1, -1, -1}, -1, 0, 71680, 131584},
                                  {{5, -1, -1, -1}, -1, 0, 133120, 131584},
                                  {{6, -1, -1, -1}, -1, 0, 194560, 131584},
                                  {{7, -1, -1, -1}, -1, 0, 256000, 131584}};
static const Variant kVariants[8] = {{0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
                                     {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
                                     {0, 1024, 1}, {0, 1024, 1}};

} // namespace art
} // namespace hkgranular
