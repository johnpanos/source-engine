"""Bake the RFC 0011 radiosity transfer (RTRN) of a map scene.

Run inside Blender (``blender -b --factory-startup --python-exit-code 9
--python tools/quality/radiosity_transfer_bake.py -- ...``); `pbrt_map_build.py`
runs it after the probe volume, whose probes it gathers into. It reads the same
normalized stage, scene model, material policy (smooth normals, no normal
maps) and light paths as the lightmap and probe bakes, so its light is theirs.
radiosity_transfer.py documents the model and encoding.

Sources. Each scene emitter (UsdLux area light), distant light, the dome (sky)
and each emissive material is one source, named after its USD prim (or
material). In that order the first 32 get light styles 32, 33, ... (the order
the collision compile gives `light` entities of those names, so
`ent_fire <name> TurnOff` switches them in game); later ones are fixed.

Patches. The world meshes' surfaces are sampled uniformly by area and the
samples binned per mesh, side, dominant normal axis and cube cell of the
patch size. A closed (manifold) mesh has one side, its normals' side; an
open one (a single wall plane) two, because Cycles shades both sides of a
face. A patch's position and normal are its samples' area-weighted means, its
albedo the material policy's diffuse albedo there: base colour or texture
times (1 - metallic) (1 - transmission).

Transfer and gather. From random samples of each patch, cosine-weighted rays
against a BVH of the world (dynamic props excluded, emitters as occluders)
give its form factors: the fraction of rays whose first hit lies on each
patch (a hit on the unlit side of a closed mesh, or on an emitter, is
absorbed; rays that leave the world are lost, the sky is a source). From
each active PRBV probe, uniform rays give its gather: the hit patch's
spherical-harmonic weights, convolved with the cosine lobe. Patches no probe
gathers from, directly or through other patches, are dropped.

Direct light. Per source, with every other source disabled, one Cycles
DIFFUSE DIRECT bake onto receiver quads (invisible to light transport) gives
each patch's direct light (the mean over a few of its samples) and each
probe's 6 x 6 octahedral direct light, in the PRBV's texel order.
"""

import argparse
import hashlib
import json
import math
import sys
import time
from pathlib import Path

import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree
from mathutils.kdtree import KDTree

sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_scene  # noqa: E402
import pbrt_blender  # noqa: E402
import probe_volume  # noqa: E402
import radiosity_transfer  # noqa: E402

SOURCE_UNITS_PER_METER = 39.37007874015748
INTERIOR = probe_volume.IRRADIANCE_TILE - 2
FIRST_STYLE = 32
MAX_SWITCHED = 32  # vbsp MAX_SWITCHED_LIGHTS
SAMPLES_PER_PATCH_EDGE = 4  # surface samples per patch edge length
RECEIVERS_PER_PATCH = 4
RAY_OFFSET = 1e-4  # meters: ray origins off their surface
RECEIVER_OFFSET = 1e-3
QUAD_HALF_METERS = 5e-3
BAKE_WIDTH = 256
SEED = 20260925


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def srgb_to_linear(values):
    values = np.asarray(values, dtype=np.float64)
    return np.where(values <= 0.04045, values / 12.92, ((values + 0.055) / 1.055) ** 2.4)


# ---------------------------------------------------------------- sources

def prim_name(path):
    return str(path).rstrip("/").rsplit("/", 1)[-1] or str(path)


def collect_sources(scene, materials):
    """[{name, kind, style, objects, world, material}] in style order."""
    sources = []
    for index, shape in enumerate(scene["emitters"]):
        sources.append({"name": prim_name(shape.get("source") or pbrt_blender.emitter_name(
            index, shape)), "kind": "light",
            "objects": [pbrt_blender.emitter_name(index, shape)]})
    for index, light in enumerate(scene.get("distant_lights", [])):
        sources.append({"name": prim_name(light.get("source") or "Sun%02d" % index),
                        "kind": "light", "objects": ["Sun%02d" % index]})
    if scene["environment"]:
        sources.append({"name": prim_name(scene["environment"].get("source") or "Sky"),
                        "kind": "sky", "world": True})
    for name, material in sorted(materials.items()):
        shader = material.node_tree.nodes.get("Principled BSDF")
        if shader is None:
            continue
        strength = shader.inputs["Emission Strength"]
        color = shader.inputs["Emission Color"]
        if strength.default_value > 0 and (color.is_linked or
                                           max(color.default_value[:3]) > 0):
            sources.append({"name": name, "kind": "emissive", "material": material})
    names = set()
    for index, source in enumerate(sources):
        base, suffix = source["name"][:40], 1
        while source["name"] in names or not source["name"]:
            source["name"] = "%s_%d" % (base, suffix)
            suffix += 1
        names.add(source["name"])
        source["style"] = FIRST_STYLE + index if index < MAX_SWITCHED else -1
    return sources


def isolate(sources, active):
    """Enable only source `active` (None: every source)."""
    for index, source in enumerate(sources):
        on = active is None or index == active
        for name in source.get("objects", []):
            obj = bpy.data.objects.get(name)
            if obj is None:
                raise ValueError("source %s lost its object %s" % (source["name"], name))
            obj.hide_render = not on
        if source.get("world"):
            background = bpy.context.scene.world.node_tree.nodes.get("Background")
            source.setdefault("strength", background.inputs["Strength"].default_value)
            background.inputs["Strength"].default_value = source["strength"] if on else 0.0
        if source.get("material"):
            shader = source["material"].node_tree.nodes.get("Principled BSDF")
            source.setdefault("strength", shader.inputs["Emission Strength"].default_value)
            shader.inputs["Emission Strength"].default_value = source["strength"] if on else 0.0


# ---------------------------------------------------------------- albedo

class Albedo:
    """The material policy's diffuse albedo of a scene shape at a UV."""

    def __init__(self, scene):
        self.scene = scene
        self.root = map_scene.material_root(scene)
        self.images = {}

    def texture(self, record):
        path = (self.root / record["file"]).resolve()
        key = str(path)
        if key not in self.images:
            image = bpy.data.images.load(key, check_existing=True)
            width, height = image.size
            pixels = np.array(image.pixels[:], dtype=np.float32).reshape(height, width,
                                                                        image.channels)
            linear = record.get("colorspace", "auto") == "raw" or \
                path.suffix.lower() in (".exr", ".hdr") or image.is_float
            rgb = pixels[..., :3] if image.channels >= 3 else np.repeat(pixels[..., :1], 3, -1)
            if not linear:
                rgb = srgb_to_linear(rgb).astype(np.float32)
            self.images[key] = rgb
        return self.images[key]

    def evaluate(self, material_name, uv):
        """(N, 3) diffuse albedo at (N, 2) texture coordinates."""
        summary = map_scene.material_summary(self.scene, material_name)
        count = len(uv)
        if summary["base_texture"]:
            record = summary["textures"].get("base") or {"file": summary["base_texture"]}
            rgb = self.texture(record)
            height, width = rgb.shape[:2]
            x = np.mod(np.floor(uv[:, 0] * width), width).astype(int)
            y = np.mod(np.floor(uv[:, 1] * height), height).astype(int)
            base = rgb[y, x].astype(np.float64)  # Blender rows run bottom-up, like UV v
            scale = np.asarray(record.get("scale", [1, 1, 1, 1])[:3], dtype=np.float64)
            bias = np.asarray(record.get("bias", [0, 0, 0, 0])[:3], dtype=np.float64)
            base = base * scale + bias
        else:
            base = np.tile(np.asarray(summary["base_color"][:3], dtype=np.float64), (count, 1))
        diffuse = (1.0 - float(summary["metallic"])) * (1.0 - float(summary["transmission"]))
        return np.clip(base * diffuse, 0.0, 1.0)


# ---------------------------------------------------------------- patches

def mesh_triangles(obj):
    """World-space triangles of an evaluated mesh: (points (T, 3, 3), normals
    (T, 3), uv (T, 3, 2) or None, closed)."""
    depsgraph = bpy.context.evaluated_depsgraph_get()
    evaluated = obj.evaluated_get(depsgraph)
    mesh = evaluated.to_mesh()
    mesh.calc_loop_triangles()
    matrix = obj.matrix_world
    vertices = np.array([tuple(matrix @ v.co) for v in mesh.vertices], dtype=np.float64)
    loops = np.array([tuple(t.loops) for t in mesh.loop_triangles], dtype=np.int64)
    verts = np.array([tuple(t.vertices) for t in mesh.loop_triangles], dtype=np.int64)
    uv_layer = mesh.uv_layers.get("st")
    uv = None
    if uv_layer is not None and len(loops):
        coords = np.array([tuple(d.uv) for d in uv_layer.data], dtype=np.float64)
        uv = coords[loops]
    # Closed: every edge is shared by exactly two faces.
    uses = {}
    for polygon in mesh.polygons:
        for key in polygon.edge_keys:
            uses[key] = uses.get(key, 0) + 1
    closed = bool(uses) and all(count == 2 for count in uses.values())
    evaluated.to_mesh_clear()
    if not len(verts):
        return np.zeros((0, 3, 3)), np.zeros((0, 3)), None, closed
    points = vertices[verts]
    normals = np.cross(points[:, 1] - points[:, 0], points[:, 2] - points[:, 0])
    return points, normals, uv, closed


def sample_surfaces(objects, albedo_of, spacing, rng):
    """Area-uniform samples: dict of arrays position, normal (unit, front),
    weight (area each), object, triangle, albedo; and each object's
    closedness."""
    out = {k: [] for k in ("position", "normal", "weight", "object", "albedo")}
    closed = []
    for index, obj in enumerate(objects):
        points, cross, uv, is_closed = mesh_triangles(obj)
        closed.append(is_closed)
        if not len(points):
            continue
        area = 0.5 * np.linalg.norm(cross, axis=1)
        keep = area > 1e-12
        points, cross, area = points[keep], cross[keep], area[keep]
        uv = uv[keep] if uv is not None else None
        normals = cross / (2 * area[:, None])
        expected = area / (spacing * spacing)
        counts = np.floor(expected + rng.random(len(area))).astype(int)
        # Every triangle bigger than a quarter sample gets at least one.
        counts = np.maximum(counts, (expected > 0.25).astype(int))
        tri = np.repeat(np.arange(len(area)), counts)
        if not len(tri):
            continue
        r1, r2 = rng.random(len(tri)), rng.random(len(tri))
        s = np.sqrt(r1)
        bary = np.stack([1 - s, s * (1 - r2), s * r2], axis=-1)
        position = np.einsum("nk,nkd->nd", bary, points[tri])
        if uv is not None:
            coords = np.einsum("nk,nkd->nd", bary, uv[tri])
        else:
            coords = np.zeros((len(tri), 2))
        out["position"].append(position)
        out["normal"].append(normals[tri])
        out["weight"].append(area[tri] / counts[tri])
        out["object"].append(np.full(len(tri), index))
        out["albedo"].append(albedo_of(obj, coords))
    samples = {k: np.concatenate(v) if v else np.zeros((0,)) for k, v in out.items()}
    return samples, closed


def axis_bucket(normals):
    axis = np.argmax(np.abs(normals), axis=1)
    sign = np.take_along_axis(normals, axis[:, None], 1)[:, 0] >= 0
    return axis * 2 + sign


class PatchIndex:
    """Patches from binned samples, and hit-point lookup."""

    def __init__(self, samples, closed, patch_size):
        self.patch_size = patch_size
        self.closed = closed
        sides = []
        for side in (1, -1):
            chosen = np.ones(len(samples["object"]), dtype=bool) if side == 1 else \
                ~np.asarray(closed, dtype=bool)[samples["object"]]
            sides.append((side, np.nonzero(chosen)[0]))
        keys, members = {}, []
        sample_patch = {1: np.full(len(samples["object"]), -1),
                        -1: np.full(len(samples["object"]), -1)}
        for side, index in sides:
            normal = samples["normal"][index] * side
            key_rows = np.column_stack([samples["object"][index], np.full(len(index), side),
                                        axis_bucket(normal),
                                        np.floor(samples["position"][index] / patch_size)
                                        .astype(np.int64)])
            for row, sample in zip(map(tuple, key_rows), index):
                patch = keys.get(row)
                if patch is None:
                    patch = keys[row] = len(members)
                    members.append([])
                members[patch].append(sample)
                sample_patch[side][sample] = patch
        self.keys = keys
        self.members = [np.asarray(m) for m in members]
        self.sample_patch = sample_patch
        count = len(members)
        self.side = np.zeros(count, dtype=int)
        for key, patch in keys.items():
            self.side[patch] = key[1]
        self.position = np.zeros((count, 3))
        self.normal = np.zeros((count, 3))
        self.area = np.zeros(count)
        self.albedo = np.zeros((count, 3))
        self.object = np.zeros(count, dtype=int)
        for patch, m in enumerate(self.members):
            w = samples["weight"][m]
            self.area[patch] = w.sum()
            self.position[patch] = (samples["position"][m] * w[:, None]).sum(0) / w.sum()
            n = (samples["normal"][m] * self.side[patch] * w[:, None]).sum(0)
            self.normal[patch] = n / max(np.linalg.norm(n), 1e-12)
            self.albedo[patch] = (samples["albedo"][m] * w[:, None]).sum(0) / w.sum()
            self.object[patch] = samples["object"][m[0]]
        self.trees = {}
        for patch in range(count):
            self.trees.setdefault((self.object[patch], self.side[patch]), []).append(patch)
        built = {}
        for key, patches in self.trees.items():
            tree = KDTree(len(patches))
            for k, patch in enumerate(patches):
                tree.insert(Vector(self.position[patch]), k)
            tree.balance()
            built[key] = (tree, patches)
        self.trees = built

    def lookup(self, obj, side, normal, location):
        """The patch a hit belongs to, or -1 (absorbed)."""
        if side < 0 and self.closed[obj]:
            return -1
        n = np.asarray(normal) * side
        key = (obj, side, int(axis_bucket(n[None])[0])) + tuple(
            np.floor(np.asarray(location) / self.patch_size).astype(np.int64))
        patch = self.keys.get(key)
        if patch is not None:
            return patch
        entry = self.trees.get((obj, side))
        if entry is None:
            return -1
        tree, patches = entry
        _, k, _ = tree.find(Vector(location))
        return patches[k]


# ---------------------------------------------------------------- tracing

def world_bvh(objects):
    vertices, polygons, owner = [], [], []
    depsgraph = bpy.context.evaluated_depsgraph_get()
    for index, obj in enumerate(objects):
        mesh = obj.evaluated_get(depsgraph).to_mesh()
        base = len(vertices)
        vertices.extend(obj.matrix_world @ v.co for v in mesh.vertices)
        for polygon in mesh.polygons:
            polygons.append([base + i for i in polygon.vertices])
            owner.append(index)
        obj.to_mesh_clear()
    return BVHTree.FromPolygons(vertices, polygons, epsilon=0.0), np.asarray(owner)


def cosine_directions(normals, rng):
    """One cosine-weighted direction about each unit normal (N, 3)."""
    n = np.asarray(normals, dtype=np.float64)
    u1, u2 = rng.random(len(n)), rng.random(len(n))
    r, phi = np.sqrt(u1), 2 * math.pi * u2
    helper = np.where((np.abs(n[:, 0]) < 0.9)[:, None], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0])
    t = np.cross(n, helper)
    t /= np.linalg.norm(t, axis=1, keepdims=True)
    b = np.cross(n, t)
    return (r * np.cos(phi))[:, None] * t + (r * np.sin(phi))[:, None] * b + \
        np.sqrt(np.maximum(0.0, 1.0 - u1))[:, None] * n


def first_hits(bvh, owner, patches, origins, directions, limit):
    """Patch index per ray (-1 when absorbed or lost)."""
    hits = np.full(len(directions), -1)
    for k in range(len(directions)):
        direction = Vector(directions[k])
        location, normal, index, _ = bvh.ray_cast(Vector(origins[k]), direction, limit)
        if location is None:
            continue
        obj = int(owner[index])
        if obj < 0:
            continue  # an emitter: absorbed
        side = 1 if normal.dot(direction) < 0 else -1
        hits[k] = patches.lookup(obj, side, tuple(normal), tuple(location))
    return hits


# ---------------------------------------------------------------- receivers

def receiver_mesh(quads):
    """One small quad per (position, normal) receiver, each on its own bake
    texel (probe_volume_bake's receiver construction)."""
    height = max(1, (len(quads) + BAKE_WIDTH - 1) // BAKE_WIDTH)
    vertices, faces, uvs = [], [], []
    for texel, (position, normal) in enumerate(quads):
        tx, ty = texel % BAKE_WIDTH, texel // BAKE_WIDTH
        d = Vector(normal).normalized()
        a = d.orthogonal().normalized()
        b = d.cross(a)
        base = len(vertices)
        for su, sv in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
            vertices.append(Vector(position) + (a * su + b * sv) * QUAD_HALF_METERS)
            uvs.append(((tx + 0.5 + su * 0.5) / BAKE_WIDTH, (ty + 0.5 + sv * 0.5) / height))
        faces.append((base, base + 1, base + 2, base + 3))
    mesh = bpy.data.meshes.new("RadiosityReceivers")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    layer = mesh.uv_layers.new(name="receivers")
    for loop in mesh.loops:
        layer.data[loop.index].uv = uvs[loop.vertex_index]
    for polygon in mesh.polygons:
        polygon.use_smooth = False
    obj = bpy.data.objects.new("RadiosityReceivers", mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.visible_diffuse = obj.visible_glossy = obj.visible_transmission = False
    obj.visible_shadow = obj.visible_volume_scatter = False
    obj.visible_camera = True
    material = bpy.data.materials.new("RadiosityReceiver")
    material.use_nodes = True
    tree = material.node_tree
    tree.nodes.clear()
    output = tree.nodes.new("ShaderNodeOutputMaterial")
    diffuse = tree.nodes.new("ShaderNodeBsdfDiffuse")
    diffuse.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    tree.links.new(diffuse.outputs["BSDF"], output.inputs["Surface"])
    target = tree.nodes.new("ShaderNodeTexImage")
    target.name = "BakeTarget"
    tree.nodes.active = target
    mesh.materials.append(material)
    return obj, target, height


def bake_direct(obj, target, height, name, work):
    image = bpy.data.images.new(name, width=BAKE_WIDTH, height=height, alpha=True,
                                float_buffer=True)
    target.image = image
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    if bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT"}) != {"FINISHED"}:
        raise RuntimeError("Cycles could not bake the radiosity receivers")
    path = work / (name + ".exr")
    scene = bpy.context.scene
    scene.render.image_settings.file_format = "OPEN_EXR"
    scene.render.image_settings.color_depth = "32"
    image.save_render(filepath=str(path.resolve()), scene=scene)
    pixels = np.array(image.pixels[:], dtype=np.float32).reshape(height, BAKE_WIDTH, 4)
    return pixels[..., :3].reshape(-1, 3), path


# ---------------------------------------------------------------- main

def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--prbv", type=Path, required=True)
    parser.add_argument("--patch-size", type=float, required=True, help="meters")
    parser.add_argument("--transfer-rays", type=int, default=256)
    parser.add_argument("--gather-rays", type=int, default=4096)
    parser.add_argument("--samples", type=int, required=True)
    parser.add_argument("--device", choices=pbrt_blender.DEVICES,
                        default=pbrt_blender.DEFAULT_DEVICE)
    parser.add_argument("--light-paths", default="gi-reference")
    parser.add_argument("--out", type=Path, required=True, help="RTRN file")
    parser.add_argument("--work", type=Path, required=True)
    args = parser.parse_args(arguments)
    started = time.monotonic()
    rng = np.random.default_rng(SEED)
    scene = map_scene.parse(args.scene)
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=False) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the normalized stage")
    meshes = pbrt_blender.source_meshes()
    if sorted(obj.name for obj in meshes) != sorted(s["name"] for s in scene["shapes"]):
        raise ValueError("normalized stage meshes differ from the scene model")
    pbrt_blender.rebind_materials(scene, normal_maps=False)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    props = map_scene.prop_shape_names(scene)
    for obj in meshes:
        if obj.name in props:
            obj.hide_render = True
    world = [obj for obj in meshes if obj.name not in props]
    emitters = [obj for obj in bpy.data.objects if obj.type == "MESH" and
                obj.name.startswith(pbrt_blender.EMITTER_PREFIXES)]
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    # By scene material name (Blender may suffix its datablocks).
    materials = {assignments[obj.name]: obj.data.materials[0] for obj in world
                 if obj.data.materials}
    sources = collect_sources(scene, materials)
    if not sources:
        raise ValueError("the scene has no light source")

    # Patches.
    albedo = Albedo(scene)
    spacing = args.patch_size / SAMPLES_PER_PATCH_EDGE
    samples, closed = sample_surfaces(
        world, lambda obj, uv: albedo.evaluate(assignments[obj.name], uv), spacing, rng)
    patches = PatchIndex(samples, closed, args.patch_size)
    bvh, owner = world_bvh(world + emitters)
    owner = np.where(owner < len(world), owner, -1)
    sampled = time.monotonic()

    # Probes of the paired volume, in PRBV order (meters).
    prbv = args.prbv.read_bytes()
    probe_positions, probe_active = radiosity_transfer.prbv_probe_positions(prbv)
    probe_positions = probe_positions / SOURCE_UNITS_PER_METER
    extent = float(np.ptp(np.vstack([samples["position"], probe_positions]), axis=0).max())
    limit = 4.0 * max(extent, 1.0)

    # Form factors.
    count = len(patches.members)
    transfer = [dict() for _ in range(count)]
    for p in range(count):
        members = patches.members[p]
        chosen = members[rng.integers(0, len(members), args.transfer_rays)]
        directions = cosine_directions(samples["normal"][chosen] * patches.side[p], rng)
        origins = samples["position"][chosen] + \
            samples["normal"][chosen] * patches.side[p] * RAY_OFFSET
        for q in first_hits(bvh, owner, patches, origins, directions, limit):
            if q >= 0 and q != p:
                transfer[p][q] = transfer[p].get(q, 0.0) + 1.0 / args.transfer_rays
    traced = time.monotonic()

    # Probe gather.
    rays = probe_volume_bake_directions(args.gather_rays)
    basis = radiosity_transfer.sh_basis(rays) * radiosity_transfer.BAND_SCALE
    weight = 4 * math.pi / len(rays)
    gather = []
    for i, position in enumerate(probe_positions):
        entry = {}
        if probe_active[i]:
            hits = first_hits(bvh, owner, patches, np.repeat(position[None], len(rays), 0),
                              rays, limit)
            for k, q in enumerate(hits):
                if q >= 0:
                    entry[q] = entry.get(q, 0.0) + basis[k] * weight
        gather.append(entry)
    gathered = time.monotonic()

    # Keep the patches whose light reaches a probe, directly or through others.
    reached = set()
    frontier = [q for entry in gather for q in entry]
    while frontier:
        q = frontier.pop()
        if q in reached:
            continue
        reached.add(q)
        frontier.extend(r for r in transfer[q] if r not in reached)
    keep = np.array(sorted(reached), dtype=int)
    remap = np.full(count, -1)
    remap[keep] = np.arange(len(keep))

    # Direct light per source: patch receivers (a few samples each) and probe
    # receivers (the PRBV interior directions).
    directions = probe_volume.interior_directions(INTERIOR).reshape(-1, 3)
    quads, owners = [], []
    for new, p in enumerate(keep):
        members = patches.members[p]
        chosen = members[rng.integers(0, len(members), RECEIVERS_PER_PATCH)]
        for s in chosen:
            normal = samples["normal"][s] * patches.side[p]
            quads.append((samples["position"][s] + normal * RECEIVER_OFFSET, normal))
            owners.append(new)
    patch_quads = len(quads)
    for position in probe_positions:
        for d in directions:
            quads.append((position, d))
    device = pbrt_blender.configure_cycles(args.samples, args.device)
    light_paths = pbrt_blender.configure_light_paths(args.light_paths)
    render = bpy.context.scene
    render.cycles.use_adaptive_sampling = False
    render.cycles.seed = SEED
    render.render.bake.use_clear = True
    render.render.bake.margin = 0
    render.render.bake.use_pass_color = False
    receivers, target, height = receiver_mesh(quads)
    args.work.mkdir(parents=True, exist_ok=True)
    owners = np.asarray(owners)
    injection, probe_direct, images = [], [], {}
    for s, source in enumerate(sources):
        isolate(sources, s)
        light, path = bake_direct(receivers, target, height, "RadiosityDirect_%02d" % s, args.work)
        images[source["name"]] = {"exr": path.name, "exr_sha256": sha256(path)}
        per_patch = np.zeros((len(keep), 3))
        np.add.at(per_patch, owners, light[:patch_quads])
        per_patch /= RECEIVERS_PER_PATCH
        injection.append([(int(p), tuple(float(v) for v in per_patch[p]))
                          for p in np.nonzero(per_patch.max(axis=1) > 0)[0]])
        direct = light[patch_quads:patch_quads + len(probe_positions) * len(
            directions)].reshape(len(probe_positions), len(directions), 3).copy()
        direct[~probe_active.astype(bool)] = 0.0  # inside geometry: never sampled
        probe_direct.append(direct)
    isolate(sources, None)
    baked = time.monotonic()

    patch_records = {"position": patches.position[keep] * SOURCE_UNITS_PER_METER,
                     "normal": patches.normal[keep],
                     "area": patches.area[keep] * SOURCE_UNITS_PER_METER ** 2,
                     "albedo": patches.albedo[keep]}
    transfer_rows = [[(int(remap[q]), f) for q, f in transfer[p].items() if remap[q] >= 0]
                     for p in keep]
    gather_rows = [[(int(remap[q]), list(w)) for q, w in entry.items() if remap[q] >= 0]
                   for entry in gather]
    data = radiosity_transfer.build(
        [{"name": s["name"], "kind": s["kind"], "style": s["style"]} for s in sources],
        patch_records, transfer_rows, injection, gather_rows,
        np.clip(np.stack(probe_direct), 0.0, None), radiosity_transfer.prbv_topology_hash(prbv))
    result = radiosity_transfer.Transfer(data, prbv)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(data)

    # Agreement with the probe bake at the baked state: the radiosity model's
    # probe indirect light and the sources' direct light against the PRBV.
    light, _ = result.solve()
    total, indirect = result.probe_light(light)
    volume = probe_volume.Volume(prbv)
    grid = volume.grids[0]
    ref_total, ref_indirect = [], []
    for g in volume.grids:
        for i in range(g.count):
            tiles = []
            for layer in (0, 1):
                ox, oy = g.tile_origin(g.irradiance_origin[layer], probe_volume.IRRADIANCE_TILE,
                                       i)
                tiles.append(g.atlas[oy + 1:oy + 1 + INTERIOR, ox + 1:ox + 1 + INTERIOR, :3]
                             .reshape(-1, 3))
            ref_total.append(tiles[0])
            ref_indirect.append(tiles[1])
    ref_total, ref_indirect = np.array(ref_total), np.array(ref_indirect)
    active = probe_active.astype(bool)

    def relative(a, b):
        return float(np.abs(a[active] - b[active]).mean() / max(np.abs(b[active]).mean(), 1e-9))

    receipt = {"schema": "rtrn-bake/v1", "status": "pass",
               "scene_sha256": scene["source_sha256"], "stage_sha256": sha256(args.stage),
               "prbv_sha256": sha256(args.prbv),
               "environment_sha256": sha256(args.environment) if args.environment else None,
               "rtrn": args.out.name, "rtrn_sha256": hashlib.sha256(data).hexdigest(),
               "rtrn_bytes": len(data), "cycles_device": device, "samples": args.samples,
               "seed": SEED, "light_paths": light_paths, "normal_maps": False,
               "patch_size_m": args.patch_size, "transfer_rays": args.transfer_rays,
               "gather_rays": args.gather_rays, "receivers_per_patch": RECEIVERS_PER_PATCH,
               "sources": [{"name": s["name"], "kind": s["kind"], "style": s["style"]}
                           for s in sources],
               "patches": {"sampled": count, "kept": int(len(keep)),
                           "closed_meshes": int(sum(closed)), "meshes": len(world)},
               **{"info": result.info()},
               "baked_state": {
                   "probe_direct_vs_prbv": relative(total - indirect,
                                                    ref_total - ref_indirect),
                   "probe_indirect_vs_prbv": relative(indirect, ref_indirect),
                   "probe_total_vs_prbv": relative(total, ref_total),
                   "note": "mean absolute difference over active probes' texels, relative "
                           "to the Cycles probe bake; the product publishes the bake plus "
                           "the change, so these bound the change's model error"},
               "bake_images": images,
               "seconds": {"sample": sampled - started, "transfer": traced - sampled,
                           "gather": gathered - traced, "bake": baked - gathered,
                           "total": time.monotonic() - started},
               "baker_sha256": sha256(__file__)}
    del grid
    (args.work / "rtrn-bake.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) +
                                              "\n")
    print("RTRN_BAKE " + json.dumps({"status": "pass", "patches": int(len(keep)),
                                     "sources": [s["name"] for s in sources],
                                     "bytes": len(data), "baked_state": receipt["baked_state"]}))


def probe_volume_bake_directions(count):
    i = np.arange(count) + 0.5
    z = 1.0 - 2.0 * i / count
    r = np.sqrt(np.maximum(0.0, 1.0 - z * z))
    phi = math.pi * (3.0 - math.sqrt(5.0)) * i
    return np.stack([r * np.cos(phi), r * np.sin(phi), z], axis=-1)


if __name__ == "__main__":
    main()
