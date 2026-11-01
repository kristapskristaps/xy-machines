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

## Browser selection, builds and USB installation

Use the official Open-XY browser app with the published catalog plus this
repository's private machines. Local versions take precedence if IDs overlap.
The rejected HK effects and HK Granular are excluded; their sources remain intact.
Catalog extensions are included in the optional extension picker.

```sh
./xy browser
```

This serves the most recent verified personal kit at
`http://127.0.0.1:8928/` using the pinned SDK's allowlisted Vite server. Use
Chrome or Edge for Web MIDI and WebUSB. The exported `opxy-builder.html` also
opens directly in Chrome, including offline, so a running server is optional.
All generated native objects, stock content and browser packages stay in the
external private directory.

Inside the browser, load the original device OS and your last installed
`layout.json`, choose machines and extensions, build, and follow the Mac USB
transfer guide. Loading the layout preserves saved machine numbers. Device USB
permission and the physical boot-partition write/reboot remain user actions.
The app builds a new firmware selection from already compiled objects; it does
not hot-load machines into running firmware. Presets/projects referencing an
omitted machine need that machine restored before they can work normally.

To refresh the pool after editing machine code or updating the pinned catalog:

```sh
./xy browser --refresh --stock-os /external/private/stock-1.2.1/os.ldr
```

Export runs all upstream kit checks before replacing the cached kit. A failed
refresh preserves the previous verified kit. `--build-only` exports without
starting a server; `--port` chooses another localhost port without stopping
another app. The launcher verifies the cached HTML hash and the pinned SDK's
runtime signature before serving it.

The SDK pin includes the pending upstream builder changes in
[PR 279](https://github.com/etcetc000/open-xy/pull/279): Expert mode, automatic machine and
extension selection saving, explanations for a disabled Build button, and the
independent catalog-check correction from PR 277. Native DSP and platform
sources are unchanged. Setup fetches the exact pinned commit from the official
SDK repository; existing SDK checkouts are never reset.

Choices survive refreshes in the same browser at the same address. The OS file
must be chosen again. Audition links (`#preview/machines` or
`#preview/extensions`) cannot build firmware: use **Go to OS setup**, then load
your OS. The selection must fit the memory meter before Build becomes available.

For repeat installs choose **Expert mode** after the entry notice. It remembers
that preference. Load a config/layout or reuse the saved selection, choose your
OS, then use **Build & copy to XY** with the device already in dev-menu MTP mode
and OpenMTP/Field Kit closed. The SDK asks for the USB device, builds, sends
content before the OS, and verifies every file by reading it back. The final
screen gives the physical write/reboot steps in one line. **Save config** works
before a build; **Build ZIP** is available without direct USB transfer.

## Fast edit/listen loop

```sh
./xy new mybass --name "My Bass" --controls 8
./xy test mybass
./xy preview mybass --watch
```

New machines get SDK source/tests/docs and an inventory entry. Effects use
`--kind effect` and four controls. The SDK does not scaffold sample engines;
use upstream Timestretch and HK Granular as the worked examples.

The synth preview prints a free localhost URL. Enable audio and play. Save C++,
artwork or metadata: watch mode builds a fresh authenticated snapshot and reloads
after success. Playing a key or phrase starts audio in the current SDK. Broken edits keep the previous
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

The SDK browser workbench supports synths and drums and uses an audition gate;
it does not emulate native mono/glide, stock envelopes, filters or sends.
Effects/sample engines use the same test/build commands plus existing auditions:

- Hundekraft: see `machines/hundekraft/README.md` for its live stereo effect
  workbench, A/B, test phrases and local audio files.
- Halo: `machines/halo/tools/audition.py` builds/serves its live workbench.
- X0X: see `machines/x0x/README.md` for the 24-pad drum audition, seven kits and independent pad controls.
- Rebound: `machines/rebound/tools/audition.cpp` renders clean ping-pong and
  above-unity feedback rides; see its README for the compile/render commands.
- HK Granular's audition is a recording; native sample assignment still needs
  hardware validation.

Pass the pinned SDK path to specialist tools where supported. No browser or
recording establishes hardware CPU or native display acceptance.

## Repeatable device updates

For a full-collection audition, use the batch plan in
[hardware-testing.md](hardware-testing.md). It covers layout/project isolation,
repeatable listening tests and reusing a capture batch for a complete image.

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
profiling with full synth control sweeps by default. Use `--sweep off` only for
a quick midpoint diagnostic; it cannot qualify a synth catalog CPU record.
Stage validates firmware/compile-report and asset hashes and writes
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

Use the upstream transfer paths: `python sdk.py device push <stage>/files` on
Windows, or OpenMTP/the SDK browser installation guide on macOS. The Windows
command includes backups, complete readback and rollback. Its WPD backend is
Windows-only. Do not add a parallel transfer script. OpenMTP transfers still
need backups and readback verification. No transfer writes the boot partition.

With USB audio off and the project saved/stopped:

```sh
./xy profile --package /private/new-package --machine vader --installed
```

This explicitly starts an active device diagnostic. It collects one complete
run by default and the SDK's revision-bound report. Use `--runs 3` for repeated
measurements. Upstream requires a second run to confirm silent settings; add
repeats for random algorithms as needed. Full synth sweeps can take minutes. Restart without saving the
diagnostic settings afterwards. Host estimates cannot replace these measurements;
the synthetic whole-voice metric excludes graphics and total project scheduling.

Use `--all` instead of `--machine vader` to generate every included machine's
report from the same capture batch. Repeating `--machine` selects a subset of
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
