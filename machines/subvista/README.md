# Subvista

An eight-voice subtractive synth prototype for the Open-XY SDK, ID `subvista`.
Machine code and tests live at this repository's root.

Two slightly detuned, band-limited wavetable oscillators blend saw and pulse,
with a quiet sine sub-oscillator, into a resonant two-pole low-pass filter.
A per-note contour opens the cutoff and decays over roughly 300 ms. The SDK
browser supplies its audition amplitude gate; native stock envelopes follow
this oscillator on the device.

| Control | Behavior |
| --- | --- |
| Cutoff | Low-pass cutoff, exponential sweep from 60 Hz to 15 kHz |
| Reso | Filter resonance, damped through a pronounced peak |
| Shape | Saw to pulse blend |
| Contour | Amount of the per-note downward filter sweep |

The display animates a signal-flow schematic: raw waveform, filter aperture,
filtered waveform and cutoff curve. These drawings describe the controls;
they are not measured audio scopes. The workbench's Waveform tab is the live
output scope. Reduced motion stops continuous movement.

Five starting presets: Soft Current, Glass Teeth, Hollow Reed, Acid Bloom,
and Open Circuit. Click Enable audio, then Play phrase or use
`A W S E D F T G Y H U J K`; Escape stops all notes.

## Current validation

The SDK machine checks, WebAssembly preview, and host sound/display tests pass.
Host tests cover filter attenuation, 27 control combinations, pitch extremes,
output bounds, deterministic retriggering, and 800 display frames including
reduced motion. The host tests also pass AddressSanitizer and UBSan.

The host CPU estimate is about 27,000 Blackfin instructions per voice block,
above the SDK's 15,000 target. This is a browser prototype: optimize and measure
on hardware before considering device polyphony or publication. No native
firmware build or hardware validation has been performed.

## This Mac's preview setup

The existing Apple Clang has wasm32 support. A local copy of the Homebrew LLD
linker and its runtime libraries is in the SDK's ignored `local/wasm-tools/`;
the full LLVM installation failed due to low disk space. The SDK itself is
unchanged. From `../open-xy`:

```sh
export PATH="$PWD/local/wasm-tools/lld/23.1.4/bin:$PATH"
PYTHON=/Users/tapsy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3
"$PYTHON" sdk.py test --machine ../subvista --cxx /usr/bin/clang++
"$PYTHON" sdk.py preview --machine ../subvista --cxx /usr/bin/clang++ --out local/subvista-preview-002
"$PYTHON" tools/serve_harness.py --preview local/subvista-preview-002
```

Use a fresh output directory on each rebuild. The current served snapshot is
`local/subvista-preview-001` at http://127.0.0.1:8917/.


## Develop

Keep the [SDK](https://github.com/etcetc000/open-xy) beside this repository as
`../open-xy`, on `staging`; adjust the commands if yours is elsewhere. The
[first-machine guide](https://github.com/etcetc000/open-xy/blob/staging/docs/first-machine.md)
covers prerequisites. Read the
[core machines](https://github.com/etcetc000/open-xy-core-machines) as worked
examples and the [machine API](https://github.com/etcetc000/open-xy/blob/staging/docs/api.md)
for the supported interfaces. Agent instructions are in [AGENTS.md](AGENTS.md).

Build and audition the synth in the browser:

```sh
python ../open-xy/sdk.py preview --machine . --out ../previews/subvista-001
python ../open-xy/tools/serve_harness.py --preview ../previews/subvista-001
```

```sh
python ../open-xy/sdk.py test --machine . --cxx clang++ --host-cxx g++
```

Extend `tests/` as the machine's behavior grows. This is a prototype, not a
hardware-qualified instrument. Keep generated output outside this
repository. The generated workflow checks source, sound tests and synth previews
on Linux, and compiles the native module on Windows; it never uses a device.
Its `OPEN_XY_REF` starts at `staging`; pin a compatible SDK commit for a release.

## Hardware and publication

Follow the SDK's [firmware build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md)
and [hardware profiling workflow](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
Keep firmware and stock content outside every checkout. Commit the machine,
build with profiling enabled, install when authorized and collect three runs.
The host estimate printed by `test` cannot substitute for those measurements.

For the builder, follow [catalog submission](https://github.com/etcetc000/open-xy/blob/staging/docs/catalog.md#submit-a-machine).
Every new or changed draft/published entry needs a hardware CPU record tied to
its pinned commit. Hardware validation may be performed by a maintainer if you
do not own an XY. Publication remains pending until that evidence is available.

The generated starter is covered by [LICENSE](LICENSE). Preserve notices for
code you reuse and include the licences of any dependencies or ports.
