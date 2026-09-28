#!/usr/bin/env python3
"""Link-map evidence for the render core's wiring (RFC 0016 A.7).

    python3 tools/render/core_links.py dedicated <tree>
    python3 tools/render/core_links.py client <tree>

`dedicated` passes when the tree records no render-core target and none of
its binaries defines render-core code or the engine's binding entry point:
the dedicated server cannot reach the core by construction.

`client` passes when the launcher defines RenderCore_Create (the composition
root links render.composition), the engine exports Engine_BindRenderCore and
defines no render-core function of its own (it reaches the core only through
the ports in the binding), and no other shared library defines render-core
code (one copy of the core per process).

Inline template instances that name render types (weak symbols) are header
code, not a linked module, and are ignored.
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

CORE_NAMESPACE = re.compile(
    r'\brender::(math|device|graph|shaderlib|resources|material|scene|frame|renderer|pass|legacy)::')
CORE_C_ENTRY = re.compile(r'^RenderCore_[A-Za-z]+$')
BINDING = 'Engine_BindRenderCore'
TARGET = re.compile(r'^render_')


def strong_symbols(nm_output):
    """Names of defined, non-weak text and data symbols in `nm -C` output."""
    names = []
    for line in nm_output.splitlines():
        parts = line.split(' ', 2)
        if len(parts) == 3 and parts[1] in ('T', 't', 'D', 'd', 'B', 'b', 'R', 'r'):
            names.append(parts[2])
    return names


def core_code(symbols):
    """Symbols that are render-core code: a function or object of a core
    module's namespace, or a composition C entry point. Template instances
    only mention render types in their arguments and are weak, so they never
    reach here; a strong symbol whose own name begins in a core namespace is
    core code."""
    found = []
    for name in symbols:
        base = name.split('(')[0]
        if CORE_C_ENTRY.match(base) or (CORE_NAMESPACE.match(base) and not base.startswith('std::')):
            found.append(name)
    return found


def nm(path):
    result = subprocess.run(['nm', '-C', '--defined-only', str(path)], capture_output=True, text=True)
    return result.stdout if result.returncode == 0 else ''


def binaries(tree):
    tree = Path(tree)
    found = []
    for path in sorted(tree.rglob('*')):
        if not path.is_file() or 'install' in path.relative_to(tree).parts:
            continue
        if path.suffix == '.so' or (path.suffix == '' and path.stat().st_mode & 0o111 and
                                    path.read_bytes()[:4] == b'\x7fELF'):
            found.append(path)
    return found


def recorded_targets(tree):
    record = json.loads((Path(tree) / 'toolchain-invocations.json').read_text())
    targets, uses = set(), set()
    for entry in record.get('entries', []):
        targets.add(entry['target'])
        uses.update(entry.get('use', []))
    return targets, uses


def check_dedicated(targets, uses, symbols_by_binary):
    errors = [f'dedicated tree builds render-core target {t}' for t in sorted(targets) if TARGET.match(t)]
    errors += [f'dedicated tree links render-core target {u}' for u in sorted(uses) if TARGET.match(u)]
    for binary, symbols in sorted(symbols_by_binary.items()):
        for name in core_code(symbols):
            errors.append(f'{binary}: defines render-core code {name}')
        if BINDING in symbols:
            errors.append(f'{binary}: exports {BINDING}')
    return errors


def check_client(symbols_by_binary):
    errors = []
    launcher = [b for b in symbols_by_binary if Path(b).name in ('liblauncher.so',)]
    engine = [b for b in symbols_by_binary if Path(b).name in ('libengine.so',)]
    if not launcher:
        errors.append('no launcher library in the tree')
    if not engine:
        errors.append('no engine library in the tree')
    for binary in launcher:
        if 'RenderCore_Create' not in symbols_by_binary[binary]:
            errors.append(f'{binary}: does not define RenderCore_Create (the root links no core)')
    for binary in engine:
        if BINDING not in symbols_by_binary[binary]:
            errors.append(f'{binary}: does not export {BINDING}')
        for name in core_code(symbols_by_binary[binary]):
            errors.append(f'{binary}: engine defines render-core code {name}; it must reach the core '
                          f'through the binding')
    for binary, symbols in sorted(symbols_by_binary.items()):
        if binary in launcher or binary in engine:
            continue
        # Executables that statically compose the whole product are roots too.
        if Path(binary).suffix != '.so':
            continue
        for name in core_code(symbols):
            errors.append(f'{binary}: a second copy of render-core code ({name})')
            break
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('mode', choices=['dedicated', 'client'])
    parser.add_argument('tree')
    args = parser.parse_args(argv)
    found = binaries(args.tree)
    if not found:
        print(f'core_links: no binaries in {args.tree}; build the tree first')
        return 1
    symbols = {str(b): strong_symbols(nm(b)) for b in found}
    if args.mode == 'dedicated':
        targets, uses = recorded_targets(args.tree)
        errors = check_dedicated(targets, uses, symbols)
    else:
        errors = check_client(symbols)
    for error in errors:
        print('core_links:', error)
    print(f'core_links: {args.mode} {args.tree}: {len(found)} binaries, {len(errors)} error(s)')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
