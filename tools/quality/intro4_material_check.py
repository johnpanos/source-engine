#!/usr/bin/env python3
"""Pixel checks for the fixed Intro4 glass, indicator and decal game captures.

Use --commands to print the portal_boot console-command list. Run it with
host_framerate 0.015, r_core_world 1, r_core_world_strict 1,
r_core_dynamic_draws 0 and FSR quality at 1024x768. Then pass its output directory
to --capture. These checks certify these visible behaviors, not all materials,
glass optics, complete frame performance or platform support.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import re

import numpy as np
from PIL import Image


def commands():
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


def inspect(images):
    if len(images) < 8 or any(image.shape != (768, 1024, 3) for image in images):
        raise ValueError('eight complete 1024x768 RGB captures are required')
    checks = []

    def check(name, passed, value):
        checks.append(dict(name=name, passed=bool(passed), value=float(value)))

    # The old FSR error transmitted the bright neutral core-only hatch here.
    glass = images[0][145:485, 680:770]
    hatch = np.mean((glass.min(axis=2) > .4) & (np.ptp(glass, axis=2) < .08))
    check('glass.no-unrendered-attachment-hatch', hatch < .01, hatch)
    # Turning the transmitting renderables off must change the pane. This
    # prevents a missing Refract draw from passing only the no-hatch check.
    difference = np.max(np.abs(images[0][165:300, 600:730] -
                               images[1][165:300, 600:730]), axis=2)
    changed = np.mean(difference > .04)
    check('glass.transmitting-cohort-reaches-game', changed > .015, changed)

    for index in (2, 3, 4, 5):
        strip = images[index][440:745, 495:530]
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
    mural_delta = np.max(np.abs(images[6][445:535, 382:640] -
                                images[7][445:535, 382:640]), axis=2)
    changed = np.mean(mural_delta > .04)
    check('decal.modulate-mural-reaches-game', changed > .2, changed)
    return checks


def sensitivity(images):
    checks = []
    for name, mutation, expected in (
            ('hatch', lambda data: data[0].__setitem__((slice(145, 485), slice(680, 770)), .7),
             'glass.no-unrendered-attachment-hatch'),
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
    parser.add_argument('--capture', type=Path)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    if args.commands:
        print(json.dumps(commands(), indent=2))
        return 0
    if not args.capture or not args.out:
        parser.error('--capture and --out are required')
    report = dict(schema='intro4-material-pixels/v1', capture=str(args.capture.resolve()),
                  checks=[], inputs=[], status='fail')
    try:
        evidence = json.loads((args.capture / 'evidence.json').read_text())
        if evidence['status'] != 'pass' or evidence['map'] != 'sp_a1_intro4_relit':
            raise ValueError('the required Intro4 game boot did not pass')
        # portal_boot appends its final capture, diagnostics and shutdown.
        if evidence['console_script']['commands'][:len(commands())] != commands():
            raise ValueError('the capture command sequence does not match this fixture')
        screenshots = sorted((args.capture / 'runtime/portal2/screenshots').glob('*.tga'))
        images = []
        for path in screenshots:
            with Image.open(path) as image:
                images.append(np.asarray(image.convert('RGB'), dtype=np.float32) / 255)
            report['inputs'].append(dict(path=str(path),
                                         sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        log = (args.capture / 'runtime/portal2/console.log').read_text(errors='replace')
        failures = re.findall(r'views queued \d+ drawn \d+ failed (\d+)', log)
        if not failures or any(int(value) for value in failures):
            raise ValueError('missing core statistics or claimed-view failure')
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
