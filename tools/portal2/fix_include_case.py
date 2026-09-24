#!/usr/bin/env python3
"""Correct include spellings in the Portal 2 target for case-sensitive filesystems.

The imported Portal 2 sources were authored on Windows and spell many includes
with a case that differs from the file on disk (e.g. "BaseEntity.h" for
baseentity.h). For every source selected by the build-p2 configuration and every
header it reaches, each #include is resolved the way the original Windows build
did (quoted includes: includer directory, then -I directories in order, matching
case-insensitively). When the first match's on-disk spelling differs, the include
is rewritten to it. Only files under a portal2/ directory are edited; other
files (e.g. Portal 1 sources the Portal 2 build shares) are reported. Line
endings and all other bytes are preserved.

  python3 tools/portal2/fix_include_case.py            # report only
  python3 tools/portal2/fix_include_case.py --write    # apply
"""

import argparse
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import p2_compile_check  # noqa: E402

INCLUDE = re.compile(rb'^([ \t]*#[ \t]*include[ \t]*)(["<])([^">\r\n]+)([">])', re.M)
_listing = {}


def entries(directory):
	if directory not in _listing:
		try:
			_listing[directory] = os.listdir(directory)
		except OSError:
			_listing[directory] = None
	return _listing[directory]


def exact(directory, relative):
	return os.path.isfile(os.path.join(directory, relative))


def folded(directory, relative):
	"""Return the on-disk relative spelling of `relative` under `directory`, or None."""
	current = directory
	parts = []
	for part in relative.replace('\\', '/').split('/'):
		if part in ('', '.'):
			continue
		if part == '..':
			current = os.path.dirname(current)
			parts.append('..')
			continue
		names = entries(current)
		if names is None:
			return None
		matches = [n for n in names if n.casefold() == part.casefold()]
		if len(matches) != 1:
			return None
		parts.append(matches[0])
		current = os.path.join(current, matches[0])
	return '/'.join(parts) if os.path.isfile(current) else None


def resolve(includer, name, quoted, includes):
	"""Return (found_path, corrected_spelling_or_None)."""
	name = name.replace('\\', '/')
	search = ([os.path.dirname(includer)] if quoted else []) + includes
	# The sources were built on Windows, where the first directory holding a
	# case-insensitive match wins even if a later directory has an exact match.
	for directory in search:
		spelling = folded(directory, name)
		if spelling is not None:
			return os.path.normpath(os.path.join(directory, spelling)), spelling
	return None, None


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--build-dir', default='build-p2')
	parser.add_argument('--write', action='store_true')
	args = parser.parse_args()

	edits = {}
	foreign = set()
	for side in ('server', 'client'):
		_cxx, _flags, _defines, includes, sources = p2_compile_check.project(ROOT / args.build_dir, side)
		includes = [i for i in includes if os.path.isdir(i)]
		pending = [s for s in sources if os.path.isfile(s)]
		seen = set(pending)
		while pending:
			path = pending.pop()
			data = open(path, 'rb').read()
			for match in INCLUDE.finditer(data):
				name = match.group(3).decode('latin1')
				found, spelling = resolve(path, name, match.group(2) == b'"', includes)
				if found is None:
					continue
				if spelling is not None and spelling != name:
					if '/portal2/' in path:
						edits.setdefault(path, {})[name] = spelling
					else:
						foreign.add(f'{os.path.relpath(path, ROOT)}: "{name}" -> "{spelling}" ({side})')
				if found.startswith(str(ROOT)) and found not in seen:
					seen.add(found)
					pending.append(found)

	total = 0
	for path, fixes in sorted(edits.items()):
		for old, new in sorted(fixes.items()):
			print(f'{os.path.relpath(path, ROOT)}: "{old}" -> "{new}"')
			total += 1
		if args.write:
			data = open(path, 'rb').read()

			def replace(match):
				name = match.group(3).decode('latin1')
				new = fixes.get(name)
				if new is None:
					return match.group(0)
				return match.group(1) + match.group(2) + new.encode('latin1') + match.group(4)

			open(path, 'wb').write(INCLUDE.sub(replace, data))
	for line in sorted(foreign):
		print('not edited (outside portal2/):', line)
	print(f'{total} include spellings in {len(edits)} files{" rewritten" if args.write else ""}', file=sys.stderr)
	return 0


if __name__ == '__main__':
	sys.exit(main())
