# weave_chords: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.423%** of the stock 580,352-cycle block budget. Midpoint run: 3.168%. Every control at its heaviest position: 2.811%. Heaviest single setting: Root 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.168% |
| Chord 0.0% | 3.163% |
| Chord 12.5% | 3.156% |
| Chord 25.0% | 3.172% |
| Chord 37.5% | 3.302% |
| Chord 50.0% | 3.204% |
| Chord 62.5% | 3.182% |
| Chord 75.0% | 2.997% |
| Chord 87.5% | 3.157% |
| Chord 100.0% | 3.172% |
| Voicing 0.0% | 2.809% |
| Voicing 12.5% | 2.786% |
| Voicing 25.0% | 2.963% |
| Voicing 37.5% | 3.069% |
| Voicing 50.0% | 3.349% |
| Voicing 62.5% | 3.252% |
| Voicing 75.0% | 3.163% |
| Voicing 87.5% | 3.026% |
| Voicing 100.0% | 3.018% |
| Colour 0.0% | 3.084% |
| Colour 12.5% | 3.003% |
| Colour 25.0% | 3.122% |
| Colour 37.5% | 3.003% |
| Colour 50.0% | 3.339% |
| Colour 62.5% | 3.003% |
| Colour 75.0% | 3.122% |
| Colour 87.5% | 3.001% |
| Colour 100.0% | 3.084% |
| Root 0.0% | 3.310% |
| Root 12.5% | 3.285% |
| Root 25.0% | 3.304% |
| Root 37.5% | 3.285% |
| Root 50.0% | 3.423% |
| Root 62.5% | 3.210% |
| Root 75.0% | 3.279% |
| Root 87.5% | 3.267% |
| Root 100.0% | 3.281% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 50.0%, Root 50.0% | 2.811% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
