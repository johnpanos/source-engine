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

--exact compares two captures of the same backend draw by draw, in order, on
every field (seq excluded; floats within FLOAT_TOLERANCE): the frozen-fixture
comparison of RFC 0016 K0 (tools/render/view_oracle.py), where the frame must
not change at all.

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


def _masked(draw, declared):
    """`draw` with the declared nondeterministic components set to None:
    declared maps (shader, field) to the component indices to ignore."""
    if not declared or draw is None:
        return draw
    fields = {field: indices for (shader, field), indices in declared.items()
              if shader == draw.get("shader")}
    if not fields:
        return draw
    masked = dict(draw)
    for field, indices in fields.items():
        value = masked.get(field)
        if isinstance(value, list):
            masked[field] = [None if i in indices else item for i, item in enumerate(value)]
    return masked


def compare_exact(reference, candidate, declared=None):
    """Draws that differ, in order: [{index, field(s), reference, candidate}].

    Every field of every record is compared except `seq` and the components
    `declared` names (see _masked); a draw missing from either side is a
    difference at its index."""
    reference = [_masked(draw, declared) for draw in reference]
    candidate = [_masked(draw, declared) for draw in candidate]
    differences = []
    for index in range(max(len(reference), len(candidate))):
        ref = reference[index] if index < len(reference) else None
        cand = candidate[index] if index < len(candidate) else None
        if ref is None or cand is None:
            differences.append({"index": index, "fields": ["(missing)"],
                                "reference": ref, "candidate": cand})
            continue
        fields = sorted(key for key in (set(ref) | set(cand)) - {"seq"}
                        if _normalize(ref.get(key)) != _normalize(cand.get(key)))
        if fields:
            differences.append({"index": index, "material": ref.get("material"),
                                "pass": ref.get("pass"), "fields": fields,
                                "reference": {key: ref.get(key) for key in fields},
                                "candidate": {key: cand.get(key) for key in fields}})
    return differences


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
    parser.add_argument("--exact", action="store_true",
                        help="same-backend frozen comparison: every draw, in order, every field")
    args = parser.parse_args(argv)
    try:
        ref_header, ref_draws = read_fixture(args.reference)
        cand_header, cand_draws = read_fixture(args.candidate)
    except (OSError, ValueError) as error:
        print("draw-state diff: invalid input: %s" % error, file=sys.stderr)
        return 2
    if args.exact:
        differences = compare_exact(ref_draws, cand_draws)
        report = {"schema": "source-draw-state-exact-diff/v1",
                  "reference": {"path": str(args.reference), **ref_header},
                  "candidate": {"path": str(args.candidate), **cand_header},
                  "draws": [len(ref_draws), len(cand_draws)], "differences": differences}
        if args.out:
            args.out.write_text(json.dumps(report, indent=2) + "\n")
        print("draw-state exact diff: %d/%d draws, %d differ%s"
              % (len(ref_draws), len(cand_draws), len(differences),
                 "; first at %d: %s" % (differences[0]["index"], differences[0]["fields"])
                 if differences else ""))
        return 1 if differences else 0
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
