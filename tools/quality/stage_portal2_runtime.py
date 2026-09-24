#!/usr/bin/env python3
"""Stage licensed Portal 2 content and an independently built game target."""

import argparse
from pathlib import Path
import subprocess
import sys

import portal_boot


# The retail Portal 2 filesystem derives these mounts from its single
# "Game |gameinfo_path|." line: update, then the highest DLC folder down to
# dlc1, then portal2, each with its pak01 VPK ahead of its loose files, plus a
# GameBin path per game directory. This engine's SDK 2013 filesystem does none
# of that implicitly, so the staged gameinfo spells the same order out.
RETAIL_OVERLAY_DIRS = ("update", "portal2_dlc2", "portal2_dlc1")


def retail_search_paths(contents):
    lines = ["\t\tSearchPaths", "\t\t{"]
    for name in RETAIL_OVERLAY_DIRS:
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s/pak01_dir.vpk" % name)
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s" % name)
    lines += [
        "\t\t\tgame+mod\t\t\t|gameinfo_path|pak01_dir.vpk",
        "\t\t\tgame+mod+mod_write+game_write+default_write_path\t|gameinfo_path|.",
        "\t\t\tgamebin\t\t\t\t|gameinfo_path|bin",
        "\t\t\tplatform\t\t\t|gameinfo_path|../platform",
        "\t\t\tplatform\t\t\t|gameinfo_path|../retail_platform",
        "\t\t}",
    ]
    start = contents.find("\t\tSearchPaths")
    open_brace = contents.find("{", start)
    close_brace = contents.find("}", open_brace)
    if start < 0 or open_brace < 0 or close_brace < 0:
        raise ValueError("staged Portal 2 gameinfo.txt has no SearchPaths block")
    return contents[:start] + "\n".join(lines) + contents[close_brace + 1:]


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
    for name in RETAIL_OVERLAY_DIRS + ("platform",):
        source = steam_root / name
        link = runtime / ("retail_platform" if name == "platform" else name)
        if not (source / ("pak01_dir.vpk" if name != "platform" else "resource")).exists():
            raise ValueError("Portal 2 installation lacks " + name)
        if link.is_symlink() and link.resolve() == source:
            continue
        if link.exists() or link.is_symlink():
            raise ValueError("staged %s is not a link to the selected Steam installation" % link.name)
        link.symlink_to(source, target_is_directory=True)
    gameinfo = runtime / "portal2/gameinfo.txt"
    gameinfo.write_text(retail_search_paths(gameinfo.read_text()))

    # The old GameUI expects this legacy name, while Portal 2 ships the same
    # menu artwork under portal2_product_1_widescreen.vtf in its VPK.
    background = runtime / "portal2/materials/console/background_menu_widescreen.vtf"
    if not background.is_file():
        background.parent.mkdir(parents=True, exist_ok=True)
        with background.open("wb") as output:
            result = subprocess.run(
                ["vpk", "--pipe", "--filter", "materials/console/portal2_product_1_widescreen.vtf", str(source_vpk)],
                stdout=output, stderr=subprocess.PIPE, check=False)
        if result.returncode or background.stat().st_size < 4:
            background.unlink(missing_ok=True)
            raise ValueError("could not stage the installed Portal 2 menu image")
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
