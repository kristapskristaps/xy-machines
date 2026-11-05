# choir: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.264%** of the stock 580,352-cycle block budget. Midpoint run: 3.047%. Every control at its heaviest position: 2.538%. Heaviest single setting: Root 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.047% |
| Chord 0.0% | 2.867% |
| Chord 12.5% | 2.988% |
| Chord 25.0% | 2.913% |
| Chord 37.5% | 3.122% |
| Chord 50.0% | 2.954% |
| Chord 62.5% | 3.026% |
| Chord 75.0% | 2.669% |
| Chord 87.5% | 2.991% |
| Chord 100.0% | 2.914% |
| Voicing 0.0% | 2.681% |
| Voicing 12.5% | 2.684% |
| Voicing 25.0% | 2.878% |
| Voicing 37.5% | 2.959% |
| Voicing 50.0% | 3.166% |
| Voicing 62.5% | 2.901% |
| Voicing 75.0% | 2.983% |
| Voicing 87.5% | 2.754% |
| Voicing 100.0% | 2.834% |
| Colour 0.0% | 3.035% |
| Colour 12.5% | 3.029% |
| Colour 25.0% | 2.969% |
| Colour 37.5% | 3.024% |
| Colour 50.0% | 3.188% |
| Colour 62.5% | 3.027% |
| Colour 75.0% | 2.968% |
| Colour 87.5% | 3.024% |
| Colour 100.0% | 3.035% |
| Root 0.0% | 3.140% |
| Root 12.5% | 3.058% |
| Root 25.0% | 3.134% |
| Root 37.5% | 3.056% |
| Root 50.0% | 3.264% |
| Root 62.5% | 3.035% |
| Root 75.0% | 3.111% |
| Root 87.5% | 3.035% |
| Root 100.0% | 3.113% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 50.0%, Root 50.0% | 2.538% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
