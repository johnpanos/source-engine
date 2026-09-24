#!/usr/bin/env python3
"""Scene front end of the map pipeline: one reader API over PBRT and USD scenes.

`pbrt_scene` owns PBRT parsing and its material translation policy;
`usd_scene` (which needs `pxr`) owns OpenUSD traversal and writes a
`map-scene/v1` JSON model. Every other pipeline step — plain Python, OpenUSD
Python and Blender alike — reads scenes through this module, so a step never
branches on the source format itself. The shared model:

  shapes          [{name, material, ...}] one material per mesh
  materials       {name: ...}; read through `material_summary`
  emitters        area emitter meshes ({kind, points/indices or radius,
                  world_from_object, emission}); `space: "stage"` marks
                  geometry already in stage (Z-up) space
  distant_lights  [{direction, irradiance, angle_degrees}] (USD only)
  environment     sky / dome light or None; `environment_equirect` resamples it
  camera, film    reference view; `camera_pose` gives eye/forward/up in stage space

Material summaries always carry the full channel set: `textures` maps
base/roughness/metallic/occlusion/normal/emission/opacity to UsdUVTexture-like
records ({file, channel, scale, bias, colorspace}), plus `emission_color`,
`opacity` and `opacity_threshold`. PBRT summaries get empty defaults.

    python3 tools/quality/map_scene.py <scene.pbrt | scene.json>
    python3 tools/quality/map_scene.py --meshes <scene>
"""

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbrt_scene  # noqa: E402

SCHEMA = "map-scene/v1"
USD_SUFFIXES = (".usd", ".usda", ".usdc", ".usdz")
CHANNEL_DEFAULTS = {"textures": {}, "emission_color": None, "opacity": 1.0,
                    "opacity_threshold": 0.0}
sky_display = pbrt_scene.sky_display
coated_albedo = pbrt_scene.coated_albedo


def is_usd(path):
    return str(path).lower().endswith(USD_SUFFIXES)


def parse(path):
    """Read a PBRT scene or an extracted `map-scene/v1` model."""
    path = Path(path)
    if path.suffix == ".json":
        scene = json.loads(path.read_text())
        if scene.get("schema") != SCHEMA:
            raise ValueError("scene model schema must be " + SCHEMA)
        scene["model"] = str(path.resolve())
        return scene
    if is_usd(path):
        raise ValueError("USD scenes are read after extraction: run usd_scene.py extract "
                         "and pass its map-scene/v1 JSON")
    scene = pbrt_scene.parse(path)
    scene["format"] = "pbrt"
    scene.setdefault("distant_lights", [])
    return scene


def is_usd_scene(scene):
    return scene.get("format") == "usd"


def material_summary(scene, name):
    if is_usd_scene(scene):
        return dict(CHANNEL_DEFAULTS, **scene["materials"][name])
    return dict(CHANNEL_DEFAULTS, **pbrt_scene.material_summary(scene, name))


def material_root(scene):
    """Directory that relative texture paths in summaries resolve against."""
    return Path(scene["source"]).parent


def camera_pose(scene):
    """Reference camera eye, forward and up in stage (Z-up) space."""
    if is_usd_scene(scene):
        return {key: tuple(value) for key, value in scene["camera"]["pose"].items()}
    return pbrt_scene.camera_pose(scene)


def emitter_to_stage(shape):
    """4x4 row-major matrix taking an emitter's object space to stage space."""
    world = shape["world_from_object"]
    if shape.get("space") == "stage":
        return tuple(tuple(row) for row in world)
    return pbrt_scene.matmul(pbrt_scene.PBRT_TO_USD, world)


def emitter_name(index, shape):
    if shape.get("name"):
        return shape["name"]
    return ("LightDisk" if shape["kind"] == "disk" else "LightQuad") + "%02d" % index


def bilinear_wrap(image, x, y):
    """Sample (H, W, C) at float pixel centers, wrapping x and clamping y."""
    import numpy as np
    height, width = image.shape[:2]
    x = x - 0.5
    y = np.clip(y - 0.5, 0.0, height - 1.0)
    x0 = np.floor(x).astype(np.int64)
    y0 = np.floor(y).astype(np.int64)
    fx, fy = (x - x0)[..., None], (y - y0)[..., None]
    x1, y1 = (x0 + 1) % width, np.minimum(y0 + 1, height - 1)
    x0 %= width
    top = image[y0, x0] * (1 - fx) + image[y0, x1] * fx
    bottom = image[y1, x0] * (1 - fx) + image[y1, x1] * fx
    return top * (1 - fy) + bottom * fy


def pipeline_directions(width):
    """Unit directions of the pipeline's Z-up equirect (row 0 = zenith).

    Columns follow Blender's environment convention (see
    `pbrt_scene.environment_equirect`): phi = (0.5 - u) 2 pi.
    """
    import numpy as np
    height = width // 2
    u, v = np.meshgrid((np.arange(width) + 0.5) / width, 1.0 - (np.arange(height) + 0.5) / height)
    phi = (0.5 - u) * 2.0 * np.pi
    elevation = (v - 0.5) * np.pi
    return np.stack((np.cos(elevation) * np.cos(phi), np.cos(elevation) * np.sin(phi),
                     np.sin(elevation)), axis=-1)


def dome_texel_coordinates(local, pole, width, height):
    """OpenEXR latlong pixel coordinates of dome-local directions.

    Pole Y: longitude 0 is +Z and pi/2 is +X. Pole Z (UsdLux DomeLight_1 on a
    Z-up stage): longitude 0 is -Y and pi/2 is +X. Pixel x = 0 has longitude
    +pi; row 0 has latitude +pi/2.
    """
    import numpy as np
    if pole == "Y":
        latitude = np.arcsin(np.clip(local[..., 1], -1.0, 1.0))
        longitude = np.arctan2(local[..., 0], local[..., 2])
    else:
        latitude = np.arcsin(np.clip(local[..., 2], -1.0, 1.0))
        longitude = np.arctan2(local[..., 0], -local[..., 1])
    x = (np.pi - longitude) / (2.0 * np.pi) * width
    y = (np.pi / 2 - latitude) / np.pi * height
    return x, y


def read_linear_image(path):
    import imageio.v3 as iio
    import numpy as np
    pixels = np.asarray(iio.imread(path), dtype=np.float32)
    if pixels.ndim == 2:
        pixels = np.repeat(pixels[..., None], 3, axis=2)
    pixels = pixels[..., :3]
    if Path(path).suffix.lower() not in (".exr", ".hdr", ".pfm"):
        # 8/16-bit dome maps are display-encoded sRGB.
        scale = 65535.0 if pixels.max() > 255.5 else 255.0
        encoded = pixels / scale
        pixels = np.where(encoded <= 0.04045, encoded / 12.92,
                          ((encoded + 0.055) / 1.055) ** 2.4).astype(np.float32)
    return pixels


def environment_equirect(scene, width=2048):
    """The scene's sky as a linear Z-up equirect (H, W, 3), row 0 = zenith."""
    import numpy as np
    if not is_usd_scene(scene):
        return pbrt_scene.environment_equirect(scene, width)
    environment = scene["environment"]
    radiance = np.asarray(environment["radiance"], dtype=np.float32)
    if environment["kind"] == "constant":
        return np.broadcast_to(radiance, (width // 2, width, 3)).astype(np.float32).copy()
    source = read_linear_image(environment["filename"])
    directions = pipeline_directions(width)
    rotation = np.asarray(environment["local_to_pipeline"], dtype=np.float64)
    local = directions @ rotation.T  # rotation is orthonormal: inverse = transpose
    x, y = dome_texel_coordinates(local, environment["pole"], source.shape[1], source.shape[0])
    return (bilinear_wrap(source, x, y) * radiance).astype(np.float32)


def mesh_bounds(scene, shape):
    """World AABB of a shape in stage (Z-up) units: (min, max)."""
    if is_usd_scene(scene):
        return tuple(shape["bounds"][0]), tuple(shape["bounds"][1])
    return pbrt_scene.mesh_bounds(scene, shape)


def mesh_table(scene):
    """Meshes sorted by bounding volume, largest first, for collision decisions."""
    rows = []
    for shape in scene["shapes"]:
        low, high = mesh_bounds(scene, shape)
        extent = tuple(high[i] - low[i] for i in range(3))
        rows.append((extent[0] * extent[1] * extent[2], shape["name"], shape["material"],
                     low, high))
    rows.sort(key=lambda row: -row[0])
    eye = camera_pose(scene)["eye"]
    lines = ["camera eye (stage m): %.2f %.2f %.2f" % tuple(eye),
             "%-24s %-20s %-26s %-26s %s" % ("mesh", "material", "min (m)", "max (m)",
                                              "encloses eye")]
    for _, name, material, low, high in rows:
        inside = all(low[i] <= eye[i] <= high[i] for i in range(3))
        lines.append("%-24s %-20s %-26s %-26s %s" % (
            name[:24], material[:20], " ".join("%7.2f" % v for v in low),
            " ".join("%7.2f" % v for v in high), "yes" if inside else ""))
    return "\n".join(lines)


def inventory(scene):
    materials = {}
    for shape in scene["shapes"]:
        materials.setdefault(shape["material"], []).append(shape["name"])
    return {"format": scene.get("format"), "source_sha256": scene["source_sha256"],
            "film": scene["film"], "camera_fov_degrees": scene["camera"]["fov_degrees"],
            "camera": camera_pose(scene), "mesh_count": len(scene["shapes"]),
            "emitter_count": len(scene["emitters"]),
            "distant_light_count": len(scene.get("distant_lights", [])),
            "environment": bool(scene["environment"]),
            "materials": {name: material_summary(scene, name) for name in sorted(materials)},
            "meshes_by_material": materials}


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--meshes":
        print(mesh_table(parse(sys.argv[2])))
    elif len(sys.argv) == 2:
        print(json.dumps(inventory(parse(sys.argv[1])), indent=2, sort_keys=True, default=str))
    else:
        raise SystemExit(__doc__)
