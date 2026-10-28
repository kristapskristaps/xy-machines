// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace hkwasher {
namespace art {
const unsigned G_water = 0;
const unsigned G_bub0 = 1;
const unsigned G_bub1 = 2;
const unsigned G_bub2 = 3;
const unsigned G_bub3 = 4;
const unsigned G_bub4 = 5;
const unsigned G_bub5 = 6;
const unsigned G_bub6 = 7;
const unsigned G_bub7 = 8;
const unsigned G_bub8 = 9;
const unsigned G_socks = 10;
const unsigned G_sock = 11;
const unsigned G_base0 = 12;
const unsigned G_bar0 = 13;
const unsigned G_base1 = 14;
const unsigned G_bar1 = 15;
const unsigned G_base2 = 16;
const unsigned G_bar2 = 17;
const unsigned G_base3 = 18;
const unsigned G_bar3 = 19;
struct Piece {
  int parents[4], variantGroup, variant, cx, cy;
};
struct Variant {
  int low, high, count;
};
static const Piece kPieces[30] = {{{-1, -1, -1, -1}, -1, 0, 102400, 77824},
                                  {{-1, -1, -1, -1}, -1, 0, 161536, 76288},
                                  {{-1, -1, -1, -1}, -1, 0, 102400, 77824},
                                  {{0, -1, -1, -1}, -1, 0, 101325, 108544},
                                  {{1, -1, -1, -1}, -1, 0, 129434, 38502},
                                  {{2, -1, -1, -1}, -1, 0, 139059, 49459},
                                  {{3, -1, -1, -1}, -1, 0, 63078, 93286},
                                  {{4, -1, -1, -1}, -1, 0, 71066, 99942},
                                  {{5, -1, -1, -1}, -1, 0, 146534, 91955},
                                  {{6, -1, -1, -1}, -1, 0, 143565, 99942},
                                  {{7, -1, -1, -1}, -1, 0, 82227, 115814},
                                  {{8, -1, -1, -1}, -1, 0, 92058, 122675},
                                  {{9, -1, -1, -1}, -1, 0, 119194, 120627},
                                  {{10, -1, -1, -1}, -1, 0, 75930, 59597},
                                  {{10, -1, -1, -1}, -1, 0, 76851, 46285},
                                  {{10, -1, -1, -1}, -1, 0, 78490, 60160},
                                  {{10, -1, -1, -1}, -1, 0, 98458, 70861},
                                  {{10, -1, -1, -1}, -1, 0, 99379, 57549},
                                  {{10, -1, -1, -1}, -1, 0, 101018, 71424},
                                  {{10, -1, -1, -1}, -1, 0, 120986, 82125},
                                  {{10, -1, -1, -1}, -1, 0, 121907, 68813},
                                  {{10, -1, -1, -1}, -1, 0, 123546, 82688},
                                  {{11, -1, -1, -1}, -1, 0, 252467, 90163},
                                  {{11, -1, -1, -1}, -1, 0, 269056, 75674},
                                  {{11, -1, -1, -1}, -1, 0, 253133, 91341},
                                  {{11, -1, -1, -1}, -1, 0, 294400, 78336},
                                  {{12, -1, -1, -1}, -1, 0, 1536, 2048},
                                  {{14, -1, -1, -1}, -1, 0, 3584, 2048},
                                  {{16, -1, -1, -1}, -1, 0, 5632, 2048},
                                  {{18, -1, -1, -1}, -1, 0, 7680, 2048}};
static const Variant kVariants[20] = {
    {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
    {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
    {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1},
    {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}, {0, 1024, 1}};

} // namespace art
} // namespace hkwasher
