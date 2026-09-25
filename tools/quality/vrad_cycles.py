#!/usr/bin/env python3
"""vrad, then the Blender/Cycles light: a drop-in for a compile's light step.

    tools/quality/vrad_cycles.py [vrad options] <map>
    tools/quality/vrad_cycles.py -game <gamedir> -final maps/mymap --cycles-quality \\
        legacy-relight-preview

Anything that runs vrad (Hammer's `$light_exe`, a compile script doing
vbsp, vvis, vrad) can run this instead, with the same command line. It runs
the real vrad with every argument except its own `--cycles-*` options. Once
vrad has written the BSP, it relights that BSP with the map pipeline
(`legacy_bsp_relight.relight`, the same path as a shipped map's relight):

  * Cycles bakes the world's light, the probe volume, the radiosity transfer
    and the SDF volume from the map's faces, materials and vrad's lights;
  * the relit BSP2 carries every gameplay lump of vrad's BSP unchanged,
    including vvis's visibility and vrad's lightmaps for the faces the relight
    leaves to the legacy renderer;
  * the map is published as `<map>_relit` for `./play`.

vrad's `-game` (or `-vproject`, or $VPROJECT) directory is searched for the
map's materials before the pipeline's game runtime, as vrad searched it.
vrad's own BSP is never written by the relight, so a failed bake leaves the
regular compile's output intact; the exit status is then nonzero. A vrad
failure stops before the relight with vrad's exit status.

Options for this script (removed before vrad runs):
  --cycles-vrad PATH       the vrad to run (default: the pipeline toolchain's)
  --cycles-quality NAME    map export profile (default legacy-relight)
  --cycles-map-name NAME   published map name (default <map>_relit)
  --cycles-out DIR         build directory (default quality-results/relight/<name>)
  --cycles-toolchain FILE  pipeline toolchain file (default the provisioned one)
  --cycles-from STEP       rebuild this pipeline step and every later one
  --cycles-device DEVICE   Cycles device for every bake: gpu (the profiles'),
                           cpu or auto (explicit opt-ins for GPU-less hosts)
  --cycles-boot            boot the relit map headless on native Vulkan
  --cycles-keep-going      finish the map when a pixel gate fails
  --cycles-no-publish      do not publish the map for ./play
"""

import os
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import legacy_bsp_relight  # noqa: E402
from cycles_device import DEVICES  # noqa: E402
from pbrt_map_build import LEGACY_QUALITY, STEPS  # noqa: E402

VALUE_OPTIONS = {"--cycles-vrad", "--cycles-quality", "--cycles-map-name", "--cycles-out",
                 "--cycles-toolchain", "--cycles-from", "--cycles-device"}
FLAG_OPTIONS = {"--cycles-boot", "--cycles-keep-going", "--cycles-no-publish"}
# vrad options naming the game directory (vrad.cpp ParseCommandLine).
GAME_OPTIONS = {"-game", "-vproject"}


class UsageError(Exception):
    pass


def split_arguments(argv):
    """(this script's options, vrad's arguments in their order)."""
    options, vrad = {}, []
    index = 0
    while index < len(argv):
        argument = argv[index]
        name, equals, value = argument.partition("=")
        if name in VALUE_OPTIONS:
            if not equals:
                index += 1
                if index == len(argv):
                    raise UsageError(name + " needs a value")
                value = argv[index]
            options[name[len("--cycles-"):].replace("-", "_")] = value
        elif argument in FLAG_OPTIONS:
            options[argument[len("--cycles-"):].replace("-", "_")] = True
        elif argument.startswith("--cycles-"):
            raise UsageError("unknown option " + argument)
        else:
            vrad.append(argument)
        index += 1
    return options, vrad


def map_bsp(vrad_args, cwd):
    """The BSP vrad writes, or None when the command names no map: vrad's map
    is its last argument; it strips any extension and loads `<map>.bsp`."""
    if not vrad_args or vrad_args[-1].startswith("-"):
        return None
    # The last argument may be the value of an option, not a map.
    if len(vrad_args) > 1 and vrad_args[-2].lower() in GAME_OPTIONS:
        return None
    path = Path(vrad_args[-1])
    path = path.with_suffix(".bsp") if path.suffix else path.with_name(path.name + ".bsp")
    return path if path.is_absolute() else Path(cwd) / path


def game_directory(vrad_args, environ, cwd):
    """vrad's game directory: the last -game/-vproject value, else $VPROJECT."""
    game = None
    for index, argument in enumerate(vrad_args[:-1]):
        if argument.lower() in GAME_OPTIONS:
            game = vrad_args[index + 1]
    game = game or environ.get("VPROJECT")
    if not game:
        return None
    path = Path(game)
    return path if path.is_absolute() else Path(cwd) / path


def default_map_name(bsp):
    name = bsp.stem.lower() + "_relit"
    if not re.fullmatch(r"[a-z0-9_]+", name):
        raise UsageError("map name %r is not [a-z0-9_]; give --cycles-map-name" % name)
    return name


def main(argv=None, environ=None, cwd=None):
    argv = sys.argv[1:] if argv is None else argv
    environ = os.environ if environ is None else environ
    cwd = Path.cwd() if cwd is None else Path(cwd)
    try:
        options, vrad_args = split_arguments(argv)
        bsp = map_bsp(vrad_args, cwd)
        name = options.get("map_name") or (default_map_name(bsp) if bsp else None)
        if options.get("from") and options["from"] not in STEPS:
            raise UsageError("--cycles-from takes one of: " + ", ".join(STEPS))
        if options.get("device") and options["device"] not in DEVICES:
            raise UsageError("--cycles-device takes one of: " + ", ".join(DEVICES))
    except UsageError as error:
        print("vrad_cycles: " + str(error), file=sys.stderr)
        return 2
    for key in ("out", "toolchain"):
        if options.get(key):
            options[key] = cwd / options[key]
    toolchain = legacy_bsp_relight.load_toolchain(options.get("toolchain"))
    vrad = options.get("vrad") or str(Path(toolchain["compile_tools"]) / "vrad")
    status = subprocess.run([vrad] + vrad_args, cwd=cwd).returncode
    if status or bsp is None:
        # A failed vrad, or a command with no map (-help): nothing to relight.
        return status
    if not bsp.is_file():
        print("vrad_cycles: vrad succeeded but wrote no %s" % bsp, file=sys.stderr)
        return 1
    print("vrad_cycles: relighting %s with Cycles as %s" % (bsp, name), flush=True)
    try:
        legacy_bsp_relight.relight(
            bsp, name, options.get("out") or legacy_bsp_relight.default_out(name), toolchain,
            options.get("quality") or LEGACY_QUALITY,
            game_directory(vrad_args, environ, cwd), options.get("from"),
            options.get("boot", False), options.get("keep_going", False),
            not options.get("no_publish", False), options.get("device"))
    except SystemExit as failure:
        print("vrad_cycles: the Cycles relight failed (%s); vrad's %s is unchanged" %
              (failure, bsp), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
