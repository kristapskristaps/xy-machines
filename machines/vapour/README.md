# Vapour

A Clouds-derived stereo granular send for OP-XY OS 1.2.1. It records incoming
audio, overlaps up to 8 grains, transposes them, freezes the recording buffer,
and adds diffusion. It uses the [Open-XY SDK](https://github.com/etcetc000/open-xy).

The initial sound emphasizes the granular return: lower dry level,
a longer window and upward transposition. The default density keeps its previous
overlap demand; the effect does not add voices. For a frozen cloud, play input first,
then enable Freeze and move Position/Pitch. Density has a silent zone near 50%.

## Controls

| Control | Behavior |
| --- | --- |
| Position | How far back in the rolling recording to take grains |
| Size | Grain duration, approximately 32–512 ms |
| Pitch | Continuous transposition, −24 to +24 semitones |
| Density | Regular grains below center, a silent zone at center, randomized grains above center |
| Texture | Rectangular through triangular/Hann windows; the upper quarter adds diffusion |
| Feedback | Returns the granular signal to the recording, up to 95% |
| Freeze | Stops recording while grains continue playing |
| Dry | Source dry level through the stock send mixer; the effect itself returns wet audio only |

On the native send page, press an encoder to alternate its control: Position /
Texture, Size / Feedback, Pitch / Freeze, and Density / Dry.

The rolling recording holds 65,536 stereo 16-bit frames, about 1.49 seconds.
Stereo spread is fixed at 75%. The send borrows 282,624 bytes of scratch memory;
each send has independent state. Reset and effect changes cannot expose old
scratch audio. Recording resumes through a short crossfade after Freeze.

Density is bounded to eight overlapping grains (six linear and two cubic
readers) to reduce peak CPU cost. Grain duration, pitch, position and the
44.1 kHz capture rate are preserved. A shared stereo pan calculation removes
two gain multiplies per grain sample; complementary gains change by at most
two 16-bit sample units. The diffusion network runs only in the upper
quarter of Texture; enabling it starts fresh delay history. All eight controls
keep their order and mapping. The lower overlap limit changes the densest cloud
textures compared with the original Clouds grain count.

This ports Clouds' granular mode. Time stretch, looping-delay and spectral
modes, reverb, audio-quality modes and saved frozen buffers are not implemented.
The current SDK supplies the stock FX page for send effects, rather than a
custom animated display. Vapour has its own grain-field machine glyph.

## DSP and verification

[Provenance](vendor/VENDOR.md) records the original algorithms and the fixed-point
adaptation. Scheduling, grain windows, stereo placement and diffusion follow
Clouds; interpolation and envelope tests compare against independent
floating-point formulas. There is no floating-point arithmetic in audio callbacks.

```sh
./xy test vapour --native --cpu
```

Run these commands from the collection root with its configured pinned SDK.
The native check includes a synth because the SDK startup check needs one.
The sound tests cover stereo grains, dirty scratch, Freeze, reset and missing
buffers. The arithmetic test exhausts the reachable normalization-difference
range and checks diffusion gain against an independent wide reference; the
existing grain test compares interpolation and envelopes with floating-point
formulas. Run all `tests/*.cpp` with AddressSanitizer and
UndefinedBehaviorSanitizer for additional host checks.

The optimized callback uses an exact reciprocal for normalization's division
by 100 and a bounded multiply/shift for the allpass gain. Diffusion is inlined
for the two stereo channels. The native sample loops no longer call the
normalization divider or diffusion function. The Mando/Vapour native check adds
136 bytes to the module and reduces Vapour's reported audio stack from 748 to
716 bytes. State, scratch allocation and the adapted vendor sources are unchanged.

Compare complete stereo output, state and all scratch words after every block,
then collect paired host timings:

```sh
python machines/vapour/tools/benchmark.py --baseline 75fcda3 --sdk /path/to/open-xy
python machines/vapour/tools/benchmark.py --baseline 75fcda3 --sdk /path/to/open-xy \
  --verify-only --full-grid
```

The default comparison covers 259 scenarios; the full grid covers 1,027, each
for 256 blocks. Cases include eight primed grains, cold/stale scratch,
wraparound, reset, missing/short buffers, Freeze transitions, full-scale stereo
input and automation through Density/Texture boundaries. Timing uses primed
grains, 512 warmup blocks and five alternating pairs. Against `75fcda3`, the
tested macOS compilers showed roughly flat overall medians and about 20% lower
host time in the primed maximum-Texture case. Device CPU remains unmeasured.

Send effects have no SDK browser audition. The host renderer runs the actual
callback and adds a quiet stereo test phrase:

```sh
mkdir -p /tmp/vapour-audition
c++ -std=c++11 -O2 -I/path/to/open-xy/sdk/include \
  machines/vapour/tools/audition.cpp -o /tmp/vapour-audition/render
/tmp/vapour-audition/render cloud /tmp/vapour-audition/cloud.wav
/tmp/vapour-audition/render freeze /tmp/vapour-audition/freeze.wav
```

`cloud` plays a phrase into dense grains with diffusion. `freeze` holds the
recording at three seconds, transposes it down an octave at six seconds, then
resumes recording at nine seconds with a new phrase. These recordings exercise
host DSP; they do not emulate the stock mixer or establish hardware acceptance.

Hardware timing and normal multi-track playback remain pending. Follow the
SDK's [profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
before catalog submission. All firmware builds stay outside every checkout.

MIT; original DSP by Émilie Gillet. See [vendor/LICENSE](vendor/LICENSE).
