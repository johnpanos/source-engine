"""Scene emitters lit through their Cycles lamps, checked against analytic light.

    blender -b --factory-startup --python-exit-code 9 \\
        --python tools/quality/tests/blender_emitter_lamps.py

Each case builds one emitter the way usd_scene writes it (a tessellated mesh
plus its analytic `shape`) over a white Lambertian floor. The floor is seen
from above by an orthographic camera, with direct light only. The floor's
radiance is compared, per pixel and averaged over each pixel's footprint,
with E / pi, where E is integrated over the emitter's analytic surface:
L * cone(cos_l) * cos_l * cos_r * dA / d^2.

  * energy and shape: the lamp path (pbrt_blender.restore_emitters) matches
    the analytic floor within MAX_ERROR for a sphere, a rectangle turned
    about its normal, and a tilted Source spot disk with a cone (the cone's
    direction and falloff included)
  * the mesh-only path (the emitter material, no lamp) matches the same
    expectation within its tessellation error, so both paths carry the same
    light and cone semantics
  * noise: at equal samples the lamp's pixel noise is at most 1/NOISE_RATIO
    of the mesh-only path's: 20x below it for a sphere, the reason for the
    lamp, and no worse for a flat emitter
  * the mesh is left for camera rays only: the camera sees the bulb at its
    radiance, it throws no shadow on its own lamp, and nothing is lit twice
  * switching: hiding pbrt_blender.emitter_objects together leaves the
    floor black

Prints one `EMITTER_LAMPS <json>` line; exits nonzero on any violation.
"""

import json
import math
import sys
from pathlib import Path

import bpy
import numpy as np

HERE = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HERE))
import pbrt_blender  # noqa: E402

RESOLUTION = 96
EXTENT = 4.0
SAMPLES = 256
NOISE_SAMPLES = 16
MAX_ERROR = 0.02
MESH_ERROR = 0.06
# A tessellated sphere is sampled triangle by triangle, half facing away; a
# flat emitter's few triangles all face the floor and already sample well,
# so its lamp need only be no noisier.
NOISE_RATIO = {"sphere": 20.0, "disk": 1.0, "rect": 1.0}
RADIANCE = (900.0, 1000.0, 1100.0)


def sphere_points(radius, segments=16, rings=8):
    points = [(0.0, 0.0, radius)]
    for ring in range(1, rings):
        theta = math.pi * ring / rings
        for segment in range(segments):
            phi = 2 * math.pi * segment / segments
            points.append((radius * math.sin(theta) * math.cos(phi),
                           radius * math.sin(theta) * math.sin(phi), radius * math.cos(theta)))
    points.append((0.0, 0.0, -radius))
    faces = [(0, 1 + (s + 1) % segments, 1 + s) for s in range(segments)]
    for ring in range(rings - 2):
        base = 1 + ring * segments
        for s in range(segments):
            a, b = base + s, base + (s + 1) % segments
            faces += [(a, b, b + segments), (a, b + segments, a + segments)]
    last = len(points) - 1
    base = 1 + (rings - 2) * segments
    faces += [(base + s, base + (s + 1) % segments, last) for s in range(segments)]
    return np.array(points), faces


def frame(normal):
    """Orthonormal (u, v, n) with n the emitting direction."""
    n = np.asarray(normal, float) / np.linalg.norm(normal)
    helper = np.array((1.0, 0.0, 0.0)) if abs(n[0]) < 0.9 else np.array((0.0, 1.0, 0.0))
    u = np.cross(helper, n)
    u /= np.linalg.norm(u)
    return u, np.cross(n, u), n


def emitter(case):
    """A scene emitter as usd_scene writes it, and surface samples of its
    analytic shape: (points, normals, areas); a sphere's normals face out."""
    centre = np.array(case["centre"])
    one_sided = case["kind"] != "sphere"
    if case["kind"] == "sphere":
        local, faces = sphere_points(case["radius"])
        points = centre + local
        shape = {"kind": "sphere", "centre": centre.tolist(), "radius_m": case["radius"]}
        theta, phi = np.meshgrid((np.arange(64) + 0.5) / 64 * math.pi,
                                 (np.arange(128) + 0.5) / 128 * 2 * math.pi, indexing="ij")
        normals = np.stack([np.sin(theta) * np.cos(phi), np.sin(theta) * np.sin(phi),
                            np.cos(theta)], -1).reshape(-1, 3)
        samples = centre + case["radius"] * normals
        areas = (case["radius"] ** 2 * np.sin(theta) * (math.pi / 64) *
                 (2 * math.pi / 128)).reshape(-1)
    elif case["kind"] == "disk":
        u, v, n = frame(case["normal"])
        # usd_scene: centre vertex then 32 rim vertices, wound to emit along n.
        ring = [centre + case["radius"] * (math.cos(2 * math.pi * k / 32) * u +
                                           math.sin(2 * math.pi * k / 32) * v)
                for k in range(32)]
        points = np.array([centre] + ring)
        faces = [(0, 1 + k, 1 + (k + 1) % 32) for k in range(32)]
        shape = {"kind": "disk", "centre": centre.tolist(), "radius_m": case["radius"],
                 "normal": n.tolist()}
        radius, angle = np.meshgrid((np.arange(32) + 0.5) / 32 * case["radius"],
                                    (np.arange(96) + 0.5) / 96 * 2 * math.pi, indexing="ij")
        samples = (centre + radius[..., None] * (np.cos(angle)[..., None] * u +
                                                 np.sin(angle)[..., None] * v)).reshape(-1, 3)
        areas = (radius * (case["radius"] / 32) * (2 * math.pi / 96)).reshape(-1)
        normals = np.tile(n, (len(samples), 1))
    else:
        width, height = case["size"]
        u, v, n = frame(case["normal"])
        spin = math.radians(case["spin"])
        u, v = math.cos(spin) * u + math.sin(spin) * v, -math.sin(spin) * u + math.cos(spin) * v
        # usd_scene: corners (-,-), (+,-), (+,+), (-,+) in (u, v), emitting
        # along -(u x v); here u x v = n, so flip v to emit along n.
        v = -v
        corners = [centre + a * width / 2 * u + b * height / 2 * v
                   for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
        points = np.array(corners)
        faces = [(0, 2, 1), (0, 3, 2)]
        shape = {"kind": "rect", "centre": centre.tolist()}
        a, b = np.meshgrid((np.arange(48) + 0.5) / 48 - 0.5, (np.arange(48) + 0.5) / 48 - 0.5)
        samples = (centre + a.reshape(-1, 1) * width * u + b.reshape(-1, 1) * height * v)
        areas = np.full(len(samples), width * height / 48 ** 2)
        normals = np.tile(n, (len(samples), 1))
    record = {"kind": "trianglemesh", "space": "stage", "name": case["name"],
              "points": points.tolist(), "indices": [i for face in faces for i in face],
              "world_from_object": np.eye(4).tolist(),
              "emission": {"radiance": list(RADIANCE), "scale": 1.0, "one_sided": one_sided,
                           "two_sided": not one_sided}, "shape": shape}
    if case.get("cone"):
        record["cone"] = case["cone"]
    return record, (samples, normals, areas)


def expected_floor(record, surface):
    """Floor radiance E / pi per pixel, averaged over a 4 x 4 footprint."""
    samples, normals, areas = surface
    sub = 4
    cells = (np.arange(RESOLUTION * sub) + 0.5) / (RESOLUTION * sub) * EXTENT - EXTENT / 2
    x, y = np.meshgrid(cells, -cells)
    floor = np.stack([x.reshape(-1), y.reshape(-1), np.zeros(x.size)], -1)
    luminance = np.mean(RADIANCE)
    cone = record.get("cone")
    total = np.zeros(len(floor))
    for start in range(0, len(samples), 512):
        s, n, a = samples[start:start + 512], normals[start:start + 512], \
            areas[start:start + 512]
        delta = floor[:, None, :] - s[None]
        d2 = (delta ** 2).sum(-1)
        direction = delta / np.sqrt(d2)[..., None]
        cos_l = (direction * n[None]).sum(-1)
        cos_r = np.clip(-direction[..., 2], 0.0, None)
        weight = np.clip(cos_l, 0.0, None)
        if cone:
            ramp = np.clip((cos_l - cone["outer"]) /
                           max(cone["inner"] - cone["outer"], 1e-6), 0.0, 1.0)
            weight = weight * (ramp ** cone["exponent"] if cone["exponent"] not in (0.0, 1.0)
                               else ramp)
        total += (luminance * weight * cos_r * a[None] / d2).sum(-1)
    image = total.reshape(RESOLUTION * sub, RESOLUTION * sub) / math.pi
    return image.reshape(RESOLUTION, sub, RESOLUTION, sub).mean(axis=(1, 3))


def build(record, lamp):
    pbrt_blender.clear_scene()
    for block in (bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras):
        for item in list(block):
            block.remove(item)
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    cycles = scene.cycles
    cycles.device = "CPU"
    cycles.use_adaptive_sampling = False
    cycles.use_denoising = False
    cycles.max_bounces = 0
    scene.render.resolution_x = scene.render.resolution_y = RESOLUTION
    scene.render.resolution_percentage = 100
    scene.render.filter_size = 0.01
    scene.view_settings.view_transform = "Standard"
    world = scene.world or bpy.data.worlds.new("World")
    scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.0
    floor = bpy.data.meshes.new("Floor")
    half = EXTENT
    floor.from_pydata([(-half, -half, 0), (half, -half, 0), (half, half, 0), (-half, half, 0)],
                      [], [(0, 1, 2, 3)])
    white = bpy.data.materials.new("White")
    white.use_nodes = True
    nodes = white.node_tree.nodes
    nodes.clear()
    diffuse = nodes.new("ShaderNodeBsdfDiffuse")
    diffuse.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    output = nodes.new("ShaderNodeOutputMaterial")
    white.node_tree.links.new(diffuse.outputs["BSDF"], output.inputs["Surface"])
    floor.materials.append(white)
    scene.collection.objects.link(bpy.data.objects.new("Floor", floor))
    camera = bpy.data.cameras.new("Camera")
    camera.type = "ORTHO"
    camera.ortho_scale = EXTENT
    camera_object = bpy.data.objects.new("Camera", camera)
    camera_object.location = (0.0, 0.0, 5.0)
    scene.collection.objects.link(camera_object)
    scene.camera = camera_object
    mesh = pbrt_blender.add_emitter(0, record)
    if lamp:
        pbrt_blender.restore_emitters({"emitters": [record]})
    else:
        mesh.data.materials.append(pbrt_blender.emitter_material(0, record))
    return mesh


def render(samples, seed, path):
    scene = bpy.context.scene
    scene.cycles.samples = samples
    scene.cycles.seed = seed
    scene.render.image_settings.file_format = "OPEN_EXR"
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)
    image = bpy.data.images.load(str(path))
    pixels = np.array(image.pixels[:], dtype=np.float64).reshape(RESOLUTION, RESOLUTION, 4)
    bpy.data.images.remove(image)
    # Blender stores rows bottom-up; flip so row 0 is +y, as expected_floor.
    return pixels[::-1, :, :3].mean(axis=2)


CASES = [
    {"name": "LightQuad00", "kind": "sphere", "centre": (0.3, -0.2, 1.0), "radius": 0.08},
    {"name": "LightQuad01", "kind": "rect", "centre": (-0.2, 0.1, 0.9), "size": (0.4, 0.15),
     "normal": (0.2, 0.1, -1.0), "spin": 30.0},
    {"name": "LightDisk02", "kind": "disk", "centre": (-0.5, 0.2, 1.0), "radius": 0.06,
     "normal": (0.5, -0.1, -0.85), "cone": {"inner": 0.95, "outer": 0.8, "exponent": 2.0}},
]


def main():
    work = Path(bpy.app.tempdir or "/tmp") / "emitter-lamps"
    work.mkdir(parents=True, exist_ok=True)
    failures, report = [], {}
    for case in CASES:
        record, surface = emitter(case)
        expected = expected_floor(record, surface)
        lit = expected > 0.05 * expected.max()
        # The bulb covers the floor beneath it in the camera's view.
        x = (np.arange(RESOLUTION) + 0.5) / RESOLUTION * EXTENT - EXTENT / 2
        gx, gy = np.meshgrid(x, -x)
        reach = case.get("radius", max(case.get("size", (0, 0)))) + 2 * EXTENT / RESOLUTION
        clear = np.hypot(gx - case["centre"][0], gy - case["centre"][1]) > reach
        region = lit & clear
        row = {"pixels": int(region.sum())}
        for lamp in (True, False):
            label = "lamp" if lamp else "mesh"
            mesh = build(record, lamp)
            if pbrt_blender.lamp_kind(record) != case["kind"]:
                failures.append("%s: lamp_kind disagrees with the case" % case["name"])
            image = render(SAMPLES, 1, work / ("%s-%s.exr" % (case["name"], label)))
            error = float(np.abs(image[region] - expected[region]).sum() /
                          expected[region].sum())
            first = render(NOISE_SAMPLES, 1, work / "noise-a.exr")
            second = render(NOISE_SAMPLES, 2, work / "noise-b.exr")
            noise = float(np.sqrt(np.mean((first[region] - second[region]) ** 2) / 2) /
                          np.mean(expected[region]))
            row[label] = {"error": error, "noise": noise}
            limit = MAX_ERROR if lamp else MESH_ERROR
            if error > limit:
                failures.append("%s %s: floor error %.4f > %.4f" % (case["name"], label, error,
                                                                    limit))
            if lamp:
                objects = pbrt_blender.emitter_objects(0, record)
                if objects != [record["name"], record["name"] + pbrt_blender.LAMP_SUFFIX]:
                    failures.append("%s: emitter objects %s" % (case["name"], objects))
                if mesh.visible_shadow or mesh.visible_diffuse or not mesh.visible_camera:
                    failures.append("%s: the bulb mesh is not camera-only" % case["name"])
                if case["kind"] == "sphere":
                    col = int((case["centre"][0] + EXTENT / 2) / EXTENT * RESOLUTION)
                    rowi = int((EXTENT / 2 - case["centre"][1]) / EXTENT * RESOLUTION)
                    seen = float(image[rowi, col])
                    row["bulb"] = seen
                    if abs(seen - np.mean(RADIANCE)) > 0.02 * np.mean(RADIANCE):
                        failures.append("%s: camera sees the bulb at %.1f, not %.1f"
                                        % (case["name"], seen, np.mean(RADIANCE)))
                for name in objects:
                    bpy.data.objects[name].hide_render = True
                dark = render(4, 1, work / "dark.exr")
                row["switched_off_max"] = float(dark.max())
                if dark.max() > 0:
                    failures.append("%s: light left on after hiding %s" % (case["name"],
                                                                           objects))
        row["noise_ratio"] = row["mesh"]["noise"] / max(row["lamp"]["noise"], 1e-12)
        if row["noise_ratio"] < NOISE_RATIO[case["kind"]]:
            failures.append("%s: lamp noise only %.1fx below the mesh" % (case["name"],
                                                                          row["noise_ratio"]))
        report[case["name"]] = row
    print("EMITTER_LAMPS " + json.dumps({"status": "fail" if failures else "pass",
                                         "cases": report, "failures": failures},
                                        sort_keys=True))
    if failures:
        raise SystemExit("\n".join(failures))


main()
