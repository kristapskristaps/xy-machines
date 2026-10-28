// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkfmbell {
namespace art {
const unsigned G_clapper = 0;
const unsigned G_base0 = 1;
const unsigned G_base1 = 2;
const unsigned G_base2 = 3;
const unsigned G_base3 = 4;
const unsigned G_bar0 = 5;
const unsigned G_bar1 = 6;
const unsigned G_bar2 = 7;
const unsigned G_bar3 = 8;
struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[13] = {{{-1, -1, -1, -1}, -1, 0, 163840, 70656},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 59392},
                                  {{0, -1, -1, -1}, -1, 0, 163840, 73728},
                                  {{0, -1, -1, -1}, -1, 0, 163840, 104448},
                                  {{-1, -1, -1, -1}, -1, 0, 163840, 79974},
                                  {{1, -1, -1, -1}, -1, 0, 36864, 32768},
                                  {{2, -1, -1, -1}, -1, 0, 290816, 32768},
                                  {{3, -1, -1, -1}, -1, 0, 36864, 131072},
                                  {{4, -1, -1, -1}, -1, 0, 290816, 131072},
                                  {{5, -1, -1, -1}, -1, 0, 36864, 28160},
                                  {{6, -1, -1, -1}, -1, 0, 290816, 28160},
                                  {{7, -1, -1, -1}, -1, 0, 36864, 126464},
                                  {{8, -1, -1, -1}, -1, 0, 290816, 126464}};
static const Variant kVariants[9] = {{0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
                                     {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
                                     {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}};

} // namespace art
} // namespace hkfmbell
