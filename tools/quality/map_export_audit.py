#!/usr/bin/env python3
"""Audit a finished map build against its export-quality profile.

`pbrt_map_build.py` runs this as its final `audit` gate. The profile
(`quality/map_export_profiles/<name>.json`, default `source2`) declares what a
release-quality export must contain; the audit reads the build's own receipts
and fails when any requirement is not met:

  * lightmap bake samples, OIDN denoising and the directional (normal-map-aware)
    bake, packaged as a 2:1 directional LMAP page;
  * a map reflection probe in the LMAP page;
  * every authored material channel exported (base, metallic, roughness,
    occlusion, normal, opacity; emission not dropped) and no texture above
    the profile's size;
  * WMSH tangent-frame fallbacks under the declared fraction;
  * a passing runtime gate when the map was booted.

    python3 tools/quality/map_export_audit.py --build quality-results/usd-room-map \\
        --profile quality/map_export_profiles/source2.json --out audit.json
"""

import argparse
import json
from pathlib import Path


def load(path):
    path = Path(path)
    return json.loads(path.read_text()) if path.is_file() else None


def audit(build, profile, booted):
    rules = profile.get("audit") or {}
    checks = []

    def check(name, passed, detail):
        checks.append({"check": name, "status": "pass" if passed else "fail", "detail": detail})

    bake = load(build / "lighting" / "atlas.exr.json") or {}
    denoise = load(build / "lighting" / "atlas-denoised.exr.json") or {}
    ktx2 = load(build / "lighting" / "atlas.ktx2.json") or {}
    content = load(build / "content.json") or {}
    wmsh = next((load(path) for path in sorted(build.glob("*.wmsh.json"))), None) or {}
    if "min_lightmap_samples" in rules:
        check("lightmap-samples", bake.get("samples", 0) >= rules["min_lightmap_samples"],
              {"samples": bake.get("samples"), "required": rules["min_lightmap_samples"],
               "device": bake.get("device")})
    if rules.get("require_denoise"):
        check("lightmap-denoise", bool(denoise.get("denoiser")),
              {"denoiser": denoise.get("denoiser")})
    if rules.get("require_directional"):
        directional = load(build / "lighting" / "atlas-directional.exr.json") or {}
        check("directional-lightmap",
              bool(bake.get("directional")) and directional.get("status") == "pass" and
              ktx2.get("layout") == "directional-2x1",
              {"layout": ktx2.get("layout"),
               "fitted_texels": directional.get("fitted_texels"),
               "clamped_fraction": directional.get("clamped_fraction"),
               "rnm_colour_residual": directional.get("rnm_colour_residual")})
    if rules.get("require_reflection_probe"):
        check("reflection-probe", bool(ktx2.get("reflection_probe")),
              {"probe": ktx2.get("reflection_probe")})
    if rules.get("require_authored_channels"):
        missing = {}
        for name, material in content.get("materials", {}).items():
            exported = material.get("exported_channels") or {}
            lost = []
            for channel in material.get("authored_channels", []):
                value = exported.get(channel)
                if channel == "base" and value != "texture":
                    lost.append(channel)
                elif channel in ("metallic", "roughness", "occlusion", "normal") and \
                        value != "texture":
                    lost.append(channel)
                elif channel == "opacity" and value not in ("cutout", "transmission"):
                    lost.append(channel)
                elif channel == "emission" and (value or "").startswith("dropped"):
                    lost.append(channel)
            if exported.get("emission", "") and str(exported["emission"]).startswith("dropped"):
                lost.append("emission")
            if lost:
                missing[name] = sorted(set(lost))
        check("authored-channels", bool(content) and not missing,
              {"materials": len(content.get("materials", {})), "missing": missing})
    if "max_texture_size" in rules:
        limit = rules["max_texture_size"]
        oversized = {name: material.get("encoded_dimensions")
                     for name, material in content.get("materials", {}).items()
                     if max(material.get("encoded_dimensions") or [0]) > limit}
        check("texture-size", not oversized, {"limit": limit, "oversized": oversized})
    if "max_wmsh_normal_fallback_fraction" in rules:
        meshes = wmsh.get("source_meshes", [])
        triangles = sum(mesh.get("triangles", 0) for mesh in meshes)
        fallbacks = sum(mesh.get("normal_fallbacks", 0) for mesh in meshes) / 3.0
        fraction = fallbacks / triangles if triangles else 1.0
        check("wmsh-normals", bool(meshes) and
              fraction <= rules["max_wmsh_normal_fallback_fraction"],
              {"triangles": triangles, "fallback_fraction": fraction})
    if rules.get("require_runtime_gate_when_booted") and booted:
        gate = load(build / "camera-boot" / "gate.json") or {}
        check("runtime-gate", gate.get("status") == "pass",
              {"status": gate.get("status")})
    failed = [item["check"] for item in checks if item["status"] == "fail"]
    return {"schema": "map-export-audit/v1", "profile": profile["name"],
            "status": "pass" if not failed else "fail", "failed": failed, "checks": checks}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--booted", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    result = audit(args.build, json.loads(args.profile.read_text()), args.booted)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"status": result["status"], "failed": result["failed"]}))
    if result["status"] != "pass":
        raise SystemExit("map export audit failed: " + ", ".join(result["failed"]))


if __name__ == "__main__":
    main()
