#!/usr/bin/env python3
"""Drive the Portal 2 advanced video menu's render core lighting rows.

The generated includes are exact slices of production code:
  vadvancedvideo.cpp  the command prefixes and choice names, the rows' layout
                      (AddRenderCoreQualityRow and the render core part of
                      PreApplyControlSettings, which sets the rows' lists and
                      navigation), their strings, the row lookup,
                      SetRenderCoreQualityState, the preset and row branches of
                      OnCommand, NavigateToChild and UpdateDescription;
  vhybridbutton.cpp   the list button's keyboard/controller, mouse click,
                      hover and selection handlers.
The fixture lays the rows out from a resource tree, makes each a list button
from its laid-out list, and drives every row by keyboard, controller and mouse
as a player does, checking what the rows and the description then show. It
also checks the presets against the render_quality settings of the Portal 2
product profile. No display, game assets or VGUI singleton is needed.

Seeded defects must fail the same checks:
  --seed-stale-preset   a row choice no longer refreshes the rows, so the
                        preset row keeps showing Low/High after an edit
  --seed-profile-drift  the profile's Low keeps runtime direct light on
  --seed-broken-nav     Down from the preset row skips Ambient Occlusion
  --seed-mouse-reversed a click left of the choice selects the next one
"""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import conformance

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "game/client/portal2/gameui/portal2/vadvancedvideo.cpp"
BUTTON_SOURCE = "game/client/portal2/gameui/portal2/vhybridbutton.cpp"
BUTTON_METHODS = (
    "void BaseModHybridButton::OnKeyCodePressed( vgui::KeyCode code )",
    "void BaseModHybridButton::ChangeDialogListSelection( ListSelectionChange_t eNext )",
    "void BaseModHybridButton::SetCurrentSelection( const char *pText )",
    "const char *BaseModHybridButton::GetCurrentSelection()",
    "void BaseModHybridButton::OnMousePressed( vgui::MouseCode code )",
    "bool BaseModHybridButton::GetDialogListButtonCenter( int &x, int &y )",
    "void BaseModHybridButton::OnCursorEntered()",
)
FIXTURE = "unittests/vguitest/test_render_core_lighting_menu.cpp"
PROFILE = "quality/product_profiles/portal2-linux-native-vulkan-high.json"
CONVARS = ("r_core_ao_quality", "r_core_shadow_quality", "r_core_depth_prepass",
           "r_core_shadow_movers", "r_core_runtime_direct")


def block(source, start, end_marker="\n}"):
    """The text from `start` (which must occur once) through the end marker."""
    if source.count(start) != 1:
        raise ValueError("expected one production definition: " + start.strip())
    begin = source.index(start)
    end = source.index(end_marker, begin) + len(end_marker)
    return source[begin:end]


def button_slice(seed_mouse_reversed=False):
    source = (ROOT / BUTTON_SOURCE).read_text().replace("\r\n", "\n")
    text = "\n\n".join(block(source, signature + "\n{") for signature in BUTTON_METHODS) + "\n"
    if seed_mouse_reversed:
        if text.count("if ( iPosX < nButtonX )") != 1:
            raise ValueError("the click side test moved")
        text = text.replace("if ( iPosX < nButtonX )", "if ( iPosX > nButtonX )")
    return text


def menu_slice(seed_stale_preset=False, seed_broken_nav=False):
    source = (ROOT / SOURCE).read_text().replace("\r\n", "\n")
    parts = [line for line in source.splitlines() if line.startswith("#define VIDEO_CORE_")]
    for array in ("s_RenderCoreQualityNames", "s_RenderCoreToggleNames",
                  "s_RenderCorePresetNames"):
        parts.append(block(source, "static const char *const %s[] = {" % array, "\n};"))
    parts.append(block(source, "static const char *RenderCoreRowPrefix( RenderCoreLightingRow row )"))
    parts.append(block(source, "static const RenderCoreLightingRow *RenderCoreRowOfCommand("))
    parts.append(block(source, "static void AddRenderCoreQualityStrings()"))
    parts.append(block(source, "static KeyValues *AddRenderCoreQualityRow("))
    # The render core part of PreApplyControlSettings, from its own guard.
    layout = block(source, "void CAdvancedVideo::PreApplyControlSettings( KeyValues *pResourceData )")
    layout = layout[layout.index("\tif ( pResourceData->FindKey( \"DrpCoreAO\" )"):]
    if seed_broken_nav:
        broken = layout.replace('pPreset->SetString( "navDown", "DrpCoreAO" );',
                                'pPreset->SetString( "navDown", "DrpCoreShadows" );')
        if broken == layout:
            raise ValueError("the preset row's navigation link moved")
        layout = broken
    parts.append("void Menu::LayOut( KeyValues *pResourceData )\n{\n" + layout)
    for method in ("void CAdvancedVideo::NavigateToChild( Panel *pNavigateTo )",
                   "void CAdvancedVideo::UpdateDescription( Panel *pControl )"):
        parts.append(block(source, method).replace("CAdvancedVideo::", "Menu::"))
    state = block(source, "void CAdvancedVideo::SetRenderCoreQualityState()")
    parts.append(state.replace("void CAdvancedVideo::", "void Menu::"))
    # The two OnCommand branches, from the preset branch up to the next one.
    begin = source.index("\telse if ( StringHasPrefix( command, VIDEO_CORE_PRESET_COMMAND_PREFIX ) )")
    end = source.index("\telse if ( !V_stricmp( \"Cancel\", command )", begin)
    branches = source[begin:end]
    if branches.count("SetRenderCoreQualityState();") != 2:
        raise ValueError("expected the preset and row branches to refresh the rows")
    if seed_stale_preset:
        row = branches.index("RenderCoreRowOfCommand( command )")
        branches = branches[:row] + branches[row:].replace(
            "\t\tSetRenderCoreQualityState();\n", "", 1)
    parts.append("bool Menu::OnCommand( const char *command )\n{\n\tif ( false )\n\t{\n\t}\n" +
                 branches + "\telse\n\t\treturn false;\n\treturn true;\n}")
    return "\n\n".join(parts) + "\n"


def profile_presets(seed_profile_drift=False):
    """The profile's low and high values of the five ConVars, as an include."""
    profile = json.loads((ROOT / PROFILE).read_text())
    quality = profile["intent"]["render_quality"]
    lines = []
    for name in ("low", "high"):
        settings = dict(quality[name]["settings"])
        if seed_profile_drift and name == "low":
            settings["r_core_runtime_direct"] = "1"
        values = ", ".join(str(int(settings[convar])) for convar in CONVARS)
        lines.append("static const gameui::RenderCoreLighting kProfile%s = { %s };"
                     % (name.capitalize(), values))
    return "\n".join(lines) + "\n"


def run(output, seed_stale_preset=False, seed_profile_drift=False, seed_broken_nav=False,
        seed_mouse_reversed=False):
    output.mkdir(parents=True, exist_ok=True)
    (output / "render_core_lighting_button.inc").write_text(button_slice(seed_mouse_reversed))
    (output / "render_core_lighting_menu.inc").write_text(
        menu_slice(seed_stale_preset, seed_broken_nav))
    (output / "render_core_lighting_profile.inc").write_text(
        profile_presets(seed_profile_drift))
    profile = conformance.load_profile(str(ROOT / "quality/profiles"), "linux-headless-core")
    profile["generated_include_roots"] = []
    executable = output / "render_core_lighting_menu_test"
    command = conformance.build_command(
        str(ROOT), os.environ.get("CONFORMANCE_CXX", "g++"), profile,
        # vhybridbutton.cpp is a permissive-mode legacy file: it binds L"" to
        # wchar_t * and writes if ( ( a == b ) ), which clang rejects.
        {"sources": [FIXTURE], "extra_flags": ["-I", str(output), "-Wno-write-strings",
                                               "-Wno-parentheses-equality"]},
        str(executable), config="release")
    (output / "compile-command.json").write_text(json.dumps(command, indent=2) + "\n")
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=60)
    (output / "compile.log").write_text(compiled.stdout + compiled.stderr)
    if compiled.returncode:
        print(compiled.stdout + compiled.stderr)
        return compiled.returncode
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    (output / "run.log").write_text(result.stdout + result.stderr)
    print(result.stdout, end="")
    print(result.stderr, end="", file=sys.stderr)
    return result.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=os.environ.get("CONFORMANCE_OUT"))
    parser.add_argument("--seed-stale-preset", action="store_true")
    parser.add_argument("--seed-profile-drift", action="store_true")
    parser.add_argument("--seed-broken-nav", action="store_true")
    parser.add_argument("--seed-mouse-reversed", action="store_true")
    args = parser.parse_args()
    if not args.out:
        parser.error("--out (or CONFORMANCE_OUT) is required")
    return run(args.out.resolve(), args.seed_stale_preset, args.seed_profile_drift,
               args.seed_broken_nav, args.seed_mouse_reversed)


if __name__ == "__main__":
    sys.exit(main())
