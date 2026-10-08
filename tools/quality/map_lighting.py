#!/usr/bin/env python3
"""The one lighting back end for every map: a compiled BSP in, a lit BSP2 out.

Front ends differ only in how they make the BSP:

  VMF             vmf_map_build.py --lighting (and Hammer's build, which runs it)
  generators      portal2_gi_chamber.py, and any script that writes a VMF
  a regular compile  vrad_cycles.py (a vrad drop-in)
  a shipped map   legacy_bsp_relight.py
  USD-native      usd_map_compile.py --lighting
  PBRT/USD scene  pbrt_map_build.py --manifest (its own front end compiles a
                  collision BSP from the scene, then lights it here, in one run)

The back end (`pbrt_map_build.Pipeline`) takes the BSP and, optionally, an
authored scene for the visuals (a PBRT or USD scene in the map's space). With
no scene it derives one from the BSP's faces, materials and lights
(`legacy_bsp_scene.py`). It bakes through the one baker seam
(`light_baker.py`: lightmap layers, directional page, reflection probes,
probe volume, radiosity transfer and SDF volume), packs the world mesh and the
lighting lumps into a BSP2, gates that the map's gameplay lumps are carried
byte for byte (`gameplay_identity.py`), and publishes it for ./kiln play portal.

    python3 tools/quality/map_lighting.py --bsp maps/room.bsp --map room \\
        [--scene room.usda] [--quality source2] [--runtime out/portal2-linux-native-vulkan/dev/runtime] \\
        [--medium '{"scattering_per_m": 0.06, "absorption_per_m": 0.01, "anisotropy": 0.3}']

`--medium` (the manifest's `medium`, participating_medium.py) is an explicit,
opt-in participating medium the lightmap bake's light paths cross; without it
the map bakes exactly as before.

`--preview [RUNG]` runs the same steps at one rung of Valve's own three
(`pbrt_map_build.QUALITY_LADDER`, documented there): `preview`, `full-compile`
(bare `--preview`, Valve's F9 default) or `final-compile`, for checking the chain
and the look without a production bake's cost. Only the production profile and
the declared fixture profiles load otherwise; a preview run records
`"preview": true` in its manifest and `"production": false` in its summary, so it
can never be read as a production export.
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import pbrt_map_build  # noqa: E402
import pbrt_map_toolchain  # noqa: E402


def load_toolchain(path=None):
    """The map pipeline's toolchain file (default: the provisioned one)."""
    profile, _ = pbrt_map_toolchain.load_profiles()
    return pbrt_map_toolchain.load(path or ROOT / profile["layout"]["toolchain_file"])


def manifest_for(bsp, name, scene=None, quality=None, game=None, runtime=None, device=None,
                 extra=None, medium=None, preview=False):
    """The back end's manifest for `bsp` (and an authored `scene`)."""
    manifest = {"schema": "pbrt-map-manifest/v1", "map": name, "bsp": str(Path(bsp).resolve()),
                "quality": quality or pbrt_map_build.LEGACY_QUALITY,
                "credit": "Lit from %s; its gameplay lumps are carried unchanged" %
                          Path(bsp).name}
    if scene:
        manifest["scene"] = str(Path(scene).resolve())
    if game:
        manifest["legacy_game"] = str(Path(game).resolve())
    if runtime:
        manifest["legacy_runtime"] = str(Path(runtime).resolve())
    # The lightmap block is merged, not replaced: a front end's `extra` may add
    # this map's seam gate while `--device` still names the Cycles device.
    lightmap = dict((extra or {}).get("lightmap") or {})
    if device:
        lightmap.setdefault("device", device)
    if lightmap:
        manifest["lightmap"] = lightmap
    if preview:
        # The same steps at the named rung of `pbrt_map_build.QUALITY_LADDER`
        # (Valve's own three), for checking the chain and the look. Recorded in
        # the manifest and the summary: not a production export. The top rung is
        # the production profile itself, so naming it asks for production, not a
        # preview of it.
        manifest["quality"] = pbrt_map_build.QUALITY_LADDER[preview]
        if manifest["quality"] != pbrt_map_build.DEFAULT_QUALITY:
            manifest["preview"] = True
    manifest.update({name: value for name, value in (extra or {}).items()
                     if name != "lightmap"})
    if medium is not None:
        if "medium" in (extra or {}):
            raise ValueError("the medium is given once: as `medium`, not in `extra`")
        manifest["medium"] = medium
    return manifest


def light(bsp, name, out, toolchain, scene=None, quality=None, game=None, runtime=None,
          device=None, force_from=None, boot=False, keep_going=False, publish=True, extra=None,
          medium=None, preview=False, max_seam_p99=None):
    """Light the compiled map `bsp` as map `name`, built in `out`.

    `max_seam_p99` waives the stitched seam gate's default 99th percentile for
    this map (the manifest's lightmap.seam_gate, `lightmap_ktx2.py`'s
    `--max-seam-p99`); the value the bake actually measured is recorded either
    way, and it is this map's own recorded limit, not a skipped check.

    `scene` is an authored visual scene (PBRT or USD, in the map's space);
    without one the scene is derived from the BSP. `game` is the directory the
    BSP was compiled against (vbsp/vrad `-game`), searched for materials
    first; `runtime` is the staged game runtime the materials come from
    (default the toolchain's; the portal2 profile's packaged runtime for
    Portal 2). `device`
    overrides every bake's Cycles device. `medium` (participating_medium.py)
    is a participating medium the lightmap bake's light paths cross, recorded
    in the bake receipt; None bakes without one. Returns the gameplay identity;
    raises SystemExit when a step or the identity gate fails (the input BSP is
    never written). A published build is playable with ./kiln play portal <map> (or portal2)."""
    out = Path(out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    extra = dict(extra or {})
    if max_seam_p99 is not None:
        extra.setdefault("lightmap", {})["seam_gate"] = {"p99": max_seam_p99}
    manifest_path = out / "manifest.json"
    manifest_path.write_text(json.dumps(manifest_for(bsp, name, scene, quality, game, runtime,
                                                     device, extra, medium, preview),
                                        indent=2) + "\n")
    manifest = pbrt_map_build.load_manifest(manifest_path)
    try:
        pipeline = pbrt_map_build.Pipeline(manifest, toolchain, out, force_from, boot,
                                           keep_going, publish=publish)
    except ValueError as error:
        if "seam_gate" not in str(error):
            raise
        # The gate has one owner: a per-run limit cannot replace the profile's.
        raise SystemExit("%s\nThe stitched seam gate is owned by the map export profile, so "
                         "a per-map --max-seam-p99 only reaches a fixture lane. Set "
                         "lightmap.seam_gate {\"p99\": x} in the profile the run names (the "
                         "measured value is recorded either way)." % error)
    failure = None
    try:
        pipeline.build()
    except SystemExit as error:
        # A failed candidate can finish for diagnosis but cannot replace the
        # previously published package.
        if not (keep_going and pipeline.failed_gates):
            raise
        failure = error
    identity = json.loads(pipeline.paths["identity"].read_text())
    if publish and not failure:
        print("published; play it with ./kiln play portal " + name)
    if failure:
        raise failure
    return identity


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True, help="a compiled map (v20/v21)")
    parser.add_argument("--map", required=True, help="output map name")
    parser.add_argument("--scene", type=Path, help="an authored visual scene (PBRT or USD)")
    parser.add_argument("--out", type=Path, help="build directory (default "
                        "quality-results/lighting/<map>)")
    parser.add_argument("--quality", help="map export profile (default source2)")
    parser.add_argument("--game", type=Path, help="the BSP's compile game directory")
    parser.add_argument("--runtime", type=Path, help="staged game runtime for materials")
    parser.add_argument("--device", choices=pbrt_map_build.cycles_device.DEVICES)
    parser.add_argument("--toolchain", type=Path)
    parser.add_argument("--from", dest="force_from", choices=pbrt_map_build.STEPS)
    parser.add_argument("--keep-going", action="store_true")
    parser.add_argument("--max-seam-p99", type=float,
                        help="this map's stitched seam gate, 99th percentile (relative), in "
                             "place of lightmap_ktx2.py's 0.002; recorded in the manifest")
    parser.add_argument("--preview", nargs="?", const="full-compile", default=None,
                        choices=sorted(pbrt_map_build.QUALITY_LADDER),
                        metavar="RUNG",
                        help="run every step at this rung of Valve's ladder (%s), for "
                             "checking the chain and the look; the run is recorded as "
                             "non-production. Bare --preview is Valve's Full Compile"
                             % ", ".join("%s=%s" % item for item in
                                         sorted(pbrt_map_build.QUALITY_LADDER.items())))
    parser.add_argument("--no-publish", action="store_true")
    parser.add_argument("--medium", type=json.loads,
                        help="a participating medium for the lightmap bake, as JSON "
                             "(participating_medium.py)")
    args = parser.parse_args()
    light(args.bsp, args.map, args.out or ROOT / "quality-results/lighting" / args.map,
          load_toolchain(args.toolchain), args.scene, args.quality, args.game, args.runtime,
          args.device, args.force_from, keep_going=args.keep_going,
          publish=not args.no_publish, medium=args.medium, preview=args.preview,
          max_seam_p99=args.max_seam_p99)
    return 0


if __name__ == "__main__":
    sys.exit(main())
