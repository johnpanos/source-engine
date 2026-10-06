#!/usr/bin/env python3
"""Stage licensed Portal 2 content and an independently built game target."""

import argparse
from pathlib import Path
import shutil
import subprocess
import sys

import portal_boot


# The retail Portal 2 filesystem derives these mounts from its single
# "Game |gameinfo_path|." line: update, then the highest DLC folder down to
# dlc1, then portal2, each with its pak01 VPK ahead of its loose files, plus a
# GameBin path per game directory. This engine's SDK 2013 filesystem does none
# of that implicitly, so the staged gameinfo spells the same order out.
RETAIL_OVERLAY_DIRS = ("update", "portal2_dlc2", "portal2_dlc1")
# The first of them is retail's write path: portal2_linux saves config.cfg
# (every archived ConVar) to update/cfg, and Steam Cloud syncs that file.
RETAIL_WRITE_DIR = RETAIL_OVERLAY_DIRS[0]


# What retail portal2_linux writes besides update/cfg, seen in an installation
# by modification time after install: update/glshaders.cfg,
# update/pak01.vpk.sound.cache, update/sound/sound.cache,
# update/save/game_instructor_counts.txt (the player's hint counts), and
# portal2/SAVE/<steamid>/*.sav, which Steam Auto-Cloud syncs
# (steam_autocloud.vdf). In update/ only VPKs and these read-only content
# directories are linked; every other entry is private.
RETAIL_SHARED_UPDATE_DIRS = ("resource", "scripts")
# Write directories a mirror starts empty: no player saves or hint counts in,
# no harness saves out.
RETAIL_EMPTY_WRITE_DIRS = ("update/save", "portal2/SAVE", "portal2/screenshots")
# A retail mirror's write paths, which a retail launch declares to the launch sandbox.
RETAIL_WRITE_PATHS = ("update", "update/cfg") + RETAIL_EMPTY_WRITE_DIRS + ("portal2/cfg",)
# A retail launch's first arguments: settings from the mirror's own
# cfg/config.cfg, never the player's Steam Cloud config. With cloud settings
# on, retail loads the cloud config at every map start (over the command
# line) and saves the session's archived cvars back into it on exit
# (hud_quickinfo 0, joystick 0, ...), which then apply to the player's game.
RETAIL_ENGINE_ARGS = ("+cl_cloud_settings", "0")
# The offline AV1 transcodes of the retail movies (tools/video/transcode_av1.py),
# linked into the runtime under this name. Each game directory's media is mounted
# just ahead of that directory, keeping retail's precedence between same-named
# movies (portal2_dlc2/media/valve.bik over portal2's).
AV1_MEDIA_LINK = "media_av1"
# The default transcode root (./play_p2's P2_AV1_MEDIA). AV1 is the launcher's
# default video provider, so every staging mounts it when it exists.
DEFAULT_AV1_MEDIA = Path(__file__).resolve().parents[2] / "run/media-av1"


def private_retail_write_dir(steam_root, mirror):
    """Give a retail mirror private write locations.

    A mirror that links update/ whole lets the retail binary save a harness's
    settings (hud_quickinfo 0, closecaption 0, ...) into the installation: into
    the player's Steam Cloud config, and into every staged runtime, whose
    gameinfo mounts that update/ ahead of its own cfg. update/ becomes a
    directory of links to the installation's VPKs and read-only content, with
    private copies of cfg/ and every other entry and an empty save/.
    portal2/SAVE and portal2/screenshots start empty; callers link the rest of
    portal2/ afterwards and skip entries that exist. A linked portal2_linux is
    replaced by a copy: the engine takes its base directory, and so where it
    writes, from the executable's resolved path. Links in mirrors made before
    are converted.
    """
    steam_root = Path(steam_root).resolve()
    source = steam_root / RETAIL_WRITE_DIR
    target = Path(mirror) / RETAIL_WRITE_DIR
    if target.is_symlink():
        target.unlink()
    target.mkdir(parents=True, exist_ok=True)
    for child in source.iterdir():
        link = target / child.name
        if child.name == "cfg" or "%s/%s" % (RETAIL_WRITE_DIR, child.name) in \
                RETAIL_EMPTY_WRITE_DIRS:
            continue
        shared = child.suffix.lower() == ".vpk" or child.name in RETAIL_SHARED_UPDATE_DIRS
        if shared:
            if not link.exists() and not link.is_symlink():
                link.symlink_to(child)
            continue
        if link.is_symlink():
            link.unlink()
        if link.exists():
            continue
        if child.is_dir():
            shutil.copytree(child, link, symlinks=True)
        else:
            shutil.copy2(child, link)
    if not (target / "cfg").is_dir():
        shutil.copytree(source / "cfg", target / "cfg", symlinks=True)
    for relative in RETAIL_EMPTY_WRITE_DIRS:
        path = Path(mirror) / relative
        if path.is_symlink():
            path.unlink()
        path.mkdir(parents=True, exist_ok=True)
    executable = Path(mirror) / "portal2_linux"
    if executable.is_symlink():
        executable.unlink()
        shutil.copy2(steam_root / "portal2_linux", executable)
    return target


def retail_search_paths(contents, mount_custom=False, av1_media=()):
    """av1_media: the game directories with an AV1 media tree under AV1_MEDIA_LINK."""
    lines = ["\t\tSearchPaths", "\t\t{"]
    if mount_custom:
        # Ahead of retail content, as in the Portal gameinfo: the Android app
        # installs its touch-control icons into <game>/custom/android_touch.
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|custom/*")
    for name in RETAIL_OVERLAY_DIRS:
        if name in av1_media:
            lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s/%s" % (AV1_MEDIA_LINK, name))
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s/pak01_dir.vpk" % name)
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s" % name)
    if "portal2" in av1_media:
        lines.append("\t\t\tgame+mod\t\t\t|gameinfo_path|../%s/portal2" % AV1_MEDIA_LINK)
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


def stage_av1_media(runtime, av1_media):
    """Link the AV1 content root into the runtime; return the game dirs it serves.

    None removes the link, so a runtime staged without AV1 media mounts none."""
    link = runtime / AV1_MEDIA_LINK
    if av1_media is None:
        if link.is_symlink():
            link.unlink()
        return ()
    av1_media = Path(av1_media).resolve()
    if not (av1_media / "manifest.json").is_file():
        raise ValueError("%s has no AV1 transcodes (tools/video/transcode_av1.py)" % av1_media)
    if link.is_symlink() and link.resolve() != av1_media:
        link.unlink()
    elif link.exists() and not link.is_symlink():
        raise ValueError("staged %s is not a link" % AV1_MEDIA_LINK)
    if not link.is_symlink():
        link.symlink_to(av1_media, target_is_directory=True)
    return tuple(sorted(p.name for p in av1_media.iterdir() if (p / "media").is_dir()))


def default_av1_media():
    """The default AV1 root when it has transcodes, else None (and a warning)."""
    if (DEFAULT_AV1_MEDIA / "manifest.json").is_file():
        return DEFAULT_AV1_MEDIA
    print("stage_portal2_runtime: no AV1 movies in %s; run tools/video/transcode_av1.py "
          "(movies will not play with the default av1 provider)" % DEFAULT_AV1_MEDIA,
          file=sys.stderr)
    return None


def stage_content(steam_root, runtime, mount_custom=False, av1_media="default"):
    """av1_media: an AV1 transcode root, "default" for DEFAULT_AV1_MEDIA when
    present, or None to mount none."""
    if av1_media == "default":
        av1_media = default_av1_media()
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
    av1_dirs = stage_av1_media(runtime, av1_media)
    gameinfo = runtime / "portal2/gameinfo.txt"
    contents = gameinfo.read_text()
    staged = retail_search_paths(contents, mount_custom, av1_dirs)
    if staged != contents:  # keep the modification time a device sync compares
        gameinfo.write_text(staged)

    # The old GameUI expects these legacy names, while Portal 2 ships the same
    # menu artwork as portal2_product_1{,_widescreen}.vtf in its VPK. The
    # startup graphic is chosen by the display's aspect ratio and a missing
    # image is fatal, so both are required (a 4:3 display uses the plain one).
    for suffix in ("_widescreen", ""):
        background = runtime / ("portal2/materials/console/background_menu%s.vtf" % suffix)
        if background.is_file():
            continue
        background.parent.mkdir(parents=True, exist_ok=True)
        with background.open("wb") as output:
            result = subprocess.run(
                ["vpk", "--pipe", "--filter",
                 "materials/console/portal2_product_1%s.vtf" % suffix, str(source_vpk)],
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
    parser.add_argument("--mount-custom", action="store_true",
                        help="also mount <game>/custom/* (the Android app's touch icons)")
    parser.add_argument("--mount-published", action="store_true",
                        help="mount the map pipeline's published maps (run/maps) as "
                             "portal2/custom/pbrt-<map>, as ./play does for Portal "
                             "(implies --mount-custom)")
    parser.add_argument("--av1-media", type=Path,
                        help="the AV1 transcode root (tools/video/transcode_av1.py) mounted "
                             "ahead of each game directory (default: run/media-av1 when it "
                             "has transcodes)")
    parser.add_argument("--no-av1-media", action="store_true",
                        help="mount no AV1 movies (for -video-provider bink)")
    args = parser.parse_args(argv)
    try:
        vpk = stage_content(args.steam_root, args.runtime,
                            args.mount_custom or args.mount_published,
                            None if args.no_av1_media else args.av1_media or "default")
        print("Portal 2 content: " + str(vpk.resolve()))
        if args.mount_published:
            import playable_maps
            mounted, skipped = playable_maps.mount(args.runtime, game="portal2")
            for record in mounted.values():
                print("play_p2: published map " + playable_maps.describe(record))
            for name, reason in skipped.items():
                print("play_p2: published map %s not mounted: %s" % (name, reason))
        if args.build:
            launcher = install_source_build(args.build, args.runtime)
            print("Portal 2 source launcher: " + str(launcher))
    except (OSError, ValueError) as error:
        parser.exit(1, "play_p2: %s\n" % error)
    return 0


if __name__ == "__main__":
    sys.exit(main())
