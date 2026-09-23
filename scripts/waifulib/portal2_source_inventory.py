"""Inventory files named by the imported Portal 2 VPC projects.

This is a source-presence preflight, not a claim that the projects compile.
The VPC declarations include conditional source entries; the scan deliberately
reports their union so no missing source is silently hidden by one host profile.
"""

import argparse
import ast
import json
import re
from pathlib import Path
from types import SimpleNamespace

import vpc_parser


SOURCE_REF = re.compile(r'"([^"\n]+\.(?:cpp|c|h))"', re.IGNORECASE)


def project_files(root, side, include_econ=False):
    names = [f'{side}_portal_base.vpc', f'{side}_portal2.vpc']
    if include_econ:
        names.append(f'{side}_econ_base.vpc')
    return [root / 'game' / side / name for name in names]


def source_references(project):
    pending = ''
    for physical in project.read_bytes().decode('latin1').splitlines():
        line = physical.strip()
        if line.startswith('//'):
            continue
        pending += line
        if pending.endswith('\\'):
            pending = pending[:-1] + ' '
            continue
        if '$File' in pending and not pending.startswith('-$File'):
            for match in SOURCE_REF.finditer(pending):
                yield match.group(1)
        pending = ''


def resolve_reference(root, side, reference):
    path = reference.replace('\\', '/')
    if path.startswith('$SRCDIR/'):
        return root / path[len('$SRCDIR/'):]
    return root / 'game' / side / path


def case_insensitive_file(path, root):
    """Resolve legacy VPC spelling without treating a different file as a match."""
    try:
        parts = path.relative_to(root).parts
    except ValueError:
        return None
    current = root
    for part in parts:
        if not current.is_dir():
            return None
        matches = [p for p in current.iterdir() if p.name.casefold() == part.casefold()]
        if len(matches) != 1:
            return None
        current = matches[0]
    return current if current.is_file() else None


def inventory(root, include_econ=False):
    root = Path(root).resolve()
    records = []
    for side in ('client', 'server'):
        for project in project_files(root, side, include_econ):
            for reference in source_references(project):
                target = resolve_reference(root, side, reference)
                found = case_insensitive_file(target, root)
                records.append({
                    'side': side,
                    'project': str(project.relative_to(root)),
                    'reference': reference,
                    'path': str(target.relative_to(root)),
                    'present': found is not None,
                })
    return records


def selected_inventory(root, build):
    """Read the source selection Waf will use for one configured build."""
    root, build = Path(root).resolve(), Path(build).resolve()
    records = []
    for side in ('client', 'server'):
        cache = build / 'c4che/game' / (side + '_cache.py')
        defines = None
        for line in cache.read_text().splitlines():
            key, separator, literal = line.partition(' = ')
            if separator and key == 'DEFINES':
                defines = ast.literal_eval(literal)
        if not isinstance(defines, list) or not all(isinstance(item, str) for item in defines):
            raise ValueError('invalid Waf defines in ' + str(cache))
        folder = root / 'game' / side
        env = SimpleNamespace(SUBPROJECT_PATH=[str(folder)], DEFINES=defines)
        projects = [f'{side}_base.vpc', f'{side}_portal_base.vpc', f'{side}_portal2.vpc']
        sources = vpc_parser.parse_vpcs(env, projects, '../..')['sources']
        for source in sources:
            target = (folder / source).resolve()
            records.append({
                'side': side,
                'project': ', '.join(str(folder / name) for name in projects),
                'reference': source,
                'path': str(target.relative_to(root)),
                'present': target.is_file(),
            })
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--include-econ', action='store_true')
    parser.add_argument('--selected-build', type=Path,
                        help='report files selected by an existing Waf Portal 2 build')
    parser.add_argument('--json', action='store_true')
    args = parser.parse_args()
    records = (selected_inventory(args.root, args.selected_build) if args.selected_build
               else inventory(args.root, args.include_econ))
    missing = [record for record in records if not record['present']]
    if args.json:
        print(json.dumps({'references': len(records), 'missing': missing}, indent=2))
    else:
        print(f'{len(missing)} missing of {len(records)} declared Portal 2 source references')
        for record in missing:
            print(f"{record['side']}: {record['path']} ({record['project']})")
    return 1 if missing else 0


if __name__ == '__main__':
    raise SystemExit(main())
