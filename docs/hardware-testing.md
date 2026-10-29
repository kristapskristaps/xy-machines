# Audition the complete collection efficiently

Use two lab images, one flash per image, and audition every machine in that
image before changing firmware. Fix findings in batches and reflash only the
affected image. Preset-only tuning does not need an OS rewrite while its engine
contract is unchanged.

## Validated batch sizes

Native compilation with the pinned SDK and this chat's last-known installed
layout produced these module footprints on 2026-10-29:

| Lab bank | Collection | Coverage | Module footprint |
| --- | --- | --- | ---: |
| A | `collections/standalone.json` | 6 synths, 2 effects, 1 sample engine | 1,471.7 KiB |
| B | `collections/hundekraft.json` | 12 synths, 13 effects, 1 sample engine | 1,066.2 KiB |

The module allowance is 2,040 KiB. Both groups link successfully. Combining all
35 with the last-known layout fails SDK engine-number allocation before linking;
effects also occupy engine-number slots. These are native module checks, not
bootable image verification, device acceptance or measured CPU results. Do not
assume the device still has this chat's last-known layout if another thread has
installed an update.

## Bank/project discipline

Back up the actual current firmware, layout and replaced content before the
first lab installation. Keep that known-working configuration for restoration.
Use a saved, stopped stock-only **LAB BOOT** project before changing banks, so
automatic startup never opens a project that depends on the previous bank.

Keep **LAB A** and **LAB B** projects and a saved layout for each bank. Only open
a bank's custom-machine projects while that matching bank is installed. Normal
projects may reference machines omitted from a lab image. Separate lab layouts
are not interchangeable, and carrying every reservation across the entire
35-machine collection can exhaust the SDK's ID pool. Never silently reuse a
personal project's IDs or treat a lab layout as a universal collection layout.

For each bank, build with its deliberately chosen saved layout, stage native
presets for a first install, back up replaced files, verify complete readbacks,
then write the OS and reboot. When revising that bank later, use that bank's
last verified installed layout. Use a real current device readback to choose the
initial layout; do not infer it from an old package filename. Device work still
requires an explicit user instruction and physical MTP/flash/reboot readiness.

## One shared audition routine

Create the lab projects on the XY rather than synthesizing undocumented project
files. Use the same tempo, note phrase, velocities, dry input and mixer settings
for comparisons. Keep a stock reference sound/input and save test presets.

| Kind | Quick sweep | Stress/edge checks |
| --- | --- | --- |
| Synth | Default and named presets; low/mid/high notes; sweep every control | Retrigger, glide, 1/4/8 voices where supported, note-off/release, preset/project reload |
| Effect | Same drums, pluck and sustained chord; dry/wet A/B; control extremes | Freeze/hold on/off, tails after input stops, reset/reload, feedback extremes, stereo image |
| Sample | Same mono/stereo sources and a percussion loop | Trim endpoints, reverse, retrigger, pitch range, assignment changes, grain-density extremes |

Test each internal model separately: one Weave engine entry does not mean its
15 algorithms have all been auditioned. The standard CPU benchmark uses fixed
settings, not every model or preset. Listen for clicks, DC/level jumps, lost
fundamental, harsh upper notes, stuck tails/voices and control discontinuities.
Check the native display, page switching and reduced motion independently.

For each finding record **bank, machine ID, internal model, source commit,
preset/parameter values, note or input, expected/actual behaviour and a short
audio capture**. This makes fixes reproducible instead of relying on memory.
Prioritize the Hundekraft algorithms already flagged by host estimates (Arc,
Wavetable, SID, Clouds, Reverb and Palme) for demanding-setting hardware tests;
host estimates alone do not establish their device cost.

## Three captures per image, not per machine

After installing/confirming a bank, save/stop the project and leave USB audio off:

```sh
./xy profile --package /private/lab-a-package --all --installed
```

This collects **three complete sequential captures total** for that image, then
generates an individual SDK CPU report for every included machine using those
same captures. Stock reference cases are not published as collection machines.
For only selected reports, repeat `--machine`:

```sh
./xy profile --package /private/lab-a-package \
  --machine vader --machine weave --installed
```

Two lab images therefore need six complete captures, not 105. Output includes
`batch-summary.json`, the three captures and per-machine reports in external
private storage. A failed run aborts; incomplete captures never become zero-cost
results. Raw captures stay private because they can contain device identity.
Restart without saving the diagnostic's temporary settings before auditioning.

## Improvement cycle

1. Flash A once; audition all nine, record findings, collect its three CPU runs.
2. Return to LAB BOOT; flash B once; repeat for all 26 and their internal models.
3. Fix the recorded problems on the computer with focused tests/browser auditions.
4. Rebuild/reflash only affected banks; repeat the exact recorded comparisons.
5. Remeasure changed release revisions, then restore the saved everyday firmware
   and its layout before resuming normal projects.

Do not flash for each knob setting or individual preset. Do not derive maximum
polyphony or total-project CPU by multiplying the synthetic whole-voice metric;
test demanding real playback separately. Test projects and listening feedback
are useful acceptance evidence in addition to the standard catalog CPU records.
