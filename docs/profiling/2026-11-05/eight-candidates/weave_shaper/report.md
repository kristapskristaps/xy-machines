# weave_shaper: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.684%** of the stock 580,352-cycle block budget. Midpoint run: 3.684%. Every control at its heaviest position: 2.899%. Heaviest single setting: Alt mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.684% |
| Harmonic 0.0% | 2.921% |
| Harmonic 12.5% | 2.892% |
| Harmonic 25.0% | 2.889% |
| Harmonic 37.5% | 2.861% |
| Harmonic 50.0% | 2.835% |
| Harmonic 62.5% | 2.834% |
| Harmonic 75.0% | 2.855% |
| Harmonic 87.5% | 2.829% |
| Harmonic 100.0% | 2.925% |
| Timbre 0.0% | 2.876% |
| Timbre 12.5% | 2.853% |
| Timbre 25.0% | 2.786% |
| Timbre 37.5% | 2.861% |
| Timbre 50.0% | 2.860% |
| Timbre 62.5% | 2.902% |
| Timbre 75.0% | 2.832% |
| Timbre 87.5% | 2.849% |
| Timbre 100.0% | 2.846% |
| Morph 0.0% | 2.879% |
| Morph 12.5% | 2.869% |
| Morph 25.0% | 2.892% |
| Morph 37.5% | 2.861% |
| Morph 50.0% | 2.818% |
| Morph 62.5% | 2.852% |
| Morph 75.0% | 2.878% |
| Morph 87.5% | 2.838% |
| Morph 100.0% | 2.832% |
| Alt mix 0.0% | 2.795% |
| Alt mix 12.5% | 2.821% |
| Alt mix 25.0% | 2.797% |
| Alt mix 37.5% | 2.820% |
| Alt mix 50.0% | 3.571% |
| Alt mix 62.5% | 3.261% |
| Alt mix 75.0% | 3.236% |
| Alt mix 87.5% | 3.261% |
| Alt mix 100.0% | 3.236% |
| combined (as read from the reports): Harmonic 100.0%, Timbre 62.5%, Morph 25.0%, Alt mix 50.0% | 2.899% |

Recommended: at most 3.5%. **Warning: 3.684% of the block budget at the midpoint is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
