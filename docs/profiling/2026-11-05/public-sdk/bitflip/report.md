# bitflip: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.879%** of the stock 580,352-cycle block budget. Midpoint run: 3.680%. Every control at its heaviest position: 3.239%. Heaviest single setting: Model FM.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.680% |
| Model Pulse | 3.088% |
| Model Tri | 3.059% |
| Model Twin | 3.592% |
| Model Wave | 3.052% |
| Model FM | 3.879% |
| Model Metal | 3.790% |
| Model Noise | 2.940% |
| Model Logic | 3.041% |
| Color 0.0% | 3.084% |
| Color 12.5% | 3.078% |
| Color 25.0% | 3.084% |
| Color 37.5% | 3.078% |
| Color 50.0% | 3.084% |
| Color 62.5% | 3.079% |
| Color 75.0% | 3.082% |
| Color 87.5% | 3.079% |
| Color 100.0% | 3.084% |
| Edge 0.0% | 3.078% |
| Edge 12.5% | 3.561% |
| Edge 25.0% | 3.555% |
| Edge 37.5% | 3.560% |
| Edge 50.0% | 3.554% |
| Edge 62.5% | 3.560% |
| Edge 75.0% | 3.554% |
| Edge 87.5% | 3.558% |
| Edge 100.0% | 3.554% |
| Sweep 0.0% | 3.084% |
| Sweep 12.5% | 3.078% |
| Sweep 25.0% | 3.081% |
| Sweep 37.5% | 3.078% |
| Sweep 50.0% | 3.082% |
| Sweep 62.5% | 3.111% |
| Sweep 75.0% | 3.151% |
| Sweep 87.5% | 3.174% |
| Sweep 100.0% | 3.201% |
| Chord Off | 3.079% |
| Chord Oct | 3.232% |
| Chord Major | 3.224% |
| Chord Minor | 3.249% |
| Chord Sus | 3.236% |
| Chord Maj7 | 3.236% |
| Chord Min7 | 3.215% |
| Chord Power | 3.276% |
| Speed 8Hz | 3.079% |
| Speed 12Hz | 3.084% |
| Speed 16Hz | 3.079% |
| Speed 24Hz | 3.082% |
| Speed 32Hz | 3.079% |
| Speed 40Hz | 3.084% |
| Speed 64Hz | 3.079% |
| Speed 80Hz | 3.082% |
| Bits 2 | 3.153% |
| Bits 3 | 3.159% |
| Bits 4 | 3.154% |
| Bits 5 | 3.157% |
| Bits 6 | 3.153% |
| Bits 7 | 3.157% |
| Bits 8 | 3.153% |
| Bits 9 | 3.159% |
| Bits 10 | 3.154% |
| Bits 11 | 3.157% |
| Bits 12 | 3.154% |
| Bits 13 | 3.159% |
| Bits 14 | 3.154% |
| Bits 15 | 3.157% |
| Bits 16 | 3.079% |
| Clock 1.4k | 3.153% |
| Clock 2.8k | 3.153% |
| Clock 5.5k | 3.166% |
| Clock 8.8k | 3.154% |
| Clock 11.0k | 3.162% |
| Clock 14.7k | 3.177% |
| Clock 22.1k | 3.197% |
| Clock 44.1k | 3.078% |
| combined (as read from the reports): Model FM, Color 0.0%, Edge 12.5%, Sweep 100.0%, Chord Power, Speed 12Hz, Bits 3, Clock 22.1k | 3.239% |

Recommended: at most 3.5%. **Warning: 3.879% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
