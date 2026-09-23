#!/usr/bin/env python3
"""Stage licensed Portal 2 content and an independently built game target."""

import argparse
from pathlib import Path
import sys

import portal_boot


def stage_content(steam_root, runtime):
    steam_root, runtime = Path(steam_root).resolve(), Path(runtime).resolve()
    source_vpk = steam_root / "portal2/pak01_dir.vpk"
    if not source_vpk.is_file():
        raise ValueError("Portal 2 installation lacks portal2/pak01_dir.vpk")
    if not runtime.exists():
        portal_boot.stage_runtime(steam_root, runtime, game="portal2", content_only=True)
    staged_vpk = runtime / "portal2/pak01_dir.vpk"
    if not staged_vpk.is_symlink() or staged_vpk.resolve() != source_vpk:
        raise ValueError("staged Portal 2 VPK does not point to the selected Steam installation")
    if not (runtime / "portal2/gameinfo.txt").is_file():
        raise ValueError("staged Portal 2 gameinfo.txt is missing")
    return staged_vpk


def install_source_build(build, runtime):
    build, runtime = Path(build).resolve(), Path(runtime).resolve()
    caches = list((build / "c4che").glob("*_cache.py"))
    if not any("GAMES = 'portal2'" in cache.read_text() or
               'GAMES = "portal2"' in cache.read_text() for cache in caches):
        raise ValueError("build directory is not configured with --build-games=portal2")
    portal_boot.install_build(build, runtime, game="portal2")
    gamebin = runtime / "portal2/bin"
    for name in ("libclient.so", "libserver.so"):
        if not (gamebin / name).is_file():
            raise ValueError("Portal 2 source build lacks " + name)
    return runtime / "hl2_launcher"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--steam-root", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path)
    args = parser.parse_args(argv)
    try:
        vpk = stage_content(args.steam_root, args.runtime)
        print("Portal 2 content: " + str(vpk.resolve()))
        if args.build:
            launcher = install_source_build(args.build, args.runtime)
            print("Portal 2 source launcher: " + str(launcher))
    except (OSError, ValueError) as error:
        parser.exit(1, "play_p2: %s\n" % error)
    return 0


if __name__ == "__main__":
    sys.exit(main())
