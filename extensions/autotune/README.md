# Auto Tune for OP-XY

Auto Tune is an [Open-XY](https://github.com/etcetc000/open-xy) extension for OS
1.2.1. It moves one sung or played note at a time toward the nearest note in
the selected key and scale at A440. It tracks roughly 70–900 Hz and preserves
both channels of the 3.5 mm input. Unpitched sounds pass through with the same
15.2 ms base delay.

## Try it

1. Use a firmware build that includes Auto Tune.
2. Open **Aux → 5 (External Audio) → M1**.
3. Turn the dark encoder to **mic**, **headset**, or **audio input**, then click
   it to enable monitoring. Set **Drive** and **Level** for the source.
4. Hold **Shift** on M1. The four encoders read **Tilt Mic**, **Tune**,
   **Retune**, and **Amount** when Tilt Mic is also installed.
5. Set **Tune** to your beat's major or natural minor key. **Auto** follows
   the current Brain scale and uses Chromatic when Brain has no valid scale.
6. For an obvious correction, select **Hard** and **100%**, then slide between
   notes. Turn Tune to **Off** or Amount to **0%** to hear the direct input.

Tune starts at Off, Retune at Smooth and Amount at 100%. Choices last until
reboot. Hard scans for pitch faster and accepts a wider range of voiced
input. The Amount control sets how far the pitch moves toward its target.

Correction covers what Aux 5 sends to the mix and FX. The sampler's raw mic
and line inputs use their own path.

## Build and check

Use an Open-XY SDK checkout that supports live input and the paired Q15 distance
helper. From the XY Machines root, run the extension checks.

```sh
python /path/to/open-xy/sdk.py test --extension extensions/autotune
python /path/to/open-xy/sdk.py build --extension extensions/autotune --machine machines/tandem \
  --stock-os /path/to/stock-1.2.1/os.ldr --out /path/to/private/autotune-dev
```

Use your machine collection and its existing `--layout` when building for
your projects. Store stock and generated firmware outside source checkouts.
See [QA.md](QA.md) for the physical device check and [hardware measurements](CPU.md)
for the measured DSP workload and firmware identity.

The tracker normalizes the decimated input before the [YIN difference search](https://pubmed.ncbi.nlm.nih.gov/12002874/)
to preserve sensitivity at the Aux mixer's signal levels.
Two interpolated delay taps one detected period apart shift both channels
together. Fixed-point DSP and audio memory are preallocated.
