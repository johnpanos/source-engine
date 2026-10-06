#!/usr/bin/env python3
"""Dump full class layouts (i386 offsets, members, methods) by class name.

dwarf_skeleton.py skips records whose header exists in this checkout. This
reads the same dSYM and renders the named records from one compile unit even
when their header is present, to compare retail layouts with this fork's.

  python3 tools/portal2/dsym/class_layout.py --dsym <DWARF> --unit game/client/c_baseplayer.cpp \
      --out <file> C_BasePlayer CSetActiveSplitScreenPlayerGuard
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dwarf_skeleton as ds


def main():
	ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	ap.add_argument('--dsym', required=True)
	ap.add_argument('--unit', required=True, help='repository-relative .cpp of the compile unit')
	ap.add_argument('--out', required=True)
	ap.add_argument('--label', default='')
	ap.add_argument('classes', nargs='+')
	args = ap.parse_args()
	dwarf = ds.load_dwarf(args.dsym)
	want = set(args.classes)
	found = {}
	for cu in dwarf.iter_CUs():
		top = cu.get_top_DIE()
		n = top.attributes.get('DW_AT_name')
		if n is None:
			continue
		raw = ds.text(n.value)
		cd = top.attributes.get('DW_AT_comp_dir')
		if not raw.startswith('/') and cd is not None:
			raw = ds.text(cd.value) + '/' + raw
		if ds.source_relative(raw) != args.unit:
			continue
		unit = ds.Unit(dwarf, cu)
		unit.files_cu = args.unit

		def visit(die):
			for c in die.iter_children():
				if c.tag == 'DW_TAG_namespace':
					visit(c)
				elif c.tag in ds.RECORD_TAGS and unit.name(c) in want \
						and not unit.attr(c, 'DW_AT_declaration') and unit.name(c) not in found:
					file, line = unit.decl(c)
					found[unit.name(c)] = f'// {file}:{line}\n' + '\n'.join(ds.render_record(unit, c))
		visit(unit.top)
	out = [f'// Class layouts from {args.label or args.dsym}', f'// Unit {args.unit}; i386 offsets; reconstruction aid.', '']
	for name in args.classes:
		out.append(found.get(name, f'// {name}: not found as a complete record in this unit'))
		out.append('')
	Path(args.out).parent.mkdir(parents=True, exist_ok=True)
	Path(args.out).write_text('\n'.join(out))
	print(f'wrote {args.out}: {len(found)}/{len(want)} found', file=sys.stderr)


if __name__ == '__main__':
	main()
