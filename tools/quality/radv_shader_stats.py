#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Parse RADV_DEBUG=shaderstats compiler diagnostics; these are not GPU timings."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

import render_profile

REQUIRED = ("Driver pipeline hash", "Hash", "SGPRs", "VGPRs", "Spilled SGPRs",
            "Spilled VGPRs", "Code size", "Scratch size", "Subgroups per SIMD", "Instructions")


def analyze(path):
    text = path.read_text()
    rows, active, stage = [], None, ""
    for line in text.splitlines():
        if re.fullmatch(r"[A-Za-z][A-Za-z0-9 /_-]*Shader(?: as [A-Za-z0-9 /_-]+)?:", line):
            stage = line[:-1]
        elif line == "*** SHADER STATS ***":
            if active is not None or not stage:
                raise render_profile.ProfileError("nested or unnamed shader statistics")
            active = {"stage": stage}
        elif line == "********************" and active is not None:
            if any(name not in active for name in REQUIRED):
                raise render_profile.ProfileError("incomplete shader statistics")
            rows.append(active)
            active, stage = None, ""
        elif active is not None:
            field = re.fullmatch(r"([A-Za-z][A-Za-z0-9 ()_-]*): ([0-9]+)", line)
            if not field or field[1] in active:
                raise render_profile.ProfileError("malformed or duplicated shader statistic")
            active[field[1]] = int(field[2])
    if active is not None or not rows:
        raise render_profile.ProfileError("truncated or absent shader statistics")
    return {"schema": "radv-shader-stats/v1", "source": str(path.resolve()),
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(), "shaders": rows,
            "semantics": "static compiler counts and residency limits, not dynamic occupancy, "
                         "frame cost, draw count or GPU time; includes compilation repeats; "
                         "pipeline hashes do not establish material/pass identity"}


def tsv(report):
    names = ("Driver pipeline hash", "stage", "Hash", "VGPRs", "SGPRs", "Spilled VGPRs",
             "Spilled SGPRs", "Scratch size", "Instructions", "Code size", "Subgroups per SIMD")
    rows = ["pipeline_hash\tstage\tshader_hash\tvgprs\tsgprs\tspilled_vgprs\tspilled_sgprs\t"
            "scratch_bytes\tinstructions\tcode_bytes\tmax_subgroups_per_simd"]
    rows.extend("\t".join(str(row[name]) for name in names) for row in report["shaders"])
    return "\n".join(rows) + "\n"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--json", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        report = analyze(args.log)
        args.json.write_text(json.dumps(report, indent=2) + "\n")
        print(tsv(report), end="")
        return 0
    except (OSError, ValueError) as error:
        parser.exit(2, "radv_shader_stats: %s\n" % error)


if __name__ == "__main__":
    sys.exit(main())
