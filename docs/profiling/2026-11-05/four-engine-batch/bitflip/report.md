# bitflip: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.778%** of the stock 580,352-cycle block budget. Midpoint run: 3.546%. Every control at its heaviest position: 3.107%. Heaviest single setting: Model FM.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.546% |
| Model Pulse | 2.976% |
| Model Tri | 2.986% |
| Model Twin | 3.459% |
| Model Wave | 2.953% |
| Model FM | 3.778% |
| Model Metal | 3.774% |
| Model Noise | 2.901% |
| Model Logic | 2.980% |
| Color 0.0% | 2.976% |
| Color 12.5% | 2.995% |
| Color 25.0% | 2.976% |
| Color 37.5% | 2.998% |
| Color 50.0% | 2.976% |
| Color 62.5% | 2.998% |
| Color 75.0% | 2.976% |
| Color 87.5% | 3.000% |
| Color 100.0% | 2.976% |
| Edge 0.0% | 2.997% |
| Edge 12.5% | 3.413% |
| Edge 25.0% | 3.430% |
| Edge 37.5% | 3.412% |
| Edge 50.0% | 3.429% |
| Edge 62.5% | 3.412% |
| Edge 75.0% | 3.429% |
| Edge 87.5% | 3.412% |
| Edge 100.0% | 3.427% |
| Sweep 0.0% | 2.974% |
| Sweep 12.5% | 2.997% |
| Sweep 25.0% | 2.974% |
| Sweep 37.5% | 2.998% |
| Sweep 50.0% | 2.976% |
| Sweep 62.5% | 3.023% |
| Sweep 75.0% | 3.037% |
| Sweep 87.5% | 3.085% |
| Sweep 100.0% | 3.085% |
| Chord Off | 2.998% |
| Chord Oct | 3.111% |
| Chord Major | 3.125% |
| Chord Minor | 3.108% |
| Chord Sus | 3.125% |
| Chord Maj7 | 3.105% |
| Chord Min7 | 3.124% |
| Chord Power | 3.120% |
| Speed 8Hz | 2.998% |
| Speed 12Hz | 2.974% |
| Speed 16Hz | 2.998% |
| Speed 24Hz | 2.972% |
| Speed 32Hz | 2.995% |
| Speed 40Hz | 2.974% |
| Speed 64Hz | 2.998% |
| Speed 80Hz | 2.974% |
| Bits 2 | 3.076% |
| Bits 3 | 3.055% |
| Bits 4 | 3.078% |
| Bits 5 | 3.055% |
| Bits 6 | 3.075% |
| Bits 7 | 3.052% |
| Bits 8 | 3.078% |
| Bits 9 | 3.053% |
| Bits 10 | 3.078% |
| Bits 11 | 3.055% |
| Bits 12 | 3.078% |
| Bits 13 | 3.055% |
| Bits 14 | 3.078% |
| Bits 15 | 3.055% |
| Bits 16 | 2.998% |
| Clock 1.4k | 3.020% |
| Clock 2.8k | 3.050% |
| Clock 5.5k | 3.037% |
| Clock 8.8k | 3.072% |
| Clock 11.0k | 3.056% |
| Clock 14.7k | 3.095% |
| Clock 22.1k | 3.088% |
| Clock 44.1k | 2.998% |
| combined (as read from the reports): Model FM, Color 87.5%, Edge 25.0%, Sweep 87.5%, Chord Major, Speed 8Hz, Bits 4, Clock 14.7k | 3.107% |

Recommended: at most 3.5%. **Warning: 3.778% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
