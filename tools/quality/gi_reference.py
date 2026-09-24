#!/usr/bin/env python3
"""Cycles total and indirect-only references for the RFC 0011 GI fixtures.

    python3 tools/quality/gi_reference.py render [--fixture NAME]... [--samples N]
    python3 tools/quality/gi_reference.py check [--fixture NAME]...

`render` extracts every state of each fixture (`quality/fixtures/gi/<name>/
fixture.json`) through `usd_scene.py extract`, the map pipeline's own USD
front end, renders each fixture camera in Cycles with separated light passes
(`gi_reference_blender.py`), and writes the versioned references beside the
fixture:

    references/<state>.<camera>.total.exr     Combined (linear scene radiance)
    references/<state>.<camera>.indirect.exr  DiffInd: indirect diffuse light
                                              (irradiance / pi, no albedo)
    references/<state>.<camera>.index.png     object index (region masks)
    references/references.json                Blender/Cycles build, device,
        samples, seed, light paths, composed-stage digests, image hashes and
        per-region means of total, indirect, direct light and albedo

Light units: Cycles' diffuse light passes (DiffDir, DiffInd) are the light a
white Lambertian surface would return, E / pi. The map pipeline's lightmap
bake (DIFFUSE without COLOR) and the runtime indirect debug view use the same
unit, so a region comparison needs no conversion.

`check` validates the checked-in references against their fixture
(declared states and cameras present, stage digests current, image hashes)
and applies the closed forms: the furnace must reach L = Le / (1 - rho), and
the probe-grid floor its analytic sky and sun radiance. `render` also renders
the furnace with Blender's default light paths (4 diffuse bounces) and
records that the analytic check rejects it, proving the oracle can fail.
"""

import argparse
import hashlib
import json
import math
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import map_scene  # noqa: E402
import pbrt_map_toolchain  # noqa: E402

FIXTURES = ROOT / "quality/fixtures/gi"
WORK = ROOT / "quality-results/rfc0011-references"
SCHEMA = "gi-references/v1"
DEFAULT_SAMPLES = 2048
DEFAULT_SEED = 20260924
REGION_EROSION = 2
# Relative tolerance for the Cycles references against closed forms. The
# estimator is unbiased; at 2048 samples a region mean is well inside 1%.
ANALYTIC_TOLERANCE = 0.01
PASS_NAMES = {"total": "Combined", "indirect": "DiffInd", "direct": "DiffDir",
              "albedo": "DiffCol", "emission": "Emit", "environment": "Env",
              "glossy_direct": "GlossDir", "glossy_indirect": "GlossInd",
              "index": "IndexOB"}
# Blender 5 writes long pass names; Blender 4 the short ones above.
LONG_PASS_NAMES = {"Diffuse Direct": "DiffDir", "Diffuse Indirect": "DiffInd",
                   "Diffuse Color": "DiffCol", "Emission": "Emit", "Environment": "Env",
                   "Glossy Direct": "GlossDir", "Glossy Indirect": "GlossInd",
                   "Object Index": "IndexOB", "Depth": "Depth", "Combined": "Combined"}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


# ------------------------------------------------------------------ EXR

def read_exr(path):
    """Channels of an uncompressed scanline OpenEXR file: {name: (H, W) float32}.

    Blender writes the multilayer render with `exr_codec = NONE`, as one part
    (Blender 4) or one part per pass (Blender 5). This reader accepts only
    uncompressed scanline parts and rejects tiled, deep or compressed data
    rather than guessing. Multipart channels are named "<part>.<channel>".
    """
    data = Path(path).read_bytes()
    if data[:4] != b"\x76\x2f\x31\x01":
        raise ValueError("%s is not an OpenEXR file" % path)
    version = struct.unpack_from("<I", data, 4)[0]
    multipart = bool(version & 0x1000)
    # Tiled (0x200) and deep (0x800) data are rejected; long names (0x400) are fine.
    if version & 0xFF != 2 or version & 0xA00:
        raise ValueError("%s: only scanline EXR is supported" % path)
    pos = 8

    def read_header():
        nonlocal pos
        header = {}
        while data[pos] != 0:
            end = data.index(b"\0", pos)
            name = data[pos:end].decode()
            pos = end + 1
            end = data.index(b"\0", pos)
            kind = data[pos:end].decode()
            pos = end + 1
            size = struct.unpack_from("<i", data, pos)[0]
            pos += 4
            header[name] = (kind, data[pos:pos + size])
            pos += size
        pos += 1
        return header
    headers = [read_header()]
    if multipart:
        while data[pos] != 0:
            headers.append(read_header())
        pos += 1
    parts = []
    for header in headers:
        if header["compression"][1] != b"\x00":
            raise ValueError("%s: compressed EXR is not supported" % path)
        if multipart and header.get("type", ("", b"scanlineimage\0"))[1].rstrip(b"\0") != \
                b"scanlineimage":
            raise ValueError("%s: only scanline parts are supported" % path)
        channels = []
        raw = header["channels"][1]
        offset = 0
        while raw[offset] != 0:
            end = raw.index(b"\0", offset)
            name = raw[offset:end].decode()
            pixel_type = struct.unpack_from("<i", raw, end + 1)[0]
            channels.append((name, pixel_type))
            offset = end + 1 + 16
        xmin, ymin, xmax, ymax = struct.unpack("<iiii", header["dataWindow"][1])
        height = ymax - ymin + 1
        chunks = struct.unpack_from("<i", header["chunkCount"][1])[0] if multipart else height
        table = struct.unpack_from("<%dQ" % chunks, data, pos)
        pos += 8 * chunks
        part_name = header["name"][1].rstrip(b"\0").decode() if "name" in header else None
        parts.append((part_name, channels, (xmin, ymin, xmax, ymax), table))
    sizes = {0: 4, 1: 2, 2: 4}  # UINT, HALF, FLOAT
    dtypes = {0: np.uint32, 1: np.float16, 2: np.float32}
    out = {}
    for number, (part_name, channels, (xmin, ymin, xmax, ymax), table) in enumerate(parts):
        width, height = xmax - xmin + 1, ymax - ymin + 1
        names = [(part_name + "." + name) if part_name else name for name, _ in channels]
        for name in names:
            out[name] = np.zeros((height, width), np.float32)
        for chunk_offset in table:
            cursor = chunk_offset
            if multipart:
                if struct.unpack_from("<i", data, cursor)[0] != number:
                    raise ValueError("%s: chunk belongs to another part" % path)
                cursor += 4
            y, size = struct.unpack_from("<ii", data, cursor)
            cursor += 8
            for name, (_, pixel_type) in zip(names, channels):
                out[name][y - ymin] = np.frombuffer(data, dtypes[pixel_type], width, cursor)
                cursor += width * sizes[pixel_type]
    return out


def render_passes(path):
    """{pass name: (H, W, C)} from a Blender multilayer render."""
    channels = read_exr(path)
    passes = {}
    for name, values in channels.items():
        parts = name.split(".")
        if len(parts) < 2:
            continue
        passes.setdefault(parts[-2], {})[parts[-1]] = values
    ordered = {}
    for name, components in passes.items():
        keys = [k for k in ("R", "G", "B") if k in components] or sorted(components)
        ordered[LONG_PASS_NAMES.get(name, name)] = np.stack([components[k] for k in keys],
                                                            axis=-1)
    return ordered


# ------------------------------------------------------------------ fixtures

def load_fixture(name):
    path = FIXTURES / name / "fixture.json"
    fixture = json.loads(path.read_text())
    if fixture.get("schema") != "gi-fixture/v1" or fixture.get("name") != name:
        raise ValueError("invalid GI fixture " + str(path))
    fixture["directory"] = path.parent
    return fixture


def fixture_names():
    return json.loads((FIXTURES / "index.json").read_text())["fixtures"]


def state_stage(fixture, state):
    layer = fixture["states"][state]["layer"]
    return fixture["directory"] / (layer or fixture["stage"])


def region_masks(index_image, object_index, regions, scene=None):
    """{region: bool mask} from object-index pixels and mesh-name regions.

    A region entry names a mesh, or a dynamic model (its stand-in meshes)."""
    from scipy.ndimage import binary_erosion
    props = {prop["name"]: prop["shapes"] for prop in map_scene.props(scene or {})}
    masks = {}
    for region, entries in regions.items():
        meshes = [name for entry in entries for name in props.get(entry, [entry])]
        numbers = [object_index[name] for name in meshes if name in object_index]
        missing = [name for name in meshes if name not in object_index]
        if missing:
            raise ValueError("region %s names unknown meshes: %s" % (region, ", ".join(missing)))
        mask = np.isin(index_image, numbers)
        masks[region] = binary_erosion(mask, iterations=REGION_EROSION)
    return masks


def region_stats(passes, masks):
    stats = {}
    for region, mask in masks.items():
        count = int(mask.sum())
        entry = {"pixels": count}
        for key, name in PASS_NAMES.items():
            if name in passes and count:
                entry[key] = [float(v) for v in passes[name][mask].mean(axis=0)[:3]]
        stats[region] = entry
    return stats


def luminance(rgb):
    return 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2]


# ------------------------------------------------------------------ analytic

def analytic_checks(fixture, regions_by_view):
    """[(name, expected, observed, ok)] for the closed forms a fixture declares."""
    checks = []
    analytic = fixture.get("analytic")
    if not analytic:
        return checks

    def check(name, expected, observed):
        ok = abs(observed - expected) <= ANALYTIC_TOLERANCE * max(abs(expected), 1e-3)
        checks.append({"check": name, "expected": expected, "observed": observed, "ok": ok})
    if fixture["name"] == "furnace":
        walls = regions_by_view[("default", "inside")]["walls"]
        check("furnace walls total radiance = Le / (1 - rho)", analytic["radiance"],
              luminance(walls["total"]))
        check("furnace walls direct light = Le", analytic["diffuse_light_direct"],
              luminance(walls["direct"]))
        check("furnace walls indirect light = rho Le / (1 - rho)",
              analytic["diffuse_light_indirect"], luminance(walls["indirect"]))
    elif fixture["name"] == "probe-grid":
        rho = analytic["albedo"]
        sky = regions_by_view[("sky", "floor")]["floor"]
        check("probe-grid sky: floor radiance = rho * sky", rho * analytic["sky_radiance"],
              luminance(sky["total"]))
        sun = analytic["sun"]
        sun_floor = regions_by_view[("sun", "floor")]["floor"]
        expected = rho * sun["irradiance"] * math.sin(math.radians(sun["elevation_degrees"])) \
            / math.pi
        check("probe-grid sun: floor radiance = rho E sin(e) / pi", expected,
              luminance(sun_floor["total"]))
    return checks


# ------------------------------------------------------------------ render

class Tools:
    def __init__(self, toolchain_path=None):
        profile, _ = pbrt_map_toolchain.load_profiles()
        self.values = pbrt_map_toolchain.load(
            toolchain_path or ROOT / profile["layout"]["toolchain_file"])

    def usd_python(self, script, arguments, log):
        env = dict(os.environ, PYTHONPATH=self.values["usd_pythonpath"],
                   PXR_PLUGINPATH_NAME=str(Path(self.values["compile_tools"]) /
                                           "share/sourceWorld"))
        run([self.values["usd_python"], HERE / script] + arguments, env, log)

    def blender(self, script, arguments, log):
        env = dict(os.environ, OCIO=str(ROOT / self.values["ocio"])
                   if not os.path.isabs(self.values["ocio"]) else self.values["ocio"])
        run([self.values["blender"], "-b", "--factory-startup", "--python-exit-code", "9",
             "--python", HERE / script, "--"] + arguments, env, log)


def run(command, env, log):
    log.parent.mkdir(parents=True, exist_ok=True)
    with log.open("a") as stream:
        stream.write("$ " + " ".join(map(str, command)) + "\n")
        stream.flush()
        result = subprocess.run([str(c) for c in command], stdout=stream,
                                stderr=subprocess.STDOUT, env=env, cwd=ROOT)
    if result.returncode:
        tail = log.read_text().splitlines()[-20:]
        raise SystemExit("command failed (%d); log %s:\n  %s" %
                         (result.returncode, log, "\n  ".join(tail)))


def render_state(tools, fixture, state, work, samples, seed, device, light_paths):
    """Render every camera of one state; returns (render receipt, scene model)."""
    directory = work / fixture["name"] / state
    if light_paths != "gi-reference":
        directory = directory.with_name(state + "." + light_paths)
    if directory.exists():
        shutil.rmtree(directory)
    directory.mkdir(parents=True)
    log = directory / "log.txt"
    scene_path, stage_path = directory / "scene.json", directory / "stage.usdc"
    tools.usd_python("usd_scene.py", ["extract", "--scene", state_stage(fixture, state),
                                      "--out-scene", scene_path, "--out-stage", stage_path],
                     log)
    scene = map_scene.parse(scene_path)
    arguments = ["--scene", scene_path, "--stage", stage_path]
    if scene["environment"]:
        import imageio.v3 as iio
        environment = directory / "environment.exr"
        iio.imwrite(environment, map_scene.environment_equirect(scene, 2048).astype(np.float32))
        arguments += ["--environment", environment]
    cameras = directory / "cameras.json"
    cameras.write_text(json.dumps({"film": fixture["film"],
                                   "horizontal_fov_degrees": fixture["horizontal_fov_degrees"],
                                   "cameras": fixture["cameras"]}, indent=2, sort_keys=True))
    tools.blender("gi_reference_blender.py", arguments + [
        "--cameras", cameras, "--out-dir", directory / "render", "--samples", str(samples),
        "--seed", str(seed), "--device", device, "--light-paths", light_paths], log)
    receipt = json.loads((directory / "render" / "render.json").read_text())
    return receipt, scene, directory / "render"


def write_references(fixture, results):
    """Write the versioned reference images and references.json."""
    import imageio.v3 as iio
    from PIL import Image
    references = fixture["directory"] / "references"
    if references.exists():
        shutil.rmtree(references)
    references.mkdir()
    views = {}
    regions_by_view = {}
    renders = {}
    for state, (receipt, scene, render_dir) in sorted(results.items()):
        renders[state] = {key: receipt[key] for key in (
            "blender", "cycles_device", "samples", "seed", "denoising", "light_paths",
            "renderer_sha256")}
        renders[state]["composed_stage_sha256"] = scene["source_sha256"]
        renders[state]["stage_layers"] = {
            str(Path(path).relative_to(ROOT)): sha256(path)
            for path in scene["source_files"] if Path(path).is_file() and
            Path(path).suffix.startswith(".usd")}
        for camera, output in sorted(receipt["renders"].items()):
            passes = render_passes(render_dir / output["exr"])
            index_image = np.rint(passes["IndexOB"][..., 0]).astype(np.int32)
            masks = region_masks(index_image, receipt["object_index"],
                                 fixture["regions"][camera], scene)
            stats = region_stats(passes, masks)
            regions_by_view[(state, camera)] = stats
            stem = "%s.%s" % (state, camera)
            files = {}
            for key, name in (("total", "Combined"), ("indirect", "DiffInd")):
                path = references / ("%s.%s.exr" % (stem, key))
                iio.imwrite(path, passes[name][..., :3].astype(np.float32))
                files[key] = {"file": path.name, "sha256": sha256(path)}
            if index_image.max() > 255:
                raise ValueError("more than 255 objects in a region index")
            index_path = references / ("%s.index.png" % stem)
            Image.fromarray(index_image.astype(np.uint8)).save(index_path, optimize=False)
            files["index"] = {"file": index_path.name, "sha256": sha256(index_path)}
            views[stem] = {"state": state, "camera": camera, "files": files,
                           "object_index": receipt["object_index"], "regions": stats,
                           "glossy_max": float(max(passes["GlossDir"].max(),
                                                   passes["GlossInd"].max()))}
    checks = analytic_checks(fixture, regions_by_view)
    record = {"schema": SCHEMA, "fixture": fixture["name"],
              "fixture_sha256": sha256(fixture["directory"] / "fixture.json"),
              "light_units": "diffuse light passes are E / pi (a white Lambertian's return)",
              "region_erosion_pixels": REGION_EROSION, "renders": renders, "views": views,
              "analytic": checks,
              "status": "pass" if all(c["ok"] for c in checks) else "fail"}
    (references / "references.json").write_text(json.dumps(record, indent=2, sort_keys=True) +
                                                "\n")
    return record


def cmd_render(args):
    tools = Tools(args.toolchain)
    status = 0
    for name in args.fixture or fixture_names():
        fixture = load_fixture(name)
        results = {}
        for state in sorted(fixture["states"]):
            print("[%s/%s] rendering..." % (name, state), flush=True)
            results[state] = render_state(tools, fixture, state, args.work, args.samples,
                                          args.seed, args.device, "gi-reference")
        record = write_references(fixture, results)
        if name == "furnace":
            record["negative_controls"] = furnace_negative_control(
                tools, fixture, args.work, args.samples, args.seed, args.device)
            (fixture["directory"] / "references" / "references.json").write_text(
                json.dumps(record, indent=2, sort_keys=True) + "\n")
            if not all(control["rejected"] for control in record["negative_controls"]):
                record["status"] = "fail"
        print("[%s] %s: %s" % (name, record["status"], "; ".join(
            "%s %.4f vs %.4f" % (c["check"], c["observed"], c["expected"])
            for c in record["analytic"]) or "no closed form"), flush=True)
        status |= record["status"] != "pass"
    return status


def furnace_negative_control(tools, fixture, work, samples, seed, device):
    """Blender's default 4 diffuse bounces must fail the furnace closed form."""
    receipt, scene, render_dir = render_state(tools, fixture, "default", work, samples, seed,
                                              device, "blender-default")
    passes = render_passes(render_dir / receipt["renders"]["inside"]["exr"])
    index_image = np.rint(passes["IndexOB"][..., 0]).astype(np.int32)
    masks = region_masks(index_image, receipt["object_index"], fixture["regions"]["inside"],
                         scene)
    stats = region_stats(passes, masks)
    checks = analytic_checks(fixture, {("default", "inside"): stats})
    return [{"control": "blender-default light paths (4 diffuse bounces)",
             "light_paths": receipt["light_paths"], "checks": checks,
             "rejected": not all(c["ok"] for c in checks)}]


# ------------------------------------------------------------------ check

def cmd_check(args):
    failures = []
    for name in args.fixture or fixture_names():
        fixture = load_fixture(name)
        path = fixture["directory"] / "references" / "references.json"
        if not path.is_file():
            failures.append("%s: no references (run gi_reference.py render)" % name)
            continue
        record = json.loads(path.read_text())
        if record.get("schema") != SCHEMA:
            failures.append("%s: references schema" % name)
            continue
        if record["fixture_sha256"] != sha256(fixture["directory"] / "fixture.json"):
            failures.append("%s: fixture.json changed since the references were rendered" % name)
        for state in fixture["states"]:
            render = record["renders"].get(state)
            if not render:
                failures.append("%s: state %s has no reference" % (name, state))
                continue
            for layer, digest in render["stage_layers"].items():
                if not (ROOT / layer).is_file() or sha256(ROOT / layer) != digest:
                    failures.append("%s/%s: stage layer %s changed" % (name, state, layer))
            for camera in fixture["cameras"]:
                view = record["views"].get("%s.%s" % (state, camera))
                if not view:
                    failures.append("%s: %s.%s missing" % (name, state, camera))
                    continue
                for entry in view["files"].values():
                    file = path.parent / entry["file"]
                    if not file.is_file() or sha256(file) != entry["sha256"]:
                        failures.append("%s: %s differs from its record" % (name, entry["file"]))
                for region, stats in view["regions"].items():
                    if stats["pixels"] < 50:
                        failures.append("%s: %s.%s region %s has %d pixels" %
                                        (name, state, camera, region, stats["pixels"]))
                if view["glossy_max"] > 1e-4:
                    failures.append("%s: %s.%s has a glossy lobe (%g); fixtures are "
                                    "Lambertian" % (name, state, camera, view["glossy_max"]))
        for check in record["analytic"]:
            if not check["ok"]:
                failures.append("%s: %s observed %.4f expected %.4f" %
                                (name, check["check"], check["observed"], check["expected"]))
        for control in record.get("negative_controls", []):
            if not control["rejected"]:
                failures.append("%s: negative control not rejected: %s" %
                                (name, control["control"]))
        if name == "furnace" and not record.get("negative_controls"):
            failures.append("furnace: no negative control recorded")
    for failure in failures:
        print("FAIL " + failure)
    print("GI references: %s" % ("FAIL" if failures else "pass"))
    return 1 if failures else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    render = commands.add_parser("render")
    render.add_argument("--fixture", action="append")
    render.add_argument("--samples", type=int, default=DEFAULT_SAMPLES)
    render.add_argument("--seed", type=int, default=DEFAULT_SEED)
    render.add_argument("--device", choices=("cpu", "gpu", "auto"), default="auto")
    render.add_argument("--work", type=Path, default=WORK)
    render.add_argument("--toolchain", type=Path)
    check = commands.add_parser("check")
    check.add_argument("--fixture", action="append")
    args = parser.parse_args()
    return cmd_render(args) if args.command == "render" else cmd_check(args)


if __name__ == "__main__":
    sys.exit(main())
