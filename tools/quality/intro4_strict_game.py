#!/usr/bin/env python3
"""Run the actual Intro4 material captures with strict FSR on and off.

Uses portal_boot's isolated runtime and desktop game, then its deterministic
pixel oracle. Output must be fresh. No successful mode substitutes for a missing
or failed mode; this is material evidence, not a frame-performance gate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import shutil
import sys

from intro4_material_check import commands


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--build', type=Path, help='optional built product overlay; omit to test installed runtime')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--fsr-assets', type=Path, required=True)
    parser.add_argument('--fsr-scale', choices=('0.5', '0.588235', '0.666667', '1'),
                        default='0.5')
    parser.add_argument('--timeout', type=int, default=300)
    parser.add_argument('--scene', choices=('all', 'materials', 'doors', 'cables', 'emissives', 'signage', 'cameras', 'particles'),
                        default='all')
    args = parser.parse_args()
    if args.out.exists():
        parser.error('evidence already exists; use a fresh output directory')
    args.out.mkdir(parents=True)
    results = []
    root = Path(__file__).resolve().parents[2]
    # Freeze the oracle with the command sequence for this run. A source edit
    # during the second mode must not silently change its acceptance contract.
    oracle_path = args.out / "intro4_material_check.py"
    shutil.copyfile(root / "tools/quality/intro4_material_check.py", oracle_path)
    oracle_sha256 = hashlib.sha256(oracle_path.read_bytes()).hexdigest()
    for mode, scale in (('fsr-on', args.fsr_scale), ('fsr-off', '0')):
        capture = args.out / mode
        boot = [sys.executable, str(root / 'tools/quality/portal_boot.py'),
                '--runtime', str(args.runtime.resolve()),
                '--out', str(capture.resolve()), '--game', 'portal2',
                '--renderer', 'native-vulkan', '--require-vulkan',
                '--map', 'sp_a1_intro4_relit', '--timeout', str(args.timeout),
                '--map-after-start',
                '--capture-wait', '120', '--allow-user-display', '--no-mouse',
                '--physics', 'vphysics_box3d', '--engine-arg=-physics_shape_inertia',
                '--engine-arg=-vkemitparallel', '--engine-arg=1']
        if args.build is not None:
            boot.extend(['--build', str(args.build.resolve())])
        if mode == 'fsr-on':
            boot.extend(['--engine-arg=-fsr', '--engine-arg=-fsr-assets',
                         f'--engine-arg={args.fsr_assets.resolve()}'])
        for setting in ('sv_cheats 1', 'host_framerate 0.015', 'mat_queue_mode 2',
                        'r_core_world 1', 'r_core_world_strict 1', 'r_core_dynamic_draws 0',
                        'r_indirect_producer baked', 'r_core_runtime_direct 1',
                        'cl_render_start_graph 2', 'sv_querycache_job_graph 2',
                        'mat_colorcorrection 1', 'cl_surface_core_emission 1',
                        'cl_surface_core_emission_strength 16',
                        f'r_temporal_scale {scale}'):
            boot.extend(['--startup-command', setting])
        sequence = commands(args.scene)
        if args.scene != 'all':
            sequence = sequence + ['r_temporal_scale']
        for command in sequence:
            boot.extend(['--console-command', command])
        (args.out / f'{mode}-command.json').write_text(json.dumps(boot, indent=2) + '\n')
        print(f'{mode}: launching strict desktop game', flush=True)
        with (args.out / f'{mode}-boot.log').open('w') as log:
            boot_status = subprocess.run(boot, cwd=root, stdout=log,
                                         stderr=subprocess.STDOUT).returncode
        oracle = [sys.executable, str(oracle_path.resolve()),
                  '--scene', args.scene, '--temporal-scale', scale,
                  '--capture', str(capture.resolve()),
                  '--out', str((args.out / f'{mode}-pixels.json').resolve())]
        oracle_status = subprocess.run(oracle, cwd=root).returncode
        results.append(dict(mode=mode, scale=scale, boot_exit=boot_status,
                            oracle_exit=oracle_status))
        print(f'{mode}: boot={boot_status}, pixels={oracle_status}', flush=True)
    passed = all(row['boot_exit'] == 0 and row['oracle_exit'] == 0 for row in results)
    (args.out / 'evidence.json').write_text(json.dumps(
        dict(schema='intro4-strict-modes/v1', status='pass' if passed else 'fail',
             oracle_sha256=oracle_sha256, modes=results),
        indent=2) + '\n')
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
