# vader: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 67 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.883%** of the stock 580,352-cycle block budget. Midpoint run: 2.963%. Every control at its heaviest position: 2.277%. Heaviest single setting: Drive 37.5%.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 2.963% | 4.167% |
| Punch 0.0% | 2.306% | 4.633% |
| Punch 12.5% | 2.287% | 2.635% |
| Punch 25.0% | 2.310% | 2.757% |
| Punch 37.5% | 2.289% | 2.803% |
| Punch 50.0% | 2.310% | 2.864% |
| Punch 62.5% | 2.290% | 2.860% |
| Punch 75.0% | 2.312% | 2.863% |
| Punch 87.5% | 2.292% | 2.823% |
| Punch 100.0% | 2.312% | 2.809% |
| Fall 0.0% | 2.275% | 2.585% |
| Fall 12.5% | 2.300% | 2.696% |
| Fall 25.0% | 2.284% | 2.664% |
| Fall 37.5% | 2.315% | 2.728% |
| Fall 50.0% | 2.306% | 2.640% |
| Fall 62.5% | 2.348% | 2.716% |
| Fall 75.0% | 2.361% | 2.678% |
| Fall 87.5% | 2.434% | 2.695% |
| Fall 100.0% | 2.495% | 2.660% |
| Drive 0.0% | 2.307% | 2.681% |
| Drive 12.5% | 3.420% | 4.923% |
| Drive 25.0% | 3.801% | 4.681% |
| Drive 37.5% | 3.883% | 5.531% |
| Drive 50.0% | 3.661% | 4.535% |
| Drive 62.5% | 3.777% | 5.304% |
| Drive 75.0% | 3.865% | 4.593% |
| Drive 87.5% | 3.719% | 5.409% |
| Drive 100.0% | 3.757% | 4.588% |
| Shape round | 2.289% | 5.470% |
| Shape tube | 2.309% | 2.683% |
| Shape diode | 2.306% | 2.608% |
| Shape fold | 2.307% | 2.663% |
| Click 0.0% | 2.287% | 2.600% |
| Click 12.5% | 2.310% | 3.111% |
| Click 25.0% | 2.290% | 3.033% |
| Click 37.5% | 2.312% | 3.070% |
| Click 50.0% | 2.290% | 3.041% |
| Click 62.5% | 2.310% | 3.064% |
| Click 75.0% | 2.290% | 3.023% |
| Click 87.5% | 2.313% | 3.090% |
| Click 100.0% | 2.290% | 3.050% |
| Sub 0.0% | 2.309% | 2.689% |
| Sub 12.5% | 2.287% | 2.667% |
| Sub 25.0% | 2.309% | 2.673% |
| Sub 37.5% | 2.287% | 2.605% |
| Sub 50.0% | 2.309% | 2.667% |
| Sub 62.5% | 2.289% | 2.644% |
| Sub 75.0% | 2.307% | 2.696% |
| Sub 87.5% | 2.287% | 2.650% |
| Sub 100.0% | 2.309% | 2.679% |
| Tone 0.0% | 2.267% | 2.635% |
| Tone 12.5% | 2.289% | 2.696% |
| Tone 25.0% | 2.269% | 2.618% |
| Tone 37.5% | 2.313% | 2.707% |
| Tone 50.0% | 2.290% | 2.635% |
| Tone 62.5% | 2.289% | 2.699% |
| Tone 75.0% | 2.269% | 2.644% |
| Tone 87.5% | 2.289% | 2.710% |
| Tone 100.0% | 2.287% | 2.622% |
| Boost 0.0% | 2.327% | 2.705% |
| Boost 12.5% | 2.315% | 2.652% |
| Boost 25.0% | 2.321% | 2.707% |
| Boost 37.5% | 2.310% | 2.684% |
| Boost 50.0% | 2.335% | 2.710% |
| Boost 62.5% | 2.287% | 2.654% |
| Boost 75.0% | 2.307% | 2.698% |
| Boost 87.5% | 2.287% | 2.631% |
| Boost 100.0% | 2.309% | 2.678% |
| combined (as read from the reports): Punch 75.0%, Fall 100.0%, Drive 37.5%, Shape tube, Click 87.5%, Sub 0.0%, Tone 37.5%, Boost 50.0% | 2.277% | 2.461% |

Worst observed block: **5.531%** (Drive 37.5%), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Mean 3.883% of the block budget at Drive 37.5% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md); Worst observed block peak 5.531% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
