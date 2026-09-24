#!/usr/bin/env python3
"""Compile selected Portal 2 client/server sources with the build-p2 settings.

Waf refuses to build the Portal 2 client or server while any selected source is
missing. This reuses the configured build-p2 cache and the same VPC selection so
individual files can be checked while reconstruction is in progress. It is a
compile check only and says nothing about linking or runtime behavior.

  python3 tools/portal2/p2_compile_check.py server game/server/portal2/paint_sphere.cpp
  python3 tools/portal2/p2_compile_check.py client --all-present --jobs 16
  python3 tools/portal2/p2_compile_check.py server --probe game/server/portal2/paint_sphere.cpp

--probe is for reconstruction work while the imported base game still fails to
compile. It replaces each missing header outside the checked files with an empty
stub in <build-dir>/p2-probe-shims (never in the source tree), retries, and
reports only diagnostics located in the checked files and their headers (or the
--own paths). An empty stub hides that header's declarations, so errors that
name its types are base-port gaps, not proof of a defect in the checked file.
"""

import argparse
import ast
import re
import concurrent.futures
import json
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
# The imported Portal 2 sources spell some includes from the source root.
PORTAL2_INCLUDES = ['../..']
FATAL_MISSING = re.compile(r'^(?P<src>[^:\n]+):\d+:\d+: fatal error: (?P<hdr>[^:]+): No such file or directory', re.M)
DIAGNOSTIC = re.compile(r'^(?P<file>/[^:\n]+):(?P<line>\d+):(?P<col>\d+): (?P<kind>fatal error|error|warning): (?P<msg>.*)$', re.M)


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
	includes = [os.path.normpath(game_dir / i) for i in BASE_INCLUDES[side] + game['includes'] + PORTAL2_INCLUDES]
	includes += cache.get('INCLUDES', []) + cache.get('INCLUDES_SDL2', [])
	flags = [f for f in cache['CXXFLAGS'] if f not in ('-MMD', '-w') and not f.startswith('-L')]
	# Mirrors the Portal 2 force-include in game/{client,server}/wscript.
	flags += ['-include', str(ROOT / 'game/shared/portal2/portal2_base_compat.h')]
	sources = [os.path.normpath(game_dir / s) for s in game['sources']]
	return cache['CXX'], flags, defines, includes, sources


def command(cxx, flags, defines, includes, source, extra, max_errors=50):
	return (list(cxx) + flags + ['-fsyntax-only', '-w', f'-fmax-errors={max_errors}'] + extra
			+ ['-D' + d for d in defines] + ['-I' + i for i in includes] + [source])


def own_paths(source, extra_own):
	stem = os.path.splitext(source)[0]
	own = {source, stem + '.h'}
	directory, base = os.path.split(stem)
	if base.startswith('c_'):
		own.add(os.path.join(directory, base + '.h'))
	return own | set(extra_own)


def probe(cxx, flags, defines, includes, source, extra, shim_root, own):
	"""Compile with empty stubs for missing foreign headers; return own diagnostics."""
	shims = []
	for _ in range(200):
		cmd = command(cxx, flags, defines, includes + [str(shim_root)], source, extra, max_errors=0)
		result = subprocess.run(cmd, cwd=os.path.dirname(source), capture_output=True, text=True)
		output = result.stdout + result.stderr
		fatal = FATAL_MISSING.search(output)
		if not fatal:
			break
		header = fatal.group('hdr')
		includer = os.path.normpath(fatal.group('src'))
		if own is not None and includer in own:
			# A checked file names a header that does not exist: that is its own gap.
			shims.append(f'{header} (included by checked file {os.path.relpath(includer, ROOT)})')
		else:
			shims.append(header)
		stub = shim_root / header
		if stub.exists():
			# The spelling resolved relative to the includer only; nothing a stub can fix.
			return shims, [(includer, 0, 0, 'fatal error', f'{header}: No such file or directory')], output
		stub.parent.mkdir(parents=True, exist_ok=True)
		stub.write_text('// p2_compile_check probe stub: header absent from this checkout\n#pragma once\n')
	diagnostics = []
	for match in DIAGNOSTIC.finditer(output):
		path = os.path.normpath(match.group('file'))
		if (own is None or path in own) and match.group('kind') != 'warning':
			diagnostics.append((path, int(match.group('line')), int(match.group('col')), match.group('kind'),
								match.group('msg')))
	return shims, diagnostics, output


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
	parser.add_argument('--probe', action='store_true', help='stub missing foreign headers; report own diagnostics')
	parser.add_argument('--own', action='append', default=[], help='additional paths whose diagnostics count')
	parser.add_argument('--show-shims', action='store_true', help='list stubbed headers per file in --probe')
	parser.add_argument('--all-diagnostics', metavar='JSON',
						help='with --probe: record every error in every file to JSON (whole-target census)')
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

	if args.probe:
		shim_root = ROOT / args.build_dir / 'p2-probe-shims' / args.side
		shim_root.mkdir(parents=True, exist_ok=True)
		extra_own = [os.path.normpath(ROOT / p) for p in args.own]
		if args.all_diagnostics:
			def census(f):
				shims, diagnostics, _output = probe(cxx, flags, defines, includes, f, extra, shim_root, None)
				return os.path.relpath(f, ROOT), shims, [
					(os.path.relpath(p, ROOT), line, kind, msg) for p, line, _col, kind, msg in diagnostics]
			# Stub creation races are benign (identical content), so files run concurrently.
			with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
				results = list(pool.map(census, files))
			with open(args.all_diagnostics, 'w') as handle:
				json.dump([{'file': f, 'stubs': s, 'errors': e} for f, s, e in results], handle, indent=1)
			clean = sum(1 for _f, _s, e in results if not e)
			print(f'{clean}/{len(results)} files without errors; '
				  f'{sum(len(e) for _f, _s, e in results)} errors recorded in {args.all_diagnostics}', file=sys.stderr)
			return 0 if clean == len(results) else 1
		failed = 0
		for f in files:
			shims, diagnostics, _output = probe(cxx, flags, defines, includes, f, extra, shim_root,
												own_paths(f, extra_own))
			rel = os.path.relpath(f, ROOT)
			print(f'{"PASS" if not diagnostics else "FAIL"} {rel} ({len(diagnostics)} own diagnostics, '
				  f'{len(shims)} stubbed headers)')
			if args.show_shims:
				for shim in shims:
					print(f'  stub: {shim}')
			if diagnostics:
				failed += 1
				if not args.quiet:
					for path, line, col, kind, msg in diagnostics:
						print(f'  {os.path.relpath(path, ROOT)}:{line}:{col}: {kind}: {msg}')
		print(f'{len(files) - failed}/{len(files)} without own diagnostics (probe mode)', file=sys.stderr)
		return 1 if failed else 0

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
