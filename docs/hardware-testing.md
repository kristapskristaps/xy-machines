# Audition the collection efficiently

Batch listening tests into a small number of firmware installs. DSP and display
changes require a rebuild, boot-partition write and reboot; preset-only tuning
can be transferred and reloaded while its engine contract remains unchanged.

## Choose a kit

`collections/hundekraft-test.json` selects **12 synths**. The 13 HK effects
remain excluded after failing device listening tests; do not add them to a
normal hardware kit.

`collections/standalone.json` selects nine synths, one drum machine and two
effects. `collections/hundekraft.json` retains all 25 HK sources, including the
excluded effects. The complete inventory has 37 entries;
it is not a hardware kit. Every combined image must pass the shared 2,032 KiB
module limit. Do not assume the complete inventory fits together.

## Preserve projects and layout

Back up the actual current firmware, layout and replaced content before a lab
installation. Another chat may have installed a newer image; an old package
filename is not evidence of what is currently on the device. Preserve the
last verified layout when building so existing machine numbers remain stable.
The SDK assigns numbers through layout.json, not machine.json.

Before changing the installed kit, open and save a stopped stock-only **LAB BOOT**
project. Only open a project's custom machines when their matching kit is
installed. Keep test projects, install manifests and layouts for each kit.
Do not reuse personal machine numbers or silently overwrite edited presets.

Build from clean committed source, stage presets for a first installation,
back up replaced files and verify complete readbacks. Use upstream device push
on Windows, or its documented OpenMTP/browser flow on Mac. Confirm the physical
boot-partition write and reboot before claiming installation. All firmware,
backups, captures and packages stay in external private storage.

## Repeatable listening

Create lab projects on the XY. Use the same tempo, phrase, velocities, dry input
and mixer settings, with a stock reference sound/input and saved test presets.

| Kind | Quick sweep | Edge checks |
| --- | --- | --- |
| Synth | Presets; low/mid/high notes; every control and internal model | Retrigger, glide, chords, note-off/release, preset/project reload |
| Effect | Drums, pluck and sustained chord; dry/wet A/B; extremes | Freeze/hold, tails, feedback, reset/reload, stereo image |
| Sample | Mono/stereo sources and percussion loop | Trim, reverse, retrigger, pitch, assignment, grain density |

Listen for clicks, level jumps, missing fundamental, harsh high notes, stuck
voices/tails and discontinuous controls. Check display/page switching separately.
For findings record **kit, machine/model, source commit, preset/settings, input,
expected/actual behavior and an audio capture**. Host cost estimates can suggest
where to investigate; they do not establish hardware CPU or usable polyphony.

## Share profiling captures across reports

Install a profiling-enabled package, confirm reboot, save/stop the project and
leave USB audio off:

```sh
./xy profile --package /private/hk-package --all --installed
```

One complete capture is the current upstream default. It covers all cases in
the image, and the same capture generates each machine's revision-bound report.
Use `--runs 3` for repeats, or repeat `--machine <id>` to select reports without
repeating the capture batch. Confirm silent settings with a second run and add
runs for random algorithms as directed by the SDK report. Full synth sweeps
can take minutes: each control is exercised individually and the heaviest
positions are combined. They do not audition every possible preset or interaction.

`./xy build` enables full sweeps by default. `--sweep off` is a faster midpoint
check that cannot qualify a synth catalog record. Investigate synth settings
above upstream's recommended 3.5%, recording the heaviest setting. Never infer
total project CPU or maximum polyphony by multiplying a synthetic voice result.

Output includes `batch-summary.json`, raw captures and individual reports in
external private storage. Failed/incomplete captures are not zero-cost results.
Restart without saving diagnostic settings before listening. Fix recorded issues
in batches, rebuild only affected kits and remeasure changed release revisions.
Restore the backed-up everyday image/layout before resuming normal projects.
