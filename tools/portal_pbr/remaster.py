#!/usr/bin/env python3
"""Upscale staged Portal PBR sets with a pinned external GPU upscaler.

The model changes color only. The companion C++ tool restores source alpha and
resizes normal/MRAO channels without asking a color model to invent vector data.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

from PIL import Image

from workflow import ROOT, import_to_armorpaint


CHANNELS_SOURCE = ROOT / "tools/portal_pbr/remaster_channels.cpp"
CHANNELS_BINARY = ROOT / "build/portal-pbr/remaster_channels"
EXPECTED_RELEASE_SHA256 = "e5aa6eb131234b87c0c51f82b89390f5e3e642b7b70f2b9bbe95b6a285a40c96"


def digest(path: Path) -> str:
    hashed = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            hashed.update(chunk)
    return hashed.hexdigest()


def build_channels() -> None:
    if CHANNELS_BINARY.exists() and CHANNELS_BINARY.stat().st_mtime >= CHANNELS_SOURCE.stat().st_mtime:
        return
    CHANNELS_BINARY.parent.mkdir(parents=True, exist_ok=True)
    flags = subprocess.check_output(["pkg-config", "--cflags", "--libs", "libpng"], text=True).split()
    subprocess.run(
        ["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", str(CHANNELS_SOURCE),
         "-o", str(CHANNELS_BINARY), *flags],
        check=True,
    )


def candidate_name(entry: dict) -> str:
    path = entry["source_vmt"]
    assert path.startswith("materials/") and path.endswith(".vmt")
    return path[len("materials/"):-len(".vmt")]


def target_scale(entry: dict, max_edge: int) -> int:
    base = entry["base"]
    scale = min(4, max_edge // max(base["width"], base["height"]))
    if scale not in (2, 4):
        raise ValueError(f"{entry['source_vmt']}: target size needs a scale below 2")
    return scale


def make_jobs(entries: list[dict], source: Path, max_edge: int) -> dict[tuple[str, int], dict]:
    jobs = {}
    for entry in entries:
        name = candidate_name(entry)
        scale = target_scale(entry, max_edge)
        key = (entry["base"]["source"], scale)
        original = source / "textures/portal_pbr" / name / "basecolor.png"
        if not original.is_file():
            raise FileNotFoundError(original)
        if key not in jobs:
            token = hashlib.sha256((key[0] + ":" + str(scale)).encode()).hexdigest()[:20]
            jobs[key] = {"original": original, "scale": scale, "token": token}
    return jobs


def run_model(jobs: dict, out: Path, binary: Path, model_dir: Path) -> None:
    for scale in (4, 2):
        group = [job for job in jobs.values() if job["scale"] == scale]
        if not group:
            continue
        work = out / "model-work" / str(scale)
        inputs, outputs = work / "input", work / "output"
        inputs.mkdir(parents=True, exist_ok=True)
        outputs.mkdir(parents=True, exist_ok=True)
        for job in group:
            job["model_file"] = outputs / (job["token"] + ".png")
        pending = [job for job in group if not job["model_file"].is_file()]
        for old in inputs.glob("*.png"):
            old.unlink()
        if pending:
            for job in pending:
                target = inputs / (job["token"] + ".png")
                try:
                    os.link(job["original"], target)
                except OSError:
                    shutil.copyfile(job["original"], target)
            print(f"Upscaling {len(pending)} distinct base images at {scale}x", flush=True)
            started = time.monotonic()
            subprocess.run(
                [str(binary), "-i", str(inputs), "-o", str(outputs),
                 "-n", "realesrgan-x4plus", "-s", str(scale), "-m", str(model_dir),
                 "-t", "128", "-f", "png"],
                check=True,
            )
            print(f"Model pass {scale}x finished in {time.monotonic() - started:.1f}s", flush=True)
        missing = [job["token"] for job in group if not job["model_file"].is_file()]
        if missing:
            raise RuntimeError(f"upscaler missed {len(missing)} inputs: {missing[:5]}")


def image_info(path: Path) -> dict:
    with Image.open(path) as image:
        image.load()
        return {"width": image.width, "height": image.height, "mode": image.mode}


def copy_material(source: Path, out: Path, name: str, family: str) -> Path:
    relative = Path("materials") / family / (name + ".vmt")
    original, target = source / relative, out / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(original, target)
    return target


def convert_set(entry: dict, source: Path, out: Path, job: dict) -> dict:
    name = candidate_name(entry)
    texture = out / "textures/portal_pbr" / name
    texture.mkdir(parents=True, exist_ok=True)
    original = source / "textures/portal_pbr" / name
    scale = job["scale"]
    width, height = entry["base"]["width"] * scale, entry["base"]["height"] * scale
    model_info = image_info(job["model_file"])
    if (model_info["width"], model_info["height"]) != (width, height):
        raise ValueError(f"{name}: unexpected model dimensions {model_info}")

    base = texture / "basecolor.png"
    alpha_mode = "onebit" if entry["base"]["flags"] & 0x1000 else "linear"
    subprocess.run(
        [str(CHANNELS_BINARY), "compose-base", str(original / "basecolor.png"),
         str(job["model_file"]), str(base), alpha_mode],
        check=True,
    )
    files = {"basecolor": str(base)}
    for channel in ("normal", "mrao"):
        old = original / (channel + ".png")
        if channel == "normal" and "normal" not in entry:
            continue
        if not old.is_file():
            raise FileNotFoundError(old)
        new = texture / (channel + ".png")
        subprocess.run(
            [str(CHANNELS_BINARY), channel, str(old), str(new), str(width), str(height)],
            check=True,
        )
        files[channel] = str(new)

    candidate = copy_material(source, out, name, "portal_pbr")
    fallback = copy_material(source, out, name, "portal_pbr_fallback")
    return {
        "name": name, "source_vmt": entry["source_vmt"], "scale": scale,
        "source_dimensions": [entry["base"]["width"], entry["base"]["height"]],
        "output_dimensions": [width, height], "alpha_method": alpha_mode,
        "textures": files, "candidate": str(candidate), "fallback": str(fallback),
        "source_review": entry["review"],
        "source_base_sha256": digest(original / "basecolor.png"),
        "output_sha256": {channel: digest(Path(path)) for channel, path in files.items()},
        "status": "remastered_requires_review_and_runtime_integration",
    }


def verify_set(entry: dict, source: Path) -> list[str]:
    """Read pixels for QA; all production image writes happen in the C++ helper."""
    problems = []
    name = entry["name"]
    original = source / "textures/portal_pbr" / name
    width, height = entry["output_dimensions"]
    scale = entry["scale"]
    with Image.open(original / "basecolor.png") as old, Image.open(entry["textures"]["basecolor"]) as new:
        if new.size != (width, height):
            problems.append("basecolor dimensions differ from requested target")
        old_alpha = old.convert("RGBA").getchannel("A")
        new_alpha = new.convert("RGBA").getchannel("A")
        # Check the source-preserving interpolation independently at texel centers.
        for y in range(old.height):
            for x in range(old.width):
                actual = new_alpha.getpixel((x * scale + scale // 2, y * scale + scale // 2))
                if entry["alpha_method"] == "onebit":
                    expected = 255 if old_alpha.getpixel((x, y)) >= 128 else 0
                else:
                    fraction = 0.5 / scale
                    right = min(x + 1, old.width - 1)
                    below = min(y + 1, old.height - 1)
                    a = old_alpha.getpixel((x, y))
                    b = old_alpha.getpixel((right, y))
                    c = old_alpha.getpixel((x, below))
                    d = old_alpha.getpixel((right, below))
                    expected = round((a * (1 - fraction) + b * fraction) * (1 - fraction)
                                     + (c * (1 - fraction) + d * fraction) * fraction)
                if abs(expected - actual) > 1:
                    problems.append("source alpha resampling differs from expected values")
                    break
            if problems:
                break
    for channel, path in entry["textures"].items():
        with Image.open(path) as image:
            if image.size != (width, height):
                problems.append(f"{channel} dimensions differ from basecolor")
            if channel == "normal":
                sample = image.convert("RGB")
                for y in range(0, height, max(1, height // 32)):
                    for x in range(0, width, max(1, width // 32)):
                        vector = [value / 127.5 - 1.0 for value in sample.getpixel((x, y))]
                        length = math.sqrt(sum(value * value for value in vector))
                        if abs(length - 1.0) > 0.02:
                            problems.append("tangent normal is not unit length")
                            break
                    if problems:
                        break
            if channel == "mrao":
                with Image.open(original / "mrao.png") as old_mrao:
                    source_mrao = old_mrao.convert("RGB")
                    output_mrao = image.convert("RGB")
                    for y in range(0, source_mrao.height, max(1, source_mrao.height // 16)):
                        for x in range(0, source_mrao.width, max(1, source_mrao.width // 16)):
                            sx, sy = x * scale + scale // 2, y * scale + scale // 2
                            source_rgb = source_mrao.getpixel((x, y))
                            output_rgb = output_mrao.getpixel((sx, sy))
                            if any(abs(a - b) > 64 for a, b in zip(source_rgb, output_rgb)):
                                problems.append("MRAO channels differ materially from source")
                                break
                        if problems:
                            break
    return problems


def run(args: argparse.Namespace) -> int:
    source, out = args.source.resolve(), args.out.resolve()
    if source == out or source in out.parents or out in source.parents:
        raise ValueError("source and output must be separate directories")
    manifest = json.loads((source / "manifest.json").read_text(encoding="utf-8"))
    if manifest["schema"] != "portal-pbr-staging-v1":
        raise ValueError("unsupported source manifest schema")
    entries = [entry for entry in manifest["materials"] if entry["candidate"]]
    if args.limit:
        entries = entries[:args.limit]
    if not entries:
        raise ValueError("source manifest contains no selected candidates")
    if not args.upscaler.is_file() or not args.model_dir.is_dir():
        raise FileNotFoundError("upscaler executable or model directory missing")
    out.mkdir(parents=True, exist_ok=True)
    build_channels()
    jobs = make_jobs(entries, source, args.max_edge)
    run_model(jobs, out, args.upscaler.resolve(), args.model_dir.resolve())

    sets = []
    failures = []
    paths = []
    for index, entry in enumerate(entries, start=1):
        name = candidate_name(entry)
        try:
            job = jobs[(entry["base"]["source"], target_scale(entry, args.max_edge))]
            result = convert_set(entry, source, out, job)
            result["qa_problems"] = verify_set(result, source)
            if result["qa_problems"]:
                failures.append({"name": name, "error": "; ".join(result["qa_problems"])})
            sets.append(result)
            paths.extend(Path(path) for path in result["textures"].values())
        except (OSError, ValueError, subprocess.CalledProcessError) as exc:
            failures.append({"name": name, "error": str(exc)})
        if index % 25 == 0 or index == len(entries):
            print(f"Converted and checked {index}/{len(entries)} material sets", flush=True)

    result = {
        "schema": "portal-pbr-remaster-v1", "source_manifest": str(source / "manifest.json"),
        "source_manifest_sha256": digest(source / "manifest.json"),
        "source_candidate_count": manifest["candidate_count"], "selected_count": len(entries),
        "remastered_count": len(sets), "texture_count": len(paths),
        "max_edge": args.max_edge, "scale_counts": {
            str(scale): sum(item["scale"] == scale for item in sets) for scale in (2, 4)
        },
        "upscaler": {
            "name": "Real-ESRGAN ncnn Vulkan", "model": "realesrgan-x4plus",
            "executable": str(args.upscaler.resolve()), "executable_sha256": digest(args.upscaler),
            "model_param_sha256": digest(args.model_dir / "realesrgan-x4plus.param"),
            "model_bin_sha256": digest(args.model_dir / "realesrgan-x4plus.bin"),
            "official_release_archive_sha256": EXPECTED_RELEASE_SHA256,
        },
        "channel_helper_sha256": digest(CHANNELS_BINARY),
        "sets": sets, "failures": failures, "engine_ready": False,
    }
    if args.import_armorpaint and not failures:
        imports, saved = import_to_armorpaint(
            paths, args.armorpaint_dir, args.armorpaint_project, args.new_armorpaint_project,
        )
        result["armorpaint_import_count"] = len(imports)
        result["armorpaint_import_failures"] = [item for item in imports if "error" in item]
        result["armorpaint_project"] = saved
        (out / "armorpaint-imports.json").write_text(
            json.dumps({"imports": imports, "project": saved}, indent=2) + "\n", encoding="utf-8",
        )
        if len(imports) != len(paths) or result["armorpaint_import_failures"] or not saved or not saved["verified"]:
            failures.append({"name": "ArmorPaint", "error": "import, save, or reopen verification failed"})
    (out / "manifest.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Remastered {len(sets)}/{len(entries)} sets and {len(paths)} textures; failures: {len(failures)}")
    print("Manifest:", out / "manifest.json")
    return 1 if failures else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True, help="Original Portal PBR staging output")
    parser.add_argument("--out", type=Path, required=True, help="Separate high-resolution output directory")
    parser.add_argument("--upscaler", type=Path, required=True, help="Pinned realesrgan-ncnn-vulkan executable")
    parser.add_argument("--model-dir", type=Path, required=True, help="Directory with realesrgan-x4plus model")
    parser.add_argument("--max-edge", type=int, default=4096, help="Maximum target edge; default 4096")
    parser.add_argument("--limit", type=int, default=0, help="Optional material count for a pilot run")
    parser.add_argument("--import-armorpaint", action="store_true", help="Import all high-resolution PNGs via MCP")
    parser.add_argument("--new-armorpaint-project", action="store_true", help="Start a fresh ArmorPaint project")
    parser.add_argument("--armorpaint-project", type=Path, help="Save and reopen the ArmorPaint project here")
    parser.add_argument("--armorpaint-dir", type=Path,
                        default=ROOT / "thirdparty/armorpaint/paint/build/out")
    args = parser.parse_args()
    if args.max_edge < 2 or args.max_edge > 4096:
        parser.error("max-edge must be between 2 and 4096")
    if (args.armorpaint_project or args.new_armorpaint_project) and not args.import_armorpaint:
        parser.error("ArmorPaint project options require --import-armorpaint")
    if args.import_armorpaint and not args.armorpaint_project:
        parser.error("--import-armorpaint requires --armorpaint-project for verification")
    return run(args)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"remaster: {error}", file=sys.stderr)
        raise SystemExit(1) from error
