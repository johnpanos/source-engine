#!/usr/bin/env python3
"""Pixel checks for fixed Intro4 materials and gameplay doorway captures.

Use --commands to print the portal_boot console-command list. Run it with
host_framerate 0.015, r_core_world 1, r_core_world_strict 1,
r_core_dynamic_draws 0 and FSR quality at 1024x768. Then pass its output directory
to --capture. These checks certify these visible behaviors, not all materials,
glass optics, complete frame performance or platform support. Desktop HiDPI
captures use regions scaled to the actual pixels; images are never downsampled.
Use --scene doors for the closed/open door and independent indicator-box cycle.
"""
import argparse
import copy
import hashlib
import json
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


def commands(scene='materials'):
    if scene == 'doors':
        return door_commands()
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
    parser.add_argument('--scene', choices=('materials', 'doors'), default='materials')
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
                (args.scene == 'doors' and (not strict or not dynamic))):
            raise ValueError('strict/default-cohort game queries are missing or incorrect')
        failures = re.findall(r'views queued \d+ drawn \d+ failed (\d+)', log)
        if not failures or any(int(value) for value in failures):
            raise ValueError('missing core statistics or claimed-view failure')
        report['checks'] = (inspect_doors(images) + door_sensitivity(images)
                            if args.scene == 'doors' else inspect(images) + sensitivity(images))
        report['status'] = 'pass' if all(check['passed'] for check in report['checks']) else 'fail'
    except (KeyError, OSError, ValueError) as error:
        report['error'] = str(error)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Intro4 material pixels: {report['status']}, {len(report['checks'])} checks")
    return 0 if report['status'] == 'pass' else 1


if __name__ == '__main__':
    raise SystemExit(main())
