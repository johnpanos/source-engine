#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Compare two draw-state fixtures (source-draw-state/v1) material by material.

Run: python3 tools/quality/draw_state_diff.py --reference dx9-0.jsonl \\
         --candidate vulkan-native-0.jsonl [--out report.json]

Two captures of "the same" frame from different backends never line up draw for
draw: particles, animation and timing differ. Draws are therefore grouped by
(material, pass) and compared by the state each group was drawn with. The
report lists, worst first, the fields on which the candidate backend applied
different state than the reference, and the materials only one side drew.

Exit 0 when no field differs, 1 when differences were found, 2 on invalid input.
"""

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import sys


SCHEMA = "source-draw-state/v1"
# Fields that describe what a pass was drawn with. Viewport and target size are
# excluded from the default comparison: they follow the window, not the material.
STATE_FIELDS = ("shader", "target", "samplers", "blend", "src_blend", "dst_blend",
                "depth_test", "depth_write", "alpha_test_ref", "modulation",
                "base_texture_transform", "submitted")
FLOAT_TOLERANCE = 1e-4


class FixtureError(ValueError):
    pass


def read_fixture(path):
    lines = Path(path).read_text().splitlines()
    if not lines:
        raise FixtureError("%s is empty" % path)
    header = json.loads(lines[0])
    if header.get("schema") != SCHEMA:
        raise FixtureError("%s is not a %s fixture" % (path, SCHEMA))
    draws = [json.loads(line) for line in lines[1:] if line]
    if len(draws) != header.get("draws"):
        raise FixtureError("%s declares %s draws but holds %d" % (path, header.get("draws"), len(draws)))
    return header, draws


def _normalize(value):
    if isinstance(value, float):
        return round(value / FLOAT_TOLERANCE) * FLOAT_TOLERANCE
    if isinstance(value, list):
        return tuple(_normalize(item) for item in value)
    if isinstance(value, dict):
        return tuple(sorted((key, _normalize(item)) for key, item in value.items()))
    return value


def _group(draws):
    groups = defaultdict(list)
    for draw in draws:
        groups[(draw["material"], draw["pass"])].append(draw)
    return groups


def _states(draws, field):
    """Distinct values a group was drawn with, most frequent first."""
    counts = Counter(_normalize(draw.get(field)) for draw in draws)
    return [value for value, _ in counts.most_common()]


def compare(reference, candidate):
    ref_groups, cand_groups = _group(reference), _group(candidate)
    differences = []
    for key in sorted(set(ref_groups) & set(cand_groups)):
        for field in STATE_FIELDS:
            ref_values, cand_values = _states(ref_groups[key], field), _states(cand_groups[key], field)
            if set(ref_values) != set(cand_values):
                differences.append({
                    "material": key[0], "pass": key[1], "field": field,
                    "reference": _plain(ref_values), "candidate": _plain(cand_values),
                    "reference_draws": len(ref_groups[key]), "candidate_draws": len(cand_groups[key])})
    by_field = Counter(item["field"] for item in differences)
    return {
        "compared_groups": len(set(ref_groups) & set(cand_groups)),
        "differences": differences,
        "differences_by_field": dict(by_field.most_common()),
        "reference_only": sorted("%s#%d" % key for key in set(ref_groups) - set(cand_groups)),
        "candidate_only": sorted("%s#%d" % key for key in set(cand_groups) - set(ref_groups)),
    }


def _plain(values):
    def convert(value):
        if isinstance(value, tuple):
            if value and all(isinstance(item, tuple) and len(item) == 2 and isinstance(item[0], str)
                             for item in value):
                return {name: convert(item) for name, item in value}
            return [convert(item) for item in value]
        return value
    return [convert(value) for value in values]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args(argv)
    try:
        ref_header, ref_draws = read_fixture(args.reference)
        cand_header, cand_draws = read_fixture(args.candidate)
    except (OSError, ValueError) as error:
        print("draw-state diff: invalid input: %s" % error, file=sys.stderr)
        return 2
    report = {"schema": "source-draw-state-diff/v1",
              "reference": {"path": str(args.reference), **ref_header},
              "candidate": {"path": str(args.candidate), **cand_header},
              **compare(ref_draws, cand_draws)}
    if args.out:
        args.out.write_text(json.dumps(report, indent=2) + "\n")
    print("draw-state diff: %d groups compared, %d differing fields, %d reference-only, %d candidate-only"
          % (report["compared_groups"], len(report["differences"]),
             len(report["reference_only"]), len(report["candidate_only"])))
    for field, count in report["differences_by_field"].items():
        print("  %-24s %d" % (field, count))
    return 1 if report["differences"] else 0


if __name__ == "__main__":
    sys.exit(main())
