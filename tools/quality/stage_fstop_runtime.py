#!/usr/bin/env python3
"""Stage the F-Stop target: Portal content, Valve's F-Stop content, fstop build.

The F-Stop product is the Portal 1 game plus Valve's F-Stop prototype sources
(`./waf configure --build-games=fstop`, game/{server,client}/fstop). Its
runtime is a Portal runtime (seeded once from --base-runtime, assets shared by
symlink) with an `fstop` game directory in front of Portal's content:

  fstop/                      the MOD and write path; the fstop client/server in bin/
  Portal and HL2              exactly as portal/gameinfo.txt mounts them
  fstop_valve/                Valve's portal2 asset directories (links)
  fstop_valve_tempcontent/    Valve's portal2_tempcontent asset directories (links)

The content root is Valve's own F-Stop-era tree: Steam2 depot 852 version 0
(extracted; its top level holds portal2/ and portal2_tempcontent/). F-Stop
was developed inside that Portal 2 tree, so its camera, photo, HUD, chicken,
farm and android assets live there. They are mounted after Portal and HL2, so
Portal's own content is unchanged and only what Portal lacks comes from Valve.
The depot's maps are Portal 2 maps for Portal 2 game code and are not mounted.

Scripts and strings come from the same tree: Valve's sound scripts for the
F-Stop NPCs and props, its F-Stop HUD layout and PhotoInventory.res, and its
fstop_* strings. The depot had already dropped the camera and placement weapon
scripts (its weapon_manifest.txt comments them out) and the Weapon_Camera
sounds, so those are authored here against Valve's models and sound files.
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
# (content-root directory, runtime link directory, asset directories mounted from it).
VALVE_CONTENT = (
    ("portal2", "fstop_valve", ("materials", "models", "sound", "scenes", "expressions",
                                "particles")),
    ("portal2_tempcontent", "fstop_valve_tempcontent", ("materials", "models", "sound")),
)
# Directories earlier stagings mounted from the Lever Softworks remake's content.
RETIRED_LINKS = ("fstop_content", "fstop_imported")
# Valve's F-Stop sound scripts (portal2/scripts); each is added to the manifest.
VALVE_SOUND_SCRIPTS = ("game_sounds_props_aperture.txt", "game_sounds_spheres_auto_generated.txt",
                       "npc_sounds_android.txt", "npc_sounds_chicken.txt",
                       "npc_sounds_hover_turret.txt", "npc_sounds_mannequin.txt",
                       "npc_sounds_zombie_aperture.txt")
# Valve's F-Stop particle files (portal2/particles) the F-Stop entities name and
# Portal's manifest lacks: airvent_* and geyser_* (prop_air_vent, prop_geyser),
# feathers (npc_chicken), fizzler_* (trigger_photo_eraser) and zombie_* (zombies).
VALVE_PARTICLES = ("airvents.pcf", "chicken.pcf", "fizzler.pcf", "geyser.pcf", "zombie.pcf")
# HL2 sound scripts (already mounted) for the NPCs F-Stop spawns: prop_tombstone's
# zombies with their headcrabs, and the android's strider footsteps.
HL2_SOUND_SCRIPTS = ("npc_sounds_zombie.txt", "npc_sounds_headcrab.txt",
                     "npc_sounds_strider.txt")
# Scripts the depot no longer carries, authored against its models and sounds.
AUTHORED_SCRIPTS = {
    "weapon_camera.txt": """WeaponData
{
	"printname"		"#FSTOP_Camera"
	"viewmodel"		"models/weapons/v_cam.mdl"
	"playermodel"		"models/weapons/w_cam.mdl"
	"anim_prefix"		"cam"
	"bucket"		"0"
	"bucket_position"	"0"
	"clip_size"		"1"
	"primary_ammo"		"None"
	"secondary_ammo"	"None"
	"weight"		"4"
	"item_flags"		"0"
	"autoswitchto"		"1"
	SoundData
	{
		"single_shot"		"Weapon_Camera.Capture"
		"single_shot_npc"	"Weapon_Camera.Capture"
	}
}
""",
    "weapon_placement.txt": """WeaponData
{
	"printname"		"#FSTOP_Placement"
	"viewmodel"		"models/weapons/v_photo.mdl"
	"playermodel"		"models/weapons/w_cam.mdl"
	"anim_prefix"		"cam"
	"bucket"		"1"
	"bucket_position"	"0"
	"clip_size"		"1"
	"primary_ammo"		"None"
	"secondary_ammo"	"None"
	"weight"		"4"
	"item_flags"		"0"
	"autoswitchto"		"1"
	SoundData
	{
		"single_shot"		"Weapon_Portalgun.fire_blue"
		"double_shot"		"Weapon_Portalgun.fire_red"
	}
}
""",
    "game_sounds_fstop.txt": """"Weapon_Camera.Capture"
{
	"channel"		"CHAN_WEAPON"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"wave"		"camera/snapshot.wav"
}

"Weapon_Camera.Release"
{
	"channel"		"CHAN_WEAPON"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"wave"		"camera/release.wav"
}

"PhotoInventory.Erased"
{
	"channel"		"CHAN_ITEM"
	"volume"		"0.9"
	"soundlevel"	"SNDLVL_NORM"
	"rndwave"
	{
		"wave"	"camera/photo_erase1.wav"
		"wave"	"camera/photo_erase2.wav"
		"wave"	"camera/photo_erase3.wav"
	}
}
""",
}
AUTHORED_SOUND_SCRIPTS = ("game_sounds_fstop.txt",)
# Strings the authored weapon scripts name; Valve's are taken by prefix.
AUTHORED_TOKENS = {"FSTOP_Camera": "CAMERA", "FSTOP_Placement": "PHOTOS"}
VALVE_TOKEN_PREFIX = "fstop_"
# Resource files the F-Stop HUD loads (LoadControlSettings) that Portal lacks.
RESOURCE_FILES = ("photoinventory.res", "controlhelper.res", "indicator.res")
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
    """Replace `destination` with a lowercase mirror of `source_root`'s asset directories.

    The engine looks assets up by lowercase path and Valve's tree came from a
    case-insensitive Windows filesystem (materials/HUD, models/Camera), so each
    file is linked under its lowercased relative path. Of names that differ only
    in case, the first in sorted order is kept. Returns the mirrored directory
    names and the number of files dropped that way.
    """
    if destination.is_symlink() or destination.is_file():
        destination.unlink()
    elif destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)
    linked, shadowed = [], 0
    for name in names:
        source = find_child(source_root, name)
        if source is None or not source.is_dir():
            continue
        linked.append(name)
        for path in sorted(source.rglob("*")):
            if not path.is_file():
                continue
            target = destination / name / path.relative_to(source).as_posix().lower()
            if target.exists() or target.is_symlink():
                shadowed += 1
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            target.symlink_to(path.resolve())
    return linked, shadowed


def search_paths(portal_gameinfo):
    """F-Stop's SearchPaths lines: fstop, Portal's content paths, then Valve's F-Stop content."""
    start = portal_gameinfo.find("SearchPaths")
    open_brace = portal_gameinfo.find("{", start)
    close_brace = portal_gameinfo.find("}", open_brace)
    if start < 0 or open_brace < 0 or close_brace < 0:
        raise ValueError("portal/gameinfo.txt has no SearchPaths block")
    first, lines = [], [
        "\t\t\tgame+mod+mod_write+game_write+default_write_path\t|gameinfo_path|.",
        "\t\t\tgamebin\t\t\t\t|gameinfo_path|bin",
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
    lines += ["\t\t\tgame\t\t\t\t%s" % link for _, link, _ in VALVE_CONTENT]
    return portal_gameinfo[:start], first + lines, portal_gameinfo[close_brace + 1:]


def write_gameinfo(runtime):
    portal_gameinfo = (runtime / "portal/gameinfo.txt").read_text(encoding="latin-1")
    head, lines, tail = search_paths(portal_gameinfo)
    head = re.sub(r'(\n\s*game\s+)"[^"]*"', r'\1"F-Stop"', head, count=1)
    head = re.sub(r'(\n\s*title\s+)"[^"]*"', r'\1"F-STOP"', head, count=1)
    text = head + "SearchPaths\n\t\t{\n" + "\n".join(lines) + "\n\t\t}" + tail
    (runtime / GAME / "gameinfo.txt").write_text(text, encoding="latin-1")


def write_scripts(runtime, content_scripts):
    """Valve's F-Stop sound scripts, the authored ones, and Portal's manifest naming them all."""
    scripts = runtime / GAME / "scripts"
    scripts.mkdir(parents=True, exist_ok=True)
    copied = []
    for name in VALVE_SOUND_SCRIPTS:
        source = find_child(content_scripts, name) if content_scripts.is_dir() else None
        if source is not None and source.is_file():
            shutil.copyfile(source, scripts / name)
            copied.append(name)
    for name, text in AUTHORED_SCRIPTS.items():
        (scripts / name).write_text(text, encoding="latin-1")
    found = source_content.ContentResolver(runtime).read("scripts/game_sounds_manifest.txt")
    manifest = found[0] if found else None
    if manifest is None:
        raise ValueError("the Portal runtime has no scripts/game_sounds_manifest.txt")
    manifest = manifest.decode("latin-1")
    close = manifest.rfind("}")
    added = "".join('\t"precache_file"\t\t"scripts/%s"\n' % name
                    for name in copied + list(AUTHORED_SOUND_SCRIPTS + HL2_SOUND_SCRIPTS))
    manifest = manifest[:close] + "\n\t// F-Stop\n" + added + manifest[close:]
    (scripts / "game_sounds_manifest.txt").write_text(manifest, encoding="latin-1")
    return copied


def write_particles(runtime):
    """particles/particles_manifest.txt: Portal's manifest plus Valve's F-Stop particle files."""
    found = source_content.ContentResolver(runtime).read("particles/particles_manifest.txt")
    if not found or found[0] is None:
        raise ValueError("the Portal runtime has no particles/particles_manifest.txt")
    manifest = found[0].decode("latin-1")
    listed = {m.lower() for m in re.findall(r'"file"\s+"!?particles/([^"]+)"', manifest)}
    added = [name for name in VALVE_PARTICLES if name not in listed]
    close = manifest.rfind("}")
    manifest = (manifest[:close] + "\n\t// F-Stop (Valve's portal2 particles)\n" +
                "".join('\t"file"\t\t"particles/%s"\n' % name for name in added) + manifest[close:])
    particles = runtime / GAME / "particles"
    particles.mkdir(parents=True, exist_ok=True)
    (particles / "particles_manifest.txt").write_text(manifest, encoding="latin-1")
    return added


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
    """resource/fstop_english.txt: Portal's tokens plus Valve's F-Stop ones and the authored ones.

    Valve's are the fstop_* tokens of its portal2_english.txt that Portal lacks.

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
    source = (find_child(content_resource, "portal2_english.txt")
              if content_resource.is_dir() else None)
    candidates = list(AUTHORED_TOKENS.items())
    if source is not None:
        for line in decode_localization(source.read_bytes()).splitlines():
            match = token.match(line)
            if (match and not match.group(3)
                    and match.group(1).lower().startswith(VALVE_TOKEN_PREFIX)):
                candidates.append((match.group(1), match.group(2)))
    for name, value in candidates:
        if name.lower() not in present:
            present.add(name.lower())
            extra.append('\t\t"%s"\t\t"%s"' % (name, value))
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
    roots = {name: find_child(content_root, name) if content_root.is_dir() else None
             for name, _, _ in VALVE_CONTENT}
    if not all(root is not None and root.is_dir() for root in roots.values()):
        raise SystemExit("stage_fstop_runtime: %s is not Valve's depot 852 tree (needs %s)"
                         % (content_root, " and ".join(roots)))
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
    for name in RETIRED_LINKS:
        retired = runtime / name
        if retired.is_symlink() or retired.is_file():
            retired.unlink()
        elif retired.exists():
            shutil.rmtree(retired)
    linked, shadowed = [], 0
    for name, link, directories in VALVE_CONTENT:
        mirrored, dropped = link_assets(roots[name], runtime / link, directories)
        linked += ["%s/%s" % (name, directory) for directory in mirrored]
        shadowed += dropped
    write_gameinfo(runtime)
    write_blob_material(runtime)
    valve = roots["portal2"]
    content_scripts = find_child(valve, "scripts") or valve / "scripts"
    copied = write_scripts(runtime, content_scripts)
    particles = write_particles(runtime)
    hud = write_hud_layout(runtime, content_scripts)
    tokens = write_localization(runtime, find_child(valve, "resource") or valve / "resource")
    print("stage_fstop_runtime: Valve content %s (%d case duplicates skipped); Valve sound "
          "scripts %s; particles %s; %d HUD elements; %d F-Stop tokens"
          % (",".join(linked) or "-", shadowed, ",".join(copied) or "-",
             ",".join(particles) or "-", hud, tokens))
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
                        help="Valve's extracted depot 852 tree (portal2/, portal2_tempcontent/)")
    parser.add_argument("--build", type=Path, help="a Waf tree configured with --build-games=fstop")
    args = parser.parse_args(argv)
    stage(args.runtime, args.base_runtime, args.content_root, args.build)
    return 0


if __name__ == "__main__":
    sys.exit(main())
