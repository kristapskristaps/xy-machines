#include "../models.h"
#include <cstdio>
#include <cstdlib>
static void require(bool ok) { if (!ok) std::abort(); }
int main() {
    using namespace x0x;
    unsigned cases = 0;
    for (unsigned model = 0; model < model_count; ++model) for (unsigned raw = 0; raw < 128; ++raw) {
        unsigned choices[4], size = 0, position = 0;
        for (unsigned f = 0; f < 4; ++f) if (family_masks[model] & (1u << f)) {
            if (family_index(model,raw) == f) position = size;
            choices[size++] = f;
        }
        for (int direction = -1; direction <= 1; direction += 2) {
            const unsigned value = stepped_value(model,style,raw,direction);
            const int next = int(position) + direction;
            const unsigned expected = choices[next < 0 ? 0 : next >= int(size) ? size - 1 : unsigned(next)];
            require(value <= 127 && family_index(model,value) == expected);
            if (expected == choices[position]) require(value == raw);
            for (unsigned slot = 1; slot < control_count(model); ++slot) {
                const int linear = int(raw) + direction;
                require(stepped_value(model,slot,raw,direction) == unsigned(linear < 0 ? 0 : linear > 127 ? 127 : linear));
            }
            ++cases;
        }
    }
    require(stepped_value(kick,tune,127,2147483647) == 127);
    require(stepped_value(kick,tune,0,(-2147483647-1)) == 0);
    require(stepped_value(ride,style,48,1) == 112);
    require(stepped_value(ride,style,112,-1) == 48);
    require(stepped_value(model_count,style,16,1) == 16);
    std::printf("PASS: %u one-detent family edits, boundaries, missing families and continuous controls\n",cases);
}
