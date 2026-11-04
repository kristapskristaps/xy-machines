# vader: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 67 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.883%** of the stock 580,352-cycle block budget. Midpoint run: 2.963%. Every control at its heaviest position: 2.277%. Heaviest single setting: Drive 37.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.963% |
| Punch 0.0% | 2.306% |
| Punch 12.5% | 2.287% |
| Punch 25.0% | 2.310% |
| Punch 37.5% | 2.289% |
| Punch 50.0% | 2.310% |
| Punch 62.5% | 2.290% |
| Punch 75.0% | 2.312% |
| Punch 87.5% | 2.292% |
| Punch 100.0% | 2.312% |
| Fall 0.0% | 2.275% |
| Fall 12.5% | 2.300% |
| Fall 25.0% | 2.284% |
| Fall 37.5% | 2.315% |
| Fall 50.0% | 2.306% |
| Fall 62.5% | 2.348% |
| Fall 75.0% | 2.361% |
| Fall 87.5% | 2.434% |
| Fall 100.0% | 2.495% |
| Drive 0.0% | 2.307% |
| Drive 12.5% | 3.420% |
| Drive 25.0% | 3.801% |
| Drive 37.5% | 3.883% |
| Drive 50.0% | 3.661% |
| Drive 62.5% | 3.777% |
| Drive 75.0% | 3.865% |
| Drive 87.5% | 3.719% |
| Drive 100.0% | 3.757% |
| Shape round | 2.289% |
| Shape tube | 2.309% |
| Shape diode | 2.306% |
| Shape fold | 2.307% |
| Click 0.0% | 2.287% |
| Click 12.5% | 2.310% |
| Click 25.0% | 2.290% |
| Click 37.5% | 2.312% |
| Click 50.0% | 2.290% |
| Click 62.5% | 2.310% |
| Click 75.0% | 2.290% |
| Click 87.5% | 2.313% |
| Click 100.0% | 2.290% |
| Sub 0.0% | 2.309% |
| Sub 12.5% | 2.287% |
| Sub 25.0% | 2.309% |
| Sub 37.5% | 2.287% |
| Sub 50.0% | 2.309% |
| Sub 62.5% | 2.289% |
| Sub 75.0% | 2.307% |
| Sub 87.5% | 2.287% |
| Sub 100.0% | 2.309% |
| Tone 0.0% | 2.267% |
| Tone 12.5% | 2.289% |
| Tone 25.0% | 2.269% |
| Tone 37.5% | 2.313% |
| Tone 50.0% | 2.290% |
| Tone 62.5% | 2.289% |
| Tone 75.0% | 2.269% |
| Tone 87.5% | 2.289% |
| Tone 100.0% | 2.287% |
| Boost 0.0% | 2.327% |
| Boost 12.5% | 2.315% |
| Boost 25.0% | 2.321% |
| Boost 37.5% | 2.310% |
| Boost 50.0% | 2.335% |
| Boost 62.5% | 2.287% |
| Boost 75.0% | 2.307% |
| Boost 87.5% | 2.287% |
| Boost 100.0% | 2.309% |
| combined (as read from the reports): Punch 75.0%, Fall 100.0%, Drive 37.5%, Shape tube, Click 87.5%, Sub 0.0%, Tone 37.5%, Boost 50.0% | 2.277% |

Recommended: at most 3.5%. **Warning: 3.883% of the block budget at Drive 37.5% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
