# Acidline

A mono acid voice with a four-stage 303 ladder, band-limited saw/triangle/square,
linked drive and clipping, an octave sub and darkening. The monochrome display
shows a twisting fluid membrane, suspended bead clusters and small bubbles.

## Playing and slide

All six native factory presets select stock **legato** and stock portamento amount
**1800** (a raw preset value, not milliseconds). Existing projects retain their
saved play mode and portamento; load a new preset or edit M2 + Shift to use these
settings. Save edited sounds before loading a factory preset.

Overlap notes for connected phrases; leave gaps for fresh attacks. XY also has a
Portamento step component for selective pitch glides. Acidline consumes the stock
sample-by-sample pitch ramp exactly and never adds a second glide. A pitch change
without a new start preserves its internal filter-envelope decay; a new start
retriggers it. Tests establish this machine-side behavior, **not** whether stock
Legato supplies the necessary callbacks for every sequenced overlap. That needs
an on-device listening test. This is a stock-legato slide candidate, not a claim
of complete TB-303 sequencer/circuit emulation. The public synth API has no
explicit slide/tie/gate flag; if stock restarts tied voices, proper suppression
needs a separate SDK API change rather than a pitch-change heuristic.

## Performance controls

Press any encoder to switch pages. Saved indices and parameter IDs are preserved.

| Encoder | Main page | Second page |
| --- | --- | --- |
| 1 | **Flow**: opens the filter across its logarithmic range | **Length**: short pluck to long filter decay |
| 2 | **Bite**: resonance, bass compensation and extra envelope sweep | **Shape**: Saw / Triangle / Square |
| 3 | **Burn**: pre-filter drive, soft clipping and linked darkening | **Body**: octave-down sine blend |
| 4 | **Sweep**: filter movement within the headroom above Flow | **Shade**: bright edge to dark low-pass |

Bite adds up to one quarter of the unused envelope depth. Burn adds up to one
quarter of the remaining darkening range. These couplings are computed once per
block. Length still controls the contour independently. The main page is the
performance surface; the second page shapes the underlying voice. These are real
sound changes, including for existing projects. Control slots have not been
removed or reordered, and factory raw sound values remain intact.

Each track retains its own stock mono/legato behavior. The 16-slot DSP pool
supports independent tracks and stock fades; it is not per-track polyphony.
Stock supplies amplitude envelopes, velocity, mixing and effects. Filter accents
can use parameter locks on Flow, Bite, Burn or Sweep.

## Display

A monochrome flow cell shows the signal passing through an inlet, a valve, a
heated reservoir and an outlet. The case and channel are fixed artwork.

| Sound control | Animation |
| --- | --- |
| Flow | Inlet position along the channel |
| Bite | Gap between the two valve jaws |
| Burn | Size of the heating element |
| Sweep | Droplet position and note excursion |
| Length | Inlet size and time for the droplet to settle |
| Shape | Saw, triangle or square symbol beneath the valve |
| Body | Reservoir size |
| Shade | Angle and darkness of the outlet baffle |

Each control changes the drawing on the next frame, even at rest. A note lifts
the droplet and it settles as the attack fades. Reduced motion keeps it on the
channel. Both control pages remain visible through the shared header. The
stage has ten bound nodes, and the note animation moves one.

## DSP budget and validation

This candidate uses the experimental 44.1 kHz Open303 ladder adaptation and its
150 Hz feedback high-pass, replacing the old 88.2 kHz TPT filter/FIR. It is an
intentional sound change: a darker ladder response, different resonance and
more high-note distortion aliasing. See [NOTICE.md](NOTICE.md) for attribution.
Stable output controls select specialized loops before processing samples.
There is no output lookup-cache rebuild on automation and no new slide DSP loop.

The previous catalog revision measured **3.975%** average at Shape Square
and a **4.778%** peak. The new screen needs a device responsiveness check and
a revision-bound hardware run before a catalog release. The callback timing
excludes the display.

From the XY Machines root:

```sh
./xy test acidline --native --cpu
node --test machines/acidline/tests/presets.test.mjs
./xy preview acidline --watch
```

Tests cover control extremes, headroom, independent voices, spectral behavior,
control sweet spots, exact supplied pitch ramps, tied contour continuity,
detached retriggers, display validity, idle/reduced motion and both pages.
Frozen historical renderer hashes remain unchanged. Sound comparisons apply the
same macro targets to the old filter before comparing level and spectral edge;
no old reference hashes were replaced to admit the new design.

The browser has a simple audition gate and does not reproduce stock Legato,
portamento, allocation, envelopes or device CPU. Use the dedicated acid audition
for its simulated tie/glide pattern; it too is test equipment, not stock firmware.

MIT; see LICENSE and NOTICE.md. Keep firmware and stock content outside all
checkouts. Hardware acceptance and revision-bound CPU qualification remain pending.
