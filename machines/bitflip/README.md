# Bitflip

A small videogame macro synth for Open-XY: pulse heroes, triangle bass, detuned
leads, cartridge waves, little FM bells, metallic coins and crunchy noise. Eight
models share two sound macros, a fast chord stepper and a note-start pitch sweep.
An original pixel adventurer runs through a scrolling platform world while you
play, hopping over pipes and crates and collecting coins. All wave ROM and
artwork are original.

## Play

Press any encoder to switch the complete row.

| First row | What it does | Second row | What it does |
| --- | --- | --- | --- |
| **Model** | Choose one of eight chip voices | **Chord** | Off, Oct, Major, Minor, Sus, Maj7, Min7, Power |
| **Color** | The model's primary timbre macro | **Speed** | Eight speeds, 8–80 chord steps per second |
| **Edge** | The model's secondary timbre macro | **Bits** | 2–16-bit symmetric DAC reduction |
| **Sweep** | Start −24 to +24 semitones from the key and return | **Clock** | Eight clock rates, 1.4–44.1 kHz |

**Init Pulse** is a clean square: no Edge, no sweep, no chord stepping, 16 bits
and the full sample clock. Color and Edge are different for each model:

| Model | Color | Edge |
| --- | --- | --- |
| Pulse | Pulse width, 12.5–50% | Add an octave layer |
| Tri | Add upper odd harmonics | Add an octave layer |
| Twin | Width of two pulses | Detune the second pulse, up to about 35 cents |
| Wave | Morph through five original 32-cell wave ROMs | Add an octave layer |
| FM | Integer modulator ratio, 1–8 | Phase-modulation depth |
| Metal | Integer ring ratio, 1–8 | Clean carrier to ring modulation |
| Noise | Clock from one-quarter to sixteen times the played frequency | Long 15-bit to short 7-bit noise sequence |
| Logic | Morph original XOR wave patterns | Add an octave layer |

The octave layer reaches half the oscillator mix. High notes progressively
reduce unsafe harmonics and modulation depth. The noise and reduced DAC clock
have their intended digital grit.

Sweep is captured at note start. Center means no sweep; negative starts below
the key for power-ups, positive starts above it for lasers and percussion. Its
fixed return has an approximately 50 ms time constant and settles at the played
pitch. Changing Sweep affects the next note. Color and Edge smooth during a held
note; changing Model crossfades into the new model over one block.

Chord stepping restarts on the root for each note, preserves oscillator phase
between steps and is independent for each voice. Speed is a free chip clock,
not a tempo division. With Chord off, Speed has no effect on the sound; it still sets the runner’s pace. Stock AMP, LFOs,
filter, velocity, sequencer and effects remain available; the engine does not
replace their pages.

## Factory sounds

Sixteen presets include **Pocket Hero**, **Triangle Bass**, **Twin Blades**,
**Cartridge Keys**, **Crystal Bell**, **Metal Coin**, **Pixel Dust**,
**Short Circuit**, **Glitch Dungeon**, **Major Quest**, **Minor Boss**,
**Power Up**, **Laser Down**, **Save Point** and **Game Over**, plus **Init Pulse**.
Matching native JSON files are in `presets/`; `default.json` is Init Pulse.
Some percussion and bell presets use different stock AMP settings. Reloading
an authored preset applies those settings; selecting the engine alone does not
rewrite an existing track.

Held notes keep the runner moving through layered hills, clouds, brick ground,
floating blocks and spinning coins. Four running cels alternate with standing
and jumping poses. New notes trigger hops; nearby obstacles also trigger jumps.
Coins touched by the runner disappear and increment the two-digit counter.
Speed sets the scene's pace and the eight world lights show Model. The scene
is decorative: coins and jumps do not affect synthesis or measure audio timing.
It coasts to a stop on release. Reduced motion freezes scrolling and poses while
preserving the held-note highlight; switching tracks starts a fresh world.

## Develop and audition

From the XY Machines collection root, with its pinned SDK configured:

```sh
./xy test bitflip --native --cpu
./xy preview bitflip --watch
```

Playing a key starts the preview's audio. The browser gate does not reproduce the
stock AMP, polyphony, mono/glide, filters or device scheduling. Generate the dry
factory tour, original eight-bar game loop and static alias checks with:

```sh
python machines/bitflip/resources/render_demo.py --sdk /absolute/path/to/sdk \
  --out /absolute/external/path/bitflip-audition-001
```

Choose a fresh output folder. The demonstration song uses Bitflip for every
part, including its bass, lead, chord layer, kick and noise percussion. Its host
amplitude gate is recorded in `audition-report.json`.

Regenerate tables, factory presets and vector artwork with:

```sh
python machines/bitflip/resources/generate_tables.py
python machines/bitflip/resources/configure.py
python machines/bitflip/resources/generate_panel.py
```

## Implementation and validation

The table models use nine continuously interpolated color frames and five
harmonic limits (31, 15, 7, 3 and 1), with band crossfades before the upper partial
reaches the guard. PM and ring modulation have additional high-pitch guards.
Two maximal-length, nonzero feedback shift registers supply the noise model.
All synthesis is bounded integer arithmetic with fixed per-voice storage;
audio callbacks allocate no memory and perform no I/O. The voice pool has eight
slots plus the SDK's spare; this is not a hardware polyphony guarantee.

Tests cover tuning, harmonic character, audible models, sweep direction and
settling, chord timing, DAC changes, deterministic restart, 1,024 model/control
corners, random signed automation, read-only input ownership, one million
independently checked pitch products, every quantizer input, complete noise
periods, all table bounds, both display pages, note response, track changes,
reduced motion, complete world wraps, running cels, jumps and coin collection. The audition renderer
also checks periodic models for static nonharmonic foldback with deliberate
clock reduction and chord stepping off.

The local native module occupies about 149 KiB including the SDK. The
host CPU estimate was approximately 7,200 instructions per voice block against
the SDK's 15,000 advisory target. These are local development results, not a
measured device CPU percentage or qualification below the recommended 3.5%.

Run [QA.md](QA.md) on the device before release and follow the SDK's
[hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
On 2026-11-04, revision `72bd776d1868337638221126a3c432e5c87c1419`
booted in a verified resident lab collection and completed hardware CPU sweeps.
The whole-voice result was approximately **8.0%**, above the recommended 3.5%.
The stock midpoint workload was heaviest; adding the octave layer with Edge
also reached about 7.8%. The table interpolation and octave path need to be
lighter to meet the 3.5% target. Graphics are excluded from this measurement.

The Bitflip lab collection was left installed at the user's request; the
original eight-machine collection is preserved separately. The combined
nine-machine collection exceeded the resident memory budget, so this test
does not establish that Bitflip fits alongside all eight existing machines.
Hardware listening, controls, animation and polyphony acceptance remain pending.
Keep firmware, native objects, builds, captures and generated audio outside every
checkout. Local builds do not authorize installation or diagnostics.

MIT licensed; see [LICENSE](LICENSE).
