#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Stage a runnable Portal tree from a Waf build for the dev launcher (run.sh).

This is launcher policy layered on the shared staging owner: it reuses
portal_boot.stage_runtime()/install_build() to seed a runtime and overlay the
newest build/, then sanitizes the result so module loading is deterministic --
every dead 32-bit .so (unusable in the 64-bit engine, and the source of the
"wrong ELF class" log spam) is removed, leaving exactly one loadable module per
name. It does not duplicate the staging knowledge portal_boot owns.
"""

import argparse
from pathlib import Path
import sys

import playable_maps
import portal_boot


def elf_class(path):
    """Return 1 for ELFCLASS32, 2 for ELFCLASS64, None if not an ELF file."""
    try:
        with open(path, "rb") as handle:
            header = handle.read(5)
    except OSError:
        return None
    return header[4] if header[:4] == b"\x7fELF" else None


def sanitize(runtime):
    """Remove dead 32-bit shared objects the 64-bit engine can never load."""
    runtime = Path(runtime)
    removed = []
    for directory in (runtime / "bin", runtime / "portal/bin"):
        if not directory.is_dir():
            continue
        for shared_object in sorted(directory.glob("*.so*")):
            if elf_class(shared_object) == 1:
                shared_object.unlink()
                removed.append(str(shared_object.relative_to(runtime)))
    return removed


def stage(build, runtime, base_runtime=None):
    build, runtime = Path(build).resolve(), Path(runtime).resolve()
    # Seed a runnable tree once (Portal content + third-party libs, big assets
    # shared by symlink); later launches keep it and only re-overlay build/.
    if not (runtime / "portal/gameinfo.txt").is_file():
        if runtime.exists():
            raise SystemExit("run.sh: %s exists but has no portal/gameinfo.txt; "
                             "remove it or point RUNTIME elsewhere" % runtime)
        if not base_runtime or not (Path(base_runtime) / "portal/gameinfo.txt").is_file():
            raise SystemExit("run.sh: no base runtime with portal/gameinfo.txt; "
                             "set BASE_RUNTIME=<dir>")
        runtime.parent.mkdir(parents=True, exist_ok=True)
        print("run.sh: seeding runtime from %s" % base_runtime)
        portal_boot.stage_runtime(base_runtime, runtime)

    installed = portal_boot.install_build(build, runtime)
    removed = sanitize(runtime)
    print("run.sh: overlaid %d build products; removed %d dead 32-bit .so"
          % (len(installed), len(removed)))
    mounted, skipped = playable_maps.mount(runtime)
    for record in mounted.values():
        print("run.sh: PBRT map " + playable_maps.describe(record))
    for name, reason in skipped.items():
        print("run.sh: PBRT map %s not mounted: %s" % (name, reason))
    return installed, removed


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", required=True)
    parser.add_argument("--runtime", required=True)
    parser.add_argument("--base-runtime", default=None)
    args = parser.parse_args(argv)
    stage(args.build, args.runtime, args.base_runtime)
    return 0


if __name__ == "__main__":
    sys.exit(main())
