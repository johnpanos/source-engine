"""The scenarios of tools/quality/fstop_mechanics_check.py, one per F-Stop mechanic.

Each names what the entity's code does (game/server/fstop) and how the log
shows it. Steps are (game seconds after the scenario starts, command); "client:" commands
run on the player's client. Positions are given in the map's rooms (shops):
setpos(room, dx, dy, z) is (dx, dy) from the room's floor center.
"""

from fstop_mechanics_check import (Scenario, count_at_least, count_change, displaced, dumped,
                                   emitted, fired, health_dropped, jumped, logged, moved,
                                   not_fired, not_logged, peak, samples)
from fstop_mechanics_map import setpos

TRACE = "sv_soundemitter_trace 1"
CAMERA = "client:give weapon_camera"


def photograph(at, setpos):
    """Stand at setpos (setpos/setang console text), raise the camera and take a photo."""
    return [(at, "client:%s; give weapon_camera" % setpos), (at + 1, "client:use weapon_camera")] \
        + press(at + 2, "attack") + press(at + 3, "attack")


def place(at, setpos):
    """Stand at setpos and place the photo (raise the placement tool, place)."""
    return [(at, "client:" + setpos)] + press(at + 1, "attack") + press(at + 2, "attack")


def press(at, button, frames=6):
    """A client button press: +button, then -button some frames later."""
    return [(at, "client:+%s; wait %d; -%s" % (button, frames, button))]


def scaled_placement(wheel):
    """Photograph the barrel with a scaling camera, raise the placement tool, turn
    the wheel (its default binds, invprev/invnext) and place it; dump the barrel."""
    steps = [(1, "client:%s; setang 20 0 0; give weapon_camera" % setpos("camera", -170, 0)),
             (2, "ent_fire weapon_camera SetScaleAbility 1"),
             (2, "client:use weapon_camera")] + press(3, "attack") + press(4, "attack") \
        + [(6, "client:setang 30 30 0")] + press(7, "attack")
    steps += [(8 + i, "client:" + command) for i, command in enumerate(wheel)]
    at = 9 + len(wheel)
    return steps + press(at, "attack") + [(at + 2, "ent_dump capturable_0")]

SCENARIOS = [
    Scenario(
        "mall_doors",
        "a shop's sliding doors open for a player in front of them and close after; the "
        "neighbouring shop's stay shut",
        steps=[(1, "client:%s; setang 0 90 0" % setpos("camera", 0, -400)),
               (4, "client:setpos -300 0 8")],
        checks=[fired("door.camera.left", "OnFullyOpen"),
                fired("door.camera.right", "OnFullyOpen"),
                fired("door.camera.left", "OnFullyClosed"),
                fired("door.camera.right", "OnFullyClosed"),
                not_fired("door.size.left", "OnFullyOpen"),
                not_fired("door.photo_lab.left", "OnFullyOpen")]),
    Scenario(
        "dispenser",
        "prop_android_dispenser dispenses an android, and another after it dies",
        steps=[
            (1, "report_entities"),
            (2, "ent_fire npc_android_basic SetHealth 0"),
            (12, "report_entities"),
        ],
        checks=[
            fired("dispenser", "OnSpawnNPC", 2),
            fired("dispenser", "OnChildKilled", 1),
            count_at_least("npc_android_basic", 1),
        ]),
    Scenario(
        "camera_capture",
        "weapon_camera photographs the barrel, weapon_placement puts it back",
        steps=[
            (1, "client:%s; setang 20 0 0; give weapon_camera" % setpos("camera", -170, 0)),
            (2, "client:use weapon_camera"),
        ] + press(3, "attack") + press(4, "attack") + [
            # The photo is placed where the placement tool points (raise, place).
            (6, "client:setang 30 30 0"),
        ] + press(7, "attack") + press(8, "attack"),
        checks=[
            fired("capturable_0", "OnCameraCapture"),
            fired("capturable_0", "OnCameraRelease"),
        ]),
    Scenario(
        "photo_scale_down",
        "while a photo is placed the wheel scales it (weapon_placement::OnMouseWheel through "
        "mwheel_up); camera_reverse_scaling_direction 1 makes wheel up shrink it",
        steps=scaled_placement(["invprev", "invprev"]),
        checks=[fired("capturable_0", "OnCameraRelease"),
                dumped("scalevalue", "-2"), dumped("modelscale", "0.25")]),
    Scenario(
        "photo_scale_up",
        "wheel down (invnext, sent as mwheel_down) grows the placed photo",
        steps=scaled_placement(["invnext", "invnext"]),
        checks=[fired("capturable_0", "OnCameraRelease"),
                dumped("scalevalue", "2"), dumped("modelscale", "4.00")]),
    Scenario(
        "photo_eraser",
        "trigger_photo_eraser erases a photo carried through it; the object returns",
        steps=[
            (1, "client:%s; setang 20 0 0; give weapon_camera" % setpos("photo_lab", -230, -100)),
            (2, "client:use weapon_camera"),
        ] + press(3, "attack") + press(4, "attack") + [
            (6, "client:" + setpos("photo_lab", 48, 0)),
        ],
        checks=[
            fired("capturable_lab", "OnCameraCapture"),
            fired("photo_eraser", "OnObjectsFizzled"),
            fired("capturable_lab", "OnFizzled"),
        ]),
    Scenario(
        "photo_kept",
        "negative control: a photo carried away from the eraser is not erased",
        steps=[
            (1, "client:%s; setang 20 0 0; give weapon_camera" % setpos("photo_lab", -230, -100)),
            (2, "client:use weapon_camera"),
        ] + press(3, "attack") + press(4, "attack") + [
            (6, "client:" + setpos("photo_lab", -230, 200)),
        ],
        checks=[
            fired("capturable_lab", "OnCameraCapture"),
            not_fired("photo_eraser", "OnObjectsFizzled"),
            not_fired("capturable_lab", "OnFizzled"),
        ]),
    Scenario(
        "geyser",
        "prop_geyser erupts on its cycle (idle 5 s, 2 s build-up, 2 s eruption) and throws "
        "the player up",
        steps=[(1, "client:%s; setang 0 0 0" % setpos("geyser", 0, 0, 24))] + samples(2, 14),
        checks=[peak("z", 48, "a jump rises ~20; the eruption pushes at 600+ units/s and whole-second samples"
                     " can miss its peak")]),
    Scenario(
        "air_vent",
        "prop_air_vent's air current pushes the player standing in it",
        steps=[(1, "client:%s; setang 0 0 0" % setpos("air_vent", 0, 0, 24))] + samples(2, 8),
        checks=[displaced(64)]),
    Scenario(
        "mousetrap",
        "a scaled-up prop_mousetrap snaps on a player who steps on its arm and flings them up",
        # Walk onto the arm from behind, sampling every few frames: the flight
        # is shorter than a second.
        steps=[(0, TRACE), (1, "client:%s; setang 0 0 0" % setpos("mousetrap", -80, 0, 24)),
               (2, "client:+forward; " + "getpos; wait 5; " * 25 + "-forward")],
        checks=[emitted("Mousetrap.Snap"),
                peak("z", 100, "the eye stands 40 above the setpos height; a launch at "
                              "375 units/s up rises ~115 more")]),
    Scenario(
        "monopole",
        "prop_monopole / func_monopole_field attach the metal cube",
        steps=[(1, "report_entities"),
               (2, "ent_fire monopole_cube Wake"),
               (8, "report_entities")],
        checks=[lambda log: (fired("monopole", "OnAttach")(log)[0] or
                             fired("monopole_field", "OnAttach")(log)[0],
                             "monopole or monopole_field fired OnAttach")]),
    Scenario(
        "tombstone",
        "prop_tombstone raises npc_zombie from the ground (its model never shipped)",
        steps=[(1, "report_entities"), (12, "report_entities")],
        checks=[count_at_least("npc_zombie", 1)]),
    Scenario(
        "reflect",
        "photographing prop_reflect lifts the player 64 units above the cube",
        steps=[
            (1, "client:%s; setang 15 0 0; give weapon_camera" % setpos("reflect", -40, 0)),
            (2, "client:use weapon_camera"),
            (3, "client:getpos"),
        ] + press(4, "attack") + press(5, "attack") + samples(6, 8),
        checks=[peak("z", 40)]),
    Scenario(
        "swap",
        "photographing prop_swap trades places between the player and the cube",
        steps=[
            (1, "client:%s; setang 9 -37 0; give weapon_camera" % setpos("swap", -40, 60)),
            (2, "client:use weapon_camera"),
            (3, "client:getpos"),
        ] + press(4, "attack") + press(5, "attack") + samples(6, 7),
        checks=[displaced(60)]),
    Scenario(
        "levitator",
        "prop_levitator (the balloon) floats up to the ceiling",
        steps=[(10, "report_entities")],
        checks=[fired("levitator_high", "OnStartTouch"), count_at_least("phys_keepupright", 1)]),
    Scenario(
        "monopole_field",
        "func_monopole_field (positive) pushes the player standing on it",
        steps=[(1, "client:%s; setang 0 0 0" % setpos("monopole", -40, 0, 24))] + samples(2, 6),
        checks=[displaced(32)]),
    Scenario(
        "placement_helper",
        "info_placement_helper snaps a photo placed near it and fires OnObjectPlaced",
        steps=photograph(1, setpos("camera", -170, 0) + "; setang 20 0 0")
        + place(6, setpos("camera", -170, -180) + "; setang 20 0 0"),
        checks=[fired("capturable_0", "OnCameraRelease"),
                fired("helper_free", "OnObjectPlaced")]),
    Scenario(
        "placement_clip",
        "func_placement_clip stops a placement aimed through it",
        steps=photograph(1, setpos("camera", -170, 0) + "; setang 20 0 0")
        + place(6, setpos("photo_lab", 80, -200) + "; setang 30 0 0"),
        checks=[fired("capturable_0", "OnCameraCapture"),
                not_fired("capturable_0", "OnCameraRelease")]),
    Scenario(
        "item_photo",
        "item_photo: picking it up photographs its target (the crate)",
        steps=[(1, "client:%s; setang 40 0 0" % setpos("photo_lab", -210, 250)),
               (2, "client:+use; wait 6; -use")],
        checks=[fired("photo", "OnPickedUp"), fired("photo_crate", "OnCameraCapture")]),
    Scenario(
        "dof_controller",
        "env_dof_controller takes its inputs (SetFocusTargetRange as a float)",
        steps=[(1, "ent_fire dof SetFarBlurRadius 5"), (1, "ent_fire dof SetFocusTargetRange 123"),
               (3, "ent_dump dof")],
        checks=[dumped("far_radius", "5.00"), dumped("focus_range", "123.00")]),
    Scenario(
        "laser",
        "env_portal_laser burns and breaks the crate in its beam",
        steps=[(10, "report_entities")],
        checks=[fired("laser_target", "OnBreak")]),
    Scenario(
        "personality_sphere",
        "prop_personality_sphere (CoreType 4) talks: its sphere02 lines play",
        steps=[(0, TRACE), (1, "client:%s; setang 0 90 0" % setpos("sphere", 0, -150)),
               (12, "report_entities")],
        checks=[emitted("sphere02")]),
    Scenario(
        "linked_door",
        "prop_portal_linked_door: walking into door_a comes out of door_b",
        steps=[(1, "client:%s; setang 0 0 0" % setpos("linked_doors", -290, -200)),
               (2, "client:+forward")] + samples(2, 5),
        checks=[jumped("x", 250, "walking covers about 200 units a second")]),
    Scenario(
        "portal_tunnel",
        "prop_portal_tunnel builds its partner tunnel, portals and blockers",
        steps=[(3, "report_entities")],
        checks=[count_at_least("prop_portal_tunnel", 2), count_at_least("entity_blocker", 2)]),
    Scenario(
        "tunnel_walk",
        "walking into the tunnel on the tunnel shop's west wall comes out of its partner on "
        "the atrium's north wall",
        steps=[(3, "client:%s; setang 0 180 0" % setpos("tunnel", -120, 0)),
               (4, "client:+forward")] + samples(4, 8),
        checks=[jumped("x", 500, "the atrium exit is ~1000 units west of the shop's tunnel")]),
    Scenario(
        "chicken",
        "npc_chicken clucks as it wanders",
        steps=[(0, TRACE), (10, "report_entities")],
        checks=[emitted("NPC_Chicken.Clucks")]),
    Scenario(
        "hover_turret",
        "npc_hover_turret shoots a player in sight",
        # In its own shop the turret first has to see the player and spin up.
        steps=[(0, TRACE), (1, "client:%s; setang 0 -45 0; god" % setpos("turret", -50, 50)),
               (8, "report_entities")],
        checks=[emitted("NPC_FloorTurret.ShotSounds")]),
    Scenario(
        "androids",
        "npc_android_basic hunts and hurts the player; its add-ons are installed",
        steps=[(1, "report_entities"),
               (1, "client:%s; setang 0 0 0" % setpos("androids", -10, -50)),
               (2, "ent_dump !player"), (12, "ent_dump !player")],
        checks=[count_at_least("ai_addon_shield", 1), count_at_least("ai_addon_saw", 1),
                not_logged(r"AddOn Error"), health_dropped()]),
    Scenario(
        "blobs",
        "npc_blob_fountain and npc_blob_demomonster spawn their particles; the "
        "demo monster pours down the west concourse to the player at the start",
        steps=[(2, "report_entities"), (3, "notarget"), (30, "report_entities")],
        checks=[logged(r"blob server spawn: npc_blob_fountain"),
                logged(r"blob server spawn: npc_blob_demomonster"),
                fired("blob_monster_pad", "OnStartTouch")]),
    Scenario(
        "bots",
        "npc_medicbot answers the player's +use",
        steps=[(1, "client:%s; setang 10 90 0" % setpos("bots", -100, -230)),
               (2, "client:+use; wait 6; -use")],
        checks=[fired("medicbot", "OnPlayerUse")]),
]
