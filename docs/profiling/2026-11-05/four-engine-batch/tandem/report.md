# tandem: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **10.330%** of the stock 580,352-cycle block budget. Midpoint run: 10.127%. Every control at its heaviest position: 6.430%. Heaviest single setting: Output 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 10.127% |
| Harmonic 0.0% | 6.427% |
| Harmonic 12.5% | 6.555% |
| Harmonic 25.0% | 6.523% |
| Harmonic 37.5% | 6.545% |
| Harmonic 50.0% | 6.497% |
| Harmonic 62.5% | 6.600% |
| Harmonic 75.0% | 6.574% |
| Harmonic 87.5% | 6.688% |
| Harmonic 100.0% | 6.667% |
| Timbre 0.0% | 6.523% |
| Timbre 12.5% | 6.513% |
| Timbre 25.0% | 6.546% |
| Timbre 37.5% | 6.508% |
| Timbre 50.0% | 6.554% |
| Timbre 62.5% | 6.525% |
| Timbre 75.0% | 6.581% |
| Timbre 87.5% | 6.671% |
| Timbre 100.0% | 7.039% |
| Morph 0.0% | 6.522% |
| Morph 12.5% | 6.552% |
| Morph 25.0% | 6.505% |
| Morph 37.5% | 6.540% |
| Morph 50.0% | 6.418% |
| Morph 62.5% | 6.526% |
| Morph 75.0% | 6.506% |
| Morph 87.5% | 6.566% |
| Morph 100.0% | 6.505% |
| Output 0.0% | 6.542% |
| Output 12.5% | 6.497% |
| Output 25.0% | 6.540% |
| Output 37.5% | 6.500% |
| Output 50.0% | 10.330% |
| Output 62.5% | 6.935% |
| Output 75.0% | 6.978% |
| Output 87.5% | 6.932% |
| Output 100.0% | 6.976% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 100.0%, Morph 87.5%, Output 50.0% | 6.430% |

Recommended: at most 3.5%. **Warning: 10.330% of the block budget at Output 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
