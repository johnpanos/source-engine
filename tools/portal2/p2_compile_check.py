#!/usr/bin/env python3
"""Compile selected Portal 2 client/server sources with the build-p2 settings.

Waf refuses to build the Portal 2 client or server while any selected source is
missing. This reuses the configured build-p2 cache and the same VPC selection so
individual files can be checked while reconstruction is in progress. It is a
compile check only and says nothing about linking or runtime behavior.

  python3 tools/portal2/p2_compile_check.py server game/server/portal2/paint_sphere.cpp
  python3 tools/portal2/p2_compile_check.py client --all-present --jobs 16
"""

import argparse
import ast
import concurrent.futures
import os
import shlex
import subprocess
import sys
from pathlib import Path
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts' / 'waifulib'))
import vpc_parser  # noqa: E402

VPCS = {
	'client': ['client_base.vpc', 'client_portal_base.vpc', 'client_portal2.vpc'],
	'server': ['server_base.vpc', 'server_portal_base.vpc', 'server_portal2.vpc'],
}
BASE_INCLUDES = {
	'client': ['.', 'game_controls', '../../common', '../../public', '../../public/tier0',
			   '../../public/tier1', '../../vgui2/include', '../../vgui2/controls', '../shared'],
	'server': ['.', '../shared', '../../utils/common', '../shared/econ', 'NextBot', '../../common',
			   '../../public/tier0', '../../public/tier1', '../../public'],
}


def load_cache(build_dir, side):
	env = {}
	for name in ('_cache.py', f'game/{side}_cache.py'):
		path = build_dir / 'c4che' / name
		if not path.is_file():
			continue
		for line in path.read_text().splitlines():
			key, sep, value = line.partition(' = ')
			if sep:
				try:
					env[key.strip()] = ast.literal_eval(value)
				except (ValueError, SyntaxError):
					pass
	if env.get('GAMES') != 'portal2':
		raise SystemExit(f'{build_dir} is not configured for portal2')
	return env


def project(build_dir, side):
	cache = load_cache(build_dir, side)
	env = SimpleNamespace(DEFINES=list(cache['DEFINES']), SUBPROJECT_PATH=[str(ROOT / 'game' / side)])
	cwd = os.getcwd()
	try:
		game = vpc_parser.parse_vpcs(env, VPCS[side], '../..')
	finally:
		os.chdir(cwd)
	game_dir = ROOT / 'game' / side
	defines = [d for d in cache['DEFINES'] + game['defines'] if d != 'PROTECTED_THINGS_ENABLE']
	includes = [os.path.normpath(game_dir / i) for i in BASE_INCLUDES[side] + game['includes']]
	includes += cache.get('INCLUDES', []) + cache.get('INCLUDES_SDL2', [])
	flags = [f for f in cache['CXXFLAGS'] if f not in ('-MMD', '-w') and not f.startswith('-L')]
	sources = [os.path.normpath(game_dir / s) for s in game['sources']]
	return cache['CXX'], flags, defines, includes, sources


def command(cxx, flags, defines, includes, source, extra):
	return (list(cxx) + flags + ['-fsyntax-only', '-w', '-fmax-errors=50'] + extra
			+ ['-D' + d for d in defines] + ['-I' + i for i in includes] + [source])


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('side', choices=sorted(VPCS))
	parser.add_argument('files', nargs='*')
	parser.add_argument('--build-dir', default='build-p2')
	parser.add_argument('--all-present', action='store_true', help='check every present selected source')
	parser.add_argument('--jobs', type=int, default=os.cpu_count())
	parser.add_argument('--print-command', action='store_true')
	parser.add_argument('--quiet', action='store_true', help='only list pass/fail per file')
	parser.add_argument('--extra', default='', help='extra compiler flags (shell-quoted)')
	args = parser.parse_args()

	cxx, flags, defines, includes, sources = project(ROOT / args.build_dir, args.side)
	files = [os.path.normpath(ROOT / f) for f in args.files]
	if args.all_present:
		files += [s for s in sources if os.path.isfile(s)]
	if not files:
		parser.error('no files selected')
	selected = set(sources)
	for f in files:
		if f not in selected:
			print(f'note: {os.path.relpath(f, ROOT)} is not selected by the {args.side} VPCs', file=sys.stderr)
	extra = shlex.split(args.extra)
	if args.print_command:
		print(shlex.join(command(cxx, flags, defines, includes, files[0], extra)))
		return 0

	def run(f):
		result = subprocess.run(command(cxx, flags, defines, includes, f, extra), cwd=ROOT / 'game' / args.side,
								capture_output=True, text=True)
		return f, result.returncode, result.stdout + result.stderr

	failed = 0
	with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
		for f, code, output in pool.map(run, files):
			rel = os.path.relpath(f, ROOT)
			print(f'{"PASS" if code == 0 else "FAIL"} {rel}')
			if code:
				failed += 1
				if not args.quiet:
					print(output)
	print(f'{len(files) - failed}/{len(files)} compiled', file=sys.stderr)
	return 1 if failed else 0


if __name__ == '__main__':
	sys.exit(main())
