# Develop and test machines

Use the computer for the edit/listen loop and batch hardware updates. The DSP
and display C++ are statically linked into one firmware image; there is no
individual-plugin hot reload over USB.

## Setup once

Requires Python 3.11+ (prefer 3.12), Node 22.12+, Git and Clang with wasm32 and
wasm-ld. Homebrew LLVM is detected automatically on Apple-silicon macOS.

```sh
./xy setup --native
./xy doctor
./xy list
```

Windows: use `xy.cmd`, or `py -3.12 tools/dev.py setup --native`. Setup creates
`.venv`, installs NumPy 2.3.5 and clones the exact SDK pin into the separate
sibling `../xy-machines-sdk`. `--native` fetches the pinned Blackfin tools.
Existing SDK checkouts are never reset. Reuse an existing pinned checkout/cache:

```sh
./xy setup --sdk /path/to/pinned-sdk --toolchain /path/to/blackfin-toolchain
```

Local configuration is ignored `local/dev.json`. Setup can also remember
`--stock-os /private/stock-1.2.1/os.ldr`, `--layout /private/installed/layout.json`
and `--private-root /private/xy-machines`. Firmware paths inside any Git checkout
are rejected. Default hardware outputs use the OS's OPXY-SDK private directory.

## Fast edit/listen loop

```sh
./xy new mybass --name "My Bass" --controls 8
./xy test mybass
./xy preview mybass --watch
```

New machines get SDK source/tests/docs and an inventory entry. Effects use
`--kind effect` and four controls. The SDK does not scaffold sample engines;
use Granular as the worked example.

The synth preview prints a free localhost URL. Enable audio and play. Save C++,
artwork or metadata: watch mode builds a fresh authenticated snapshot and reloads
after success. Re-enable audio after reload. Broken edits keep the previous
audition and print the error. Use `--port 8947` for a fixed port or `--no-serve`
to build only. It serves the SDK allowlist, never the source checkout.

```sh
./xy test vader halo
./xy test --changed
./xy test --changed --base origin/main
./xy test vader --native --cpu
```

No IDs means the complete inventory. Changed-file selection includes staged,
unstaged and untracked machine sources. Inventory/SDK-pin changes select all.
Normal tests skip the slower CPU sweep; `--cpu` enables its host estimate.
Native effects are linked with Subvista because SDK startup requires an engine.
Temporary native modules are not installable OS images.

The SDK browser workbench currently supports synths and uses an audition gate;
it does not emulate native mono/glide, stock envelopes, filters or sends.
Effects/sample engines use the same test/build commands plus existing auditions:

- Hundekraft: see `machines/hundekraft/README.md` for its live stereo effect
  workbench, A/B, test phrases and local audio files.
- Halo: `machines/halo/tools/audition.py` builds/serves its live workbench.
- Granular: `machines/granular/tools/audition.py` renders audition WAVs.
  HK Granular's audition is also a recording. Native sample assignment still
  needs hardware validation.

Pass the pinned SDK path to specialist tools where supported. No browser or
recording establishes hardware CPU or native display acceptance.

## Repeatable device updates

For a full-collection audition, use the measured two-bank plan in
[hardware-testing.md](hardware-testing.md). It covers layout/project isolation,
repeatable listening tests and reusing three captures for a complete image.

1. Choose a development **kit** containing the machines used by current projects
   plus the machine under development. The full inventory is not a default
   hardware kit and may exceed memory.
2. Keep the **last verified installed layout.json** to preserve machine numbers.
3. Run focused tests/native checks, commit clean source, then build and stage:

```sh
./xy build --kit collections/my-device.json \
  --layout /private/last-installed/layout.json \
  --stock-os /private/stock-1.2.1/os.ldr
./xy stage /private/new-package \
  --against /private/previous-verified-stage/install-manifest.json
```

The example kit is your own subset, not an included device configuration. Both
commands choose fresh external output directories by default. Build enables
profiling. Stage validates firmware/compile-report and asset hashes and writes
`files/`, `layout.json`, `install-manifest.json` and `INSTALL.md`.

The changed-file list compares bundles, not the live device. Copy only the listed
files after backing up device originals; do not delete other content. Presets
are excluded by default to protect edited sounds. Add `--presets` for a first
install or intentional factory-preset update; the SDK creates/validates them,
including authored native presets and AMP defaults. Stage never opens USB.

If only native preset JSON changed and the engine contract is unchanged, transfer
that preset and reload it without flashing. For DSP/display changes, install
matching code and artwork together—artwork can affect native node bindings.

After authorizing device work: save the project, enter dev-menu MTP, back up
replaced files, copy the staged changes, and verify full readbacks against their
hashes. Keep layout.json on the computer for the next build. Track 2 exits MTP;
track 5 writes OS to the boot partition. After “firmware written ok!” power-cycle
and confirm normal boot. The writer reads `/os.ldr`.

Whole-file libmtp reads previously stalled this Mac/XY connection. The tested
procedure uses bounded partial reads and complete hash verification. These tools
prepare bundles; they do not claim a universal one-click MTP installer or
automate the physical flash menu.

With USB audio off and the project saved/stopped:

```sh
./xy profile --package /private/new-package --machine vader --installed
```

This explicitly starts an active device diagnostic. It collects three complete
sequential runs and the SDK's revision-bound report. Restart without saving the
diagnostic settings afterwards. Host estimates cannot replace these measurements;
the synthetic whole-voice metric excludes graphics and total project scheduling.

Use `--all` instead of `--machine vader` to generate every included machine's
report from the same three captures. Repeating `--machine` selects a subset of
reports without repeating the hardware capture batch.

## Parallel work and final checks

Use separate branches/worktrees per chat. Original sibling working folders are
independent; import their later completed changes explicitly rather than
overwriting them. This collection is not an automatic mirror.

```sh
python -m unittest discover -s tests
./xy test
python tools/check_private_files.py --staged
python tools/check_private_files.py --history --all-objects
```

CI runs workflow tests, collection host checks and native compilation. Shared
tooling changes also need the pinned SDK public check. Each published machine
keeps its own tag, commit, catalog path and hardware CPU evidence. No local test,
preview or build authorizes device operations.
