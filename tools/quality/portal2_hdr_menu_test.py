#!/usr/bin/env python3
"""Execute the actual HDR resource hook with owned resource/input fixtures."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import conformance

ROOT = Path(__file__).resolve().parents[2]

def run(output, seed_overlap):
    output.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'game/client/portal2/gameui/portal2/vvideo.cpp').read_text()
    begin = source.index('void HdrVideo::PreApplyControlSettings( KeyValues *resource )\n{')
    end = source.index('\n}', begin) + 2
    method = source[begin:end]
    if seed_overlap:
        method = method.replace("key->GetName()[0] == '?'", 'false')
    (output / 'portal2_hdr_layout_method.inc').write_text(method)
    profile = conformance.load_profile(str(ROOT / 'quality/profiles'), 'linux-headless-core')
    profile['generated_include_roots'] = []
    executable = output / 'hdr_menu_layout'
    command = conformance.build_command(str(ROOT), os.environ.get('CONFORMANCE_CXX', 'g++'),
        profile, {'sources': ['unittests/vguitest/test_hdr_menu_layout.cpp'],
                  'extra_flags': ['-I', str(output)]}, str(executable), config='release')
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=60)
    (output / 'compile.log').write_text(compiled.stdout + compiled.stderr)
    if compiled.returncode:
        print(compiled.stdout + compiled.stderr)
        return compiled.returncode
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    (output / 'run.log').write_text(result.stdout + result.stderr)
    print(result.stdout + result.stderr, end='')
    return result.returncode

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=os.environ.get('CONFORMANCE_OUT'))
    parser.add_argument('--seed-overlap', action='store_true')
    args = parser.parse_args()
    if args.out:
        return run(args.out.resolve(), args.seed_overlap)
    with tempfile.TemporaryDirectory(prefix='portal2-hdr-menu-') as temporary:
        return run(Path(temporary), args.seed_overlap)

if __name__ == '__main__':
    raise SystemExit(main())
