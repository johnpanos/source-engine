#!/usr/bin/env python3
"""Audit VMTs with render.material's actual world and mesh claims, without a GPU.

The VMT reader resolves patches and the Portal/Portal 2 search path. A built
render_lab claim-batch command maps those variables and asks ClaimForDrawing
and ClaimForMesh, so
this report has no second list of supported shader parameters. --verify checks
the complete report against the checked-in inventory for the content profile.
Numeric keys left by KeyValues' unquoted scalar syntax are recorded per VMT.
The importer also records every unmapped key, including keys behind the first
claim failure, so a new term alone cannot hide a material's other gaps.
"""

import argparse
import collections
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

import material_inventory
import vmt_corpus


def claim_input(material):
    parts = [material["shader_raw"]]
    for key, value in material["vars"].items():
        parts.extend((key, value))
    return ("\0".join(parts) + "\0\0").encode("utf-8")


def claim_result(status, value):
    if status == "C":
        return {"claim": "alpha" if value == "1" else "opaque" if value == "0" else value}
    if status == "G":
        return {"gap": value}
    raise ValueError("invalid render_lab claim status " + status)


def collect(game, render_lab, scope):
    content = vmt_corpus.Game(game)
    paths = [p for p in content.vmts()
             if scope == "all" or p.startswith("materials/models/")]
    if not paths:
        raise ValueError("no VMTs in the content search path")
    loaded = {}
    records = []
    for path in paths:
        material = material_inventory.load_material(path, content.read)
        loaded[path] = material
        if "error" not in material:
            records.append((path, material))
    process = subprocess.run(
        [str(render_lab), "claim-batch"],
        input=b"".join(claim_input(material) for _, material in records),
        env={**os.environ, "LD_LIBRARY_PATH": str(render_lab.parents[2] / "tier0") +
             (":" + os.environ["LD_LIBRARY_PATH"] if os.environ.get("LD_LIBRARY_PATH") else "")},
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if process.returncode:
        raise ValueError("render_lab claim-batch failed: " + process.stderr.decode("utf-8", "replace"))
    answers = process.stdout.decode("utf-8").splitlines()
    if len(answers) != len(records):
        raise ValueError(f"render_lab answered {len(answers)} of {len(records)} materials")
    results = {}
    for path, material in loaded.items():
        if "error" in material:
            results[path] = {"gap": "VMT: " + material["error"]}
            continue
        canonical = json.dumps(material, sort_keys=True, separators=(",", ":")).encode()
        results[path] = {
            "input_sha256": hashlib.sha256(canonical).hexdigest(),
            "shader": material["shader_raw"].lower(),
            "proxies": sorted(name for name, _ in material["proxies"]),
        }
    for (path, material), answer in zip(records, answers):
        fields = answer.split("\t")
        if len(fields) != 8:
            raise ValueError("malformed render_lab claim for " + path)
        native = claim_result(fields[0], fields[1])
        without_probes = claim_result(fields[2], fields[3])
        world = claim_result(fields[4], fields[5])
        results[path]["native_probes"] = native
        results[path]["without_probes"] = without_probes
        results[path]["world"] = world
        if fields[6]:
            results[path]["numeric_keyvalues_residue"] = fields[6].split(",")
        if fields[7]:
            results[path]["unmapped_keys"] = fields[7].split(",")
        if material["proxies"]:
            results[path]["static_gap"] = "material proxy"
        elif "gap" in native and (path.startswith("materials/models/") or
                                    "gap" in world):
            results[path]["static_gap"] = (native if path.startswith("materials/models/")
                                           else world)["gap"]
    gaps = collections.Counter(
        item["static_gap"] for item in results.values() if "static_gap" in item
    )
    total = len(results)
    claimed = total - sum(gaps.values())
    by_shader = collections.defaultdict(lambda: {"total": 0, "statically_claimed": 0})
    for item in results.values():
        shader = item.get("shader", "<parse error>")
        by_shader[shader]["total"] += 1
        if "static_gap" not in item:
            by_shader[shader]["statically_claimed"] += 1
    return {
        "schema": "rendercore-material-claims/v2",
        "game": game,
        "scope": ("materials/models/**/*.vmt" if scope == "models" else "all materials/**/*.vmt") +
                 " in the game's ordered search path",
        "rule": ("render.material MapVariables + ClaimForMesh; native RPRB available"
                 if scope == "models" else
                 "render.material MapVariables + ClaimForDrawing(world PBR) or ClaimForMesh(native RPRB)"),
        "qualification": "static claim only; texture residency, model reachability, and pixels require product evidence",
        "total": total,
        "statically_claimed": claimed,
        "statically_claimed_percent": round(100 * claimed / total, 2),
        "gaps": dict(sorted(gaps.items(), key=lambda item: (-item[1], item[0]))),
        "by_shader": dict(sorted(by_shader.items())),
        "materials": results,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game", choices=("portal", "portal2"), default="portal2")
    parser.add_argument("--scope", choices=("models", "all"), default="models")
    parser.add_argument("--render-lab", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    try:
        report = collect(args.game, args.render_lab.resolve(), args.scope)
        data = (json.dumps(report, indent=2, sort_keys=True) + "\n").encode()
        if args.verify:
            if not args.out.is_file() or args.out.read_bytes() != data:
                raise ValueError("material claim inventory differs: " + str(args.out))
        else:
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_bytes(data)
        print(f"{args.game}: {report['statically_claimed']}/{report['total']} {args.scope} VMTs "
              f"statically claim a native surface point ({report['statically_claimed_percent']}%)")
        for reason, count in list(report["gaps"].items())[:15]:
            print(f"{count:4}  {reason}")
        if args.verify:
            print(f"CONFORMANCE {report['total']} 0")
        return 0
    except (OSError, ValueError, vmt_corpus.CorpusError) as error:
        print("material_claim_inventory:", error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
