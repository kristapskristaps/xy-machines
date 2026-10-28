# Vapour

A Clouds-derived stereo granular send for OP-XY OS 1.2.1. It records incoming
audio, overlaps up to 16 grains, transposes them, freezes the recording buffer,
and adds diffusion. It uses the [Open-XY SDK](https://github.com/etcetc000/open-xy).

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

Density is bounded to sixteen overlapping grains (twelve linear and four cubic
readers) to reduce peak CPU cost. The diffusion network runs only in the upper
quarter of Texture; enabling it starts fresh delay history. All eight controls
keep their order and mapping. The lower overlap limit changes the densest cloud
textures compared with the earlier thirty-two-grain build.

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
python ../open-xy/sdk.py test --machine .
```

The SDK currently needs a synth in a native collection. Compile Vapour alongside
an instrument, for example a sibling Weave repository:

```sh
python ../open-xy/sdk.py compile --machine . --machine ../open-xy-plaits \
  --out "$HOME/Library/Application Support/OPXY-SDK/private/vapour-module-001"
```

CI uses the SDK's 2op FM worked example for that composition. Send effects do
not currently have a browser audition module; render tests exercise their real
callback with stereo input, dirty scratch, Freeze, resets and missing buffers.

Hardware timing and normal multi-track playback remain pending. Follow the
SDK's [profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
before catalog submission. All firmware builds stay outside every checkout.

MIT; original DSP by Émilie Gillet. See [vendor/LICENSE](vendor/LICENSE).
