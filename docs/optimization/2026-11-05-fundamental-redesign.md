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

## Local X0X lookup follow-up (not installed or measured)

The remaining kit-wide gap is targeted with a 2048-point nearest/rounded tonal
sine table: one lookup replaces two table reads plus interpolation per partial.
A machine-local test compares 1,048,576 phases with the previous interpolated
sine: peak error must be at most 53 signed-Q15 counts and relative RMS below
0.001. This is a small intentional quantization change, not a pitch change.

PCM reads at exact sample positions skip their unused neighbor/interpolation.
Metallic stems also use the exact midpoint average for half-sample positions,
covering octave playback rates without changing their audio. 900,000 stem
reads check both fast paths and general fractional interpolation for exact
equality. Metallic branch gain calculations are hoisted out of the sample loop.

The follow-up passes all 14 X0X host tests, preview checks and Blackfin compilation.
Eight-voice capacity remains unchanged. Hardware CPU is pending; the installed
image and the published local historical report still measure X0X at 4.347%.
No source has been pushed and no further transfer has been performed.

## Listening feedback and local mix fixes (not installed)

The user accepted Atlas's sound and the main sounds of Tandem/Sift. Tandem's
Output was not a pleasant crossfade; Sift's Z mix sounded like a high-pitched
noise layer. Inspection found both inherited selectors put the whole blend
into raw values 55..72 (roughly 43..57% travel), with delayed rebuilding for
every mix change. Tandem also subtracted its alternate root while the main
root used the opposite polarity.

The revised engines prepare main and alternate endpoints separately. Mixing
uses the whole parameter range at runtime with a 64-frame ramp, without
rebuilding tables. Tandem aligns the alternate root's polarity. Sift's alternate
now combines a fundamental with a Hann-windowed lower formant (up to six times
the carrier, further limited for high notes) and moderate phase warping.
Main-only PCM remains bit-identical to the installed redesign in 37 scenes per
engine, including their initial attacks. Atlas is unchanged.

Ten host tests across Tandem/Sift and both Blackfin compilations pass. New
fixed-phase checks verify full-range linear blending, endpoint response, no
rebuilding from mix changes and no midpoint collapse at the default sound.
Across 81 Sift alternate patches, at least 76.1% of periodic-wave energy is in
the fundamental. These checks support the intended change but do not replace
listening. Before/after host sweeps are available for audition.

Private voice memory changes to 4,656 bytes for Tandem and 2,400 bytes for Sift;
saved IDs, parameter contracts and presets remain unchanged. More endpoint
reads are needed for a mixed sound, so the previous CPU figures are historical;
the fixes require fresh hardware measurement. The next local batch also contains
the unmeasured X0X lookup follow-up. No additional transfer or push has occurred.

## Sift Body revision (local, not installed)

The user still found the Z/formant alternate musically unclear and noisy.
The fourth control is now **Body**, keeping its saved parameter ID `output`.
It blends the accepted grain endpoint toward a clean played note and a
phase-locked sub octave, with quiet upper-octave and third-harmonic colour.
No formant warping remains in this endpoint. Harmonic adjusts note/sub
balance, Timbre adds octave colour and Morph adds a restrained third harmonic.
At 0% main-only PCM remains unchanged; 50% retains grain over the tonal body;
100% is the clean tonal voice. Existing preset files/values are not overwritten.

A separate half-rate phase accumulator tracks the sub octave. Its fractional
carry follows the root phase, so `(sub_phase << 1) == (phase & ~1)` holds
through pitch movement without losing odd increment bits. Wave preparation
uses the half-rate bandwidth for the body endpoint. Private voice size is
2,408 bytes.

All five Sift host tests, preview checks and Blackfin compilation pass.
The 81-patch spectrum check requires at least 90% note/sub energy, a present
played-note component, a present sub, and no significant bins beyond the
locked sub/note/octave/third family. Main PCM comparisons still cover 37
scenes. A short host phrase demonstrates clean Body, grain, then halfway.
Hardware CPU and listening acceptance of this revision remain pending.
The previous staged mix-fix image is superseded for the next installation;
prepare a fresh combined package from this revision after the sound review.
The XY remains on its earlier measured image, unchanged by these local edits.

## Public-SDK release route

The user chose a public-SDK release with one bundled default kit, keeping the
other authored kits separate rather than depending on SDK PR #422. The repo
now pins official `etcetc000/open-xy` main at
`71566d29f0087c793f392e9c09768ae3c866a379` (SDK 1.9), in a separate clean
checkout. No SDK source was edited. X0X uses the existing public drum API:
ordinary raw-value Style edits, selected-pad activity animation, and a valid
empty-region init preset backed by `drum_defaults`. Drum DSP, all 18 models,
47 type/style combinations, 24 pads and eight voices are unchanged.

All seven authored packed-pad kits are preserved in `preset-packs/x0x/`.
They regenerate byte-for-byte through the public codec, and the real public
pad editor/decoder checks every kit's model, controls and choke settings.
They are not builder-bundled. Existing presets on the XY remain untouched.
Separate-kit hardware import/save/reload still needs QA.

All 37 inventory machines pass public-SDK previews/host tests and Blackfin
compilation. X0X passes 13 C++ host tests, six browser audition tests, the real
pad-editor integration check, and all 47 sounds through the generated native
audio adapter. All 22 shared workflow/preset tests pass. The unused custom
edit callback and its obsolete one-detent C++ test were removed; public raw
stepping and saved-value boundaries are covered in the integration check.

The unchanged public SDK suite ran 424 Python tests with 18 private-fixture
skips and the same two pre-existing macOS failures (unused `kAuxTab` in the
drum-services fixture; Darwin master-filter extension registration linking).
No checks, hashes or release-host guards were bypassed. Machine checks pass
independently; the SDK suite is not claimed fully green on this Mac.

A fresh public-SDK combined image must be measured before release; older
custom-SDK reports do not qualify it. After CPU and listening acceptance,
consolidate open catalog PRs #348, #349, #350 and #354 with the new qualified
engines in one catalog-only PR against public main. Rebound #352 is already
merged and is excluded. SDK PRs remain separate; X0X no longer depends on #422.
