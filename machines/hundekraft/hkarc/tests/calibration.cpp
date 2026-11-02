#include "../core.h"
#include <cstdio>
int main() {
    opxy::u32 hash = 2166136261u;
    for (const auto& row : hkarc::port::ARC_GP) for (unsigned value : row) {
        hash = (hash ^ (value & 255u)) * 16777619u;
        hash = (hash ^ (value >> 8)) * 16777619u;
    }
    if (hash != 0xb743165du) {
        std::fprintf(stderr, "FAIL: original calibration fingerprint differs: %08x\n", hash);
        return 1;
    }
    std::puts("PASS: all 3,201 tuning values retain the original decoder fingerprint");
}
