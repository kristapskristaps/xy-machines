# crease: hardware CPU measurement

Machine commit: `06af6c4a9036c3305d3cd02a3237dea3f0944c04`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.516%** of the stock 580,352-cycle block budget. Midpoint run: 3.516%. Every control at its heaviest position: 2.724%. Heaviest single setting: Alt mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.516% |
| Harmonic 0.0% | 2.718% |
| Harmonic 12.5% | 2.838% |
| Harmonic 25.0% | 2.820% |
| Harmonic 37.5% | 2.837% |
| Harmonic 50.0% | 2.846% |
| Harmonic 62.5% | 2.808% |
| Harmonic 75.0% | 2.826% |
| Harmonic 87.5% | 2.849% |
| Harmonic 100.0% | 2.751% |
| Timbre 0.0% | 2.750% |
| Timbre 12.5% | 2.716% |
| Timbre 25.0% | 2.792% |
| Timbre 37.5% | 2.820% |
| Timbre 50.0% | 2.835% |
| Timbre 62.5% | 2.741% |
| Timbre 75.0% | 2.771% |
| Timbre 87.5% | 2.792% |
| Timbre 100.0% | 2.817% |
| Morph 0.0% | 2.829% |
| Morph 12.5% | 2.841% |
| Morph 25.0% | 2.792% |
| Morph 37.5% | 2.837% |
| Morph 50.0% | 2.829% |
| Morph 62.5% | 2.826% |
| Morph 75.0% | 2.780% |
| Morph 87.5% | 2.823% |
| Morph 100.0% | 2.783% |
| Alt mix 0.0% | 2.837% |
| Alt mix 12.5% | 2.834% |
| Alt mix 25.0% | 2.840% |
| Alt mix 37.5% | 2.838% |
| Alt mix 50.0% | 3.449% |
| Alt mix 62.5% | 3.111% |
| Alt mix 75.0% | 3.130% |
| Alt mix 87.5% | 3.117% |
| Alt mix 100.0% | 3.128% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 50.0%, Morph 12.5%, Alt mix 50.0% | 2.724% |

Recommended: at most 3.5%. **Warning: 3.516% of the block budget at the midpoint is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `877c1a4921e9de5b644c04fc6a068d00b2b5e7ae572c928879905dc6ef0224e5`.

Compile report SHA-256: `1bf0d650cae5f12ae5553dacd83fab5c05f61fe1e972f2e8fc738d464556cd4e`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
