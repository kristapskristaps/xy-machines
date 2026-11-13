# crease: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.554%** of the stock 580,352-cycle block budget. Midpoint run: 3.554%. Every control at its heaviest position: 2.734%. Heaviest single setting: Alt mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.554% |
| Harmonic 0.0% | 2.770% |
| Harmonic 12.5% | 2.713% |
| Harmonic 25.0% | 2.689% |
| Harmonic 37.5% | 2.744% |
| Harmonic 50.0% | 2.728% |
| Harmonic 62.5% | 2.730% |
| Harmonic 75.0% | 2.734% |
| Harmonic 87.5% | 2.794% |
| Harmonic 100.0% | 2.773% |
| Timbre 0.0% | 2.728% |
| Timbre 12.5% | 2.667% |
| Timbre 25.0% | 2.737% |
| Timbre 37.5% | 2.693% |
| Timbre 50.0% | 2.744% |
| Timbre 62.5% | 2.776% |
| Timbre 75.0% | 2.776% |
| Timbre 87.5% | 2.695% |
| Timbre 100.0% | 2.771% |
| Morph 0.0% | 2.698% |
| Morph 12.5% | 2.728% |
| Morph 25.0% | 2.721% |
| Morph 37.5% | 2.745% |
| Morph 50.0% | 2.707% |
| Morph 62.5% | 2.739% |
| Morph 75.0% | 2.707% |
| Morph 87.5% | 2.707% |
| Morph 100.0% | 2.675% |
| Alt mix 0.0% | 2.741% |
| Alt mix 12.5% | 2.708% |
| Alt mix 25.0% | 2.739% |
| Alt mix 37.5% | 2.708% |
| Alt mix 50.0% | 3.482% |
| Alt mix 62.5% | 3.182% |
| Alt mix 75.0% | 3.203% |
| Alt mix 87.5% | 3.182% |
| Alt mix 100.0% | 3.207% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 62.5%, Morph 37.5%, Alt mix 50.0% | 2.734% |

Recommended: at most 3.5%. **Warning: 3.554% of the block budget at the midpoint is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
