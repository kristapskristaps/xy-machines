# Local fundamental redesign candidates

These candidates follow the failed combined measurement in
[the previous batch](2026-11-05-four-engine-batch.md). The redesigned seven-machine image is now installed and measured, but not published.
Atlas, Tandem and Sift pass the CPU target; listening acceptance remains pending.
X0X still fails the under-4% CPU target.
Saved machine IDs, controls, X0X's eight-voice capacity and all 43 presets are preserved.

| Engine | Last measured hardware maximum | Candidate |
| --- | ---: | --- |
| Tandem | 10.330% | Replace four running oscillators and SRC with two preblended, independently tuned periodic components. Preserve continuous detuning and original parameter maps. |
| Sift | 8.214% | Replace two grainlets, Z discontinuity processing and SRC with a prepared formant period; smooth windows intentionally alter the Z-style alternate. |
| Atlas | 5.090% | Render the integrated wave atlas directly at 44.1 kHz; replace Hermite with linear interpolation while retaining the differentiator/low-pass reconstruction. |
| X0X | 5.521% | Update only used phase clocks per sample and compensate unused clocks once per block; replace cubic Drive arithmetic with a 257-point curve. |

## Sound and real-time costs

Tandem/Sift changes intentionally trade exact Plaits reproduction for cheaper synthesis.
Their preparation does not record a held note or freeze the current audio. Every callback
builds at most 16 phase points per component, even under continuous automation. A complete
256-point period takes 16 callbacks (23.2 ms), then crossfades over 256 frames (5.8 ms).
Control motion finishes the current snapshot and then prepares the latest values; intermediate
snapshots may be skipped. Pitch playback remains sample-accurate, but pitch-dependent timbre
preparation follows the same bounded schedule. Initial voices use a quiet simple wave while
preparing the first requested timbre. This attack change requires explicit listening QA.
Sift limits the formant/table bandwidth and replaces the Z algorithm's discontinuity behavior;
it is a more substantial tonal change than rounding precision.

Atlas retains the integrated source tables because capturing its final audio as one repeated
period damaged the high register in a rejected experiment. Direct-rate playback removes that
experiment's phase/drift problem and keeps the reconstruction filter. Its pitch targets update
at 32-frame boundaries, as in the previous source-block adapter.

X0X clock compensation is bit-identical to updating all six clocks per sample, including live
type changes. The existing 47-style frozen comparison checks this. Drive alone differs by at
most four signed-Q15 counts across all 65,535 input values and 128 control settings; its
integer curve generator and exhaustive check are machine-local. X0X's new hardware margin
is the least certain of the four; no under-4% claim is made from arithmetic counts.

## Local checks and next hardware batch

Host/preview and Blackfin compilation checks run for each candidate. Frozen original PCM
fixtures remain unchanged as references. New renderer checks cover output bounds, control
response, automation, glides, retriggers and bounded preparation; they deliberately do not
claim identical legacy PCM for a redesigned engine. Host timing is advisory and cannot fill
catalog CPU reports. Before/after WAVs include the note attacks and three registers per patch.

A combined seven-machine profiling build must preserve the installed layout and all presets.
Measure all four in one capture, also listen to the redesigned attacks, rapid notes, glides,
control/LFO motion, high registers, Sift's Z mix and Tandem's sync. Keep this branch local
until measured CPU is below 4% and listening/boot checks pass. Ideally reach 3.5% or less.

## Batch verification record

Source commit: `b6533da`, local branch `codex/fundamental-engine-redesign-20261105`.
All four machine checks and Blackfin compilations passed: 24 machine-local host tests
and 21 shared workflow tests. The combined seven-machine image rebuilds identically and
allocates 1,931,444 of 2,080,768 bytes. All 42 layout reservations match the installed image.
Staging contains nine files, only the firmware differs, and presets/projects are excluded.
Transfer and hardware results are recorded below. Nothing was pushed.

The unchanged SDK's 438-test public suite still has two existing macOS failures
(`test_pad_edit_alternates_shift_and_model_changes`, unused `kAuxTab`;
`test_registered_processors_have_independent_bindings_and_can_be_omitted`, unresolved
master-filter extension registration symbols), with 19 private-fixture skips. The suite
was rerun with a canonical temporary-directory path to eliminate unrelated symlink errors.
The SDK was not edited to hide or bypass these failures. These are separate from the
passing machine/native checks and the verified combined firmware reconstruction.

## Completed combined hardware capture

All nine transfer files passed device readback; only `/os.ldr` changed. The user
confirmed the physical OS write, normal boot, stopped playback and USB audio off.
The single capture completed all 332 cases, 334 frames, with no protocol errors.
Each report binds unchanged machine source to commit `d07d8c9677b692ca044b01ed8d9018c05e9064a8`.

| Machine | Previous hardware maximum | New hardware maximum | Heaviest setting |
| --- | ---: | ---: | --- |
| Atlas | 5.090% | **2.922%** | Lo fi 62.5% |
| Tandem | 10.330% | **2.727%** | Harmonic 87.5% |
| Sift | 8.214% | **1.753%** | Harmonic 37.5% |
| X0X | 5.521% | **4.347%** | Kick/model 0, all controls 127 |
| Bitflip (unchanged reference) | 3.778% | 3.690% | Model FM |
| Choir (unchanged reference) | 3.264% | 3.450% | Colour 50% |
| Crease (unchanged reference) | 3.554% | 3.581% | Alt mix 50% |

[Generated reports and catalog fields](../profiling/2026-11-05/fundamental-redesign/).
These are synthetic workload means, not total project CPU, every possible modulation
scenario or maximum usable polyphony. Earlier and current images have different layouts;
small differences in unchanged references are not optimization claims. The large reductions
in the redesigned synths clear both requested CPU thresholds in this capture.

X0X's snare midpoint is 4.276%, open-hat maximum 4.237%, cymbal maximum 4.259%
and ride maximum 4.219%; optimizing only the kick cannot qualify the kit. Its
whole-kit workload changed from 26.952% to 20.505%; the heaviest-voice eight-voice
workload changed from 42.138% to 33.199%. Eight-voice capacity is unchanged.

After capture, one software reboot produced an observed USB disconnect/reconnect.
A normal-mode OS 1.2.1 probe after the boot wait completed without errors. Diagnostic
settings were not saved. All seven remain installed, presets/projects untouched.
The branch stays local while X0X optimization and redesigned-sound listening QA continue.
