# Sift

Paired grainlet formants and a Z-oscillator auxiliary.

Originally extracted from Weave’s `grain` model at revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It now prepares a periodic formant blend in bounded 16-point tiles and reads that wave directly at 44.1 kHz. The paired formants and Z-style alternate retain their control roles, but the discontinuity correction is replaced by smooth windows and frequency-aware phase taps. This is an intentional tonal redesign, not an audio-identical extraction. The saved machine ID and stock envelope behavior remain unchanged. The retired combined Weave source remains in Git history.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | The relationship between the two formant frequencies and the auxiliary mode. |
| Timbre | Formant frequency. |
| Morph | Grainlet/formant shape and bleed. |
| Z mix | Crossfade between the paired grainlets and Z-oscillator auxiliary. |

Wave preparation takes about 23 ms, followed by a 5.8 ms crossfade. Moving controls finish the current bounded preparation before starting the latest target; playback never returns to the old expensive renderer. Pitch changes advance the oscillator every sample. Newly initialized voices begin with a quiet simple waveform while the first requested wave is prepared. These attack and modulation differences need a listening pass before release.

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test sift --native` or `./xy preview sift --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

The fourth control is **Z mix**: Crossfade from two grainlet formants to the Z-oscillator alternate. It is not volume. The inherited selector is main-only below about 43%, blends around 43–57%, and is alternate-only above about 57%. Compare the two endpoints to hear the difference.

## Factory presets

Four native OP-XY presets ship with this machine: **Init**, **Grain voice**, **Small vowels**, **Zed**. They use the same engine settings as the browser patches. `default.json` is Init; other presets have their own files. Stock AMP/filter/LFO settings start from the SDK’s public preset template.
