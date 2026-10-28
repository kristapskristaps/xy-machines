# XY Machines

This repository contains 35 Open-XY machines. The SDK is a separate sibling
checkout at `../open-xy`; never vendor or modify it to make a machine work.
Read the SDK's authoring, UX, API, build and profiling guides before machine work.

- Work in this repository. The original sibling repositories and the original
  `open-xy-hundekraft` folder are independent working copies used by other chats;
  do not move, reset, commit, clean or overwrite their work as part of this repo.
- Nine standalone machines live in `machines/<id>`. The 26 Hundekraft ports live
  in `machines/hundekraft/<id>` with their original collection tools and notices.
  Each folder containing `machine.json` is an independently releasable machine.
  New machines belong in `machines/<id>` without a nested Git repository.
- Use root `collection.json` as the complete inventory. Files in `collections/`
  select groups; an inventory is not a promise that all machines fit one image.
  Keep IDs, parameter contracts, licences, notices and upstream attribution intact.
- Run `python tools/check.py --sdk ../open-xy` for machine previews and host tests.
  Use `--native` for per-machine Blackfin compilation into external temporary
  directories. Run the SDK public check and the staged/history private-data scan.
  Preserve imported DSP during layout/tooling changes; check provenance hashes.
- Contributor C++ uses public opxy headers, with no OS headers, platform
  conditionals, assembly, allocation, blocking, I/O or graphics in audio callbacks.
- Firmware, stock content, device captures, firmware packages and private backups
  stay outside every checkout, including ignored folders. Never commit them.
- Building/checking locally does not authorize device operations. Hardware tests
  require user authorization, confirmed install/reboot and three complete runs.
  Measurements bind the exact source revision; old measurements are historical
  evidence, not CPU qualification for a newly reorganized release commit.
- Preserve original commits and namespaced `archive/<id>/...` branch/tag refs.
  `provenance/imports.json` describes initial imports, not mutable current hashes.
  Each release can have its own tag, commit, catalog path and CPU report. SDK
  catalog submissions target staging and change only the catalog entry.
