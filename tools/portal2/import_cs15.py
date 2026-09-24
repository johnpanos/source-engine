#!/usr/bin/env python3
"""Copy base-game files the Portal 2 target needs from the local CS:GO source drop.

The imported Portal 2 code was written against the later base game that also
shipped as cstrike15_src. Files listed in tools/portal2/cs15_imports.txt are
copied byte for byte from that drop to the same repository-relative path. A
file that already exists in the repository is never overwritten; adapt it in
place instead. The manifest records each file's source SHA-256 so the import
can be reproduced and audited. The repository's provenance and distribution
warning applies to every imported file.

  python3 tools/portal2/import_cs15.py --source <.../cstrike15_src>          # check
  python3 tools/portal2/import_cs15.py --source <.../cstrike15_src> --write  # copy
"""

import argparse
import hashlib
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LIST = ROOT / 'tools/portal2/cs15_imports.txt'
DEFAULT_SOURCE = (Path.home() / 'Downloads/April 22nd 2020, random leaked shit/mega-archive-extracted'
				  / 'full.7z/cstrike15_src')


def entries():
	"""Yield (path, recorded_sha256_or_None) from the manifest."""
	for line in LIST.read_text().splitlines():
		line = line.split('#', 1)[0].strip()
		if not line:
			continue
		parts = line.split()
		yield parts[0], (parts[1] if len(parts) > 1 else None)


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--source', type=Path, default=DEFAULT_SOURCE)
	parser.add_argument('--write', action='store_true', help='copy missing files and record hashes')
	args = parser.parse_args()

	lines = []
	failed = False
	for path, recorded in entries():
		source = args.source / path
		target = ROOT / path
		if not source.is_file():
			print(f'missing in source drop: {path}', file=sys.stderr)
			failed = True
			lines.append(f'{path} {recorded}' if recorded else path)
			continue
		digest = hashlib.sha256(source.read_bytes()).hexdigest()
		if recorded and recorded != digest:
			print(f'source differs from recorded hash: {path}', file=sys.stderr)
			failed = True
		if target.exists():
			if not recorded:
				print(f'refusing to overwrite existing repository file: {path}', file=sys.stderr)
				failed = True
		elif args.write:
			target.parent.mkdir(parents=True, exist_ok=True)
			target.write_bytes(source.read_bytes())
			os.chmod(target, 0o644)
			print(f'imported {path}')
		else:
			print(f'would import {path}')
		lines.append(f'{path} {digest}')

	if args.write:
		header = [l for l in LIST.read_text().splitlines() if l.startswith('#')]
		LIST.write_text('\n'.join(header + sorted(lines)) + '\n')
	return 1 if failed else 0


if __name__ == '__main__':
	sys.exit(main())
