"""The scenarios of tools/quality/fstop_mechanics_check.py, one per F-Stop mechanic.

Each names what the entity's code does (game/server/fstop) and how the log
shows it. Steps are (game seconds after the scenario starts, command); "client:" commands
run on the player's client.
"""

from fstop_mechanics_check import (Scenario, count_at_least, count_change, displaced, fired,
                                   logged, moved, not_fired, not_logged, peak, samples)


def press(at, button, frames=6):
    """A client button press: +button, then -button some frames later."""
    return [(at, "client:+%s; wait %d; -%s" % (button, frames, button))]

SCENARIOS = [
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
            (1, "client:setpos -1080 -450 8; setang 20 0 0; give weapon_camera"),
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
        "photo_eraser",
        "trigger_photo_eraser erases a photo carried through it; the object returns",
        steps=[
            (1, "client:setpos -1080 -450 8; setang 20 0 0; give weapon_camera"),
            (2, "client:use weapon_camera"),
        ] + press(3, "attack") + press(4, "attack") + [
            (6, "client:setpos -1230 -660 8"),
        ],
        checks=[
            fired("capturable_0", "OnCameraCapture"),
            fired("photo_eraser", "OnObjectsFizzled"),
            fired("capturable_0", "OnFizzled"),
        ]),
    Scenario(
        "photo_kept",
        "negative control: a photo carried away from the eraser is not erased",
        steps=[
            (1, "client:setpos -1080 -450 8; setang 20 0 0; give weapon_camera"),
            (2, "client:use weapon_camera"),
        ] + press(3, "attack") + press(4, "attack") + [
            (6, "client:setpos -1080 300 8"),
        ],
        checks=[
            fired("capturable_0", "OnCameraCapture"),
            not_fired("photo_eraser", "OnObjectsFizzled"),
            not_fired("capturable_0", "OnFizzled"),
        ]),
    Scenario(
        "geyser",
        "prop_geyser erupts on its cycle (idle 5 s, 2 s build-up, 2 s eruption) and throws "
        "the player up",
        steps=[(1, "client:setpos -560 -600 24; setang 0 0 0")] + samples(2, 14),
        checks=[peak("z", 48, "a jump rises ~20; the eruption pushes at 600+ units/s and whole-second samples"
                     " can miss its peak")]),
    Scenario(
        "air_vent",
        "prop_air_vent's air current pushes the player standing in it",
        steps=[(1, "client:setpos -440 -600 24; setang 0 0 0")] + samples(2, 8),
        checks=[displaced(64)]),
    Scenario(
        "mousetrap",
        "prop_mousetrap snaps on a player who steps on it and flings them",
        steps=[(1, "client:setpos -340 -560 24; setang 0 270 0")] + samples(2, 6),
        checks=[displaced(48)]),
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
            (1, "client:setpos -520 0 8; setang 15 0 0; give weapon_camera"),
            (2, "client:use weapon_camera"),
            (3, "client:getpos"),
        ] + press(4, "attack") + press(5, "attack") + samples(6, 8),
        checks=[peak("z", 40)]),
    Scenario(
        "swap",
        "photographing prop_swap trades places between the player and the cube",
        steps=[
            (1, "client:setpos -420 60 8; setang 9 -37 0; give weapon_camera"),
            (2, "client:use weapon_camera"),
            (3, "client:getpos"),
        ] + press(4, "attack") + press(5, "attack") + samples(6, 7),
        checks=[displaced(60)]),
]
