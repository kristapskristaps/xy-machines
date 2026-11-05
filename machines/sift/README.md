# Sift

Paired grainlet formants with a rounded note/sub-octave body blend.

Originally extracted from Weave’s `grain` model at revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It now prepares a periodic formant blend in bounded 16-point tiles and reads that wave directly at 44.1 kHz. The paired formants retain their main sound. Body blends toward a clean played note and phase-locked sub octave, with restrained octave and third-harmonic colour. It contains no formant-warping alternate. This is an intentional tonal redesign, not an audio-identical extraction. The saved machine ID and stock envelope behavior remain unchanged. The retired combined Weave source remains in Git history.

The first three controls shape the grain model; Body replaces the former Output/Z mix alternate. Its saved parameter ID remains `output`. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | The main formant relationship; note/sub balance in Body. |
| Timbre | Main formant frequency; upper-octave colour in Body. |
| Morph | Grainlet shape and bleed; a gentle third harmonic in Body. |
| Body | Full-range blend from grain texture toward a rounded played note and sub octave. |

Both endpoints are prepared together in about 23 ms, followed by a 5.8 ms timbre crossfade. Harmonic, Timbre, Morph and pitch changes finish the current bounded preparation before starting the latest target. Body blends the ready endpoints directly, ramps over one 64-frame block and never rebuilds the waves. Playback never returns to the old expensive renderer. Pitch changes advance the oscillator every sample. Newly initialized voices begin with a quiet simple waveform while the first requested wave is prepared. These attack and modulation differences need a listening pass before release.

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test sift --native` or `./xy preview sift --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

**Body** has a simple musical role: add low weight and soften the grain sound. At 0% the accepted grain endpoint is unchanged; halfway retains grain texture over the rounded note/sub voice; at 100% only that tonal voice remains. The whole travel is active and smoothed. It is a blend, not a volume control. The sub stays exactly one octave below the played note through pitch bends and glides. Quiet upper-octave and third-harmonic colour keep the clean endpoint useful without the former high-formant layer.

The existing saved control ID and preset values remain compatible; the factory preset name **Zed** is retained even though its alternate is now Body.

## Factory presets

Four native OP-XY presets ship with this machine: **Init**, **Grain voice**, **Small vowels**, **Zed**. They use the same engine settings as the browser patches. `default.json` is Init; other presets have their own files. Stock AMP/filter/LFO settings start from the SDK’s public preset template.
