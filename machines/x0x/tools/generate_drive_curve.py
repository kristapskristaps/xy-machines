"""Regenerate the Q15 cubic Drive curve; integer arithmetic matches the original."""
from pathlib import Path

values = []
for index in range(257):
    boosted = min(index * 128, 32767)
    half = boosted // 2
    cube = (half * half // 16384) * half // 16384
    values.append(boosted - 2 * cube // 3)
header = (
    "// Cubic saturation sampled in positive signed-Q15 units.\n"
    "// Copyright (c) 2026 Kristaps Dreija. MIT; see ../LICENSE.\n"
    "#pragma once\nnamespace x0x { namespace dsp {\n"
    "static const opxy::i16 drive_curve[257]={\n"
    + "".join("    " + ",".join(map(str, values[i:i + 12])) + ",\n" for i in range(0, len(values), 12))
    + "};\n}}\n"
)
Path(__file__).resolve().parents[1].joinpath("dsp", "drive_curve.h").write_text(header)
