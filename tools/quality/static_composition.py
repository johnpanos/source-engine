#!/usr/bin/env python3
"""Check a statically composed product (Waf --static-composition).

iOS links every first-party module into the app (AGENTS.md "Platform-compliant
composition"). scripts/waifulib/static_composition.py builds each first-party
shared library as a module object: one relocatable object in which symbols the
module did not export are local. This checker verifies the result of a build
tree independently of the Waf tool:

- the program needs no first-party shared library, and the tree built none;
- every module object keeps shared-library isolation: no hidden or
  STB_GNU_UNIQUE symbol is left global;
- no strong global symbol is defined by two module objects;
- the program defines the named entry of every module object that has one,
  and every module the caller requires is linked.

ELF only for now; Mach-O module objects come with the Apple profile.

    python3 tools/quality/static_composition.py check \\
        --tree build-static --program launcher_main/hl2_launcher \\
        --require client --require server
"""

import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
ENTRY = re.compile(r'^StaticModule_(\w+)_CreateInterface$')
# Toolchain-owned weak definitions (libstdc++ templates) may repeat; they are
# identical by the ODR and not module state.
STD_PREFIXES = ('_ZNSt', '_ZNKSt', '_ZSt', '_ZTISt', '_ZTSSt', '_ZZNSt', '_ZTVSt')


class Symbol:
    __slots__ = ('name', 'bind', 'visibility', 'section')

    def __init__(self, name, bind, visibility, section):
        self.name = name
        self.bind = bind
        self.visibility = visibility
        self.section = section

    @property
    def defined(self):
        return self.section != 'UND'


def run(argv):
    return subprocess.run(argv, check=True, capture_output=True, text=True).stdout


def symbols(path):
    """Symbol table entries of an ELF file (readelf -sW)."""
    found = []
    table = None
    for line in run(['readelf', '-sW', str(path)]).splitlines():
        if line.startswith('Symbol table'):
            table = '.dynsym' if "'.dynsym'" in line else '.symtab'
            continue
        fields = line.split()
        if table != '.symtab' or len(fields) < 8 or not fields[0].rstrip(':').isdigit():
            continue
        # Num: Value Size Type Bind Vis Ndx Name
        found.append(Symbol(fields[7], fields[4], fields[5], fields[6]))
    return found


def needed(path):
    out = run(['readelf', '-dW', str(path)])
    return re.findall(r'\(NEEDED\)\s+Shared library: \[([^\]]+)\]', out)


def first_party_libraries(modules_json):
    """Every first-party shared library recorded by CAP009."""
    data = json.loads(pathlib.Path(modules_json).read_text())
    names = set()
    section = data['capabilityModules']['sharedLibraries']
    for group in section.get('groups', []):
        names.update(group.get('targets', []))
    return names


def module_objects(tree):
    """Module objects of a tree: lib<target>.o (Waf compile outputs are *.<n>.o)."""
    return sorted(path for path in pathlib.Path(tree).rglob('lib*.o')
                  if 'toolchains' not in path.parts and path.name.count('.') == 1)


def module_name(path):
    return path.name[len('lib'):-len('.o')]


def check_module_object(path):
    errors = []
    for sym in symbols(path):
        if not sym.defined or sym.bind == 'LOCAL':
            continue
        if sym.bind == 'UNIQUE':
            errors.append('%s: %s is STB_GNU_UNIQUE; it would be shared across modules '
                          '(build with -fno-gnu-unique)' % (path, sym.name))
        elif sym.visibility in ('HIDDEN', 'INTERNAL'):
            errors.append('%s: hidden symbol %s is still global; module isolation needs '
                          'it local' % (path, sym.name))
    return errors


def check_duplicates(objects):
    owners = {}
    for path in objects:
        for sym in symbols(path):
            if sym.defined and sym.bind == 'GLOBAL' and sym.section != 'COM':
                owners.setdefault(sym.name, []).append(module_name(path))
    return ['%s is defined by module objects %s' % (name, ', '.join(sorted(mods)))
            for name, mods in sorted(owners.items())
            if len(mods) > 1 and not name.startswith(STD_PREFIXES)]


def check(tree, program, required, modules_json):
    tree = pathlib.Path(tree)
    program = tree / program
    errors = []
    first_party = first_party_libraries(modules_json)
    for library in needed(program):
        stem = re.sub(r'^lib|\.so(\.\d+)*$', '', library)
        if stem in first_party:
            errors.append('%s needs first-party shared library %s' % (program, library))
    shared = sorted(p for p in tree.rglob('*.so') if 'toolchains' not in p.parts)
    errors += ['%s: first-party shared library built in a static tree' % p for p in shared]

    objects = module_objects(tree)
    if not objects:
        errors.append('%s has no module objects; is it a --static-composition tree?' % tree)
    for path in objects:
        errors += check_module_object(path)
    errors += check_duplicates(objects)

    entries = set()
    for sym in symbols(program):
        match = ENTRY.match(sym.name)
        if match and sym.defined:
            entries.add(match.group(1))
    for path in objects:
        name = module_name(path)
        has_entry = any(ENTRY.match(s.name) and s.defined for s in symbols(path))
        if has_entry and name not in entries:
            errors.append('%s does not link module %s' % (program, name))
    for name in required:
        if name not in entries:
            errors.append('%s does not link required module %s' % (program, name))
    return errors, len(objects), sorted(entries)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    c = sub.add_parser('check', help='check a --static-composition build tree')
    c.add_argument('--tree', required=True)
    c.add_argument('--program', required=True, help='product program, relative to the tree')
    c.add_argument('--require', action='append', default=[],
                   help='module whose named entry the program must define (repeatable)')
    c.add_argument('--modules', default=str(ROOT / 'architecture' / 'modules.json'))
    args = parser.parse_args(argv)

    errors, count, entries = check(args.tree, args.program, args.require, args.modules)
    for error in errors:
        print('error: ' + error, file=sys.stderr)
    status = 'FAIL' if errors else 'PASS'
    print('%s: %d module objects, %d linked entries (%s), %d errors'
          % (status, count, len(entries), ' '.join(entries), len(errors)))
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
