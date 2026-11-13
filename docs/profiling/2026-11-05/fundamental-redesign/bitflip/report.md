# bitflip: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.690%** of the stock 580,352-cycle block budget. Midpoint run: 3.665%. Every control at its heaviest position: 3.117%. Heaviest single setting: Model FM.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.665% |
| Model Pulse | 2.953% |
| Model Tri | 3.015% |
| Model Twin | 3.444% |
| Model Wave | 3.014% |
| Model FM | 3.690% |
| Model Metal | 3.661% |
| Model Noise | 2.890% |
| Model Logic | 3.001% |
| Color 0.0% | 2.953% |
| Color 12.5% | 3.035% |
| Color 25.0% | 2.951% |
| Color 37.5% | 3.035% |
| Color 50.0% | 2.951% |
| Color 62.5% | 3.037% |
| Color 75.0% | 2.951% |
| Color 87.5% | 3.037% |
| Color 100.0% | 2.951% |
| Edge 0.0% | 3.035% |
| Edge 12.5% | 3.371% |
| Edge 25.0% | 3.514% |
| Edge 37.5% | 3.369% |
| Edge 50.0% | 3.513% |
| Edge 62.5% | 3.369% |
| Edge 75.0% | 3.516% |
| Edge 87.5% | 3.371% |
| Edge 100.0% | 3.513% |
| Sweep 0.0% | 2.950% |
| Sweep 12.5% | 3.033% |
| Sweep 25.0% | 2.950% |
| Sweep 37.5% | 3.035% |
| Sweep 50.0% | 2.950% |
| Sweep 62.5% | 3.059% |
| Sweep 75.0% | 3.014% |
| Sweep 87.5% | 3.120% |
| Sweep 100.0% | 3.064% |
| Chord Off | 3.037% |
| Chord Oct | 3.101% |
| Chord Major | 3.194% |
| Chord Minor | 3.081% |
| Chord Sus | 3.194% |
| Chord Maj7 | 3.058% |
| Chord Min7 | 3.182% |
| Chord Power | 3.098% |
| Speed 8Hz | 3.040% |
| Speed 12Hz | 2.954% |
| Speed 16Hz | 3.040% |
| Speed 24Hz | 2.950% |
| Speed 32Hz | 3.033% |
| Speed 40Hz | 2.950% |
| Speed 64Hz | 3.033% |
| Speed 80Hz | 2.951% |
| Bits 2 | 3.117% |
| Bits 3 | 3.032% |
| Bits 4 | 3.117% |
| Bits 5 | 3.032% |
| Bits 6 | 3.117% |
| Bits 7 | 3.030% |
| Bits 8 | 3.116% |
| Bits 9 | 3.029% |
| Bits 10 | 3.116% |
| Bits 11 | 3.032% |
| Bits 12 | 3.117% |
| Bits 13 | 3.033% |
| Bits 14 | 3.116% |
| Bits 15 | 3.029% |
| Bits 16 | 3.035% |
| Clock 1.4k | 3.021% |
| Clock 2.8k | 3.110% |
| Clock 5.5k | 3.033% |
| Clock 8.8k | 3.122% |
| Clock 11.0k | 3.041% |
| Clock 14.7k | 3.142% |
| Clock 22.1k | 3.073% |
| Clock 44.1k | 3.035% |
| combined (as read from the reports): Model FM, Color 62.5%, Edge 75.0%, Sweep 87.5%, Chord Major, Speed 8Hz, Bits 2, Clock 14.7k | 3.117% |

Recommended: at most 3.5%. **Warning: 3.690% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
