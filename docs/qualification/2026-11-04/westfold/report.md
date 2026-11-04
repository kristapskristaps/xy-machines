# westfold: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 72 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **9.041%** of the stock 580,352-cycle block budget. Midpoint run: 8.867%. Every control at its heaviest position: 4.080%. Heaviest single setting: Decay 100.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 8.867% |
| Overtone 0.0% | 6.607% |
| Overtone 12.5% | 8.629% |
| Overtone 25.0% | 8.641% |
| Overtone 37.5% | 8.631% |
| Overtone 50.0% | 8.646% |
| Overtone 62.5% | 8.631% |
| Overtone 75.0% | 8.646% |
| Overtone 87.5% | 8.634% |
| Overtone 100.0% | 8.640% |
| Multiply 0.0% | 8.621% |
| Multiply 12.5% | 8.615% |
| Multiply 25.0% | 8.525% |
| Multiply 37.5% | 8.537% |
| Multiply 50.0% | 8.775% |
| Multiply 62.5% | 9.006% |
| Multiply 75.0% | 8.742% |
| Multiply 87.5% | 8.577% |
| Multiply 100.0% | 8.553% |
| Balance 0.0% | 4.239% |
| Balance 12.5% | 8.632% |
| Balance 25.0% | 8.647% |
| Balance 37.5% | 8.632% |
| Balance 50.0% | 8.638% |
| Balance 62.5% | 8.631% |
| Balance 75.0% | 8.643% |
| Balance 87.5% | 8.632% |
| Balance 100.0% | 8.643% |
| Decay 0.0% | 8.592% |
| Decay 12.5% | 8.600% |
| Decay 25.0% | 8.589% |
| Decay 37.5% | 8.611% |
| Decay 50.0% | 8.609% |
| Decay 62.5% | 8.644% |
| Decay 75.0% | 8.682% |
| Decay 87.5% | 8.751% |
| Decay 100.0% | 9.041% |
| FM Ratio 0.0% | 8.562% |
| FM Ratio 12.5% | 8.637% |
| FM Ratio 25.0% | 8.711% |
| FM Ratio 37.5% | 8.775% |
| FM Ratio 50.0% | 8.890% |
| FM Ratio 62.5% | 8.913% |
| FM Ratio 75.0% | 8.910% |
| FM Ratio 87.5% | 8.896% |
| FM Ratio 100.0% | 8.905% |
| FM Depth 0.0% | 8.631% |
| FM Depth 12.5% | 8.641% |
| FM Depth 25.0% | 8.663% |
| FM Depth 37.5% | 8.690% |
| FM Depth 50.0% | 8.664% |
| FM Depth 62.5% | 8.661% |
| FM Depth 75.0% | 8.727% |
| FM Depth 87.5% | 8.661% |
| FM Depth 100.0% | 8.742% |
| Slope Hz 0.0% | 8.508% |
| Slope Hz 12.5% | 8.635% |
| Slope Hz 25.0% | 8.632% |
| Slope Hz 37.5% | 8.627% |
| Slope Hz 50.0% | 8.643% |
| Slope Hz 62.5% | 8.638% |
| Slope Hz 75.0% | 8.682% |
| Slope Hz 87.5% | 8.867% |
| Slope Hz 100.0% | 8.972% |
| Motion 0.0% | 8.618% |
| Motion 12.5% | 8.640% |
| Motion 25.0% | 8.650% |
| Motion 37.5% | 8.673% |
| Motion 50.0% | 8.664% |
| Motion 62.5% | 8.663% |
| Motion 75.0% | 8.647% |
| Motion 87.5% | 8.650% |
| Motion 100.0% | 8.638% |
| combined (as read from the reports): Overtone 50.0%, Multiply 62.5%, Balance 25.0%, Decay 100.0%, FM Ratio 62.5%, FM Depth 100.0%, Slope Hz 100.0%, Motion 37.5% | 4.080% |

Recommended: at most 3.5%. **Warning: 9.041% of the block budget at Decay 100.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `9e2971b041e82946f1cd66ca2e868f1ffbfca7d3147f387d732c4939c05d8d75`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9d484556b54cbdf18ea6ca5e9325bc012ff24ab30a0cffb5272e8eb7f7957aed`. Firmware SHA-256 above identifies its resident kernel.
