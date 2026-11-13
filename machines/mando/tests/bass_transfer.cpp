#include "../dsp.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
static void require(bool ok, const char *why) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", why);
        std::exit(1);
    }
}
int main() {
    using namespace mando::dsp;
    int previous = 0, knee = 0, error = 0, knee_error = 0;
    for (int value = 0; value <= 131200; ++value) {
        const int result = bass_saturate(value), limited = bass_knee(value);
        require(result >= previous && result <= unity,
                "bass transfer is monotone and bounded");
        require(limited >= knee && limited <= 14336,
                "bass output knee is monotone and bounded");
        require(bass_saturate(-value) == -result &&
                    bass_knee(-value) == -limited,
                "both bass curves have odd symmetry");
        require(value > 10240 || limited == value,
                "output knee preserves quiet bass signals exactly");
        error = std::max(error, std::abs(result - saturate(value)));
        knee_error =
            std::max(knee_error, std::abs(limited - output_knee(value)));
        previous = result;
        knee = limited;
    }
    require(error <= 40, "input transfer differs by no more than40Q14LSB");
    require(knee_error <= 256, "output knee differs by no more than256Q14LSB");
    std::printf("PASS: exhaustive bass transfer bounds and symmetry; input "
                "error%dQ14LSB, knee error%dQ14LSB\n",
                error, knee_error);
}
