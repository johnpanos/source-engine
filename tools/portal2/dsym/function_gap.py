#!/usr/bin/env python3
"""List what a Portal 2 dSYM defines that the configured build does not.

For every compile unit in each dSYM (pyelftools, as dwarf_skeleton.py) this
finds the file the build compiles for that unit (build-p2's
compile_commands.json, matched by basename per side), then checks each
function the unit defines for a definition or call by name in that source.
Macro-generated members (datamaps, network and panel maps, factories) are
skipped. A unit with no file anywhere in the checkout is reported ABSENT.

A missing name is a lead, not a gap: the 2010 builds predate retail, so a
function can be absent because it was renamed, inlined into a caller, moved
to another file or cut before ship. Check the retail binary before porting.

  python3 tools/portal2/dsym/function_gap.py --build build-p2 \\
      --dsym 852_3:server=<server DWARF> --dsym 852_3:client=<client DWARF> \\
      [--filter portal] [--json out.json]
"""

import argparse
import collections
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from dwarf_skeleton import Unit, load_dwarf, source_relative, text  # noqa: E402

GENERATED = re.compile(
	r'(GetDataDescMap|GetBaseMap|GetServerClass|GetClientClass|YouForgotToImplement\w*|GetPredDescMap|'
	r'GetAnimMap|GetKBMap|GetMessageMap|GetVar_\w+|KB_ChainToMap|ChainToAnimationMap|ChainToMap|'
	r'InitVar|GetPanelClassName|Cache|Create_\w+|__MsgFunc_\w+|__Create\w+_interface|'
	r'Init\w+ScriptDesc|DataMapInit<.*|ServerClassInit<.*|ClientClassInit<.*|PredMapInit<.*|'
	r'_\w+_CreateObject|CC\w+Factory|\w+_KeyValueBuilder|ActivityList|ActivityListCount|'
	r'GetScriptDesc|GetScriptInstance|GetClassScheduleIdSpace|GetSchedulingErrorName|'
	r'InitCustomSchedules|LoadedSchedules|SquadSlotName)$')


def unit_functions(dwarf, wanted):
	result = collections.defaultdict(set)
	for cu in dwarf.iter_CUs():
		path = source_relative(text(cu.get_top_DIE().attributes['DW_AT_name'].value))
		if not wanted(path):
			continue
		unit = Unit(dwarf, cu)
		result[path]
		for die in cu.iter_DIEs():
			if die.tag != 'DW_TAG_subprogram' or 'DW_AT_low_pc' not in die.attributes:
				continue
			decl_file, _ = unit.decl(die)
			if decl_file is None:
				spec = unit.ref(die, 'DW_AT_specification')
				if spec is not None:
					decl_file, _ = unit.decl(spec)
			if decl_file != path:
				continue
			try:
				result[path].add(unit.qualified(die))
			except Exception:
				continue
	return result


def compiled_files(build):
	sides = collections.defaultdict(list)
	for entry in json.load(open(Path(build) / 'compile_commands.json')):
		args = ' '.join(entry['arguments'])
		side = 'server' if '-DGAME_DLL' in args else 'client' if '-DCLIENT_DLL' in args else None
		if side:
			path = os.path.relpath(os.path.normpath(os.path.join(entry['directory'], entry['file'])))
			sides[(side, os.path.basename(path).lower())].append(path)
	return sides


def defined(name_parts, source):
	name = name_parts[-1]
	scopes = [name_parts[-2]] if len(name_parts) > 1 else []
	if scopes and scopes[0].startswith('C_'):
		scopes.append('C' + scopes[0][2:])  # client classes are often #defined to the server name
	call = re.search(r'\b' + re.escape(name) + r'\s*\(', source)
	if not scopes:
		return bool(call)
	for scope in scopes:
		if re.search(re.escape(scope) + r'\s*::\s*' + re.escape(name) + r'\s*\(', source):
			return True
		if call and re.search(r'\b' + re.escape(scope) + r'\b', source):
			return True
	return False


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--dsym', action='append', required=True, help='TAG:server|client=<DWARF file>')
	parser.add_argument('--build', default='build-p2', help='configured tree with compile_commands.json')
	parser.add_argument('--filter', default='', help='only units whose path contains this text')
	parser.add_argument('--json', help='write the full result here')
	args = parser.parse_args()

	tracked = subprocess.run(['git', 'ls-files'], capture_output=True, text=True, check=True).stdout.split('\n')
	by_name = collections.defaultdict(list)
	for path in tracked:
		by_name[os.path.basename(path).lower()].append(path)
	compiled = compiled_files(args.build)

	units = collections.defaultdict(lambda: {'sides': set(), 'functions': set()})
	for spec in args.dsym:
		tag, path = spec.split('=', 1)
		side = tag.split(':')[1]
		for unit, functions in unit_functions(load_dwarf(path), lambda p: args.filter in p).items():
			units[unit]['sides'].add(side)
			units[unit]['functions'] |= functions

	rows = []
	for unit, info in sorted(units.items()):
		key = os.path.basename(unit).lower()
		sources = sorted({p for side in info['sides'] for p in compiled.get((side, key), [])})
		if not sources:
			sources = [unit] if os.path.exists(unit) else by_name.get(key, [])
		if not sources:
			rows.append({'unit': unit, 'sources': [], 'missing': sorted(info['functions']), 'total': len(info['functions'])})
			continue
		text_all = ''.join(open(p, 'rb').read().decode('latin-1') for p in sources)
		missing = []
		for qualified in sorted(info['functions']):
			parts = [p for p in qualified.split('::') if p != '<anon>']
			if 'ClassInit<' in qualified or 'MapInit<' in qualified:
				continue
			if not parts or GENERATED.match(parts[-1]) or parts[-1].startswith(('operator', '_GLOBAL', '__')):
				continue
			if not defined(parts, text_all):
				missing.append(qualified)
		rows.append({'unit': unit, 'sources': sources, 'missing': missing, 'total': len(info['functions'])})

	for row in sorted(rows, key=lambda r: -len(r['missing'])):
		if not row['missing']:
			continue
		where = ' '.join(row['sources']) if row['sources'] else 'ABSENT'
		print(f"{len(row['missing']):4d}/{row['total']:4d} {row['unit']} -> {where}")
		print('       ' + ', '.join(row['missing'])[:400])
	if args.json:
		json.dump(rows, open(args.json, 'w'), indent=1)


if __name__ == '__main__':
	main()
