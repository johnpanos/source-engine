#!/usr/bin/env python3
"""Remaster Portal materials into high-resolution PBR sets with ArmorPaint via MCP.

Each material has a recipe in `recipes/` that records decisions only (intent,
metal/roughness/occlusion mapping, de-lighting). Facts come from the VPK: the
VMT, its DirectX 9 fallback block, and the decoded VTF layers. The pipeline is

  resolve  VMT + DX9 block -> base, bump, SSBump flag and specular-mask source
  prep     ESRGAN colour (wrap-padded so tiling survives), a tileable height
           field integrated from the Source normal/SSBump, cavity/spec/height
           masks, and removal of lighting baked into the colour
  author   an ArmorPaint project built over the MCP server: plane mesh,
           imported inputs, a named node graph, layer fill and texture export
  package  basecolor (sRGB), MRAO (linear R metal, G roughness, B AO) and an
           OpenGL (+Y) tangent normal; a PBRMetalRough candidate VMT, the
           original VMT as its fallback, schema validation, optional VTFs
  qa       size, tiling, normal convention and tilt, channel ranges, and
           agreement between the exported normal and the prepared input

Output stays outside the repository. `--stage inspect` writes the source
layers and resolved facts that a new recipe is written from.
"""

from __future__ import annotations

import argparse
import contextlib
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

import numpy as np
from PIL import Image, ImageDraw
from scipy.ndimage import gaussian_filter

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "quality"))
from workflow import HELPER, ROOT, build_helper, get_image, make_material, valid_name  # noqa: E402
from vtf_content import compile_texture  # noqa: E402

RECIPE_SCHEMA = "portal-pbr-armorpaint-recipe-v1"
MANIFEST_SCHEMA = "portal-pbr-armorpaint-remaster-v1"
MCP_DIR = ROOT / "thirdparty/armorpaint-mcp"
MCP_PYTHON = MCP_DIR / ".venv/bin/python"
PLAN_RUNNER = ROOT / "tools/portal_pbr/armorpaint_mcp_plan.py"
LAYER_RES = {2048: 0, 4096: 1, 8192: 2}
WRAP_PAD = 64
SEAM_RATIO_LIMIT = 1.0
SEAM_SOURCE_MARGIN = 1.25
TILT_LIMIT = 0.02
# Source SSBump basis vectors (tangent space, DirectX +Y down).
SSBUMP_BASIS = np.array([
    [np.sqrt(2 / 3), 0.0, 1 / np.sqrt(3)],
    [-1 / np.sqrt(6), 1 / np.sqrt(2), 1 / np.sqrt(3)],
    [-1 / np.sqrt(6), -1 / np.sqrt(2), 1 / np.sqrt(3)],
])


# --------------------------------------------------------------------------- recipes

def load_recipe(path: Path) -> dict:
    recipe = json.loads(path.read_text(encoding="utf-8"))
    problems = []
    if recipe.get("schema") != RECIPE_SCHEMA:
        problems.append("schema must be " + RECIPE_SCHEMA)
    if not valid_name(recipe.get("material", "")):
        problems.append("material must be a relative material name")
    if recipe.get("size") not in LAYER_RES:
        problems.append("size must be one of %s" % sorted(LAYER_RES))
    cavity = recipe.get("cavity", {}).get("source")
    if cavity not in ("base_alpha", "height", "none"):
        problems.append("cavity.source must be base_alpha, height or none")
    metal = recipe.get("metal", {})
    if not ("constant" in metal) ^ ("cavity_range" in metal):
        problems.append("metal needs exactly one of constant or cavity_range")
    if "cavity_range" in metal and cavity == "none":
        problems.append("metal.cavity_range needs a cavity source")
    rough = recipe.get("roughness", {})
    if not ("constant" in rough) ^ ("from_spec" in rough):
        problems.append("roughness needs exactly one of constant or from_spec")
    for key in ("seam", "noise_scale", "noise_amp"):
        if not isinstance(rough.get(key), (int, float)):
            problems.append("roughness." + key + " must be a number")
    if len(rough.get("seam_range", [])) != 2 or rough["seam_range"][0] >= rough["seam_range"][1]:
        problems.append("roughness.seam_range must be [lo, hi] on the cavity mask")
    occlusion = recipe.get("occlusion", {})
    if len(occlusion.get("from", [])) != 2 or len(occlusion.get("to", [])) != 2:
        problems.append("occlusion needs from and to ranges")
    f0 = recipe.get("f0", {})
    if not isinstance(f0.get("value_gain"), (int, float)) or not isinstance(f0.get("saturation"), (int, float)):
        problems.append("f0 needs value_gain and saturation")
    delight = recipe.get("delight", {})
    if not 0 <= delight.get("strength", -1) <= 1 or delight.get("blur_px", -1) < 0:
        problems.append("delight needs strength in [0, 1] and blur_px >= 0")
    if problems:
        raise ValueError("%s: %s" % (path, "; ".join(problems)))
    return recipe


# --------------------------------------------------------------------------- resolve

def resolve_material(vpks: list[Path], name: str, original_vmt: Path) -> dict:
    """Top-level VMT parameters overlaid by the DirectX 9 fallback block."""
    for vpk in vpks:
        result = subprocess.run(
            [str(HELPER), "material", str(vpk), "materials/" + name + ".vmt", str(original_vmt)],
            text=True, capture_output=True)
        if result.returncode == 4:
            continue
        if result.returncode:
            raise ValueError(name + ": " + (result.stderr.strip() or "VMT read failed"))
        row = json.loads(result.stdout)
        if "error" in row:
            raise ValueError(name + ": " + row["error"])
        params = {k.lower(): v for k, v in row["parameters"]}
        params.update({k.lower(): v for k, v in row["dx9_parameters"]})
        return {"vpk": str(vpk), "shader": row["shader"], "dx9_block": row["dx9_block"],
                "proxies": row["proxies"], "params": params}
    raise ValueError(name + ": VMT missing from supplied archives")


def truthy(value: str | None) -> bool:
    return value is not None and value.strip().lower() not in ("", "0", "false", "no")


def texture_name(value: str) -> str:
    return value.replace("\\", "/").lower()


def material_facts(resolved: dict) -> dict:
    params = resolved["params"]
    if "$basetexture" not in params:
        raise ValueError("material has no $basetexture")
    unsupported = sorted(k for k in ("$additive", "$translucent", "$alphatest", "$selfillum",
                                     "$envmapmask", "$basetexture2") if truthy(params.get(k)))
    if unsupported or resolved["proxies"]:
        raise ValueError("needs review before remastering: %s" %
                         ", ".join(unsupported + (["proxies"] if resolved["proxies"] else [])))
    spec = None
    if truthy(params.get("$normalmapalphaenvmapmask")) and "$bumpmap" in params:
        spec = "bump_alpha"
    elif truthy(params.get("$basealphaenvmapmask")):
        spec = "base_alpha"
    return {
        "base": texture_name(params["$basetexture"]),
        "bump": texture_name(params["$bumpmap"]) if "$bumpmap" in params else None,
        "ssbump": truthy(params.get("$ssbump")),
        "spec_mask": spec,
        "envmap": "$envmap" in params,
        "envmaptint": params.get("$envmaptint"),
        "surfaceprop": params.get("$surfaceprop"),
        "detail": params.get("$detail"),
    }


# --------------------------------------------------------------------------- image helpers

def load(path: Path) -> np.ndarray:
    return np.asarray(Image.open(path)).astype(np.float64) / 255.0


def save8(path: Path, array: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(np.clip(np.round(array * 255), 0, 255).astype(np.uint8)).save(path)


def srgb_to_linear(x):
    return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4)


def linear_to_srgb(x):
    return np.where(x <= 0.0031308, x * 12.92, 1.055 * np.power(np.maximum(x, 0), 1 / 2.4) - 0.055)


def upscale_float(x: np.ndarray, size: int, pad: int = 16) -> np.ndarray:
    """Wrap-padded bicubic resize, so a tiling texture stays seamless."""
    edge = x.shape[0]
    padded = np.pad(x, pad, mode="wrap").astype(np.float32)
    image = Image.fromarray(padded, mode="F").resize(((edge + 2 * pad) * size // edge,) * 2,
                                                     Image.BICUBIC)
    k = pad * size // edge
    return np.asarray(image)[k:k + size, k:k + size].astype(np.float64)


def poisson_height(gx: np.ndarray, grow: np.ndarray) -> np.ndarray:
    """Tileable least-squares height from d/dx and d/drow, in pixels."""
    rows, cols = gx.shape
    fx = np.fft.fftfreq(cols)[None, :]
    fy = np.fft.fftfreq(rows)[:, None]
    denominator = 4 * np.pi ** 2 * (fx ** 2 + fy ** 2)
    denominator[0, 0] = 1
    spectrum = (-2j * np.pi * fx * np.fft.fft2(gx) - 2j * np.pi * fy * np.fft.fft2(grow)) / denominator
    spectrum[0, 0] = 0
    return np.real(np.fft.ifft2(spectrum))


def gradients(h: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    return ((np.roll(h, -1, 1) - np.roll(h, 1, 1)) / 2, (np.roll(h, -1, 0) - np.roll(h, 1, 0)) / 2)


def decode_normal(rgb: np.ndarray, ssbump: bool) -> np.ndarray:
    n = rgb @ SSBUMP_BASIS if ssbump else rgb * 2 - 1
    return n / np.maximum(np.linalg.norm(n, axis=-1, keepdims=True), 1e-6)


def convention_residuals(n: np.ndarray) -> dict:
    """Integrability of a tangent normal field under each green convention.

    A real surface is curl-free, so the convention that integrates with the
    smaller residual is the one the map was authored in.
    """
    out = {}
    for name, sign in (("opengl", 1.0), ("directx", -1.0)):
        nz = np.maximum(n[..., 2], 1e-3)
        gx, grow = -n[..., 0] / nz, sign * n[..., 1] / nz
        ex, erow = gradients(poisson_height(gx, grow))
        energy = (gx ** 2 + grow ** 2).mean()
        out[name] = float(np.sqrt(((ex - gx) ** 2 + (erow - grow) ** 2).mean() / max(energy, 1e-12)))
    return out


def seam_ratio(image: np.ndarray) -> float:
    """Wrap-edge step relative to the largest interior column/row step.

    Tiling textures often put a real feature (a panel groove) on the edge, so the
    wrap step is compared with the strongest interior step, not the average.
    Above 1.0 the edge is a discontinuity no interior column or row has.
    """
    a = image.astype(np.float64)
    a = a if a.ndim == 3 else a[..., None]
    cols = np.abs(np.diff(a, axis=1)).mean(axis=(0, 2))
    rows = np.abs(np.diff(a, axis=0)).mean(axis=(1, 2))
    col = np.abs(a[:, 0] - a[:, -1]).mean() / max(cols.max(), 1e-6)
    row = np.abs(a[0] - a[-1]).mean() / max(rows.max(), 1e-6)
    return float(max(col, row))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


# --------------------------------------------------------------------------- prep

def upscale_base(source_png: Path, size: int, work: Path, upscaler: Path, models: Path) -> Path:
    rgb = np.asarray(Image.open(source_png).convert("RGB"))
    edge = rgb.shape[0]
    if rgb.shape[0] != rgb.shape[1]:
        raise ValueError("non-square base textures need a reviewed recipe")
    padded = np.pad(rgb, ((WRAP_PAD, WRAP_PAD), (WRAP_PAD, WRAP_PAD), (0, 0)), mode="wrap")
    padded_png, model_png = work / "base-wrap-pad.png", work / "base-wrap-x4.png"
    Image.fromarray(padded).save(padded_png)
    subprocess.run([str(upscaler), "-i", str(padded_png), "-o", str(model_png),
                    "-n", "realesrgan-x4plus", "-s", "4", "-m", str(models), "-f", "png"],
                   check=True, capture_output=True, timeout=900)
    up = np.asarray(Image.open(model_png).convert("RGB"))
    k = WRAP_PAD * 4
    image = Image.fromarray(up[k:k + edge * 4, k:k + edge * 4])
    if image.size[0] != size:
        image = image.resize((size, size), Image.LANCZOS)
    out = work / "base-esrgan.png"
    image.save(out)
    return out


def prepare(recipe: dict, facts: dict, layers: dict, work: Path, upscaler: Path, models: Path) -> dict:
    size = recipe["size"]
    base_rgba = load(layers["base"])
    edge = base_rgba.shape[0]
    report = {"source_edge": edge, "source_seam_ratio": {"basecolor": round(seam_ratio(base_rgba[..., :3]), 3)}}

    if layers.get("bump") is not None:
        bump = load(layers["bump"])
        if bump.shape[0] != edge:
            bump_rgb = upscale_float_rgb(bump[..., :3], edge)
        else:
            bump_rgb = bump[..., :3]
        n_src = decode_normal(bump_rgb, facts["ssbump"])
        report["source_seam_ratio"]["normal"] = round(seam_ratio(n_src), 3)
        report["source_convention_residual"] = convention_residuals(n_src)
        if report["source_convention_residual"]["directx"] > report["source_convention_residual"]["opengl"]:
            raise ValueError("source normal integrates better as OpenGL; review the recipe")
        nz = np.maximum(n_src[..., 2], 1e-3)
        h_src = poisson_height(-n_src[..., 0] / nz, -n_src[..., 1] / nz)     # DirectX: +Y down
        height = upscale_float(h_src, size) * (size / edge)                  # keep physical slope
    else:
        height = np.zeros((size, size))
    gx, grow = gradients(height)
    normal = np.dstack([-gx, -grow, np.ones_like(height)])
    normal /= np.linalg.norm(normal, axis=-1, keepdims=True)

    span = height.max() - height.min()
    height01 = (height - height.min()) / span if span > 1e-9 else np.full_like(height, 0.5)
    source = recipe["cavity"]["source"]
    if source == "base_alpha":
        cavity = upscale_float(base_rgba[..., 3], size)
    elif source == "height":
        lo, hi = recipe["cavity"]["range"]
        cavity = np.clip((height01 - lo) / (hi - lo), 0, 1)
    else:
        cavity = np.ones((size, size))
    if facts["spec_mask"] == "bump_alpha":
        spec = upscale_float(load(layers["bump"])[..., 3], size)
    elif facts["spec_mask"] == "base_alpha":
        spec = upscale_float(base_rgba[..., 3], size)
    else:
        spec = np.full((size, size), 0.5)

    base_png = upscale_base(layers["base"], size, work, upscaler, models)
    linear = srgb_to_linear(load(base_png))
    faces = cavity > 0.8
    delight = recipe["delight"]
    if delight["strength"] > 0 and layers.get("bump") is not None and faces.sum() > 1000:
        lum = np.maximum(linear @ [0.2126, 0.7152, 0.0722], 1e-4)
        X = np.stack([np.ones(faces.sum()), normal[..., 0][faces], normal[..., 1][faces]], 1)
        y = np.log(lum[faces])
        coef, *_ = np.linalg.lstsq(X, y, rcond=None)
        r2 = 1 - ((y - X @ coef) ** 2).sum() / max(((y - y.mean()) ** 2).sum(), 1e-12)
        shade = gaussian_filter(coef[1] * normal[..., 0] + coef[2] * normal[..., 1],
                                delight["blur_px"], mode="wrap")
        weight = np.clip((cavity - 0.6) / 0.3, 0, 1)
        linear = linear * np.exp(-delight["strength"] * weight * shade)[..., None]
        lum2 = np.maximum(linear @ [0.2126, 0.7152, 0.0722], 1e-4)
        after, *_ = np.linalg.lstsq(X, np.log(lum2[faces]), rcond=None)
        report["delight"] = {"baked_light_r2": round(float(r2), 3),
                             "slope_before": [round(float(c), 3) for c in coef[1:]],
                             "slope_after": [round(float(c), 3) for c in after[1:]]}

    # Unique names: ArmorPaint caches a path forever and refuses re-imports.
    token = hashlib.sha256(json.dumps(recipe, sort_keys=True).encode() +
                           layers["base"].read_bytes()).hexdigest()[:10]
    inputs = {
        "base": work / ("in-%s-base.png" % token),
        "normal": work / ("in-%s-normal-dx.png" % token),
        "masks": work / ("in-%s-masks.png" % token),
    }
    save8(inputs["base"], linear_to_srgb(np.clip(linear, 0, 1)))
    save8(inputs["normal"], normal * 0.5 + 0.5)      # DirectX; ArmorPaint converts on import
    save8(inputs["masks"], np.dstack([cavity, spec, height01]))
    report["relief_px"] = round(float(span), 2)
    report["inputs"] = {k: str(v) for k, v in inputs.items()}
    return report


def upscale_float_rgb(rgb: np.ndarray, edge: int) -> np.ndarray:
    return np.dstack([upscale_float(rgb[..., c], edge) for c in range(3)])


# --------------------------------------------------------------------------- author (MCP plan)

def graph_calls(recipe: dict) -> list[dict]:
    """The node graph as MCP calls. Output sockets follow OUTPUT_MATERIAL_PBR."""
    rough, metal = recipe["roughness"], recipe["metal"]
    nodes = [
        ("base", "TEX_IMAGE", -1400, -300, [("button", 0, 0), ("button", 1, 2)]),   # asset 0, sRGB
        ("normal", "TEX_IMAGE", -1400, 400, [("button", 0, 1), ("button", 1, 3)]),  # DirectX normal
        ("masks", "TEX_IMAGE", -1400, 100, [("button", 0, 2), ("button", 1, 1)]),   # linear
        ("split", "SEPARATE_COLOR", -1150, 100, []),
        ("ao", "MAPRANGE", -900, -100, [("float", 1, recipe["occlusion"]["from"][0]),
                                        ("float", 2, recipe["occlusion"]["from"][1]),
                                        ("float", 3, recipe["occlusion"]["to"][0]),
                                        ("float", 4, recipe["occlusion"]["to"][1]), ("button", 0, 1)]),
        ("f0", "HUE_SAT", -900, -400, [("float", 0, 0.5), ("float", 1, recipe["f0"]["saturation"]),
                                       ("float", 2, recipe["f0"]["value_gain"]), ("float", 3, 1.0)]),
        ("albedo", "MIX_RGB", -600, -300, []),
        ("rnoise", "TEX_NOISE", -1150, 350, [("float", 1, rough["noise_scale"]), ("float", 2, 6.0),
                                             ("float", 3, 0.6)]),
        ("rjit", "MAPRANGE", -900, 400, [("float", 1, 0.3), ("float", 2, 0.7),
                                         ("float", 3, -rough["noise_amp"]), ("float", 4, rough["noise_amp"])]),
        ("radd", "MATH", -700, 300, [("button", 0, 0), ("button", 1, 1)]),
        ("rseam", "VALUE", -700, 150, [("float", 0, rough["seam"], False)]),
        ("rough", "MIX_RGB", -450, 200, []),
        # seam/face blend: cavity 0 -> seam roughness, 1 -> face roughness
        ("faces", "MAPRANGE", -700, 0, [("float", 1, rough["seam_range"][0]), ("float", 2, rough["seam_range"][1]),
                                        ("float", 3, 0.0), ("float", 4, 1.0), ("button", 0, 1)]),
    ]
    if "from_spec" in rough:
        nodes.append(("rbase", "MAPRANGE", -900, 250, [("float", 1, 0.0), ("float", 2, 1.0),
                                                       ("float", 3, rough["from_spec"][0]),
                                                       ("float", 4, rough["from_spec"][1])]))
    else:
        nodes.append(("rbase", "VALUE", -900, 250, [("float", 0, rough["constant"], False)]))
    if "cavity_range" in metal:
        nodes.append(("metal", "MAPRANGE", -900, 0, [("float", 1, metal["cavity_range"][0]),
                                                     ("float", 2, metal["cavity_range"][1]),
                                                     ("float", 3, 0.0), ("float", 4, 1.0), ("button", 0, 1)]))
    else:
        nodes.append(("metal", "VALUE", -900, 0, [("float", 0, metal["constant"], False)]))
    links = [
        ("masks", 0, "split", 0), ("split", 0, "ao", 0), ("base", 0, "f0", 4),
        ("metal", 0, "albedo", 0), ("base", 0, "albedo", 1), ("f0", 0, "albedo", 2),
        ("rnoise", 0, "rjit", 0), ("rbase", 0, "radd", 0), ("rjit", 0, "radd", 1),
        ("split", 0, "faces", 0),
        ("faces", 0, "rough", 0), ("rseam", 0, "rough", 1), ("radd", 0, "rough", 2),
        ("albedo", 0, "OUT", 0), ("ao", 0, "OUT", 2), ("rough", 0, "OUT", 3),
        ("metal", 0, "OUT", 4), ("normal", 0, "OUT", 5), ("split", 2, "OUT", 7),
    ]
    if "from_spec" in rough:
        links.append(("split", 1, "rbase", 0))
    if "cavity_range" in metal:
        links.append(("split", 0, "metal", 0))
    calls = [{"tool": "ap_node_add", "args": {"type": t, "x": x, "y": y}, "save": name}
             for name, t, x, y, _ in nodes]
    for name, _, _, _, values in nodes:
        for value in values:
            kind, index, number = value[:3]
            args = {"id": {"$ref": name}, "kind": kind, "value": number}
            if kind == "button":
                args["button"] = index
            else:
                args.update(socket=index, is_input=value[3] if len(value) > 3 else True)
            calls.append({"tool": "ap_node_set_value", "args": args})
    for source, out_socket, target, in_socket in links:
        calls.append({"tool": "ap_node_connect", "args": {
            "from_id": {"$ref": source}, "from_socket": out_socket,
            "to_id": 0 if target == "OUT" else {"$ref": target}, "to_socket": in_socket}})
    return calls


def author_plan(recipe: dict, inputs: dict, plane: Path, export_dir: Path, project: Path) -> dict:
    calls = [
        {"tool": "ap_project_new"},
        {"tool": "ap_import_asset", "args": {"path": str(plane)}},
        *({"tool": "ap_import_asset", "args": {"path": str(inputs[key])}}
          for key in ("base", "normal", "masks")),
        {"tool": "ap_material_create", "args": {"name": recipe["material"].replace("/", "_")}},
        *graph_calls(recipe),
        {"tool": "ap_material_update"},
        {"tool": "ap_node_list"},
        {"tool": "ap_fill_layer"},
        {"tool": "ap_fs_mkdir", "args": {"path": str(export_dir)}},
        {"tool": "ap_export_textures", "args": {"directory": str(export_dir)}},
        {"tool": "ap_set_display_channel", "args": {"mode": "lit"}},
        {"tool": "ap_project_save_as", "args": {"path": str(project)}},
    ]
    return {"stage": recipe["material"], "calls": calls}


def plane_obj(path: Path, quads: int = 16) -> Path:
    lines = ["o remaster_plane"]
    for j in range(quads + 1):
        for i in range(quads + 1):
            lines.append("v %f %f 0" % (i / quads * 2 - 1, j / quads * 2 - 1))
    for j in range(quads + 1):
        for i in range(quads + 1):
            lines.append("vt %f %f" % (i / quads, j / quads))
    lines.append("vn 0 0 1")
    for j in range(quads):
        for i in range(quads):
            a = j * (quads + 1) + i + 1
            b, c, d = a + 1, a + quads + 2, a + quads + 1
            lines.append("f %d/%d/1 %d/%d/1 %d/%d/1" % (a, a, b, b, c, c))
            lines.append("f %d/%d/1 %d/%d/1 %d/%d/1" % (a, a, c, c, d, d))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def run_plan(plan: dict, work: Path, armorpaint_dir: Path) -> None:
    plan_path, transcript = work / "mcp-plan.json", work / "mcp-transcript.jsonl"
    plan_path.write_text(json.dumps(plan, indent=1), encoding="utf-8")
    transcript.unlink(missing_ok=True)
    subprocess.run([str(MCP_PYTHON), str(PLAN_RUNNER), str(plan_path), str(transcript),
                    str(MCP_DIR), str(armorpaint_dir)], check=True, timeout=1800)


class IsolatedArmorPaint(contextlib.AbstractContextManager):
    """Headless sway + Xwayland session running ArmorPaint at a fixed layer size.

    ArmorPaint reads its layer resolution only at startup, so the size is written
    to config.json first; the user's config is restored on exit.
    """

    def __init__(self, armorpaint_dir: Path, size: int, work: Path):
        self.dir, self.size, self.work = armorpaint_dir, size, work
        self.config = armorpaint_dir / "data/config.json"
        self.backup = work / "armorpaint-config.backup.json"
        self.sway = None

    def __enter__(self):
        if subprocess.run(["pgrep", "-x", "ArmorPaint"], capture_output=True).returncode == 0:
            raise RuntimeError("an ArmorPaint process is already running; close it or omit --launch-isolated")
        shutil.copyfile(self.config, self.backup)
        config = json.loads(self.config.read_text(encoding="utf-8"))
        plugins = set(config.get("plugins") or []) | {"armorpaint_mcp_bridge.c"}
        config.update(layer_res=LAYER_RES[self.size], plugins=sorted(plugins), window_mode=0,
                      window_w=2560, window_h=1440, window_x=0, window_y=0, window_scale=1.25)
        self.config.write_text(json.dumps(config), encoding="utf-8")
        sway_config = self.work / "sway.config"
        sway_config.write_text("output HEADLESS-1 resolution 2560x1440 scale 1 bg #202020 solid_color\n"
                               "xwayland enable\ndefault_border none\n", encoding="utf-8")
        runtime = Path(os.environ["XDG_RUNTIME_DIR"])
        before = set(runtime.glob("wayland-*"))
        env = {k: v for k, v in os.environ.items() if k not in ("DISPLAY", "WAYLAND_DISPLAY", "SWAYSOCK")}
        env.update(WLR_BACKENDS="headless", WLR_LIBINPUT_NO_DEVICES="1", WLR_RENDERER="gles2")
        self.sway = subprocess.Popen(["sway", "-c", str(sway_config)], env=env,
                                     stdout=open(self.work / "sway.log", "w"), stderr=subprocess.STDOUT)
        deadline = time.monotonic() + 10
        sockets = []
        while time.monotonic() < deadline and not sockets:
            time.sleep(0.2)
            sockets = [p for p in set(runtime.glob("wayland-*")) - before if not p.name.endswith(".lock")]
            ipc = list(runtime.glob("sway-ipc.*.%d.sock" % self.sway.pid))
            sockets = sockets if ipc else []
        if not sockets:
            raise RuntimeError("headless sway did not start; see " + str(self.work / "sway.log"))
        self.wayland, self.ipc = sockets[0].name, ipc[0]
        log = self.work / "armorpaint.log"
        subprocess.run(["swaymsg", "-s", str(self.ipc), "exec",
                        "cd '%s' && ./ArmorPaint > '%s' 2>&1" % (self.dir, log)],
                       check=True, capture_output=True)
        heartbeat = self.dir / "data/mcp_spool/heartbeat.json"
        started = time.time()
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            if heartbeat.exists() and heartbeat.stat().st_mtime > started:
                return self
            time.sleep(0.25)
        raise RuntimeError("ArmorPaint bridge heartbeat did not appear; see " + str(log))

    def screenshot(self, path: Path, geometry: str | None = None) -> None:
        command = ["grim"] + (["-g", geometry] if geometry else []) + [str(path)]
        subprocess.run(command, env={**os.environ, "WAYLAND_DISPLAY": self.wayland},
                       check=True, capture_output=True)

    def __exit__(self, *exc):
        with contextlib.suppress(Exception):
            run_plan({"stage": "quit", "calls": [{"tool": "ap_quit", "args": {"confirm": True}}]},
                     self.work, self.dir)
            time.sleep(2)
        subprocess.run(["pkill", "-x", "ArmorPaint"], capture_output=True)
        if self.sway:
            self.sway.terminate()
            with contextlib.suppress(subprocess.TimeoutExpired):
                self.sway.wait(10)
        if self.backup.exists():
            shutil.copyfile(self.backup, self.config)
        return False


# --------------------------------------------------------------------------- package + qa

def find_export(export_dir: Path) -> dict[str, Path]:
    found = {}
    for suffix in ("base", "metal", "nor", "occ", "rough"):
        matches = sorted(export_dir.glob("*_%s.png" % suffix))
        if len(matches) != 1:
            raise ValueError("export %s: expected one *_%s.png, found %d" % (export_dir, suffix, len(matches)))
        found[suffix] = matches[0]
    return found


def package(exported: dict[str, Path], textures: Path) -> dict[str, Path]:
    def gray(path):
        image = Image.open(path)
        return np.asarray(image.convert("L") if image.mode != "L" else image)
    textures.mkdir(parents=True, exist_ok=True)
    out = {"basecolor": textures / "basecolor.png", "mrao": textures / "mrao.png",
           "normal": textures / "normal.png"}
    Image.open(exported["base"]).convert("RGB").save(out["basecolor"])
    mrao = np.dstack([gray(exported["metal"]), gray(exported["rough"]), gray(exported["occ"])])
    Image.fromarray(mrao.astype(np.uint8)).save(out["mrao"])
    Image.open(exported["nor"]).convert("RGB").save(out["normal"])
    return out


def qa(recipe: dict, packaged: dict[str, Path], prepared_normal_dx: Path, source_seams: dict) -> dict:
    size = recipe["size"]
    problems, report = [], {}
    images = {k: np.asarray(Image.open(p)) for k, p in packaged.items()}
    for key, image in images.items():
        if image.shape[:2] != (size, size):
            problems.append("%s is %dx%d, expected %d" % (key, image.shape[1], image.shape[0], size))
    normal = images["normal"].astype(np.float64) / 127.5 - 1
    tilt = np.abs(normal[..., :2].reshape(-1, 2).mean(0))
    report["normal_mean_tilt"] = [round(float(t), 4) for t in tilt]
    if tilt.max() > TILT_LIMIT:
        problems.append("normal map is tilted (mean xy %s)" % report["normal_mean_tilt"])
    small = np.asarray(Image.fromarray(images["normal"]).resize((1024, 1024), Image.BILINEAR))
    residual = convention_residuals(decode_normal(small.astype(np.float64) / 255, False))
    report["normal_convention_residual"] = {k: round(v, 3) for k, v in residual.items()}
    relief = float(np.abs(normal[..., :2]).mean())
    if relief > 0.004 and residual["opengl"] >= residual["directx"]:
        problems.append("exported normal is not OpenGL +Y")
    prepared = load(prepared_normal_dx) * 2 - 1
    prepared[..., 1] *= -1                                    # expected OpenGL encoding
    report["normal_vs_prepared_mae"] = round(float(np.abs(prepared - normal).mean()), 4)
    if report["normal_vs_prepared_mae"] > 0.02:
        problems.append("exported normal differs from the prepared input")
    mrao = images["mrao"].astype(np.float64) / 255
    for index, name in enumerate(("metal", "rough", "ao")):
        channel = mrao[..., index]
        report[name] = {"min": round(float(channel.min()), 3), "mean": round(float(channel.mean()), 3),
                        "max": round(float(channel.max()), 3)}
    if mrao[..., 1].min() < 0.02:
        problems.append("roughness below the shader's 0.02 floor")
    if recipe["roughness"]["noise_amp"] > 0 and mrao[..., 1].std() < recipe["roughness"]["noise_amp"] / 10:
        problems.append("recipe asks for roughness breakup but the roughness channel is constant")
    metal = mrao[..., 0]
    report["metal_non_binary_fraction"] = round(float(((metal > 0.1) & (metal < 0.9)).mean()), 4)
    base = images["basecolor"].astype(np.float64) / 255
    metallic_pixels = metal > 0.9
    if metallic_pixels.any():
        f0 = srgb_to_linear(base[metallic_pixels]) @ [0.2126, 0.7152, 0.0722]
        report["metal_f0_luminance_mean"] = round(float(f0.mean()), 3)
    dielectric = metal < 0.1
    if dielectric.any():
        albedo = srgb_to_linear(base[dielectric]) @ [0.2126, 0.7152, 0.0722]
        report["dielectric_albedo_luminance_mean"] = round(float(albedo.mean()), 3)
    # Physically based plausibility is advisory: an art-directed finish may sit outside it.
    warnings = []
    if report.get("metal_f0_luminance_mean", 1.0) < 0.3:
        warnings.append("metal F0 luminance %.2f is below bare steel (~0.5): a dark or coated finish"
                        % report["metal_f0_luminance_mean"])
    albedo_mean = report.get("dielectric_albedo_luminance_mean")
    if albedo_mean is not None and not 0.02 <= albedo_mean <= 0.9:
        warnings.append("dielectric albedo luminance %.3f is outside 0.02-0.9" % albedo_mean)
    report["warnings"] = warnings
    report["seam_ratio"] = {k: round(seam_ratio(v), 3) for k, v in images.items()}
    for key, ratio in report["seam_ratio"].items():
        allowed = max(SEAM_RATIO_LIMIT, SEAM_SOURCE_MARGIN * source_seams.get(key, 0.0))
        if ratio > allowed:
            problems.append("%s wrap seam %.2f exceeds %.2f (source %.2f)" %
                            (key, ratio, allowed, source_seams.get(key, 0.0)))
    report["problems"] = problems
    return report


def contact_sheet(layers: dict, packaged: dict, lit: Path | None, path: Path) -> None:
    tiles = [("legacy base", layers["base"])]
    if layers.get("bump"):
        tiles.append(("legacy bump", layers["bump"]))
    tiles += [("basecolor", packaged["basecolor"]), ("mrao", packaged["mrao"]), ("normal", packaged["normal"])]
    if lit:
        tiles.append(("ArmorPaint lit", lit))
    cell = 384
    sheet = Image.new("RGB", (cell * len(tiles), cell + 22), "black")
    draw = ImageDraw.Draw(sheet)
    for index, (label, tile) in enumerate(tiles):
        image = Image.open(tile).convert("RGB")
        image.thumbnail((cell, cell), Image.LANCZOS)
        sheet.paste(image, (index * cell, 0))
        draw.text((index * cell + 4, cell + 4), label, fill="white")
    sheet.save(path)


# --------------------------------------------------------------------------- driver

def remaster(recipe_path: Path, args, session: IsolatedArmorPaint | None) -> dict:
    recipe = load_recipe(recipe_path)
    name = recipe["material"]
    root = args.out.resolve() / "work" / name
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    original = args.out.resolve() / "materials/portal_pbr_fallback" / (name + ".vmt")
    resolved = resolve_material(args.vpk, name, original)
    facts = material_facts(resolved)
    cache: dict[str, dict] = {}
    layers, decodes = {}, {}
    for key in ("base", "bump"):
        if facts[key]:
            info = get_image(args.vpk, facts[key], root / "source" / (key + ".png"), cache)
            if "error" in info:
                raise ValueError("%s %s: %s" % (key, facts[key], info["error"]))
            decodes[key] = info
            layers[key] = Path(info["file"])
    notes = []
    if facts["bump"] and facts["ssbump"] != decodes["bump"]["ssbump"]:
        # The shader follows the VMT parameter; the VTF header flag is informational.
        notes.append("VMT $ssbump=%d but the VTF SSBump flag is %d; the VMT governs" %
                     (facts["ssbump"], decodes["bump"]["ssbump"]))
    entry = {"material": name, "recipe": str(recipe_path), "recipe_sha256": sha256(recipe_path),
             "resolved": {"dx9_block": resolved["dx9_block"], "vpk": resolved["vpk"], **facts},
             "decodes": decodes, "notes": notes}
    if args.stage == "inspect":
        sheet_tiles = [("base", layers["base"])] + ([("bump", layers["bump"])] if "bump" in layers else [])
        extra = []
        for key, tile in sheet_tiles:
            rgba = np.asarray(Image.open(tile))
            extra.append((key + " rgb", Image.fromarray(rgba[..., :3])))
            extra.append((key + " alpha", Image.fromarray(rgba[..., 3])))
        sheet = Image.new("RGB", (384 * len(extra), 406), "black")
        draw = ImageDraw.Draw(sheet)
        for index, (label, image) in enumerate(extra):
            sheet.paste(image.convert("RGB").resize((384, 384), Image.NEAREST), (index * 384, 0))
            draw.text((index * 384 + 4, 388), label, fill="white")
        sheet.save(root / "source-layers.png")
        entry["inspect_sheet"] = str(root / "source-layers.png")
        return entry

    (root / "prep").mkdir()
    prep = prepare(recipe, facts, layers, root / "prep", args.upscaler, args.model_dir)
    entry["prep"] = prep
    export_dir = root / "armorpaint-export"
    project = root / (name.split("/")[-1] + "_remaster.arm")
    plan = author_plan(recipe, {k: Path(v) for k, v in prep["inputs"].items()},
                       plane_obj(root / "prep/plane.obj"), export_dir, project)
    run_plan(plan, root, args.armorpaint_dir)
    entry["mcp_calls"] = len(plan["calls"])
    lit = None
    if session:
        time.sleep(1.0)
        lit = root / "armorpaint-lit.png"
        session.screenshot(lit, "640,260 820x900")
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline and not project.exists():
        time.sleep(0.2)
    entry["armorpaint_project"] = str(project) if project.exists() else None

    textures = args.out.resolve() / "textures/portal_pbr" / name
    packaged = package(find_export(export_dir), textures)
    entry["qa"] = qa(recipe, packaged, Path(prep["inputs"]["normal"]), prep["source_seam_ratio"])
    entry["textures"] = {k: {"path": str(p), "sha256": sha256(p)} for k, p in packaged.items()}

    schema = json.loads(subprocess.check_output([str(HELPER), "schema"], text=True))
    params = {"$surfaceprop": facts["surfaceprop"]} if facts["surfaceprop"] else {}
    vmt = args.out.resolve() / "materials/portal_pbr" / (name + ".vmt")
    vmt.parent.mkdir(parents=True, exist_ok=True)
    vmt.write_text(make_material(name, params, True, schema), encoding="utf-8")
    validation = subprocess.run([str(HELPER), "validate", str(vmt), str(original)],
                                text=True, capture_output=True)
    entry["vmt"] = {"candidate": str(vmt), "fallback": str(original),
                    "schema_valid": validation.returncode == 0,
                    "validation": (validation.stdout or validation.stderr).strip()}
    if validation.returncode:
        entry["qa"]["problems"].append("candidate VMT fails the PBR schema")
    if args.vtex:
        vtf_dir = args.out.resolve() / "materials/portal_pbr" / name
        entry["vtf"] = {}
        for key, path in packaged.items():
            try:
                digest = compile_texture(Image.open(path), vtf_dir / key, args.vtex.resolve())
            except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
                entry["qa"]["problems"].append("vtex %s: %s" % (key, error))
                continue
            entry["vtf"][key] = {"path": str(vtf_dir / (key + ".vtf")), "sha256": digest,
                                 "bytes": (vtf_dir / (key + ".vtf")).stat().st_size}
    contact_sheet(layers, packaged, lit, root / "qa-sheet.png")
    entry["qa_sheet"] = str(root / "qa-sheet.png")
    return entry


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("recipes", nargs="+", type=Path, help="Recipe JSON files (see recipes/README.md)")
    parser.add_argument("--vpk", type=Path, action="append", required=True,
                        help="Portal _dir.vpk; repeat for shared archives (searched in order)")
    parser.add_argument("--out", type=Path, required=True, help="Output directory outside the repository")
    parser.add_argument("--stage", choices=("inspect", "all"), default="all")
    parser.add_argument("--upscaler", type=Path, help="Pinned realesrgan-ncnn-vulkan executable")
    parser.add_argument("--model-dir", type=Path, help="Directory with the realesrgan-x4plus model")
    parser.add_argument("--vtex", type=Path, help="Optional vtex (e.g. build/toolchains/pbrt-map-tools/vtex); writes lossless VTFs")
    parser.add_argument("--armorpaint-dir", type=Path, default=ROOT / "thirdparty/armorpaint/paint/build/out")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--launch-isolated", action="store_true",
                      help="Start ArmorPaint in a private headless sway session for this run")
    mode.add_argument("--discard-armorpaint-project", action="store_true",
                      help="Use the running ArmorPaint; each material starts a new project, discarding the open one")
    args = parser.parse_args()
    out = args.out.resolve()
    if out == ROOT or ROOT in out.parents:
        parser.error("--out must be outside the repository")
    if args.stage == "all":
        if not (args.upscaler and args.model_dir):
            parser.error("--stage all needs --upscaler and --model-dir")
        if not (args.launch_isolated or args.discard_armorpaint_project):
            parser.error("choose --launch-isolated or --discard-armorpaint-project")
        if not MCP_PYTHON.is_file():
            parser.error("armorpaint-mcp virtual environment missing: " + str(MCP_PYTHON))
    build_helper()
    out.mkdir(parents=True, exist_ok=True)
    recipes = [load_recipe(path) for path in args.recipes]
    sizes = {recipe["size"] for recipe in recipes}
    if args.stage == "all" and args.launch_isolated and len(sizes) != 1:
        parser.error("one isolated run uses one layer size; split recipes by size")
    entries, failures = [], []
    session_cm = (IsolatedArmorPaint(args.armorpaint_dir.resolve(), sizes.pop(), out)
                  if args.stage == "all" and args.launch_isolated else contextlib.nullcontext())
    with session_cm as session:
        for path in args.recipes:
            try:
                entry = remaster(path, args, session if args.launch_isolated else None)
                entries.append(entry)
                problems = entry.get("qa", {}).get("problems", [])
                if problems:
                    failures.append({"material": entry["material"], "problems": problems})
                print("%s: %s" % (entry["material"], "; ".join(problems) or "ok"), flush=True)
            except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as error:
                failures.append({"recipe": str(path), "error": str(error)})
                print("%s: %s" % (path, error), file=sys.stderr, flush=True)
    manifest = {"schema": MANIFEST_SCHEMA, "stage": args.stage, "vpks": [str(p) for p in args.vpk],
                "upscaler": {"executable": str(args.upscaler), "sha256": sha256(args.upscaler),
                             "model_bin_sha256": sha256(args.model_dir / "realesrgan-x4plus.bin")}
                if args.stage == "all" else None,
                "armorpaint_dir": str(args.armorpaint_dir), "materials": entries, "failures": failures,
                "engine_ready": False,
                "engine_ready_reason": "native PBRMetalRough runtime binding and KTX2 path are not validated"}
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print("Manifest:", out / "manifest.json")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
