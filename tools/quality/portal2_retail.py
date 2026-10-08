"""Retail Portal 2 mirror policy (one owner): the retail mounts, its write
paths and the private write directories a harness's mirror of the Steam
installation gets, so a retail run never writes into the player's install or
Steam Cloud config. Used by the retail-comparison harnesses."""

from pathlib import Path
import shutil


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
