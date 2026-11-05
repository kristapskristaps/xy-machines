# bitflip: hardware CPU measurement

Machine commit: `8fca276659ce8b4290f2429c719d479ebdd9a041`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.862%** of the stock 580,352-cycle block budget. Midpoint run: 3.510%. Every control at its heaviest position: 3.156%. Heaviest single setting: Model FM.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.510% |
| Model Pulse | 3.018% |
| Model Tri | 2.936% |
| Model Twin | 3.505% |
| Model Wave | 2.927% |
| Model FM | 3.862% |
| Model Metal | 3.676% |
| Model Noise | 2.864% |
| Model Logic | 2.898% |
| Color 0.0% | 3.017% |
| Color 12.5% | 2.933% |
| Color 25.0% | 3.017% |
| Color 37.5% | 2.933% |
| Color 50.0% | 3.017% |
| Color 62.5% | 2.934% |
| Color 75.0% | 3.017% |
| Color 87.5% | 2.933% |
| Color 100.0% | 3.015% |
| Edge 0.0% | 2.931% |
| Edge 12.5% | 3.490% |
| Edge 25.0% | 3.371% |
| Edge 37.5% | 3.488% |
| Edge 50.0% | 3.371% |
| Edge 62.5% | 3.488% |
| Edge 75.0% | 3.371% |
| Edge 87.5% | 3.488% |
| Edge 100.0% | 3.371% |
| Sweep 0.0% | 3.015% |
| Sweep 12.5% | 2.931% |
| Sweep 25.0% | 3.015% |
| Sweep 37.5% | 2.931% |
| Sweep 50.0% | 3.015% |
| Sweep 62.5% | 2.954% |
| Sweep 75.0% | 3.072% |
| Sweep 87.5% | 3.015% |
| Sweep 100.0% | 3.120% |
| Chord Off | 2.931% |
| Chord Oct | 3.188% |
| Chord Major | 3.087% |
| Chord Minor | 3.183% |
| Chord Sus | 3.075% |
| Chord Maj7 | 3.186% |
| Chord Min7 | 3.087% |
| Chord Power | 3.229% |
| Speed 8Hz | 2.933% |
| Speed 12Hz | 3.015% |
| Speed 16Hz | 2.931% |
| Speed 24Hz | 3.015% |
| Speed 32Hz | 2.933% |
| Speed 40Hz | 3.017% |
| Speed 64Hz | 2.931% |
| Speed 80Hz | 3.015% |
| Bits 2 | 3.014% |
| Bits 3 | 3.096% |
| Bits 4 | 3.012% |
| Bits 5 | 3.096% |
| Bits 6 | 3.012% |
| Bits 7 | 3.096% |
| Bits 8 | 3.012% |
| Bits 9 | 3.096% |
| Bits 10 | 3.012% |
| Bits 11 | 3.095% |
| Bits 12 | 3.012% |
| Bits 13 | 3.098% |
| Bits 14 | 3.012% |
| Bits 15 | 3.096% |
| Bits 16 | 2.933% |
| Clock 1.4k | 3.066% |
| Clock 2.8k | 2.989% |
| Clock 5.5k | 3.079% |
| Clock 8.8k | 3.009% |
| Clock 11.0k | 3.095% |
| Clock 14.7k | 3.029% |
| Clock 22.1k | 3.134% |
| Clock 44.1k | 2.933% |
| combined (as read from the reports): Model FM, Color 0.0%, Edge 12.5%, Sweep 100.0%, Chord Power, Speed 40Hz, Bits 13, Clock 22.1k | 3.156% |

Recommended: at most 3.5%. **Warning: 3.862% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `5f43133619279e6e2d631ca786e6db986c46dbffe6a47e868194bdb233b2829c`.

Compile report SHA-256: `928631edc149e1d5a22594097b307385be7d339af0888a5ae10c449225a6f33a`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
