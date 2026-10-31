# Rebound

This machine is `machines/rebound`. Follow the repository root AGENTS.md
and docs/development.md. Run `./xy test rebound --native --cpu` from the root.
Effects have no SDK browser preview; the README documents host recordings.

Use only public opxy APIs and keep audio callbacks bounded. `tables.h` comes
from `tools/generate_tables.py`; `--check` verifies it against definition.json.
Preserve the parameter order, enum labels and mappings for saved projects.
Keep sound tests here, including above-unity growth, bounded output and recovery.
