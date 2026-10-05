#!/usr/bin/env python3
"""Audit VMTs with render.material's actual world and mesh claims, without a GPU.

The VMT reader resolves patches and the Portal/Portal 2 search path. A built
render_lab claim-batch command maps those variables and asks ClaimForDrawing
and ClaimForMesh for every combination of native probes and scene color, plus
world drawing with and without a world stage. The report has no second list of
supported shader parameters. --verify checks
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
import re
import subprocess
import sys
import unittest
from pathlib import Path

import material_inventory
import vmt_corpus

DEFAULT_PROFILES = {
    "portal": (material_inventory.REPO / "quality/product_profiles/"
               "portal-linux-wayland-native-vulkan.json"),
    "portal2": (material_inventory.REPO / "quality/product_profiles/"
                "portal2-linux-native-vulkan-high.json"),
}
MESH_INPUTS = ((False, False), (False, True), (True, False), (True, True))
WORLD_INPUTS = (False, True)
BLENDS = {"0": "opaque", "1": "alpha", "2": "premultiplied", "3": "additive",
          "4": "transmittance", "5": "modulate2x", "6": "alpha-additive"}
PARAMETER = re.compile(r"\$[a-zA-Z][a-zA-Z0-9_]*")


def claim_input(material):
    parts = [material["shader_raw"]]
    for key, value in material["vars"].items():
        parts.extend((key, value))
    return ("\0".join(parts) + "\0\0").encode("utf-8")


def claim_result(status, value):
    if status == "C":
        if value not in BLENDS:
            raise ValueError("unknown render_lab blend " + value)
        return {"claim": BLENDS[value]}
    if status == "G":
        return {"gap": value}
    raise ValueError("invalid render_lab claim status " + status)


def profile_record(path, game):
    raw = path.read_bytes()
    profile = json.loads(raw)
    if profile.get("schema") != "source-product-profile/v1" or \
            profile.get("intent", {}).get("render_provider") != "native-vulkan":
        raise ValueError("material audit requires a declared native Vulkan product profile")
    quality = profile["intent"].get("render_quality", {}).get("high", {}).get("settings", {})
    if profile.get("target", {}).get("product") != game + "-client":
        raise ValueError("material audit profile targets another game")
    core_only = quality.get("r_core_world") == "1"
    return {"id": profile["id"],
            "path": str(path.resolve().relative_to(material_inventory.REPO.resolve())),
            "sha256": hashlib.sha256(raw).hexdigest(),
            "core_only_declared": core_only,
            "vmt_conditions": {**material_inventory.PROFILE,
                               "reduce_particles": False,
                               "symbols": sorted(material_inventory.SYMBOLS)}}


def verify_claim_profile(header, profile):
    conditions = profile["vmt_conditions"]
    parts = header.split("\t")
    expected = {"dx": str(conditions["dx"]), "ps20b": str(int(conditions["ps20b"])),
                "hdr": str(int(conditions["hdr"])), "srgb": str(int(conditions["srgb"])),
                "gpu": str(conditions["gpu"]),
                "lowfill": str(int(conditions["reduce_particles"]))}
    fields = {}
    for part in parts[1:]:
        key, separator, value = part.partition("=")
        if not separator or key in fields:
            raise ValueError("render_lab VMT profile differs from corpus parser")
        fields[key] = value
    symbols = fields.pop("symbols", "")
    if parts[0] != "CLAIM-BATCH/3" or fields != expected or \
            sorted(symbols.lower().split(",")) != conditions["symbols"]:
        raise ValueError("render_lab VMT profile differs from corpus parser")


def result_rows(fields):
    if len(fields) != 15:
        raise ValueError("malformed render_lab claim row: expected 15 fields")
    rows = [claim_result(fields[i], fields[i + 1]) for i in range(0, 12, 2)]
    mesh = []
    for (probes, scene_color), result in zip(MESH_INPUTS, rows[:4]):
        mesh.append({"scene_inputs": {"native_reflection_probes": probes,
                                        "linear_scene_color": scene_color}, **result})
    world = []
    for stage, result in zip(WORLD_INPUTS, rows[4:]):
        world.append({"scene_inputs": {"world_stage": stage}, **result})
    return mesh, world


def classify(path, material, mesh, world, family=""):
    # A VMT path suggests a possible geometry, not evidence of scene reachability.
    # Model VMTs need the mesh path. Other VMTs can use a world surface or mesh.
    relevant = [("model_mesh", row) for row in mesh]
    if not path.startswith("materials/models/"):
        relevant += [("world_surface", row) for row in world]
    candidates = []
    for geometry, row in relevant:
        if "claim" not in row:
            continue
        inputs = {name: value for name, value in row["scene_inputs"].items() if value}
        pass_kind = {"depth": "depth", "portal-mask": "portal_mask",
                     "water": "water"}.get(family)
        if pass_kind is None:
            pass_kind = "opaque" if row["claim"] == "opaque" else "blended"
        candidates.append({"geometry": geometry, "pass": pass_kind,
                           "blend": row["claim"], "scene_inputs": inputs})
    # Keep the smallest sufficient input set for each geometry/pass/blend.
    candidates.sort(key=lambda candidate: (candidate["geometry"], candidate["pass"],
                                           candidate["blend"], len(candidate["scene_inputs"]),
                                           sorted(candidate["scene_inputs"])))
    minimal = []
    for candidate in candidates:
        if any(earlier["geometry"] == candidate["geometry"] and
               earlier["blend"] == candidate["blend"] and
               earlier["scene_inputs"].items() <= candidate["scene_inputs"].items()
               for earlier in minimal):
            continue
        minimal.append(candidate)
    candidates = minimal
    # A proxy can change values at bind time. The baseline candidates remain
    # useful, but cannot certify the draw after its proxy executes.
    structural_gap = not candidates and all(
        "maps to no family" in row.get("gap", "") or row.get("gap", "").startswith("import:")
        for _, row in relevant)
    if material["proxies"] and not structural_gap:
        status = "dynamically_unresolved"
    elif not candidates:
        status = "unsupported"
    elif any(c["geometry"] == "world_surface" and c["pass"] == "opaque" and
             not c["scene_inputs"]
             for c in candidates) or (path.startswith("materials/models/") and
                                       any(c["pass"] == "opaque" and not c["scene_inputs"]
                                           for c in candidates)):
        status = "statically_supported"
    else:
        status = "supported_with_requirements"
    return status, candidates


def refusal_features(reason):
    """Name work areas from claim diagnostics, without redefining claim support."""
    lower = reason.lower()
    if lower.startswith("vmt: "):
        return ("vmt:missing_include" if lower.startswith("vmt: missing include ")
                else "vmt:" + lower[5:].replace(" ", "_"),)
    if lower.startswith("import: "):
        return ("import:" + lower[8:],)
    shader = re.match(r"shader (\S+) maps to no family", reason, re.IGNORECASE)
    if shader:
        return ("shader_family:" + shader.group(1).lower(),)
    program = re.match(r"family (\S+) has no program", reason, re.IGNORECASE)
    if program:
        return ("family_program:" + program.group(1).lower(),)
    mesh_point = re.match(r"family (\S+) has no mesh point", reason, re.IGNORECASE)
    if mesh_point:
        return ("mesh_point:" + mesh_point.group(1).lower(),)
    view_texture = re.search(r"per-view texture (\S+)", reason, re.IGNORECASE)
    if view_texture:
        return ("per_view_texture:" + view_texture.group(1).lower(),)
    parameters = sorted(set(parameter.lower() for parameter in PARAMETER.findall(reason)))
    if parameters:
        return tuple("parameter:" + parameter for parameter in parameters)
    unread = re.search(r"does not read ([a-z][a-z0-9_]*)$", lower)
    if unread:
        return ("parameter:$" + unread.group(1),)
    return ("diagnostic:" + lower,)


def summarize_refusals(results):
    groups = collections.defaultdict(lambda: {"refused_materials": 0,
                                              "first_refusals": 0,
                                              "unmapped_materials": 0,
                                              "examples": []})
    for path, item in results.items():
        if item["status"] != "unsupported":
            continue
        # One material is counted once per feature, even if six claim inputs
        # repeat the same gap. Distinct features may co-occur on one material.
        rows = list(item.get("mesh_claims", []))
        if not path.startswith("materials/models/"):
            rows += item.get("world_claims", [])
        reasons = {row["gap"] for row in rows if "gap" in row}
        reasons.add(item["gap"])  # Includes VMT parse failures.
        features = sorted({feature for reason in reasons for feature in refusal_features(reason)})
        item["refusal_features"] = features
        for feature in features:
            groups[feature]["refused_materials"] += 1
            if len(groups[feature]["examples"]) < 3:
                groups[feature]["examples"].append(path)
        for feature in refusal_features(item["gap"]):
            groups[feature]["first_refusals"] += 1
        for key in set(item.get("unmapped_keys", [])):
            for feature in refusal_features("the model does not read " + key):
                if feature.startswith("parameter:"):
                    groups[feature]["unmapped_materials"] += 1
    return dict(sorted(groups.items()))


def collect(game, render_lab, scope, profile_path=None):
    if profile_path is None:
        profile_path = DEFAULT_PROFILES[game]
    profile = profile_record(profile_path, game)
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
    output = process.stdout.decode("utf-8").splitlines()
    if not output:
        raise ValueError("render_lab returned no claim protocol header")
    verify_claim_profile(output[0], profile)
    answers = output[1:]
    if len(answers) != len(records):
        raise ValueError(f"render_lab answered {len(answers)} of {len(records)} materials")
    results = {}
    for path, material in loaded.items():
        if "error" in material:
            results[path] = {"status": "unsupported", "gap": "VMT: " + material["error"]}
            continue
        canonical = json.dumps(material, sort_keys=True, separators=(",", ":")).encode()
        results[path] = {
            "input_sha256": hashlib.sha256(canonical).hexdigest(),
            "shader": material["shader_raw"].lower(),
            "proxies": sorted(name for name, _ in material["proxies"]),
        }
    for (path, material), answer in zip(records, answers):
        fields = answer.split("\t")
        mesh, world = result_rows(fields)
        item = results[path]
        item["mesh_claims"] = mesh
        item["world_claims"] = world
        item["family"] = fields[14]
        item["status"], item["candidates"] = classify(
            path, material, mesh, world, item["family"])
        if fields[12]:
            item["numeric_keyvalues_residue"] = fields[12].split(",")
        if fields[13]:
            item["unmapped_keys"] = fields[13].split(",")
        if item["status"] == "unsupported":
            relevant = mesh if path.startswith("materials/models/") else world
            item["gap"] = next((r["gap"] for r in relevant if "gap" in r),
                               "no product geometry claims this material")
    gaps = collections.Counter(
        item["gap"] for item in results.values() if item["status"] == "unsupported"
    )
    total = len(results)
    counts = collections.Counter(item["status"] for item in results.values())
    by_shader = collections.defaultdict(lambda: {"total": 0, "statically_supported": 0,
                                                  "supported_with_requirements": 0,
                                                  "dynamically_unresolved": 0,
                                                  "unsupported": 0})
    for item in results.values():
        shader = item.get("shader", "<parse error>")
        by_shader[shader]["total"] += 1
        by_shader[shader][item["status"]] += 1
    feature_groups = summarize_refusals(results)
    return {
        "schema": "rendercore-material-claims/v3",
        "game": game,
        "profile": profile,
        "scope": ("materials/models/**/*.vmt" if scope == "models" else "all materials/**/*.vmt") +
                 " in the game's ordered search path",
        "rule": "render.material MapVariables + the actual world and mesh claim rules under six scene-input combinations",
        "qualification": "VMT path is a possible geometry, not scene reachability; pass is derived from blend or imported special family, not proof of integration. Proxies need per-draw evaluation. Texture residency, device pipeline/capabilities and pixels require product evidence.",
        "total": total,
        "status_counts": dict(sorted(counts.items())),
        "gaps": dict(sorted(gaps.items(), key=lambda item: (-item[1], item[0]))),
        "refusal_features": feature_groups,
        "by_shader": dict(sorted(by_shader.items())),
        "materials": results,
    }


def main():
    if sys.argv[1:] == ["self-test"]:
        suite = unittest.defaultTestLoader.discover(
            str(material_inventory.REPO / "tools/render/tests"),
            pattern="test_material_claim_inventory.py")
        result = unittest.TextTestRunner(verbosity=0).run(suite)
        if not result.wasSuccessful() or result.testsRun == 0:
            return 1
        print(f"CONFORMANCE {result.testsRun} 0")
        return 0
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game", choices=("portal", "portal2"), default="portal2")
    parser.add_argument("--scope", choices=("models", "all"), default="models")
    parser.add_argument("--render-lab", type=Path, required=True)
    parser.add_argument("--profile", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    try:
        report = collect(args.game, args.render_lab.resolve(), args.scope, args.profile)
        data = (json.dumps(report, indent=2, sort_keys=True) + "\n").encode()
        if args.verify:
            if not args.out.is_file() or args.out.read_bytes() != data:
                raise ValueError("material claim inventory differs: " + str(args.out))
        else:
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_bytes(data)
        print(f"{args.game}: {report['total']} {args.scope} VMTs; "
              + ", ".join(f"{name}={count}" for name, count in report["status_counts"].items()))
        for feature, counts in sorted(report["refusal_features"].items(),
                                      key=lambda item: (-item[1]["refused_materials"], item[0]))[:15]:
            print(f"{counts['refused_materials']:4}  {feature}")
        if args.verify:
            print(f"CONFORMANCE {report['total']} 0")
        return 0
    except (OSError, ValueError, vmt_corpus.CorpusError) as error:
        print("material_claim_inventory:", error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
