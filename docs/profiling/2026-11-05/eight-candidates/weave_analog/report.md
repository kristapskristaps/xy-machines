# weave_analog: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **9.990%** of the stock 580,352-cycle block budget. Midpoint run: 9.798%. Every control at its heaviest position: 6.369%. Heaviest single setting: Output 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 9.798% |
| Harmonic 0.0% | 6.365% |
| Harmonic 12.5% | 6.435% |
| Harmonic 25.0% | 6.441% |
| Harmonic 37.5% | 6.436% |
| Harmonic 50.0% | 6.442% |
| Harmonic 62.5% | 6.509% |
| Harmonic 75.0% | 6.523% |
| Harmonic 87.5% | 6.600% |
| Harmonic 100.0% | 6.622% |
| Timbre 0.0% | 6.430% |
| Timbre 12.5% | 6.471% |
| Timbre 25.0% | 6.471% |
| Timbre 37.5% | 6.458% |
| Timbre 50.0% | 6.458% |
| Timbre 62.5% | 6.471% |
| Timbre 75.0% | 6.485% |
| Timbre 87.5% | 6.612% |
| Timbre 100.0% | 6.992% |
| Morph 0.0% | 6.470% |
| Morph 12.5% | 6.462% |
| Morph 25.0% | 6.456% |
| Morph 37.5% | 6.447% |
| Morph 50.0% | 6.365% |
| Morph 62.5% | 6.453% |
| Morph 75.0% | 6.461% |
| Morph 87.5% | 6.487% |
| Morph 100.0% | 6.459% |
| Output 0.0% | 6.448% |
| Output 12.5% | 6.445% |
| Output 25.0% | 6.458% |
| Output 37.5% | 6.447% |
| Output 50.0% | 9.990% |
| Output 62.5% | 6.589% |
| Output 75.0% | 6.601% |
| Output 87.5% | 6.603% |
| Output 100.0% | 6.587% |
| combined (as read from the reports): Harmonic 100.0%, Timbre 100.0%, Morph 87.5%, Output 50.0% | 6.369% |

Recommended: at most 3.5%. **Warning: 9.990% of the block budget at Output 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
