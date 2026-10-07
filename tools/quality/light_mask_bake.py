"""Bake the static lights' shadow masks (LSMK, light_shadow_masks.py) of a lit map.

Run inside Blender; `pbrt_map_build.py`'s `light-masks` step drives it
through light_baker.py. It reads the lighting stage the lightmap bake wrote
(pbrt_lightmap_bake.py: its authored `lightmap_st`, so the masks share the
atlas's texels) and the map scene, and bakes only direct light: per group of
lights with disjoint reaches, the group's direct diffuse light with and
without shadows. Their ratio is each texel's visibility of the group's light;
each texel keeps its four dominant lights (Source 2's static-light shadow
masks; the runtime samples them in place of a shadow map).

Writes --out-exr (visibility, RGBA), <stem>-ids.exr (the lights' ids as
floats) and <out-exr>.json, the receipt lightmap_ktx2.py packs from.
"""

import argparse
import hashlib
import json
import math
import re
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bake_progress  # noqa: E402
import light_shadow_masks  # noqa: E402
import map_scene  # noqa: E402
import pbrt_blender  # noqa: E402

# A ratio of two same-seed bakes, so their noise is correlated; each group is
# one pair. Baked SUPERSAMPLE times finer per axis and averaged down
# (light_shadow_masks.area_visibility): Cycles bakes a texel at one point, so
# without it a hard edge is all or nothing per texel (stair-steps). SAMPLES
# per sub-texel keeps 64 per texel.
SUPERSAMPLE = 4
SAMPLES = 4
BAKE_TILE = 1024


def announce(label, size):
    print(bake_progress.progress_line(event="bake", label=label, size=[size, size],
                                      samples=bpy.context.scene.cycles.samples), flush=True)


def message(text):
    print(bake_progress.progress_line(message=text), flush=True)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def light_reach_m(shape):
    """Where an emitter's diffuse light falls to light_shadow_masks.REACH_CUTOFF
    lightmap units (the runtime's reach, render.pass.lights' kReachCutoff):
    radiance times its projected area over pi d^2, times vrad's falloff
    relative to an inverse square, within its hard radius."""
    peak = max(shape["emission"]["radiance"]) * shape["emission"].get("scale", 1.0)
    area = shape.get("area_m2") or 0.0
    projected = area / 4.0 if shape["shape"]["kind"] == "sphere" else area
    attenuation = shape.get("attenuation")
    hard = (attenuation or {}).get("radius_m", 0.0)

    def light(d):
        value = peak * projected / (math.pi * d * d)
        return value * map_scene.vrad_falloff(attenuation, d) if attenuation else value
    low, high = 1e-3, 1e4
    if light(high) > light_shadow_masks.REACH_CUTOFF:
        return hard or 0.0  # unbounded: keeps runtime shadows unless hard-limited
    for _ in range(60):
        mid = math.sqrt(low * high)
        low, high = (mid, high) if light(mid) > light_shadow_masks.REACH_CUTOFF else (low, mid)
    return min(high, hard) if hard > 0 else high


def bake_light_masks(merged, scene, path, size, render):
    """Static lights' shadow masks (light_shadow_masks.py, LSMK). Lights are
    grouped so a group's reaches are disjoint; per group, its direct diffuse
    light with / without shadows gives each texel its light and visibility,
    and each texel keeps its four dominant groups. Writes `path` (visibility,
    RGBA) and `<path stem>-ids.exr` (the ids as floats); None when no
    emitter is a map light."""
    import numpy as np
    meters = float(scene.get("meters_per_unit") or 0.0254)
    candidates = []
    for index, shape in enumerate(scene["emitters"]):
        # legacy_bsp_scene's map lights: <entity or type>_<world light> under
        # the stage's lights scope; the runtime matches them by origin.
        if not re.search(r"/Lights/[^/]*_\d+$", shape.get("source") or "") or \
                not pbrt_blender.lamp_kind(shape):
            continue
        candidates.append({"index": index, "source": shape["source"],
                           "origin": [v / meters for v in shape["shape"]["centre"]],
                           "radius": light_reach_m(shape) / meters})
    ids = light_shadow_masks.group_lights(candidates)
    lamps = {}
    for light, light_id in zip(candidates, ids):
        if light_id is None:
            continue
        names = pbrt_blender.emitter_objects(light["index"], scene["emitters"][light["index"]])
        lamps.setdefault(light_id, []).extend(
            bpy.data.objects[name] for name in names if name in bpy.data.objects)
    if not lamps:
        return None
    lit_objects = [obj for obj in bpy.data.objects if obj.hide_render is False and
                   (obj.type == "LIGHT" or obj.name.startswith(pbrt_blender.EMITTER_PREFIXES))]
    world = render.world
    background = world.node_tree.nodes.get("Background") if world and world.use_nodes else None
    strength = background.inputs["Strength"].default_value if background else None
    if background:
        background.inputs["Strength"].default_value = 0.0
    samples = render.cycles.samples
    render.cycles.samples = SAMPLES
    render.render.bake.use_pass_indirect = False
    top = light_shadow_masks.TopFour(size, size)
    # A checkpoint after each group, so a crashed bake (a GPU fault) resumes
    # where it stopped; it is keyed by the groups' lights and the size.
    checkpoint = path.with_name(path.name + ".partial.npz")
    key = json.dumps([[light["source"], light_id] for light, light_id in zip(candidates, ids)] +
                     [size, SAMPLES, SUPERSAMPLE])
    done = set()
    if checkpoint.is_file():
        saved = np.load(checkpoint)
        if str(saved["key"]) == key:
            top.light, top.visibility, top.ids = saved["light"], saved["visibility"], saved["ids"]
            done = set(int(v) for v in saved["done"])
            message("resuming light masks after %d of %d groups" % (len(done), len(lamps)))
    try:
        for light_id in sorted(lamps):
            if light_id in done:
                continue
            keep = set(lamps[light_id])
            for obj in lit_objects:
                obj.hide_render = obj not in keep
            planes = []
            for shadows in (True, False):
                for obj in keep:
                    if obj.type == "LIGHT":
                        obj.data.use_shadow = shadows
                fine = size * SUPERSAMPLE
                image = bpy.data.images.new("PbrtLightMask%d%d" % (light_id, shadows),
                                            width=fine, height=fine, alpha=True,
                                            float_buffer=True)
                for material in {slot.material for slot in merged.material_slots}:
                    target = material.node_tree.nodes["BakeTarget"]
                    target.image = image
                    material.node_tree.nodes.active = target
                announce("light mask group %d/%d %s" % (light_id, len(lamps),
                                                        "shadowed" if shadows else "unshadowed"),
                         fine)
                with pbrt_blender.direct_only_light_paths():
                    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT"})
                # foreach_get into one buffer: image.pixels[:] would build a
                # Python list of every float (tens of GB at this size).
                pixels = np.empty(fine * fine * 4, np.float32)
                image.pixels.foreach_get(pixels)
                planes.append(pixels.reshape(fine, fine, 4)[..., :3].mean(axis=2))
                del pixels
                bpy.data.images.remove(image)
            for obj in keep:
                if obj.type == "LIGHT":
                    obj.data.use_shadow = True
            open_light, visibility = light_shadow_masks.area_visibility(
                planes[0], planes[1], SUPERSAMPLE)
            top.add(light_id, np.where(open_light > 1e-6, open_light, 0.0), visibility)
            done.add(light_id)
            np.savez(str(checkpoint.with_suffix("")), key=key, light=top.light,
                     visibility=top.visibility, ids=top.ids, done=sorted(done))
    finally:
        for obj in lit_objects:
            obj.hide_render = False
        render.cycles.samples = samples
        render.render.bake.use_pass_indirect = True
        if background:
            background.inputs["Strength"].default_value = strength
        for material in {slot.material for slot in merged.material_slots}:
            material.node_tree.nodes["BakeTarget"].image = bpy.data.images["PbrtLightmap"]
    id_path = path.with_name(path.stem + "-ids.exr")
    for name, values, out in (("PbrtLightMasks", top.visibility, path),
                              ("PbrtLightMaskIds", top.ids, id_path)):
        image = bpy.data.images.new(name, width=size, height=size, alpha=True, float_buffer=True)
        image.pixels.foreach_set(values.astype(np.float32).ravel())
        image.save_render(filepath=str(out.resolve()), scene=render)
        if not out.is_file():
            raise RuntimeError("Cycles did not save " + out.name)
    checkpoint.unlink(missing_ok=True)
    return {"records": [{"origin": light["origin"], "id": light_id, "source": light["source"],
                         "reach_units": light["radius"]}
                        for light, light_id in zip(candidates, ids) if light_id is not None],
            "without_id": [light["source"] for light, light_id in zip(candidates, ids)
                           if light_id is None],
            "groups": len(lamps), "samples": SAMPLES, "supersample": SUPERSAMPLE,
            "exr_sha256": sha256(path), "ids_exr_sha256": sha256(id_path)}


def main(arguments):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True,
                        help="the lighting stage (pbrt_lightmap_bake.py --out-stage)")
    parser.add_argument("--out-exr", type=Path, required=True)
    parser.add_argument("--size", type=int, required=True)
    parser.add_argument("--device", choices=pbrt_blender.DEVICES, default="gpu")
    parser.add_argument("--seed", type=int, default=pbrt_blender.SEED)
    parser.add_argument("--exclude-material", action="append", default=[])
    parser.add_argument("--margin-texels", type=int, default=2)
    args = parser.parse_args(arguments)
    scene = map_scene.parse(args.scene)
    _, unbaked = map_scene.lightmap_exclusions(scene, args.exclude_material)
    pbrt_blender.clear_scene()
    message("importing the lighting stage")
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import the lighting stage")
    meshes = pbrt_blender.source_meshes()
    baked = []
    for obj in meshes:
        layer = obj.data.uv_layers.get("lightmap_st")
        if not layer:
            raise ValueError("lighting stage mesh lacks lightmap_st: " + obj.name)
        obj.data.uv_layers.active = layer
        if obj.name not in unbaked:
            baked.append(obj)
    if not baked:
        raise ValueError("no meshes to bake")
    pbrt_blender.rebind_materials(scene, normal_maps=False)
    pbrt_blender.restore_emitters(scene)
    props = map_scene.nonstatic_shape_names(scene)
    for obj in meshes:
        if obj.name in props:
            obj.hide_render = True
    device = pbrt_blender.configure_cycles(SAMPLES, args.device)
    pbrt_blender.pin_sampling(seed=args.seed)
    render = bpy.context.scene
    render.cycles.use_auto_tile = True
    render.cycles.tile_size = BAKE_TILE
    render.render.bake.use_clear = True
    # In the supersampled bake's texels.
    render.render.bake.margin = max(1, args.margin_texels // 2) * SUPERSAMPLE
    render.render.bake.use_pass_color = False
    render.render.bake.use_pass_direct = True
    render.render.bake.use_pass_indirect = False
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    atlas = bpy.data.images.new("PbrtLightmap", width=args.size, height=args.size, alpha=True,
                                float_buffer=True)
    # One merged, bake-only copy (pbrt_lightmap_bake.py's): the originals
    # hide from render so light transport sees the geometry once.
    bpy.ops.object.select_all(action="DESELECT")
    for obj in baked:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = baked[0]
    bpy.ops.object.duplicate()
    bpy.ops.object.join()
    merged = bpy.context.view_layer.objects.active
    merged.data.uv_layers.active = merged.data.uv_layers.get("lightmap_st")
    for obj in baked:
        obj.hide_render = True
    for material in {slot.material for slot in merged.material_slots}:
        if not material or not material.use_nodes:
            raise ValueError("baked mesh has no Cycles material")
        node = material.node_tree.nodes.new("ShaderNodeTexImage")
        node.name = "BakeTarget"
        node.image = atlas
        material.node_tree.nodes.active = node
    bpy.ops.object.select_all(action="DESELECT")
    merged.select_set(True)
    bpy.context.view_layer.objects.active = merged
    args.out_exr.parent.mkdir(parents=True, exist_ok=True)
    masks = bake_light_masks(merged, scene, args.out_exr, args.size, render)
    receipt = {"status": "pass" if masks else "none", "scope": "light-shadow-masks",
               "lighting_stage_sha256": sha256(args.stage), "size": args.size,
               "device": device, "light_masks": masks, "blender": bpy.app.version_string}
    args.out_exr.with_name(args.out_exr.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print("LIGHT_MASK_BAKE " + json.dumps({"status": receipt["status"],
                                            "lights": len(masks["records"]) if masks else 0,
                                            "groups": masks["groups"] if masks else 0}))


if __name__ == "__main__":
    main(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
