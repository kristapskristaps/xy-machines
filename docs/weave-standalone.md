# Weave standalone models

Five new IDs extract the preferred models from optimized Weave revision
`957afdbac6f2ef9916991366e24517f1483dc0e4`. Existing Weave and its saved ID remain
available for older projects. No SDK or device changes are part of this split.

| Name | ID | Original animation label | Visual mechanism | Native machine footprint |
| --- | --- | --- | --- | ---: |
| Tandem | weave_analog | analog | Paired oscillators, coupling rail and mixed output | 31.3 KiB |
| Atlas | weave_waves | waves | Layered wavetable maps with a moving locator | 80.1 KiB |
| Choir | weave_chords | chords | Five staggered reeds, voicing and a root key | 407.0 KiB |
| Sift | weave_grain | grain | Grain sifter and formant ripples | 27.1 KiB |
| Crease | weave_shaper | shaper | Folding press, jaws and a shaped ribbon | 308.2 KiB |

Figures are native non-bootable compile footprints, excluding shared SDK
support. The five-engine selection totals 888.4 KiB with shared SDK support.
Installing every sibling is not automatically smaller than full Weave: some
common tables and adapter code are duplicated. Selecting only the models needed
is the benefit; Choir alone does not carry Shaper's transfer tables or FM banks.

Each machine has four controls corresponding to Weave Harmonic, Timbre, Morph
and Output. Choir labels these Chord, Voicing, Colour and Root. Inapplicable
Strike, Mode and Seed slots are removed. No extra envelope or LFO is added.
Curated engine presets are included. Newly created IDs do not reinterpret old
Weave projects or presets; recreate a sound by copying the four corresponding
control values into the standalone machine.

Tandem, Atlas and Sift retain the existing 48 kHz source and rational converter.
Choir and Crease retain their optimized native 44.1 kHz entries. The selected
voice union is reduced from Weave's 5,468-byte voice: Tandem 336, Atlas 1,344,
Choir 400, Sift 240 and Crease 120 bytes. DSP namespaces are independent so the
machines can be linked together.

All five scenes are original retained SVG artwork, with shared native C++ frame
computation. Controls ease over 180 ms; note activity drives the mechanisms;
idle frames settle; initialization/track changes emit no fabricated note;
reduced motion renders the resting state immediately. Browser visuals were
reviewed in the SDK workbench. Fonts and rasterization on hardware may differ.

Validation includes 45 independently generated PCM and selected-state fixtures
per model, frozen against the unchanged optimized Weave (225 scenes in total).
They cover all four control sweeps, pitch extremes, glide, retriggers and moving
controls. Choir additionally retains the Chords cache, output-rail and 64-frame
oracle tests. Frame tests cover bounds, noncollapsed outlines, note activity,
idle settling, reduced motion and track changes. The five machines compile
individually and link as a combined Blackfin collection. Shared workflow and
SDK public checks are also run; their private fixtures remain explicit skips.

**Hardware CPU and playback qualification are pending for these new IDs.**
Historical Weave/Chords measurements cannot be reused as catalog CPU records.
The connected XY remains on its previous audition collection.

Regenerate DSP with `machines/weave/tools/extract_standalones.py`, passing
`--source` for the clean frozen source worktree, `--sdk` for the pinned SDK and
`--cxx` for LLVM. Regenerate original artwork with
`python machines/weave/tools/generate_split_art.py`. The independent fixture
regenerator is `generate_standalone_reference.py`, with the same source/SDK,
a host `--cxx`, and a fresh external `--out` for all compiled host artifacts.
Use `collections/weave-standalone.json` to select the five siblings.
