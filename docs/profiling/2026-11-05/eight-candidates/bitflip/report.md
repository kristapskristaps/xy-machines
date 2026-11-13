# bitflip: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **7.839%** of the stock 580,352-cycle block budget. Midpoint run: 7.839%. Every control at its heaviest position: 4.224%. Heaviest single setting: Edge 12.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 7.839% |
| Model Pulse | 4.089% |
| Model Tri | 4.096% |
| Model Twin | 5.437% |
| Model Wave | 4.094% |
| Model FM | 4.253% |
| Model Metal | 4.132% |
| Model Noise | 3.264% |
| Model Logic | 4.077% |
| Color 0.0% | 4.045% |
| Color 12.5% | 4.053% |
| Color 25.0% | 4.082% |
| Color 37.5% | 4.053% |
| Color 50.0% | 4.099% |
| Color 62.5% | 4.059% |
| Color 75.0% | 4.028% |
| Color 87.5% | 4.070% |
| Color 100.0% | 4.085% |
| Edge 0.0% | 4.048% |
| Edge 12.5% | 7.681% |
| Edge 25.0% | 7.630% |
| Edge 37.5% | 7.681% |
| Edge 50.0% | 7.633% |
| Edge 62.5% | 7.678% |
| Edge 75.0% | 7.630% |
| Edge 87.5% | 7.681% |
| Edge 100.0% | 7.633% |
| Sweep 0.0% | 4.083% |
| Sweep 12.5% | 4.047% |
| Sweep 25.0% | 4.082% |
| Sweep 37.5% | 4.048% |
| Sweep 50.0% | 4.085% |
| Sweep 62.5% | 4.068% |
| Sweep 75.0% | 4.129% |
| Sweep 87.5% | 4.114% |
| Sweep 100.0% | 4.167% |
| Chord Off | 4.050% |
| Chord Oct | 5.447% |
| Chord Major | 5.394% |
| Chord Minor | 5.432% |
| Chord Sus | 5.412% |
| Chord Maj7 | 5.451% |
| Chord Min7 | 5.406% |
| Chord Power | 5.527% |
| Speed 8Hz | 4.048% |
| Speed 12Hz | 4.082% |
| Speed 16Hz | 4.047% |
| Speed 24Hz | 4.082% |
| Speed 32Hz | 4.048% |
| Speed 40Hz | 4.097% |
| Speed 64Hz | 4.060% |
| Speed 80Hz | 4.094% |
| Bits 2 | 4.089% |
| Bits 3 | 4.125% |
| Bits 4 | 4.091% |
| Bits 5 | 4.126% |
| Bits 6 | 4.091% |
| Bits 7 | 4.126% |
| Bits 8 | 4.091% |
| Bits 9 | 4.126% |
| Bits 10 | 4.089% |
| Bits 11 | 4.126% |
| Bits 12 | 4.091% |
| Bits 13 | 4.125% |
| Bits 14 | 4.091% |
| Bits 15 | 4.126% |
| Bits 16 | 4.048% |
| Clock 1.4k | 4.211% |
| Clock 2.8k | 4.184% |
| Clock 5.5k | 4.230% |
| Clock 8.8k | 4.213% |
| Clock 11.0k | 4.253% |
| Clock 14.7k | 4.230% |
| Clock 22.1k | 4.303% |
| Clock 44.1k | 4.048% |
| combined (as read from the reports): Model Twin, Color 50.0%, Edge 12.5%, Sweep 100.0%, Chord Power, Speed 40Hz, Bits 5, Clock 22.1k | 4.224% |

Recommended: at most 3.5%. **Warning: 7.839% of the block budget at the midpoint is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
