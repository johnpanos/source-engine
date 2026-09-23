#!/usr/bin/env python3
"""Stage Portal VPK textures and candidate PBR VMTs for ArmorPaint review."""

from __future__ import annotations

import argparse
import fnmatch
import json
import math
import os
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import sys
import time

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
HELPER = ROOT / "build/portal-pbr/portal_assets"
SOURCES = [
    ROOT / "tools/portal_pbr/portal_assets.cpp",
    ROOT / "hammer/adapters/platform/disk_byte_store.cpp",
    ROOT / "hammer/core/formats/vpk_archive.cpp",
    ROOT / "hammer/core/formats/vtf_image.cpp",
    ROOT / "hammer/core/formats/material.cpp",
    ROOT / "hammer/core/formats/keyvalues.cpp",
    ROOT / "public/render/pbr_material_schema.h",
]
SUPPORTED_SHADERS = {"lightmappedgeneric", "vertexlitgeneric"}
REVIEW_PARAMS = {
    "$phongexponenttexture": "authored Phong exponent texture needs a reviewed roughness conversion",
    "$phongfresnelranges": "Source Phong Fresnel ranges need review",
    "$selfillum": "self illumination needs a reviewed emission mapping",
    "$selfillummask": "self illumination mask needs a reviewed emission mapping",
    "$detail": "detail blend is not represented by this PBR candidate",
    "$envmapmask": "environment mask needs a reviewed IBL mapping",
    "$envmap": "legacy environment map needs a reviewed IBL mapping",
    "$translucent": "translucency requires runtime PBR alpha validation",
    "$alphatest": "alpha test requires runtime PBR alpha validation",
}
UNSUPPORTED_EFFECTS = {"$additive", "$refractamount", "$frame", "$animatedtexturevar"}


def build_helper() -> None:
    if HELPER.exists() and HELPER.stat().st_mtime >= max(p.stat().st_mtime for p in SOURCES):
        return
    HELPER.parent.mkdir(parents=True, exist_ok=True)
    flags = subprocess.check_output(["pkg-config", "--cflags", "--libs", "libpng"], text=True).split()
    command = [
        "c++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
        "-I" + str(ROOT / "public"), "-I" + str(ROOT),
        *(str(p) for p in SOURCES if p.suffix == ".cpp"), "-o", str(HELPER), *flags,
    ]
    subprocess.run(command, check=True)


def valid_name(raw: str) -> str | None:
    name = raw.strip().replace("\\", "/").lower()
    if name.startswith("materials/"):
        name = name[10:]
    if name.endswith((".vtf", ".vmt")):
        name = name[:-4]
    path = PurePosixPath(name)
    if not name or path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        return None
    if any(ord(c) < 32 or c in '"' for c in name):
        return None
    return name


def parameters(row: dict) -> dict[str, str]:
    return {str(key).lower(): str(value) for key, value in row.get("parameters", [])}


def truthy(value: str | None) -> bool:
    return value is not None and value.strip().lower() not in {"", "0", "false", "no"}


def get_image(vpks: list[Path], name: str, output: Path, cache: dict[str, dict]) -> dict:
    if name in cache:
        return cache[name]
    path = "materials/" + name + ".vtf"
    info = {"error": "texture missing from supplied archives"}
    for vpk in vpks:
        result = subprocess.run(
            [str(HELPER), "decode", str(vpk), path, str(output)],
            text=True, capture_output=True,
        )
        if result.returncode:
            info = {"error": result.stderr.strip() or "decode failed"}
            if result.returncode == 4:
                continue
            break
        info = json.loads(result.stdout)
        info["file"] = str(output)
        info["source_vpk"] = str(vpk)
        info["ssbump"] = bool(info["flags"] & 0x08000000)
        break
    cache[name] = info
    return info


def quote(value: str) -> str:
    return '"' + value.replace("\\", "/").replace('"', "") + '"'


def make_material(name: str, params: dict[str, str], has_normal: bool, schema: dict) -> str:
    texture_root = "portal_pbr/" + name
    lines = [
        quote(schema["shader"]), "{",
        "\t" + quote(schema["base"]) + " " + quote(texture_root + "/basecolor"),
        "\t" + quote(schema["mrao"]) + " " + quote(texture_root + "/mrao"),
        "\t" + quote(schema["fallback"]) + " " + quote("portal_pbr_fallback/" + name),
    ]
    if has_normal:
        lines.append("\t" + quote(schema["normal"]) + " " + quote(texture_root + "/normal"))
    for key in ("$surfaceprop", schema["alphatest"], schema["alphatestreference"], schema["translucent"]):
        if key in params:
            lines.append("\t" + quote(key) + " " + quote(params[key]))
    lines.append("}")
    return "\n".join(lines) + "\n"


def import_to_armorpaint(
    paths: list[Path], armorpaint_dir: Path, project_path: Path | None, new_project: bool,
) -> tuple[list[dict], dict | None]:
    os.environ["ARMORPAINT_DIR"] = str(armorpaint_dir.resolve())
    sys.path.insert(0, str(ROOT / "thirdparty/armorpaint-mcp"))
    from armorpaint_mcp.transport import send_to_armorpaint  # pylint: disable=import-outside-toplevel

    results = []
    if new_project:
        send_to_armorpaint("project_new", {}, 30)
    for index, path in enumerate(paths, start=1):
        try:
            reply = send_to_armorpaint("import_asset", {"path": str(path.resolve())}, 300)
            results.append({"file": str(path), "result": reply})
        except Exception as exc:  # Keep a failed import visible in the manifest.
            results.append({"file": str(path), "error": str(exc)})
            break
        if index % 25 == 0:
            print(f"ArmorPaint bridge accepted {index}/{len(paths)} files", flush=True)
    save = None
    if project_path is not None:
        project_path = project_path.resolve()
        project_path.parent.mkdir(parents=True, exist_ok=True)
        previous_mtime = project_path.stat().st_mtime_ns if project_path.exists() else None
        try:
            reply = send_to_armorpaint("project_save_as", {"path": str(project_path)}, 30)
            save = {"path": str(project_path), "reply": reply, "verified": False}
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                if project_path.exists():
                    stat = project_path.stat()
                    if stat.st_size > 0 and stat.st_mtime_ns != previous_mtime:
                        save["verified"] = True
                        save["bytes"] = stat.st_size
                        break
                time.sleep(0.1)
            if save["verified"]:
                save["reopen"] = send_to_armorpaint(
                    "project_open", {"path": str(project_path)}, 300,
                )
                info = send_to_armorpaint("project_get_info", {}, 30)
                save["reopened_asset_count"] = info.get("asset_count")
                if new_project:
                    save["verified"] = save["reopened_asset_count"] == len(paths)
        except Exception as exc:
            save = {"path": str(project_path), "error": str(exc), "verified": False}
    return results, save


def convert(args: argparse.Namespace) -> int:
    vpk = args.vpk.resolve()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    build_helper()
    schema = json.loads(subprocess.check_output([str(HELPER), "schema"], text=True))
    asset_vpks = [vpk, *(path.resolve() for path in args.asset_vpk)]
    for asset_vpk in asset_vpks:
        if not asset_vpk.is_file():
            raise FileNotFoundError(asset_vpk)

    scan = subprocess.run(
        [str(HELPER), "scan", str(vpk), str(out / "legacy-materials")],
        check=True, text=True, capture_output=True,
    )
    rows = [json.loads(line) for line in scan.stdout.splitlines()]
    (out / "inventory.jsonl").write_text(scan.stdout, encoding="utf-8")
    selected = [
        row for row in rows
        if fnmatch.fnmatch(row["path"][10:-4], args.only)
    ]
    if args.limit:
        selected = selected[:args.limit]

    cache: dict[str, dict] = {}
    materials = []
    import_paths: list[Path] = []
    for row in selected:
        name = valid_name(row["path"])
        entry = {"source_vmt": row["path"], "candidate": None, "review": [], "status": "skipped"}
        materials.append(entry)
        if row.get("error"):
            entry["review"].append(row["error"])
            continue
        if not name or row["shader"].lower() not in SUPPORTED_SHADERS:
            entry["review"].append("shader is outside the two supported lit families")
            continue
        params = parameters(row)
        if row.get("proxies"):
            entry["review"].append("VMT proxies are not represented by this PBR candidate")
        if any(truthy(params.get(key)) for key in UNSUPPORTED_EFFECTS):
            entry["review"].append("additive, refractive or animated effect needs manual conversion")
            continue
        base = valid_name(params.get("$basetexture", ""))
        if not base:
            entry["review"].append("missing or invalid base texture")
            continue
        for key, reason in REVIEW_PARAMS.items():
            if truthy(params.get(key)):
                entry["review"].append(reason)

        source_base = out / "source-textures" / (base + ".png")
        base_info = get_image(asset_vpks, base, source_base, cache)
        if "error" in base_info:
            entry["review"].append("base texture: " + base_info["error"])
            continue
        texture_dir = out / "textures/portal_pbr" / name
        texture_dir.mkdir(parents=True, exist_ok=True)
        base_path = texture_dir / "basecolor.png"
        shutil.copyfile(source_base, base_path)
        entry["base"] = {"source": base, **base_info}
        entry["channels"] = {"basecolor": "sRGB encoded", "mrao": "linear"}
        image_paths = [base_path]

        normal_name = valid_name(params.get("$bumpmap", ""))
        has_normal = False
        if normal_name:
            source_normal = out / "source-textures" / (normal_name + ".png")
            normal_info = get_image(asset_vpks, normal_name, source_normal, cache)
            if "error" in normal_info:
                entry["review"].append("normal texture: " + normal_info["error"])
            elif normal_info["ssbump"]:
                entry["review"].append("SSBump encoding needs conversion before tangent-normal use")
            else:
                normal_path = texture_dir / "normal.png"
                shutil.copyfile(source_normal, normal_path)
                image_paths.append(normal_path)
                has_normal = True
                entry["normal"] = {"source": normal_name, **normal_info}
                entry["channels"]["normal"] = "linear tangent space, authored"

        roughness = 0.8
        roughness_source = "placeholder"
        exponent_text = params.get("$phongexponent")
        if exponent_text:
            try:
                exponent = float(exponent_text)
                if math.isfinite(exponent) and exponent > 0:
                    roughness = max(0.02, min(1.0, math.sqrt(2.0 / (exponent + 2.0))))
                    roughness_source = "approximation from $phongexponent"
                else:
                    entry["review"].append("invalid $phongexponent")
            except ValueError:
                entry["review"].append("non-numeric $phongexponent")
        mrao_path = texture_dir / "mrao.png"
        Image.new(
            "RGB", (base_info["width"], base_info["height"]),
            (0, round(roughness * 255), 255),
        ).save(mrao_path)
        image_paths.append(mrao_path)
        entry["mrao"] = {
            "metalness": 0.0, "roughness": roughness, "roughness_source": roughness_source,
            "ambient_occlusion": 1.0,
        }
        candidate = out / "materials/portal_pbr" / (name + ".vmt")
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_text(make_material(name, params, has_normal, schema), encoding="utf-8")
        fallback = out / "materials/portal_pbr_fallback" / (name + ".vmt")
        fallback.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(out / "legacy-materials" / (name + ".vmt"), fallback)
        checked = subprocess.run(
            [str(HELPER), "validate", str(candidate), str(fallback)],
            text=True, capture_output=True,
        )
        if checked.returncode:
            entry["review"].append("generated definition invalid: " + checked.stderr.strip())
            entry["status"] = "invalid_candidate"
            continue
        entry["schema_validation"] = json.loads(checked.stdout)
        entry["candidate"] = str(candidate)
        entry["fallback"] = str(fallback)
        entry["status"] = "candidate_requires_runtime_integration"
        import_paths.extend(image_paths)

    result = {
        "schema": "portal-pbr-staging-v1",
        "source_vpk": str(vpk),
        "asset_vpks": [str(path) for path in asset_vpks],
        "material_schema": schema,
        "source_vpk_dir_size": vpk.stat().st_size,
        "inventory_count": len(rows),
        "selection": args.only,
        "selected_count": len(selected),
        "candidate_count": sum(entry["candidate"] is not None for entry in materials),
        "materials": materials,
        "texture_decodes": cache,
        "engine_ready": False,
    }
    if args.import_armorpaint:
        imports, saved_project = import_to_armorpaint(
            import_paths, args.armorpaint_dir, args.armorpaint_project,
            args.new_armorpaint_project,
        )
        result["armorpaint_imports"] = imports
        if saved_project is not None:
            result["armorpaint_project"] = saved_project
        (out / "armorpaint-imports.json").write_text(
            json.dumps({"imports": imports, "project": saved_project}, indent=2) + "\n",
            encoding="utf-8",
        )
    (out / "manifest.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Scanned {len(rows)} VMTs; staged {result['candidate_count']} of {len(selected)} selected materials.")
    print("Manifest:", out / "manifest.json")
    if args.import_armorpaint:
        imports = result["armorpaint_imports"]
        print(f"ArmorPaint bridge accepted: {sum('error' not in item for item in imports)}/{len(imports)}")
        if args.armorpaint_project:
            print("ArmorPaint project saved:", result["armorpaint_project"]["verified"])
        if len(imports) != len(import_paths) or any("error" in item for item in imports):
            return 1
        if args.armorpaint_project and not result["armorpaint_project"]["verified"]:
            return 1
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vpk", type=Path, required=True, help="Portal 1 _dir.vpk")
    parser.add_argument("--out", type=Path, required=True, help="Output directory outside the repository")
    parser.add_argument(
        "--asset-vpk", type=Path, action="append", default=[],
        help="Additional texture VPK searched after the Portal archive (repeatable)",
    )
    parser.add_argument("--only", default="*", help="Material name glob, without materials/ or .vmt")
    parser.add_argument("--limit", type=int, default=0, help="Optional number of selected VMTs")
    parser.add_argument("--import-armorpaint", action="store_true", help="Import staged PNGs via the installed MCP bridge")
    parser.add_argument(
        "--new-armorpaint-project", action="store_true",
        help="Start a fresh ArmorPaint project before importing (discards the current live project)",
    )
    parser.add_argument(
        "--armorpaint-project", type=Path,
        help="Save the live ArmorPaint project here after import and verify the file write",
    )
    parser.add_argument(
        "--armorpaint-dir", type=Path,
        default=ROOT / "thirdparty/armorpaint/paint/build/out",
        help="ArmorPaint build/out directory with the enabled MCP bridge",
    )
    args = parser.parse_args()
    if (args.armorpaint_project or args.new_armorpaint_project) and not args.import_armorpaint:
        parser.error("ArmorPaint project options require --import-armorpaint")
    return convert(args)


if __name__ == "__main__":
    raise SystemExit(main())
