# XY Machines

34 Open-XY machines: **18 synths, 15 send effects and 1 sample engine**.
This is a source collection with independent machine releases. The SDK stays
in a separate sibling checkout. No firmware or device data is included.

## Layout

- `machines/<id>/`: Vader, Weave, Matter, Vapour, Halo, Westfold, Acidline,
  and Subvista.
- `machines/hundekraft/`: all 26 Hundekraft ports, retaining their collection
  scripts, audition sources, original attribution and per-machine folders.
- `collection.json`: all 34 machine paths. `collections/hundekraft.json` and
  `collections/standalone.json` select the two groups.
  `collections/hundekraft-test.json` selects exactly the 12 HK synths and 13 effects.
- `provenance/`: source commits, branch/tag references, original file hashes,
  snapshot records and the documentation/tooling-only layout adaptations.

An inventory does not imply that all machines fit one firmware image. Use a
subset and preserve your device's existing layout when building firmware.

## Check and develop

The standard workflow is documented in **[docs/development.md](docs/development.md)**
and [AGENTS.md](AGENTS.md). Its short commands are:

```sh
./xy setup --native
./xy new mybass --name "My Bass" --controls 8
./xy test mybass
./xy preview mybass --watch
./xy test --changed --native
```

Setup records local paths in ignored configuration. Hardware `build`, `stage`
and explicit `profile` commands are described in the guide; previews/tests never
contact a device. Existing machines' DSP and artwork are unchanged by these tools.

Use Python 3.12, Node 22.12+, NumPy 2.3.5 and Clang with wasm32 support.
The SDK is pinned to official upstream staging at `SDK_REVISION` (SDK 1.6).
`SDK_REPOSITORY` is `etcetc000/open-xy`; no private SDK fork is required.
Local checkouts used by other threads are left untouched. Weave uses the
upstream Model knob/page navigation rather than the old fork-only Shift shortcut.
The standalone Granular engine is removed from the active tree; its original
Git history remains available. HK Granular remains in the inventory, but the
Hundekraft synth/effects test kit excludes it.

```sh
git clone https://github.com/etcetc000/open-xy.git ../xy-machines-sdk
git -C ../xy-machines-sdk checkout --detach 6941bc8a18e622d520cff6c9503121c6518e8fa3
python -m pip install numpy==2.3.5
python tools/check.py --sdk ../xy-machines-sdk
python tools/check.py --sdk ../xy-machines-sdk --native-only
python tools/check_private_files.py --history --all-objects
```

On Homebrew macOS, add `--cxx /opt/homebrew/opt/llvm/bin/clang++` to host checks.
Native checks need the SDK's pinned Blackfin tools (`python sdk.py toolchain fetch`
from the SDK checkout). They compile each machine into an external temporary
directory, without stock firmware or device operations. Each effect is paired
with the small Subvista synth because SDK startup requires at least one engine.
`--native` runs both
host and native checks. For one machine, use the SDK directly:

```sh
python ../xy-machines-sdk/sdk.py test --machine machines/vader
```

Read a machine's own README and AGENTS.md before changing it. The Hundekraft
folder preserves its audition tools and source import workflow; their SDK paths
are adapted for this layout. Generated previews and recordings are not imported.
Regenerate them locally using that folder's documented commands.

## History and ongoing work

Seven existing repositories are imported with their original commits intact,
using non-squashed subtree-style merge commits. Original branch and tag refs
are retained under `archive/<machine-id>/...`. To inspect the pre-import source,
use its original commit and original root paths, for example:

```sh
git show ee31a8525ba7d381743d682eda232397f8929c83:engine.cpp
git log --all --graph --oneline
```

Hundekraft, Acidline and Subvista had no committed history; they have explicit
initial source-snapshot commits. Snapshot hashes describe the import moment,
not future source revisions. Original checkouts were neither moved nor changed.
Other chats can keep working there. Incorporate later completed changes as
separate reviewed imports; never overwrite their working folders from here.

## Independent releases and validation

Each machine can have its own release tag, commit, catalog path and hardware
CPU record. Examples are `vader-v1.0.0` with path `machines/vader` and
`hksupersaw-v1.0.0` with path `machines/hundekraft/hksupersaw`. Updating one
machine does not require repinning the others. Catalog submissions follow the
pinned SDK policy: main by default, staging when an unreleased SDK API is needed,
and only catalog entries change.

Imported build and hardware claims belong to their original source revisions.
In particular, Vader's earlier measured commit remains in history; that CPU
record is not automatically a measurement of this reorganized release. Perform
revision-bound hardware profiling, or obtain an approved carry-over, before
catalog publication. This migration does not install or test any device.

## Licences

See [LICENSING.md](LICENSING.md). Original machine and vendor licences, notices
and provenance are preserved. The root MIT licence covers collection tooling
and documentation; it does not replace the licences of the machines.

## Inventory

| ID | Name | Kind | Folder |
| --- | --- | --- | --- |
| acidline | Acidline | synth | [machines/acidline](machines/acidline/README.md) |
| halo | Halo | effect | [machines/halo](machines/halo/README.md) |
| hkarc | HK Arc | synth | [machines/hundekraft/hkarc](machines/hundekraft/hkarc/README.md) |
| hkclouds | HK Clouds | effect | [machines/hundekraft/hkclouds](machines/hundekraft/hkclouds/README.md) |
| hkcrush | HK Crush | effect | [machines/hundekraft/hkcrush](machines/hundekraft/hkcrush/README.md) |
| hkelg | HK Elg | synth | [machines/hundekraft/hkelg](machines/hundekraft/hkelg/README.md) |
| hkeva | HK EVA | synth | [machines/hundekraft/hkeva](machines/hundekraft/hkeva/README.md) |
| hkfather | HK Father | effect | [machines/hundekraft/hkfather](machines/hundekraft/hkfather/README.md) |
| hkfmbell | HK FM Bell | synth | [machines/hundekraft/hkfmbell](machines/hundekraft/hkfmbell/README.md) |
| hkgranular | HK Granular | sample | [machines/hundekraft/hkgranular](machines/hundekraft/hkgranular/README.md) |
| hkhail | HK HAIL | effect | [machines/hundekraft/hkhail](machines/hundekraft/hkhail/README.md) |
| hkinaroom | HK In a room | effect | [machines/hundekraft/hkinaroom](machines/hundekraft/hkinaroom/README.md) |
| hkmbira | HK MBIRA | synth | [machines/hundekraft/hkmbira](machines/hundekraft/hkmbira/README.md) |
| hkmixtur | HK Mixtur | synth | [machines/hundekraft/hkmixtur](machines/hundekraft/hkmixtur/README.md) |
| hkoxbow | HK Oxbow | synth | [machines/hundekraft/hkoxbow](machines/hundekraft/hkoxbow/README.md) |
| hkpalme | HK Palme | effect | [machines/hundekraft/hkpalme](machines/hundekraft/hkpalme/README.md) |
| hkpew | HK PEW | synth | [machines/hundekraft/hkpew](machines/hundekraft/hkpew/README.md) |
| hkplate | HK Plate | effect | [machines/hundekraft/hkplate](machines/hundekraft/hkplate/README.md) |
| hkreverb | HK Reverb | effect | [machines/hundekraft/hkreverb](machines/hundekraft/hkreverb/README.md) |
| hksid | HK SID | synth | [machines/hundekraft/hksid](machines/hundekraft/hksid/README.md) |
| hksnowglobe | HK Snowglobe | effect | [machines/hundekraft/hksnowglobe](machines/hundekraft/hksnowglobe/README.md) |
| hkstutter | HK Stutter | effect | [machines/hundekraft/hkstutter](machines/hundekraft/hkstutter/README.md) |
| hksupersaw | HK Supersaw | synth | [machines/hundekraft/hksupersaw](machines/hundekraft/hksupersaw/README.md) |
| hktaffy | HK Taffy | synth | [machines/hundekraft/hktaffy](machines/hundekraft/hktaffy/README.md) |
| hktape | HK Tape | effect | [machines/hundekraft/hktape](machines/hundekraft/hktape/README.md) |
| hkwarble | HK Warble | effect | [machines/hundekraft/hkwarble](machines/hundekraft/hkwarble/README.md) |
| hkwasher | HK Washer | effect | [machines/hundekraft/hkwasher](machines/hundekraft/hkwasher/README.md) |
| hkwavetable | HK Wavetable | synth | [machines/hundekraft/hkwavetable](machines/hundekraft/hkwavetable/README.md) |
| matter | Matter | synth | [machines/matter](machines/matter/README.md) |
| subvista | Subvista | synth | [machines/subvista](machines/subvista/README.md) |
| vader | Vader | synth | [machines/vader](machines/vader/README.md) |
| vapour | Vapour | effect | [machines/vapour](machines/vapour/README.md) |
| weave | Weave | synth | [machines/weave](machines/weave/README.md) |
| westfold | Westfold | synth | [machines/westfold](machines/westfold/README.md) |
