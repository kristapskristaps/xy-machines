# Atlas

A wavetable atlas with three-dimensional scanning and a crushed auxiliary.

Originally extracted from Weave’s `waves` model at revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It now renders directly at 44.1 kHz with linear reads of the integrated wavetable. The source wave atlas, coordinate scanning, differentiator and reconstruction low-pass remain. Removing Hermite reads and the extra sample-rate filter intentionally changes fine high-frequency detail. The saved machine ID and stock envelope behavior remain unchanged. The retired combined Weave source remains in Git history.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | The Z/bank coordinate of the wavetable atlas. |
| Timbre | The X coordinate. |
| Morph | The Y coordinate. |
| Lo fi | Crossfade toward the reduced-resolution auxiliary. |

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test atlas --native` or `./xy preview atlas --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

The fourth control is **Lo fi**: Crossfade from the main wavetable output to its reduced-resolution alternate. It is not volume. The inherited selector is main-only below about 43%, blends around 43–57%, and is alternate-only above about 57%. Compare the two endpoints to hear the difference.

## Factory presets

Four native OP-XY presets ship with this machine: **Init**, **Glass map**, **Low islands**, **Pixel edge**. They use the same engine settings as the browser patches. `default.json` is Init; other presets have their own files. Stock AMP/filter/LFO settings start from the SDK’s public preset template.
