#!/usr/bin/env python3
"""Stage the F-Stop target: Portal content, F-Stop content overlay, fstop build.

The F-Stop product is the Portal 1 game plus Valve's F-Stop prototype sources
(`./waf configure --build-games=fstop`, game/{server,client}/fstop). Its
runtime is a Portal runtime (seeded once from --base-runtime, assets shared by
symlink) with an `fstop` game directory in front of Portal's content:

  fstop/                the MOD and write path; the fstop client/server in bin/
  fstop_content/        the F-Stop content root's asset directories (links)
  fstop_imported/       the content root's imported_fstop asset directories
  Portal and HL2        exactly as portal/gameinfo.txt mounts them

Only asset directories of the content root are mounted. Its cfg, resource and
script trees belong to the DLLs it shipped with, so the only scripts taken are
the F-Stop weapon scripts and sound files that Portal lacks; the staged sound
manifest is Portal's with those sound files appended, and the staged
resource/fstop_english.txt is Portal's strings plus the F-Stop ones it lacks.
"""

import argparse
from pathlib import Path
import re
import shutil
import sys

import portal_boot
import source_content
import stage_runtime


GAME = "fstop"
# Asset directories mounted from <content-root>/fstop and <content-root>/imported_fstop.
CONTENT_DIRECTORIES = ("maps", "materials", "models", "particles", "sound", "scenes",
                       "expressions", "media")
IMPORTED_DIRECTORIES = ("materials", "models", "sound")
# F-Stop scripts Portal does not have. Sound files are added to the manifest.
WEAPON_SCRIPTS = ("weapon_camera.txt", "weapon_placement.txt")
SOUND_SCRIPTS = ("game_sounds_fstop.txt", "game_sounds_props_aperture.txt",
                 "npc_sounds_android.txt", "npc_sounds_chicken.txt",
                 "npc_sounds_mannequin.txt", "npc_sounds_zombie_aperture.txt")
# Resource files the F-Stop HUD loads (LoadControlSettings) that Portal lacks.
RESOURCE_FILES = ("photoinventory.res",)
# F-Stop HUD elements (game/client/fstop) the staged HUD layout must place.
HUD_ELEMENTS = ("HudControlHelper", "HudPhotoInventory", "HudViewfinder", "HudIndicator")
# Portal gameinfo entries that make Portal the MOD or its gamebin. Portal's loose
# directory stays mounted, read-only.
PORTAL_OWNED_PATHS = {"|gameinfo_path|.", "portal/bin"}
# Kept ahead of everything, as in the Portal gameinfo (portal_boot.shader_search_path).
SHADER_OVERLAY = "source-engine-shaders"

BLUE_BLOB_MATERIAL = '''"VertexLitGeneric"
{
    "$basetexture" "models/Weapons/V_physics_gun/glueblob"
    "$envmap" "env_cubemap"
    "$color2" "[0.1 0.6 0.9]"
}
'''


def find_child(directory, name):
    """The entry of `directory` named `name`, ignoring case (content is from Windows)."""
    for child in directory.iterdir():
        if child.name.lower() == name.lower():
            return child
    return None


def link_assets(source_root, destination, names):
    """Replace `destination` with links to the present asset directories of `source_root`."""
    if destination.is_symlink() or destination.is_file():
        destination.unlink()
    elif destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)
    linked = []
    for name in names:
        source = find_child(source_root, name)
        if source is not None and source.is_dir():
            (destination / name).symlink_to(source.resolve())
            linked.append(name)
    return linked


def search_paths(portal_gameinfo):
    """F-Stop's SearchPaths lines: fstop and its overlay, then Portal's content paths."""
    start = portal_gameinfo.find("SearchPaths")
    open_brace = portal_gameinfo.find("{", start)
    close_brace = portal_gameinfo.find("}", open_brace)
    if start < 0 or open_brace < 0 or close_brace < 0:
        raise ValueError("portal/gameinfo.txt has no SearchPaths block")
    first, lines = [], [
        "\t\t\tgame+mod+mod_write+game_write+default_write_path\t|gameinfo_path|.",
        "\t\t\tgamebin\t\t\t\t|gameinfo_path|bin",
        "\t\t\tgame\t\t\t\tfstop_content",
        "\t\t\tgame\t\t\t\tfstop_imported",
    ]
    for line in portal_gameinfo[open_brace + 1:close_brace].splitlines():
        entry = line.split("//", 1)[0].split()
        if len(entry) != 2 or entry[1] in PORTAL_OWNED_PATHS:
            continue
        kinds = [kind for kind in entry[0].split("+") if kind not in {"mod", "mod_write",
                                                                       "game_write",
                                                                       "default_write_path"}]
        if kinds:
            (first if SHADER_OVERLAY in entry[1] else lines).append(
                "\t\t\t%s\t\t\t%s" % ("+".join(kinds), entry[1]))
    return portal_gameinfo[:start], first + lines, portal_gameinfo[close_brace + 1:]


def write_gameinfo(runtime):
    portal_gameinfo = (runtime / "portal/gameinfo.txt").read_text(encoding="latin-1")
    head, lines, tail = search_paths(portal_gameinfo)
    head = re.sub(r'(\n\s*game\s+)"[^"]*"', r'\1"F-Stop"', head, count=1)
    head = re.sub(r'(\n\s*title\s+)"[^"]*"', r'\1"F-STOP"', head, count=1)
    text = head + "SearchPaths\n\t\t{\n" + "\n".join(lines) + "\n\t\t}" + tail
    (runtime / GAME / "gameinfo.txt").write_text(text, encoding="latin-1")


def write_scripts(runtime, content_scripts):
    scripts = runtime / GAME / "scripts"
    scripts.mkdir(parents=True, exist_ok=True)
    copied = []
    for name in WEAPON_SCRIPTS + SOUND_SCRIPTS:
        source = find_child(content_scripts, name) if content_scripts.is_dir() else None
        if source is not None and source.is_file():
            shutil.copyfile(source, scripts / name)
            copied.append(name)
    found = source_content.ContentResolver(runtime).read("scripts/game_sounds_manifest.txt")
    manifest = found[0] if found else None
    if manifest is None:
        raise ValueError("the Portal runtime has no scripts/game_sounds_manifest.txt")
    manifest = manifest.decode("latin-1")
    close = manifest.rfind("}")
    added = "".join('\t"precache_file"\t\t"scripts/%s"\n' % name
                    for name in SOUND_SCRIPTS if name in copied)
    manifest = manifest[:close] + "\n\t// F-Stop\n" + added + manifest[close:]
    (scripts / "game_sounds_manifest.txt").write_text(manifest, encoding="latin-1")
    return copied


def layout_block(text, name):
    """The `name { ... }` top-level block of a HudLayout.res, or None."""
    match = re.search(r'(?m)^\s*"?%s"?\s*$' % re.escape(name), text)
    if not match:
        return None
    open_brace = text.find("{", match.end())
    depth, index = 0, open_brace
    while index < len(text):
        depth += {"{": 1, "}": -1}.get(text[index], 0)
        if depth == 0:
            return "\t%s\n\t%s" % (name, text[open_brace:index + 1].strip())
        index += 1
    return None


def write_hud_layout(runtime, content_scripts):
    """scripts/hudlayout.res: Portal's layout plus the F-Stop HUD elements.

    The content root's layout carries the F-Stop element names; HudIndicator
    (game/client/fstop/hud_indicator.cpp) is not in it and gets the same
    full-screen default its siblings have.
    """
    found = source_content.ContentResolver(runtime).read("scripts/hudlayout.res")
    if not found or found[0] is None:
        raise ValueError("the Portal runtime has no scripts/hudlayout.res")
    portal = found[0].decode("latin-1")
    source = find_child(content_scripts, "hudlayout.res") if content_scripts.is_dir() else None
    fstop = source.read_text(encoding="latin-1").replace("\r", "") if source else ""
    blocks = []
    for name in HUD_ELEMENTS:
        if re.search(r'(?m)^\s*"?%s"?\s*$' % re.escape(name), portal):
            continue
        block = layout_block(fstop, name) or (
            '\t%s\n\t{\n\t\t"fieldName" "%s"\n\t\t"visible" "1"\n\t\t"enabled" "1"\n'
            '\t\t"wide"\t "640"\n\t\t"tall"\t "480"\n\t}' % (name, name))
        blocks.append(block)
    close = portal.rfind("}")
    text = portal[:close] + "\n\t// F-Stop\n" + "\n".join(blocks) + "\n" + portal[close:]
    (runtime / GAME / "scripts" / "hudlayout.res").write_text(text, encoding="latin-1")
    return len(blocks)


def write_blob_material(runtime):
    """Adapt Portal 2's blue gel color to a shader the F-Stop renderer supports."""
    materials = runtime / GAME / "materials/fstop"
    materials.mkdir(parents=True, exist_ok=True)
    (materials / "blob_surface_bounce.vmt").write_text(BLUE_BLOB_MATERIAL,
                                                        encoding="ascii")


def decode_localization(data):
    if data[:2] in (b"\xff\xfe", b"\xfe\xff"):
        return data.decode("utf-16")
    return data.decode("utf-8-sig", errors="replace")


def write_localization(runtime, content_resource):
    """resource/fstop_english.txt: Portal's tokens plus the content root's missing ones.

    The engine loads resource/<mod>_<language>.txt (vgui_baseui_interface.cpp),
    so with the fstop game directory Portal's portal_english.txt is not read.
    """
    found = source_content.ContentResolver(runtime).read("resource/portal_english.txt")
    if not found or found[0] is None:
        raise ValueError("the Portal runtime has no resource/portal_english.txt")
    portal = decode_localization(found[0])
    token = re.compile(r'^\s*"([^"]+)"\s+"((?:[^"\\]|\\.)*)"\s*(\[[^\]]*\])?\s*$')
    present = {match.group(1).lower() for match in map(token.match, portal.splitlines()) if match}
    extra = []
    source = find_child(content_resource, "fstop_english.txt") if content_resource.is_dir() else None
    if source is not None:
        for line in decode_localization(source.read_bytes()).splitlines():
            match = token.match(line)
            if match and not match.group(3) and match.group(1).lower() not in present:
                present.add(match.group(1).lower())
                extra.append('\t\t"%s"\t\t"%s"' % (match.group(1), match.group(2)))
    tokens = portal.find('"Tokens"')
    close = portal.rfind("}", 0, portal.rfind("}"))
    if tokens < 0 or close < 0:
        raise ValueError("resource/portal_english.txt has no Tokens block")
    text = portal[:close] + "\n\t\t// F-Stop\n" + "\n".join(extra) + "\n\t" + portal[close:]
    resource = runtime / GAME / "resource"
    resource.mkdir(parents=True, exist_ok=True)
    (resource / "fstop_english.txt").write_bytes(b"\xff\xfe" + text.encode("utf-16-le"))
    for name in RESOURCE_FILES:
        control = find_child(content_resource, name) if content_resource.is_dir() else None
        if control is not None and control.is_file():
            shutil.copyfile(control, resource / name)
    return len(extra)


def stage(runtime, base_runtime, content_root, build=None):
    runtime = Path(runtime).resolve()
    content_root = Path(content_root).resolve()
    fstop_content = find_child(content_root, "fstop") if content_root.is_dir() else None
    if fstop_content is None:
        raise SystemExit("stage_fstop_runtime: %s has no fstop content directory" % content_root)
    if not (runtime / "portal/gameinfo.txt").is_file():
        if runtime.exists():
            raise SystemExit("stage_fstop_runtime: %s exists but has no portal/gameinfo.txt"
                             % runtime)
        if not base_runtime or not (Path(base_runtime) / "portal/gameinfo.txt").is_file():
            raise SystemExit("stage_fstop_runtime: no base Portal runtime; pass --base-runtime")
        runtime.parent.mkdir(parents=True, exist_ok=True)
        print("stage_fstop_runtime: seeding %s from %s" % (runtime, base_runtime))
        portal_boot.stage_runtime(Path(base_runtime), runtime)
    (runtime / GAME / "bin").mkdir(parents=True, exist_ok=True)
    linked = link_assets(fstop_content, runtime / "fstop_content", CONTENT_DIRECTORIES)
    imported = find_child(content_root, "imported_fstop")
    linked_imported = (link_assets(imported, runtime / "fstop_imported", IMPORTED_DIRECTORIES)
                       if imported is not None else [])
    write_gameinfo(runtime)
    write_blob_material(runtime)
    content_scripts = find_child(fstop_content, "scripts") or fstop_content / "scripts"
    copied = write_scripts(runtime, content_scripts)
    hud = write_hud_layout(runtime, content_scripts)
    tokens = write_localization(runtime,
                                find_child(fstop_content, "resource") or fstop_content / "resource")
    print("stage_fstop_runtime: content %s; imported %s; scripts %s; %d HUD elements; "
          "%d F-Stop tokens" % (",".join(linked) or "-", ",".join(linked_imported) or "-",
                                ",".join(copied) or "-", hud, tokens))
    if build is not None:
        installed = portal_boot.install_build(Path(build), runtime, game=GAME)
        removed = stage_runtime.sanitize(runtime)
        print("stage_fstop_runtime: overlaid %d build products; removed %d dead 32-bit .so"
              % (len(installed), len(removed)))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--base-runtime", type=Path,
                        help="a Portal runtime (portal/gameinfo.txt) to seed a new runtime from")
    parser.add_argument("--content-root", type=Path, required=True,
                        help="directory holding the F-Stop game content (fstop/, imported_fstop/)")
    parser.add_argument("--build", type=Path, help="a Waf tree configured with --build-games=fstop")
    args = parser.parse_args(argv)
    stage(args.runtime, args.base_runtime, args.content_root, args.build)
    return 0


if __name__ == "__main__":
    sys.exit(main())
