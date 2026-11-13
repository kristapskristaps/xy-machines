# weave_grain: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **8.368%** of the stock 580,352-cycle block budget. Midpoint run: 8.249%. Every control at its heaviest position: 5.141%. Heaviest single setting: Z mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 8.249% |
| Harmonic 0.0% | 6.238% |
| Harmonic 12.5% | 6.201% |
| Harmonic 25.0% | 6.220% |
| Harmonic 37.5% | 6.200% |
| Harmonic 50.0% | 6.218% |
| Harmonic 62.5% | 6.172% |
| Harmonic 75.0% | 6.204% |
| Harmonic 87.5% | 6.162% |
| Harmonic 100.0% | 6.203% |
| Timbre 0.0% | 6.131% |
| Timbre 12.5% | 6.162% |
| Timbre 25.0% | 6.123% |
| Timbre 37.5% | 6.134% |
| Timbre 50.0% | 6.186% |
| Timbre 62.5% | 6.249% |
| Timbre 75.0% | 6.223% |
| Timbre 87.5% | 6.242% |
| Timbre 100.0% | 6.210% |
| Morph 0.0% | 5.226% |
| Morph 12.5% | 5.354% |
| Morph 25.0% | 5.786% |
| Morph 37.5% | 6.178% |
| Morph 50.0% | 6.255% |
| Morph 62.5% | 6.236% |
| Morph 75.0% | 5.873% |
| Morph 87.5% | 5.403% |
| Morph 100.0% | 5.243% |
| Z mix 0.0% | 6.197% |
| Z mix 12.5% | 6.226% |
| Z mix 25.0% | 6.198% |
| Z mix 37.5% | 6.227% |
| Z mix 50.0% | 8.368% |
| Z mix 62.5% | 5.110% |
| Z mix 75.0% | 5.072% |
| Z mix 87.5% | 5.110% |
| Z mix 100.0% | 5.071% |
| combined (as read from the reports): Harmonic 0.0%, Timbre 62.5%, Morph 50.0%, Z mix 50.0% | 5.141% |

Recommended: at most 3.5%. **Warning: 8.368% of the block budget at Z mix 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
