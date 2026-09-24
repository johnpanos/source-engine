#!/usr/bin/env python3
"""Compare staircase2 USD scene values independent of USD prim write order."""

import argparse
import hashlib
import json
from pathlib import Path

from pxr import Usd, UsdGeom


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def scene_values(path):
    stage = Usd.Stage.Open(str(path))
    if not stage:
        raise ValueError("could not open USD stage: " + str(path))
    values = {"up_axis": str(UsdGeom.GetStageUpAxis(stage)),
              "meters_per_unit": UsdGeom.GetStageMetersPerUnit(stage),
              "prims": {}}
    for prim in stage.Traverse():
        attributes = {item.GetName(): str(item.Get()) for item in prim.GetAttributes()}
        relationships = {item.GetName(): sorted(str(target) for target in item.GetTargets())
                         for item in prim.GetRelationships()}
        values["prims"][str(prim.GetPath())] = {
            "type": prim.GetTypeName(), "attributes": attributes,
            "relationships": relationships}
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--first", type=Path, required=True)
    parser.add_argument("--second", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    first = scene_values(args.first)
    second = scene_values(args.second)
    first_prims = first["prims"]
    second_prims = second["prims"]
    changed = [path for path in sorted(set(first_prims) | set(second_prims))
               if first_prims.get(path) != second_prims.get(path)]
    same = (first["up_axis"] == second["up_axis"] and
            first["meters_per_unit"] == second["meters_per_unit"] and
            not changed)
    result = {"status": "pass" if same else "fail",
              "scope": "staircase2-stage-value-equivalence",
              "first_sha256": sha256(args.first), "second_sha256": sha256(args.second),
              "first_prim_count": len(first_prims), "second_prim_count": len(second_prims),
              "changed_prim_count": len(changed), "first_changed_prim": changed[0] if changed else None,
              "up_axis_equal": first["up_axis"] == second["up_axis"],
              "meters_per_unit_equal":
                  first["meters_per_unit"] == second["meters_per_unit"]}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))
    if not same:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
