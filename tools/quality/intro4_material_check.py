#!/usr/bin/env python3
"""Pixel checks for fixed Intro4 materials and gameplay doorway captures.

Use --commands to print the portal_boot console-command list. Run it with
host_framerate 0.015, r_core_world 1, r_core_world_strict 1,
r_core_dynamic_draws 0 and FSR quality at 1024x768. Then pass its output directory
to --capture. These checks certify these visible behaviors, not all materials,
glass optics, complete frame performance or platform support. Desktop HiDPI
captures use regions scaled to the actual pixels; images are never downsampled.
Use --scene doors for the closed/open door and independent indicator-box cycle.
Use --scene cables for the rope visibility cycle under captured scene lighting.
Use --scene emissives for cyan/orange/off light on neighboring wall receivers.
Use --scene signage for isolated signs, pictograms, chamber boards and movie screens.
Use --scene cameras for attached red eyes, hide/show, movement, rotation and removal.
Use --scene particles for soft-particle fade, opaque occlusion, return and removal.
Use --scene sparks for the additive spark image, hide/show, opaque occlusion and removal.
Use --scene monitors for the two real static monitors and their live scanline proxy.
Use --scene portal-emitters for live blue/orange pulse and their receiver light.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import re

import numpy as np
from PIL import Image


def door_commands():
    door = 'just_enough_door_for_the_job-testchamber_door'
    paired = 'door_1-testchamber_door'
    left = 'section_2_texturetoggle_doorstate_a02b_left'
    right = 'section_2_texturetoggle_doorstate_a02b_right'
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 120 160 0',
              'cmd setang 0 0 0', f'cmd ent_fire {door} Unlock',
              f'cmd ent_fire {door} Close',
              'cmd ent_fire texturetoggle_exit_doorstate_a02 SetTextureIndex 0']

    def capture():
        result.extend(['wait 150', 'screenshot', 'wait 12'])

    capture()
    result.append(f'cmd ent_fire {door} Open')
    capture()
    result.append('cmd ent_fire texturetoggle_exit_doorstate_a02 SetTextureIndex 1')
    capture()
    result.append('cmd ent_fire texturetoggle_exit_doorstate_a02 SetTextureIndex 0')
    capture()
    result.append(f'cmd ent_fire {door} Close')
    capture()
    result.extend(['cmd setpos 520 -528 0', 'cmd setang 0 0 0',
                   f'cmd ent_fire {paired} Unlock', f'cmd ent_fire {paired} Open',
                   f'cmd ent_fire {left} SetTextureIndex 0',
                   f'cmd ent_fire {right} SetTextureIndex 1'])
    capture()
    result.extend([f'cmd ent_fire {left} SetTextureIndex 1',
                   f'cmd ent_fire {right} SetTextureIndex 0'])
    capture()
    result.extend([f'cmd ent_fire {left} SetTextureIndex 0',
                   f'cmd ent_fire {right} SetTextureIndex 0'])
    capture()
    result.append(f'cmd ent_fire {paired} Close')
    capture()
    result.extend(['r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws'])
    return result


def emissive_commands():
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 120 160 0',
              'cmd setang 0 0 0',
              'cmd ent_fire just_enough_door_for_the_job-testchamber_door Close']

    def cycle(toggle):
        for frame, emitting in ((0, 1), (0, 0), (1, 1), (1, 0), (0, 1)):
            result.extend(['r_area_lights_debug 2',
                           f'cmd ent_fire {toggle} SetTextureIndex {frame}',
                           f'cl_surface_core_emission {emitting}',
                           'wait 2', 'r_area_lights_debug 0',
                           'wait 150', 'screenshot', 'wait 12'])

    cycle('texturetoggle_exit_doorstate_a02')
    result.extend(['cmd ent_create env_texturetoggle targetname rc_emissive_line_toggle '
                   'target exit_doorstate_a02',
                   'cmd setpos -480 56 100', 'cmd setang 32 14 0'])
    cycle('rc_emissive_line_toggle')
    result.extend(['r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws',
                   'cl_surface_core_emission', 'cl_surface_core_emission_strength'])
    return result


def signage_commands():
    result = ['cmd noclip', 'r_drawviewmodel 0',
              'cmd ent_fire just_enough_door_for_the_job-testchamber_door Close',
              'cmd ent_fire info_sign-info_panel SetActive',
              'cmd ent_fire arrival_elevator-signs_on Trigger',
              'cmd ent_create prop_dynamic_override targetname rc_sign_receiver '
              'model models/props/metal_box.mdl',
              'cmd ent_fire rc_sign_receiver RunScriptCode '
              '"self.SetOrigin(Vector(208,192,88))"']
    cases = (
        ('signage/signage_exit', '120 192 110', '20 0 0'),
        ('signage/signage_arrow', '120 192 110', '20 0 0'),
        ('signage/signage_overlay_boxhurt', '-160 100 50', '35 45 0'),
        ('signage/signage_overlay_boxdispenser', '-160 100 50', '35 45 0'),
        ('@panels', '500 32 35', '-10 0 0'),
        ('@panels', '-1560 80 -84', '15 -90 0'),
    )
    for index, (material, position, angles) in enumerate(cases):
        if index == 2:
            result.append('cmd ent_fire rc_sign_receiver Kill')
        result.extend([f'cl_surface_core_emission_filter {material}',
                       f'cmd setpos {position}', f'cmd setang {angles}'])
        if index == 5:
            # Movie radiance follows decoded frames. Pause the existing game
            # player so the light-only on/off/on oracle compares one frame.
            result.extend(['wait 90', 'sv_pausable 1', 'cmd setpause', 'wait 2'])
        for emitting in (1, 0, 1):
            result.extend([f'cl_surface_core_emission {emitting}', 'wait 300',
                           f'echo RC_SIGNAGE_{index}_{emitting}'])
            if emitting:
                result.extend(['r_area_lights_report 1', 'wait 2'])
            result.extend(['screenshot', 'wait 12'])
    result.append('cmd unpause')
    result.extend(['cl_surface_core_emission_filter ""', 'r_core_world_stats',
                   'r_core_world_strict', 'r_core_dynamic_draws',
                   'cl_surface_core_emission', 'cl_surface_core_emission_strength',
                   'cl_surface_core_emission_filter'])
    return result


def camera_commands():
    # Resting Intro4 cameras point down. Freeze AI for a reproducible pose and
    # place a receiver beneath the first eye, keeping shadows at shipped settings.
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd ai_disable',
              'cl_surface_core_emission_filter @camera-eyes',
              'cmd setpos 205 100 100', 'cmd setang 20 -128 0',
              'cmd script rc_camera <- Entities.FindByClassnameNearest('
              '"npc_security_camera",Vector(160,0,182.152),32)',
              'cmd script rc_camera.__KeyValueFromString("targetname","rc_camera")',
              'cmd script rc_eye <- Entities.FindByClassnameNearest('
              '"env_sprite",rc_camera.GetOrigin(),32)',
              'cmd script printl(rc_eye.GetModelName())',
              'cmd script printl(rc_eye.GetMoveParent()==rc_camera)',
              'cmd script rc_eye.__KeyValueFromString("targetname","rc_camera_eye")',
              'cmd ent_create prop_dynamic_override targetname rc_camera_receiver '
              'model models/props/metal_box.mdl',
              'cmd ent_fire rc_camera_receiver RunScriptCode '
              '"self.SetOrigin(Vector(143.7,21.7,112))"']
    states = (
        ('on', 'cl_surface_core_emission 1'),
        ('off', 'cl_surface_core_emission 0'),
        ('restored', 'cl_surface_core_emission 1'),
        ('hidden', 'cmd ent_fire rc_camera_eye HideSprite'),
        ('shown', 'cmd ent_fire rc_camera_eye ShowSprite'),
        ('moved', 'cmd script rc_camera.SetOrigin(Vector(256,0,182.152))'),
        ('returned', 'cmd script rc_camera.SetOrigin(Vector(160,0,182.152))'),
        ('turned', 'cmd script rc_camera.SetAngles(180,90,0)'),
        ('unturned', 'cmd script rc_camera.SetAngles(0,90,0)'),
        ('removed', 'cmd ent_fire rc_camera_eye Kill'),
    )
    for label, control in states:
        result.extend([control, 'wait 300', f'echo RC_CAMERA_{label}',
                       'r_area_lights_report 1', 'wait 2', 'screenshot', 'wait 12'])
    result.extend(['cmd ent_fire rc_camera_receiver Kill', 'cmd ai_disable',
                   'cl_surface_core_emission_filter ""', 'r_core_world_stats',
                   'r_core_world_strict', 'r_core_dynamic_draws',
                   'cl_surface_core_emission', 'cl_surface_core_emission_strength',
                   'cl_surface_core_emission_filter', 'r_core_shadow_quality',
                   'r_core_shadow_movers'])
    return result


def particle_commands():
    # Face 1620 of Intro4's compiled world is the opaque x=256 strip,
    # y=256..288, z=0..96. Keep the entire billboard inside that strip.
    # Scale with eye distance so each pose covers the same screen pixels.
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 120 272 0',
              'cmd setang 0 0 0']

    def capture():
        result.extend(['wait 150', 'screenshot', 'wait 12'])

    capture()
    result.append('cmd ent_create env_sprite targetname rc_soft_particle '
                  'model particle/particle_noisesphere.vmt origin "254 272 64" '
                  'rendercolor "0 255 0" renderamt 255 rendermode 0 scale 0.4 '
                  'spawnflags 1')
    result.append('cmd ent_fire rc_soft_particle RunScriptCode '
                  '"self.SetOrigin(Vector(254,272,64))"')
    capture()
    for x, scale in ((229, '0.325373'), (200, '0.238806'), (260, '0.417910'), (254, '0.4')):
        result.extend([f'cmd ent_fire rc_soft_particle RunScriptCode "self.SetOrigin(Vector({x},272,64))"',
                       f'cmd ent_fire rc_soft_particle SetScale {scale}'])
        capture()
    result.append('cmd ent_fire rc_soft_particle Kill')
    capture()
    result.extend(['r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws'])
    return result


def spark_commands():
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 120 272 0',
              'cmd setang 0 0 0']

    def capture():
        result.extend(['wait 150', 'screenshot', 'wait 12'])

    capture()
    result.extend(['cmd ent_create env_sprite targetname rc_spark '
                   'model effects/spark.vmt rendercolor "255 255 255" '
                   'renderamt 255 rendermode 0 scale 0.4 spawnflags 1',
                   'cmd ent_fire rc_spark RunScriptCode "self.SetOrigin(Vector(200,272,64))"'])
    capture()
    for command in ('cmd ent_fire rc_spark HideSprite',
                    'cmd ent_fire rc_spark ShowSprite',
                    'cmd ent_fire rc_spark RunScriptCode "self.SetOrigin(Vector(260,272,64))"',
                    'cmd ent_fire rc_spark RunScriptCode "self.SetOrigin(Vector(200,272,64))"',
                    'cmd ent_fire rc_spark Kill'):
        result.append(command)
        capture()
    result.extend(['r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws'])
    return result


def inspect_sparks(images):
    validate_images(images, 7)
    samples = [region(image, 472, 344, 552, 424) for image in images[:7]]
    deltas = [float(np.abs(sample - samples[0]).mean()) for sample in samples]
    checks = []
    for suffix, passed, value in (
            ('authored-additive-image', deltas[1] > .03, deltas[1]),
            ('hidden-restores-background', deltas[2] < .005, deltas[2]),
            ('shown-restores-spark', float(np.abs(samples[3] - samples[1]).mean()) < .005,
             float(np.abs(samples[3] - samples[1]).mean())),
            ('opaque-wall-occludes', deltas[4] < .005, deltas[4]),
            ('returned-restores-spark', float(np.abs(samples[5] - samples[1]).mean()) < .005,
             float(np.abs(samples[5] - samples[1]).mean())),
            ('removed-restores-background', deltas[6] < .005, deltas[6])):
        checks.append(dict(name=f'spark.{suffix}', passed=bool(passed), value=float(value)))
    return checks


def spark_sensitivity(images):
    checks = []
    for defect, target, replacement, expected in (
            ('missing-material', 1, 0, 'authored-additive-image'),
            ('stale-hidden-spark', 2, 1, 'hidden-restores-background'),
            ('missing-shown-spark', 3, 0, 'shown-restores-spark'),
            ('drawn-through-wall', 4, 1, 'opaque-wall-occludes'),
            ('missing-return', 5, 0, 'returned-restores-spark'),
            ('stale-removed-spark', 6, 1, 'removed-restores-background')):
        mutated = list(images)
        mutated[target] = images[replacement]
        rejected = any(check['name'] == f'spark.{expected}' and not check['passed']
                       for check in inspect_sparks(mutated))
        checks.append(dict(name=f'spark.oracle.rejects-{defect}', passed=rejected))
    return checks


def inspect_particles(images):
    validate_images(images, 7)
    # Background has the authored indicator line beside the billboard. The
    # center crop contains only the noise sprite and its opaque wall receiver.
    samples = [region(image, 472, 344, 552, 424) for image in images[:7]]
    delta = [float((sample[:, :, 1] - samples[0][:, :, 1]).mean()) for sample in samples]
    near, middle, far, behind, returned = delta[1:6]
    checks = []
    for suffix, passed, value in (
            ('visible-away-from-wall', far > .06, far),
            ('near-wall-fades', near >= -.005 and near < far * .18, near),
            ('middle-gap-between-near-and-far', middle > near * 3 and middle < far * .8,
             middle),
            ('opaque-wall-occludes', abs(behind) < .005, behind),
            ('returned-depth-fade', abs(returned - near) < .005, returned - near),
            ('removed-restores-background', float(np.abs(samples[6] - samples[0]).mean()) < .005,
             float(np.abs(samples[6] - samples[0]).mean()))):
        checks.append(dict(name=f'particle.{suffix}', passed=bool(passed), value=float(value)))
    return checks


def particle_sensitivity(images):
    checks = []
    for defect, target, replacement, expected in (
            ('missing-material', 3, 0, 'visible-away-from-wall'),
            ('unfaded-at-wall', 1, 3, 'near-wall-fades'),
            ('unfaded-middle-gap', 2, 3, 'middle-gap-between-near-and-far'),
            ('drawn-through-wall', 4, 3, 'opaque-wall-occludes'),
            ('stale-depth-on-return', 5, 3, 'returned-depth-fade'),
            ('stale-particle-after-removal', 6, 3, 'removed-restores-background')):
        mutated = copy.deepcopy(images)
        mutated[target] = images[replacement].copy()
        rejected = any(check['name'] == f'particle.{expected}' and not check['passed']
                       for check in inspect_particles(mutated))
        checks.append(dict(name=f'particle.oracle.rejects-{defect}', passed=rejected))
    return checks


def monitor_commands():
    # Real static props, skins 4 (blank) and 3 (off + moving scanlines).
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 956 -860 280',
              'cmd setang 0 90 0']
    for command in (None, None, 'r_drawstaticprops 0', 'r_drawstaticprops 1'):
        if command:
            result.append(command)
        result.extend(['wait 300', 'screenshot', 'wait 12'])
    result.extend(['r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws'])
    return result


def inspect_monitors(images):
    validate_images(images, 4)
    checks = []
    for name, bounds in (('blank', (280, 435, 460, 480)),
                         ('scanline', (550, 405, 725, 470))):
        samples = [region(image, *bounds) for image in images[:4]]
        for index in (0, 1, 3):
            delta = float(np.abs(samples[index] - samples[2]).mean())
            checks.append(dict(name=f'monitor.{name}.visible-{index}',
                               passed=delta > .01, value=delta))
        if name == 'scanline':
            delta = float(np.abs(samples[1] - samples[0]).mean())
            checks.append(dict(name='monitor.scanline.live-proxy',
                               passed=delta > .002, value=delta))
    return checks


def monitor_sensitivity(images):
    checks = []
    for name, target, replacement, expected in (
            ('missing-blank', 0, 2, 'monitor.blank.visible-0'),
            ('missing-scanline', 0, 2, 'monitor.scanline.visible-0'),
            ('stale-proxy', 1, 0, 'monitor.scanline.live-proxy'),
            ('missing-restored-scanline', 3, 2, 'monitor.scanline.visible-3')):
        mutated = copy.deepcopy(images)
        mutated[target] = images[replacement].copy()
        rejected = any(row['name'] == expected and not row['passed']
                       for row in inspect_monitors(mutated))
        checks.append(dict(name=f'monitor.oracle.rejects-{name}', passed=rejected))
    return checks


def portal_emitter_commands():
    blue = 'models/props/portal_emitter_lights_on_blue'
    orange = 'models/props/portal_emitter_lights_on_orange'
    result = ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos -448 128 28',
              'cmd setang 16 -90 0', f'cl_surface_core_emission_filter {blue}',
              'cmd ent_fire portal_emitter_a_lvl3 Skin 0']

    def capture(wait=150):
        result.extend([f'wait {wait}', 'screenshot', 'wait 12'])

    capture()
    result.append('cmd ent_fire portal_emitter_a_lvl3 Skin 1')
    capture()
    result.append('cl_surface_core_emission 0')
    capture()
    result.append('cl_surface_core_emission 1')
    capture()
    capture(17)
    capture(31)
    result.extend([f'cl_surface_core_emission_filter {orange}',
                   'cmd ent_fire portal_emitter_a_lvl3 Skin 2'])
    capture()
    result.append('cl_surface_core_emission 0')
    capture()
    result.append('cl_surface_core_emission 1')
    capture()
    result.extend([f'cl_surface_core_emission_filter {blue}',
                   'cmd ent_fire portal_emitter_a_lvl3 Skin 0'])
    capture()
    result.extend(['cl_surface_core_emission_filter ""', 'r_core_world_stats',
                   'r_core_world_strict', 'r_core_dynamic_draws',
                   'cl_surface_core_emission', 'cl_surface_core_emission_strength',
                   'cl_surface_core_emission_filter'])
    return result


def inspect_portal_emitters(images):
    validate_images(images, 10)
    checks = []
    sources = [region(image, 346, 192, 386, 543) for image in images[:10]]
    receivers = [region(image, 380, 590, 620, 640) for image in images[:10]]
    for index in (1, 2, 3, 4, 5, 6, 7, 8):
        r, g, b = np.moveaxis(sources[index], 2, 0)
        blue = np.mean((b > r + .06) & (b > g + .03))
        orange = np.mean((r > b + .06) & (r > g + .03))
        selected, other = (orange, blue) if index in (6, 7, 8) else (blue, orange)
        checks.append(dict(name=f'portal-emitter.authored-color-{index}',
                           passed=bool(selected > .025 and other < .01), value=float(selected)))
    for name, on, off, channel, other in (('blue', 1, 2, 2, 0),
                                         ('blue-returned', 3, 2, 2, 0),
                                         ('orange', 6, 7, 0, 2),
                                         ('orange-returned', 8, 7, 0, 2)):
        delta = (receivers[on] - receivers[off]).mean(axis=(0, 1))
        checks.append(dict(name=f'portal-emitter.{name}-receiver',
                           passed=bool(delta[channel] > .007 and delta[channel] > 2 * delta[other]),
                           value=float(delta[channel])))
    pulse = max(float(np.abs(sources[a] - sources[b]).mean())
                for a, b in ((3, 4), (3, 5), (4, 5)))
    checks.append(dict(name='portal-emitter.live-pulse', passed=pulse > .002, value=pulse))
    for index in (0, 9):
        sample = sources[index]
        chroma = np.ptp(sample, axis=2)
        visible = float(np.mean((sample.max(axis=2) > .15) & (chroma > .06)))
        checks.append(dict(name=f'portal-emitter.off-has-no-stale-glow-{index}',
                           passed=visible < .01, value=visible))
    return checks


def portal_emitter_sensitivity(images):
    checks = []
    for name, target, replacement, expected in (
            ('missing-blue', 1, 0, 'authored-color-1'),
            ('missing-orange', 6, 0, 'authored-color-6'),
            ('stale-blue-skin', 6, 1, 'authored-color-6'),
            ('missing-blue-light', 1, 2, 'blue-receiver'),
            ('missing-orange-light', 6, 7, 'orange-receiver'),
            ('missing-blue-return', 3, 2, 'blue-returned-receiver'),
            ('stale-off', 9, 1, 'off-has-no-stale-glow-9')):
        mutated = copy.deepcopy(images)
        mutated[target] = images[replacement].copy()
        rejected = any(row['name'] == f'portal-emitter.{expected}' and not row['passed']
                       for row in inspect_portal_emitters(mutated))
        checks.append(dict(name=f'portal-emitter.oracle.rejects-{name}', passed=rejected))
    mutated = copy.deepcopy(images)
    mutated[4] = images[3].copy()
    mutated[5] = images[3].copy()
    rejected = any(row['name'] == 'portal-emitter.live-pulse' and not row['passed']
                   for row in inspect_portal_emitters(mutated))
    checks.append(dict(name='portal-emitter.oracle.rejects-frozen-pulse', passed=rejected))
    return checks


def commands(scene='materials'):
    if scene == 'all':
        result = ['wait 600', 'r_core_world_stats', 'r_temporal_scale']
        for name in ('materials', 'doors', 'cables', 'emissives', 'signage', 'cameras', 'particles', 'sparks', 'monitors', 'portal-emitters'):
            sequence = commands(name)
            result.extend(sequence if name == 'materials' else sequence[1:])
        result.append('r_temporal_scale')
        return result
    if scene == 'portal-emitters':
        return portal_emitter_commands()
    if scene == 'monitors':
        return monitor_commands()
    if scene == 'sparks':
        return spark_commands()
    if scene == 'particles':
        return particle_commands()
    if scene == 'cameras':
        return camera_commands()
    if scene == 'signage':
        return signage_commands()
    if scene == 'doors':
        return door_commands()
    if scene == 'emissives':
        return emissive_commands()
    if scene == 'cables':
        return ['cmd noclip', 'r_drawviewmodel 0', 'cmd setpos 500 -200 112',
                'cmd setang -15 90 0', 'wait 150', 'screenshot', 'wait 12',
                'r_drawropes 0', 'wait 150', 'screenshot', 'wait 12',
                'r_drawropes 1', 'wait 150', 'screenshot', 'wait 12',
                'r_core_world_stats', 'r_core_world_strict', 'r_core_dynamic_draws']
    result = ['cmd noclip', 'r_drawviewmodel 0']

    def capture(wait=150):
        result.extend([f'wait {wait}', 'screenshot', 'wait 12'])

    result.extend(['cmd setpos -220 170 110', 'cmd setang 0 180 0'])
    capture()
    result.append('r_drawtranslucentrenderables 0')
    capture()
    result.extend(['r_drawtranslucentrenderables 1', 'cmd setpos -320 56 100',
                   'cmd setang 89 0 0'])
    capture()
    result.extend(['cmd ent_create env_texturetoggle targetname rc_indicator_check '
                   'target exit_doorstate_a02',
                   'cmd ent_fire rc_indicator_check SetTextureIndex 1'])
    capture()
    result.append('cmd ent_fire rc_indicator_check SetTextureIndex 0')
    capture()
    result.append('r_renderoverlayfragment 0')
    capture(100)
    result.extend(['r_renderoverlayfragment 1', 'cmd setpos 940 -700 256',
                   'cmd setang 0 -90 0'])
    capture()
    result.append('r_renderoverlayfragment 0')
    capture(100)
    result.append('r_renderoverlayfragment 1')
    result.append('r_core_world_stats')
    return result


def region(image, x0, y0, x1, y1):
    height, width = image.shape[:2]
    return image[round(y0 * height / 768):round(y1 * height / 768),
                 round(x0 * width / 1024):round(x1 * width / 1024)]


def validate_images(images, required):
    if len(images) < required:
        raise ValueError(f'{required} complete RGB captures are required')
    shape = images[0].shape
    if (len(shape) != 3 or shape[2] != 3 or shape[0] < 768 or shape[1] < 1024 or
            shape[0] * 4 != shape[1] * 3 or any(image.shape != shape for image in images)):
        raise ValueError('complete captures of the fixed 4:3 viewport are required')


def validate_core_statistics(log):
    failures = re.findall(r'views queued \d+ drawn \d+ failed (\d+)', log)
    if not failures or any(int(value) for value in failures):
        raise ValueError('missing core statistics or claimed-view failure')
    refusals = re.findall(r'dynamic draws \d+ refused (\d+)', log)
    if not refusals or any(int(value) for value in refusals):
        raise ValueError('missing dynamic statistics or an unsupported live material draw')


def inspect_emissives(images):
    validate_images(images, 10)
    checks = []
    # Authored wall receivers beside the box and floor line. These regions
    # contain neither the emitting textures, the reticle nor a bloom halo.
    for name, base, bounds in (('box', 0, (153, 320, 235, 590)),
                               ('line', 5, (740, 350, 820, 550))):
        receivers = [region(image, *bounds) for image in images[base:base + 5]]
        cyan = receivers[0] - receivers[1]
        orange = receivers[2] - receivers[3]
        c = cyan.mean(axis=(0, 1))
        o = orange.mean(axis=(0, 1))
        restored = np.abs(receivers[4] - receivers[0]).mean()
        off_change = np.abs(receivers[3] - receivers[1]).mean()
        for suffix, passed, value in (
                ('cyan-receiver', c[2] > .03 and c[1] > .02 and c[2] > 2 * c[0], c[2]),
                ('orange-receiver', o[0] > .03 and o[1] > .02 and o[0] > 2 * o[2], o[0]),
                ('cyan-coverage', (cyan.max(axis=2) > .02).mean() > .8,
                 (cyan.max(axis=2) > .02).mean()),
                ('orange-coverage', (orange.max(axis=2) > .02).mean() > .8,
                 (orange.max(axis=2) > .02).mean()),
                ('off-removes-frame-light', off_change < .01, off_change),
                ('cyan-return', restored < .01, restored)):
            checks.append(dict(name=f'emissive.{name}.{suffix}', passed=bool(passed),
                               value=float(value)))
    # The light-only control must preserve visible emission. The source box
    # and floor dots continue to change frames while their receiver light is off.
    for base, bounds, name in ((0, (40, 153, 142, 254), 'box'),
                               (5, (568, 462, 640, 650), 'line')):
        for index, orange in ((0, False), (1, False), (2, True), (3, True), (4, False)):
            source = region(images[base + index], *bounds)
            r, g, b = np.moveaxis(source, 2, 0)
            coverage = np.mean((r > b + .2) & (g > b + .1) & (r > .35)) if orange else \
                np.mean((b > r + .15) & (g > r + .1) & (b > .3))
            checks.append(dict(name=f'emissive.{name}.visible-frame-{index}',
                               passed=bool(coverage > (.4 if name == 'box' else .03)),
                               value=float(coverage)))
    return checks


def emissive_sensitivity(images):
    checks = []
    for name, base, bounds in (('box', 0, (153, 320, 235, 590)),
                               ('line', 5, (740, 350, 820, 550))):
        for defect, index, replacement, expected in (
                ('missing-cyan-light', 0, 1, 'cyan-receiver'),
                ('missing-orange-light', 2, 3, 'orange-receiver'),
                ('stale-orange-frame', 2, 0, 'orange-receiver'),
                ('missing-return', 4, 1, 'cyan-return')):
            mutated = [image.copy() for image in images]
            region(mutated[base + index], *bounds)[:] = region(images[base + replacement], *bounds)
            rows = inspect_emissives(mutated)
            rejected = any(row['name'] == f'emissive.{name}.{expected}' and
                           not row['passed'] for row in rows)
            checks.append(dict(name=f'oracle.rejects-{name}-{defect}', passed=rejected))
    return checks


# Fixed receiver regions in the 1024x768 reference viewport. The first two
# signs use a metal-box receiver placed in front of the authored source; the
# remaining regions are authored debris, wall and panel-border receivers.
SIGN_RECEIVERS = (
    ('exit', (419, 508, 584, 559), .03, .6),
    ('arrow', (419, 508, 584, 559), .03, .6),
    ('boxhurt', (440, 465, 603, 529), .008, .1),
    ('boxdispenser', (70, 30, 210, 130), .01, .7),
    ('chamber-board', (211, 295, 227, 610), .04, .8),
    ('elevator-movie', (180, 240, 330, 500), .04, .8),
)


CAMERA_STATES = ('on', 'off', 'restored', 'hidden', 'shown', 'moved',
                 'returned', 'turned', 'unturned', 'removed')
CAMERA_RECEIVER = (413, 357, 453, 373)


def inspect_cameras(images):
    validate_images(images, 10)
    receivers = [region(image, *CAMERA_RECEIVER) for image in images[:10]]
    checks = []
    for index in (0, 2, 4, 6, 8):
        gain = receivers[index] - receivers[1]
        rgb = gain.mean(axis=(0, 1))
        coverage = float((gain[:, :, 0] > .008).mean())
        name = CAMERA_STATES[index]
        checks.extend((dict(name=f'camera.{name}.red-receiver',
                            passed=bool(rgb[0] > .005 and abs(rgb[1]) < .003 and
                                        abs(rgb[2]) < .003), value=float(rgb[0])),
                       dict(name=f'camera.{name}.receiver-coverage',
                            passed=coverage > .25, value=coverage)))
        if index:
            delta = float(np.abs(receivers[index] - receivers[0]).mean())
            checks.append(dict(name=f'camera.{name}.restoration',
                               passed=delta < .003, value=delta))
    for index in (3, 5, 7, 9):
        rgb = (receivers[index] - receivers[1]).mean(axis=(0, 1))
        # Movement changes the camera's shadow on white light. Red chroma
        # separates a stale eye contribution from that neutral shadow change.
        red = float(rgb[0] - .5 * (rgb[1] + rgb[2]))
        checks.append(dict(name=f'camera.{CAMERA_STATES[index]}.no-stale-light',
                           passed=abs(red) < .002, value=red))
    return checks


def camera_sensitivity(images):
    checks = []
    for index in (0, 2, 4, 6, 8, 3, 5, 7, 9):
        mutated = list(images)
        positive = index in (0, 2, 4, 6, 8)
        mutated[index] = images[1 if positive else 0]
        name = f'camera.{CAMERA_STATES[index]}.' + (
            'red-receiver' if positive else 'no-stale-light')
        checks.append(dict(name=f'oracle.rejects-camera-{CAMERA_STATES[index]}',
                           passed=any(row['name'] == name and not row['passed']
                                      for row in inspect_cameras(mutated))))
    return checks


def camera_reports(log, with_fizzler=False):
    checks = []
    number = r'(-?\d+(?:\.\d+)?)'
    light = re.compile(r'slotless area \d+ key \d+ at ' + ' '.join([number] * 3) +
                       r' facing ' + ' '.join([number] * 3) + r' area ' + number +
                       r' one-sided radiance ' + ' '.join([number] * 3))
    # The all-scenes door fixture leaves this authored fizzler active. Keep
    # and verify it independently; camera isolation must not extinguish it.
    fizzler = re.compile(r'slotless area \d+ key \d+ at 367\.0 160\.0 64\.0 '
                         r'facing 1\.00 0\.00 0\.00 area 16384\.0 two-sided radiance ' +
                         ' '.join([number] * 3))
    for state in CAMERA_STATES:
        segments = re.findall(rf'RC_CAMERA_{state}\b\s*(.*?)(?=RC_CAMERA_|r_core_world_stats:)',
                              log, flags=re.S)
        passed = len(segments) == 1
        if passed:
            segment = segments[0]
            summary = re.search(r'area lights generation \d+: (\d+) lit '
                                r'\((\d+) without a slot\), (\d+) dropped', segment)
            records = [list(map(float, row)) for row in light.findall(segment)]
            targets = [(-579.7, 234.3, 142.5), (23.7, -634.3, 175.0)]
            if state == 'off':
                targets = []
            elif state not in ('hidden', 'removed'):
                targets.append((239.7, 21.7, 148.7) if state == 'moved' else
                               (143.7, -21.7, 215.6) if state == 'turned' else
                               (143.7, 21.7, 148.7))
            preserved = [list(map(float, row)) for row in fizzler.findall(segment)]
            preserved_ok = (len(preserved) == int(with_fizzler) and
                            all(.05 < row[0] < .15 and .15 < row[1] < .4 and
                                .2 < row[2] < .5 for row in preserved))
            passed = bool(summary and summary[1] == '0' and summary[3] == '0' and
                          int(summary[2]) == len(targets) + int(with_fizzler) and
                          len(records) == len(targets) and preserved_ok)
            for center in targets:
                normal = (0, 0, 1 if center[2] == 215.6 else -1)
                passed = passed and any(
                    max(abs(row[k] - center[k]) for k in range(3)) < .2 and
                    max(abs(row[k + 3] - normal[k]) for k in range(3)) < .02 and
                    abs(row[6] - 4) < .01 and abs(row[7] - 8.031) < .002 and
                    row[8] == 0 and row[9] == 0 for row in records)
        checks.append(dict(name=f'camera.{state}.source-publication', passed=bool(passed)))
    for setting, expected in (('r_core_shadow_quality', '3'), ('r_core_shadow_movers', '1')):
        values = re.findall(rf'"{setting}" = "([^\"]+)"', log)
        checks.append(dict(name=f'camera.{setting}',
                           passed=bool(values and all(value == expected for value in values))))
    return checks


def camera_report_sensitivity(log, with_fizzler=False):
    checks = []
    for name, bad in (
            ('stale-position', log.replace('at 239.7 21.7 148.7', 'at 143.7 21.7 148.7')),
            ('reversed-front', log.replace('facing 0.00 0.00 -1.00', 'facing 0.00 0.00 1.00')),
            ('missing-radiance', log.replace('radiance 8.031', 'radiance 0.000')),
            ('legacy-slot', re.sub(r': 0 lit \(\d+ without a slot\)', ': 1 lit (0 without a slot)', log)),
            ('disabled-shadows', log.replace('"r_core_shadow_movers" = "1"',
                                            '"r_core_shadow_movers" = "0"')),
            ('missing-report', re.sub(r'RC_CAMERA_hidden', 'RC_MISSING_hidden', log))):
        checks.append(dict(name=f'oracle.rejects-camera-{name}',
                           passed=any(not row['passed'] for row in camera_reports(bad, with_fizzler))))
    return checks


def inspect_signage(images):
    validate_images(images, 18)
    checks = []
    for index, (name, bounds, minimum, coverage) in enumerate(SIGN_RECEIVERS):
        on, off, restored = [region(image, *bounds)
                             for image in images[index * 3:index * 3 + 3]]
        for frame, lit in ((0, on), (2, restored)):
            gain = lit - off
            mean = float(gain.mean())
            changed = float((gain.max(axis=2) > .008).mean())
            checks.extend((dict(name=f'signage.{name}.receiver-{frame}',
                                passed=mean > minimum, value=mean),
                           dict(name=f'signage.{name}.coverage-{frame}',
                                passed=changed > coverage, value=changed)))
        # The movie player is paused for this cycle. Judge the same source
        # frame on its surrounding wall; static sources use the same bound.
        delta = float(np.abs(restored - on).mean())
        checks.append(dict(name=f'signage.{name}.return', passed=delta < .01, value=delta))
    # White printed source patches remain visible during the light-only control.
    for index, bounds in ((0, (560, 280, 590, 312)), (1, (440, 280, 470, 312))):
        on, off, restored = [region(image, *bounds)
                             for image in images[index * 3:index * 3 + 3]]
        delta = max(float(np.abs(on - off).mean()), float(np.abs(restored - off).mean()))
        checks.append(dict(name=f'signage.{SIGN_RECEIVERS[index][0]}.visible-source',
                           passed=float(off.mean()) > .07 and delta < .01, value=delta))
    return checks


def signage_sensitivity(images):
    checks = []
    for index, (name, bounds, _, _) in enumerate(SIGN_RECEIVERS):
        for frame in (0, 2):
            mutated = list(images)
            mutated[index * 3 + frame] = images[index * 3 + 1]
            expected = f'signage.{name}.receiver-{frame}'
            rejected = any(row['name'] == expected and not row['passed']
                           for row in inspect_signage(mutated))
            checks.append(dict(name=f'oracle.rejects-{name}-missing-light-{frame}',
                               passed=rejected))
    return checks


def signage_reports(log):
    checks = []
    number = r'(-?\d+(?:\.\d+)?)'
    light = re.compile(r'slotless area \d+ key \d+ at ' + ' '.join([number] * 3) +
                       r' facing ' + ' '.join([number] * 3) + r' area ' + number +
                       r' one-sided radiance ' + ' '.join([number] * 3))
    # Independent gameplay-space fronts: west-facing wall signs/chamber board,
    # upward floor glyphs and the north-facing arrival movie.
    targets = (((256, 175.9, 144.2), (-1, 0, 0)),
               ((256, 207.9, 144.1), (-1, 0, 0)),
               ((-80.1, 176.1, .1), (0, 0, 1)),
               ((-48.1, 176, .1), (0, 0, 1)),
               ((657, 8.5, 104.2), (-1, 0, 0)),
               ((-1560.1, -63.3, -32), (0, 1, 0)))
    for index, (name, _, _, _) in enumerate(SIGN_RECEIVERS):
        segments = re.findall(rf'RC_SIGNAGE_{index}_1\s*(.*?)(?=RC_SIGNAGE_|"cl_surface|views queued)',
                              log, flags=re.S)
        passed = len(segments) == 2
        center, normal = targets[index]
        for segment in segments:
            summary = re.search(r'area lights generation \d+: (\d+) lit '
                                r'\((\d+) without a slot\), (\d+) dropped', segment)
            records = [list(map(float, row)) for row in light.findall(segment)]
            facing = any(max(abs(row[k] - center[k]) for k in range(3)) < .25 and
                         max(abs(row[k + 3] - normal[k]) for k in range(3)) < .02 and
                         row[6] > 0 and min(row[7:10]) > .3 for row in records)
            passed = bool(passed and summary and summary[1] == '0' and
                          int(summary[2]) > 0 and summary[3] == '0' and facing)
        checks.append(dict(name=f'signage.{name}.core-only-front', passed=passed))
    return checks


def signage_report_sensitivity(log):
    checks = []
    for name, bad in (
            ('reversed-source-front', log.replace('facing -1.00 0.00 0.00',
                                                  'facing 1.00 0.00 0.00')),
            ('legacy-light-slots', re.sub(r': 0 lit \(\d+ without a slot\)',
                                         ': 1 lit (0 without a slot)', log))):
        checks.append(dict(name=f'oracle.rejects-{name}',
                           passed=any(not row['passed'] for row in signage_reports(bad))))
    return checks


def inspect_cables(images):
    validate_images(images, 3)
    checks = []
    # The isolated hanging loop below the ironwork. Excludes the reticle and
    # upper foliage. Wind may move the rope; its exact trajectory is not promised.
    regions = [region(image, 350, 500, 675, 745) for image in images[:3]]
    for index in (0, 2):
        difference = regions[index] - regions[1]
        dark = np.mean(difference.mean(axis=2) < -2 / 255)
        light = np.mean(difference.mean(axis=2) > 2 / 255)
        energy = -np.minimum(difference, 0).mean()
        for name, passed, value in (
                ('coverage', .02 < dark < .08, dark),
                ('lit-contrast', energy > .0005, energy),
                ('no-bright-replacement', light < .001, light)):
            checks.append(dict(name=f'cable.frame-{index}.{name}',
                               passed=bool(passed), value=float(value)))
    return checks


def cable_sensitivity(images):
    checks = []
    for name, index, value in (('missing-cable', 0, images[1]),
                               ('missing-return', 2, images[1]),
                               ('opaque-region', 0, np.zeros_like(images[0]))):
        mutated = list(images)
        mutated[index] = value
        rejected = any(not check['passed'] for check in inspect_cables(mutated))
        checks.append(dict(name=f'oracle.rejects-{name}', passed=rejected))
    return checks


def inspect_doors(images):
    validate_images(images, 9)
    checks = []

    def check(name, passed, value):
        checks.append(dict(name=name, passed=bool(passed), value=float(value)))

    def box(index, bounds, orange, name):
        panel = region(images[index], *bounds)
        red, green, blue = np.moveaxis(panel, 2, 0)
        cyan = np.mean((green > red + .1) & (blue > red + .15) & (blue > .3))
        amber = np.mean((red > blue + .2) & (green > blue + .1) & (red > .35))
        glyph = np.mean(panel.max(axis=2) < .06)
        hatch = np.mean((panel.min(axis=2) > .4) & (np.ptp(panel, axis=2) < .08))
        selected, other = (amber, cyan) if orange else (cyan, amber)
        check(name, selected > .4 and other < .01 and glyph > .1 and hatch < .01, selected)

    for i in range(5):
        box(i, (40, 153, 142, 254), i == 2, f'box.single.frame-{i}')
    for i, left, right in ((5, False, True), (6, True, False), (7, False, False)):
        box(i, (192, 230, 259, 294), left, f'box.paired.left-{i}')
        box(i, (767, 230, 835, 294), right, f'box.paired.right-{i}')

    for i in (1, 2, 3):
        floor = region(images[i], 445, 515, 575, 565)
        visible = np.mean(floor.max(axis=2) > .08)
        check(f'door.single.room-visible-{i}', visible > .8 and floor.std() > .02, visible)
    closed = np.abs(region(images[4], 445, 515, 575, 565) -
                    region(images[0], 445, 515, 575, 565))
    check('door.single.close-restores-door', closed.mean() < .01, closed.mean())
    for i in (5, 6, 7):
        floor = region(images[i], 465, 470, 555, 515)
        visible = np.mean(floor.max(axis=2) > .02)
        check(f'door.paired.room-visible-{i}', visible > .6 and floor.std() > .004, visible)
    difference = np.max(np.abs(region(images[7], 465, 470, 555, 515) -
                               region(images[8], 465, 470, 555, 515)), axis=2)
    changed = np.mean(difference > .02)
    check('door.paired.close-changes-opening', changed > .5, changed)
    return checks


def door_sensitivity(images):
    checks = []
    for name, index, bounds, value, expected in (
            ('opaque-cover', 1, (445, 515, 575, 565), 0, 'door.single.room-visible-1'),
            ('paired-opaque-cover', 5, (465, 470, 555, 515), 0, 'door.paired.room-visible-5'),
            ('missing-left-box', 5, (192, 230, 259, 294), 0, 'box.paired.left-5'),
            ('missing-right-box', 5, (767, 230, 835, 294), 0, 'box.paired.right-5'),
            ('missing-glyph', 0, (40, 153, 142, 254), (.05, .5, .7), 'box.single.frame-0')):
        mutated = copy.deepcopy(images)
        region(mutated[index], *bounds)[:] = value
        rejected = any(check['name'] == expected and not check['passed']
                       for check in inspect_doors(mutated))
        checks.append(dict(name=f'oracle.rejects-{name}', passed=rejected))
    mutated = copy.deepcopy(images)
    mutated[6] = mutated[5].copy()
    rejected = any(check['name'] == 'box.paired.left-6' and not check['passed']
                   for check in inspect_doors(mutated))
    checks.append(dict(name='oracle.rejects-stale-shared-panel-frame', passed=rejected))
    return checks


def inspect(images):
    validate_images(images, 8)
    checks = []

    def check(name, passed, value):
        checks.append(dict(name=name, passed=bool(passed), value=float(value)))

    # The old FSR error transmitted the bright neutral core-only hatch here.
    glass = region(images[0], 680, 145, 770, 485)
    hatch = np.mean((glass.min(axis=2) > .4) & (np.ptp(glass, axis=2) < .08))
    check('glass.no-unrendered-attachment-hatch', hatch < .01, hatch)
    # Turning the transmitting renderables off must change the pane. This
    # prevents a missing Refract draw from passing only the no-hatch check.
    difference = np.max(np.abs(region(images[0], 600, 165, 730, 300) -
                               region(images[1], 600, 165, 730, 300)), axis=2)
    changed = np.mean(difference > .04)
    check('glass.transmitting-cohort-reaches-game', changed > .015, changed)

    for index in (2, 3, 4, 5):
        strip = region(images[index], 495, 440, 530, 745)
        red, green, blue = np.moveaxis(strip, 2, 0)
        cyan = np.mean((green > red + .1) & (blue > red + .15) & (blue > .3))
        orange = np.mean((red > blue + .2) & (green > blue + .1) & (red > .35))
        if index in (2, 4):
            check(f'indicator.frame0-visible-{index}', cyan > .25 and orange < .01, cyan)
        elif index == 3:
            check('indicator.frame1-visible', orange > .25 and cyan < .01, orange)
        else:
            check('indicator.overlay-off-control', cyan < .01 and orange < .01,
                  max(cyan, orange))
    mural_delta = np.max(np.abs(region(images[6], 382, 445, 640, 535) -
                                region(images[7], 382, 445, 640, 535)), axis=2)
    changed = np.mean(mural_delta > .04)
    check('decal.modulate-mural-reaches-game', changed > .2, changed)
    return checks


def sensitivity(images):
    checks = []
    for name, mutation, expected in (
            ('hatch', lambda data: region(data[0], 680, 145, 770, 485).__setitem__(slice(None), .7),
             'glass.no-unrendered-attachment-hatch'),
            ('missing-transmission', lambda data: data.__setitem__(0, data[1].copy()),
             'glass.transmitting-cohort-reaches-game'),
            ('missing-orange', lambda data: data.__setitem__(3, data[5].copy()),
             'indicator.frame1-visible'),
            ('missing-decal', lambda data: data.__setitem__(6, data[7].copy()),
             'decal.modulate-mural-reaches-game')):
        mutated = copy.deepcopy(images)
        mutation(mutated)
        rejected = any(check['name'] == expected and not check['passed']
                       for check in inspect(mutated))
        checks.append(dict(name=f'oracle.rejects-{name}', passed=rejected))
    return checks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--commands', action='store_true')
    parser.add_argument('--scene', choices=('materials', 'doors', 'cables', 'emissives', 'signage', 'cameras', 'particles', 'sparks', 'monitors', 'portal-emitters', 'all'),
                        default='materials')
    parser.add_argument('--temporal-scale', type=float,
                        help='require this FSR scale in startup settings and live game queries')
    parser.add_argument('--capture', type=Path)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    if args.commands:
        print(json.dumps(commands(args.scene), indent=2))
        return 0
    if not args.capture or not args.out:
        parser.error('--capture and --out are required')
    report = dict(schema='intro4-material-pixels/v1', scene=args.scene,
                  capture=str(args.capture.resolve()),
                  checks=[], inputs=[], status='fail')
    try:
        evidence = json.loads((args.capture / 'evidence.json').read_text())
        if evidence['status'] != 'pass' or evidence['map'] != 'sp_a1_intro4_relit':
            raise ValueError('the required Intro4 game boot did not pass')
        startup = args.capture / 'runtime/portal2/cfg/portal_boot_startup.cfg'
        settings = dict(line.split(None, 1) for line in startup.read_text().splitlines()
                        if line.strip() and not line.lstrip().startswith('//'))
        if any(settings.get(name) != value for name, value in (
                ('r_core_world', '1'), ('r_core_world_strict', '1'),
                ('r_core_dynamic_draws', '0'), ('host_framerate', '0.015'))):
            raise ValueError('the capture must run the default core cohorts in strict mode')
        # portal_boot appends its final capture, diagnostics and shutdown.
        sequence = commands(args.scene)
        if evidence['console_script']['commands'][:len(sequence)] != sequence:
            raise ValueError('the capture command sequence does not match this fixture')
        screenshots = sorted((args.capture / 'runtime/portal2/screenshots').glob('*.tga'))
        images = []
        for path in screenshots:
            with Image.open(path) as image:
                images.append(np.asarray(image.convert('RGB'), dtype=np.float32) / 255)
            report['inputs'].append(dict(path=str(path),
                                         sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        log = (args.capture / 'runtime/portal2/console.log').read_text(errors='replace')
        strict = re.findall(r'"r_core_world_strict" = "([^\"]+)"', log)
        if strict and any(value != '1' for value in strict):
            raise ValueError('the game reported strict mode disabled')
        dynamic = re.findall(r'"r_core_dynamic_draws" = "([^\"]+)"', log)
        if ((dynamic and any(value != '0' for value in dynamic)) or
                (args.scene in ('doors', 'cables', 'emissives', 'signage', 'cameras', 'particles', 'sparks', 'monitors', 'portal-emitters', 'all') and (not strict or not dynamic))):
            raise ValueError('strict/default-cohort game queries are missing or incorrect')
        if args.scene in ('emissives', 'signage', 'cameras', 'portal-emitters', 'all'):
            strength = re.findall(r'"cl_surface_core_emission_strength" = "([^\"]+)"', log)
            emitting = re.findall(r'"cl_surface_core_emission" = "([^\"]+)"', log)
            if (not strength or any(value != '16' for value in strength) or
                    not emitting or any(value != '1' for value in emitting)):
                raise ValueError('required emissive source policy queries are missing or incorrect')
        if args.scene in ('signage', 'portal-emitters', 'all'):
            filters = re.findall(r'"cl_surface_core_emission_filter" = "([^\"]*)"', log)
            if not filters or any(filters):
                raise ValueError('the source isolation filter was not restored')
        if args.temporal_scale is not None:
            selected = args.temporal_scale
            queried = re.findall(r'"r_temporal_scale" = "([^\"]+)"', log)
            if (not math.isfinite(selected) or not 0 <= selected <= 1 or
                    not math.isclose(float(settings.get('r_temporal_scale', 'nan')), selected,
                                     abs_tol=1e-6) or not queried or
                    any(not math.isclose(float(value), selected, abs_tol=1e-6)
                        for value in queried)):
                raise ValueError('required temporal mode settings/queries are missing or incorrect')
            report['temporal_scale'] = selected
        validate_core_statistics(log)
        if args.scene == 'all':
            validate_images(images, 86)
            report['checks'] = (inspect(images[:8]) + sensitivity(images[:8]) +
                                inspect_doors(images[8:17]) + door_sensitivity(images[8:17]) +
                                inspect_cables(images[17:20]) + cable_sensitivity(images[17:20]) +
                                inspect_emissives(images[20:30]) + emissive_sensitivity(images[20:30]) +
                                inspect_signage(images[30:48]) + signage_sensitivity(images[30:48]) +
                                signage_reports(log) + signage_report_sensitivity(log) +
                                inspect_cameras(images[48:58]) + camera_sensitivity(images[48:58]) +
                                inspect_particles(images[58:65]) + particle_sensitivity(images[58:65]) +
                                inspect_sparks(images[65:72]) + spark_sensitivity(images[65:72]) +
                                inspect_monitors(images[72:76]) + monitor_sensitivity(images[72:76]) +
                                inspect_portal_emitters(images[76:86]) + portal_emitter_sensitivity(images[76:86]) +
                                camera_reports(log, True) + camera_report_sensitivity(log, True))
        elif args.scene == 'portal-emitters':
            report['checks'] = inspect_portal_emitters(images) + portal_emitter_sensitivity(images)
        elif args.scene == 'monitors':
            report['checks'] = inspect_monitors(images) + monitor_sensitivity(images)
        elif args.scene == 'sparks':
            report['checks'] = inspect_sparks(images) + spark_sensitivity(images)
        elif args.scene == 'particles':
            report['checks'] = inspect_particles(images) + particle_sensitivity(images)
        elif args.scene == 'cameras':
            report['checks'] = (inspect_cameras(images) + camera_sensitivity(images) +
                                camera_reports(log) + camera_report_sensitivity(log))
        elif args.scene == 'signage':
            report['checks'] = (inspect_signage(images) + signage_sensitivity(images) +
                                signage_reports(log) + signage_report_sensitivity(log))
        elif args.scene == 'emissives':
            report['checks'] = inspect_emissives(images) + emissive_sensitivity(images)
        elif args.scene == 'doors':
            report['checks'] = inspect_doors(images) + door_sensitivity(images)
        elif args.scene == 'cables':
            report['checks'] = inspect_cables(images) + cable_sensitivity(images)
            native_log = (args.capture / 'stdout.log').read_text(errors='replace')
            if re.search(r'native dropped.*cable/cable', native_log):
                raise ValueError('the native frontend dropped the required cable material')
        else:
            report['checks'] = inspect(images) + sensitivity(images)
        report['status'] = 'pass' if all(check['passed'] for check in report['checks']) else 'fail'
    except (KeyError, OSError, ValueError) as error:
        report['error'] = str(error)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Intro4 material pixels: {report['status']}, {len(report['checks'])} checks")
    return 0 if report['status'] == 'pass' else 1


if __name__ == '__main__':
    raise SystemExit(main())
