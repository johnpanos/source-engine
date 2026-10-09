"""Structure ratchets (CAP012, CAP013): the shape the architecture promises,
recorded in architecture/structure.json and checked by `archlint check --all`.

CAP011 checks that include edges point down. It cannot see a composition
root that grows into a renderer, a core API that forward-declares legacy
types, a pass that receives geometry outside the scene, or a Tier 0 that
keeps every platform branch behind a provider call. These rules can.

CAP012 identifier and branch ratchets. Each rule names a scope (capability
  modules by id or `prefix.*`, minus `exceptModules`, and/or path prefixes),
  a matcher (`identifiers`: whole words in code with comments and literals
  stripped; or `platform-branches`: preprocessor conditionals that test a
  platform macro), a reason, an owner and its target. The per-file counts
  are exact: a new file or a higher count fails, and a lower count fails
  until it is recorded, so the list only shrinks and stays current.

CAP013 line ceilings. Every first-party area (the capability module that
  owns a file, or its legacy directory) has a ceiling of code lines (blank
  and comment-only lines are not counted). Above the ceiling fails. `--write`
  lowers ceilings to the current size and never raises one; growth is a
  reviewed decision made with `--raise AREA --reason TEXT`, which records the
  reason and date beside the new ceiling.

    python3 tools/archlint/archlint.py structure --verify
    python3 tools/archlint/archlint.py structure --write
    python3 tools/archlint/archlint.py structure --raise render.pass.world --reason "..."
    python3 tools/archlint/archlint.py structure --adopt RULE   # a new rule's first counts
    python3 tools/archlint/archlint.py structure --report [--top N]
"""

from __future__ import annotations

import datetime
import json
import re
import subprocess
from pathlib import Path

from capabilities import matches, owner

STRUCTURE = 'architecture/structure.json'
SCHEMA = 'archlint-structure/v1'
RULE_KEYS = {'id', 'kind', 'scope', 'identifiers', 'reason', 'owner', 'target', 'counts'}
SCOPE_KEYS = {'modules', 'exceptModules', 'paths', 'exceptPaths'}
KINDS = ('identifiers', 'platform-branches')
# Shaders are code too; the render core's size includes them.
LINE_SUFFIXES = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.inl', '.mm',
                 '.glsl', '.vert', '.frag', '.comp', '.hlsl', '.wgsl', '.metal'}
# Legacy directories this deep get one area per subdirectory.
DEEP_LEGACY = {'game', 'materialsystem', 'utils', 'public', 'tools', 'platform', 'render', 'hammer',
               'engine', 'common', 'unittests', 'external', 'devtools', 'games'}
EXCLUDED_PREFIXES = ('thirdparty/', 'dependencies/', 'lib/', 'box3d/', 'ivp/', 'games/csgo/',
                     'external/portal2_steam2_decompiled/', 'tools/archlint/tests/fixtures/')

PLATFORM_MACRO = (r'_WIN32|WIN32|_WIN64|WIN64|POSIX|_POSIX|LINUX|_LINUX|OSX|_OSX|__linux__|__APPLE__|'
                  r'__ANDROID__|ANDROID|__FreeBSD__|_PS3|PS3|_X360|X360|_GAMECONSOLE|IS_WINDOWS_PC|'
                  r'PLATFORM_\w+|__EMSCRIPTEN__|_3DS|__3DS__')
# Valve's header idiom `#ifdef _WIN32 / #pragma once / #endif` is not a
# platform branch of behavior.
PRAGMA_ONCE_GUARD = re.compile(
    r'^[ \t]*#[ \t]*if(?:def)?[ \t]+(?:defined[ \t]*\(?[ \t]*)?_WIN32[ \t]*\)?[ \t]*\n'
    r'[ \t]*#[ \t]*pragma[ \t]+once[ \t]*\n[ \t]*#[ \t]*endif', re.M)
PLATFORM_BRANCH = re.compile(
    r'^[ \t]*#[ \t]*(?:if|ifdef|ifndef|elif)\b[^\n]*\b(?:' + PLATFORM_MACRO + r')\b', re.M)


def listed_files(root):
    """First-party files, tracked and untracked but not ignored."""
    out = subprocess.run(['git', '-C', str(root), 'ls-files', '--cached', '--others', '--exclude-standard'],
                         capture_output=True, text=True, check=True).stdout.splitlines()
    return sorted(p for p in set(out)
                  if Path(p).suffix.lower() in LINE_SUFFIXES and not p.startswith(EXCLUDED_PREFIXES)
                  and not any(part.startswith('build') for part in Path(p).parts[:-1])
                  and (Path(root) / p).is_file())


# Comments and string/char literals, blanked with their newlines kept: the
# same reading as archlint's strip_comments_and_literals, by regex, because
# these rules scan the whole tree on every `check --all`.
_NOISE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)


def fast_strip(text):
    return _NOISE.sub(lambda m: ' ' + '\n' * m.group(0).count('\n'), text)


def read_structure(root):
    return json.loads((Path(root) / STRUCTURE).read_text(encoding='utf-8'))


def code_lines(stripped):
    return sum(1 for line in stripped.splitlines() if line.strip())


def area_of(relative, module):
    if module:
        return module
    parts = relative.split('/')
    if len(parts) >= 3 and parts[0] in DEEP_LEGACY:
        return f'{parts[0]}/{parts[1]}/'
    return f'{parts[0]}/' if len(parts) > 1 else '(root)/'


def in_scope(relative, scope, module):
    if any(relative.startswith(p) for p in scope.get('exceptPaths', [])):
        return False
    if any(relative.startswith(p) for p in scope.get('paths', [])):
        return True
    entries = scope.get('modules', [])
    if not entries or module is None:
        return False
    return (any(matches(e, module) for e in entries)
            and not any(matches(e, module) for e in scope.get('exceptModules', [])))


def rule_matcher(rule):
    if rule['kind'] == 'platform-branches':
        return lambda stripped: len(PLATFORM_BRANCH.findall(PRAGMA_ONCE_GUARD.sub('', stripped)))
    words = re.compile(r'(?<![A-Za-z0-9_])(?:' + '|'.join(map(re.escape, rule['identifiers'])) +
                       r')(?![A-Za-z0-9_])')
    return lambda stripped: len(words.findall(stripped))


def measure(root, document, block, strip, files=None):
    """(rule id -> {file: count}, area -> code lines) for the current tree."""
    root = Path(root)
    files = listed_files(root) if files is None else files
    rules = document.get('rules', [])
    matchers = [(rule, rule_matcher(rule)) for rule in rules if rule.get('kind') in KINDS]
    counts = {rule['id']: {} for rule in rules}
    lines = {}
    for relative in files:
        stripped = fast_strip((root / relative).read_text(encoding='utf-8', errors='replace'))
        module = owner(relative, block) if block else None
        area = area_of(relative, module)
        lines[area] = lines.get(area, 0) + code_lines(stripped)
        if Path(relative).suffix.lower() not in {'.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.inl', '.mm'}:
            continue
        for rule, matcher in matchers:
            if in_scope(relative, rule['scope'], module):
                n = matcher(stripped)
                if n:
                    counts[rule['id']][relative] = n
    return counts, lines


def shape_errors(document):
    errors = []
    if document.get('schema') != SCHEMA:
        errors.append(f'CAP012 {STRUCTURE}: schema must be {SCHEMA}')
    seen = set()
    for rule in document.get('rules', []):
        rid = rule.get('id', '?')
        if rid in seen:
            errors.append(f'CAP012 rule {rid}: declared twice')
        seen.add(rid)
        for key in sorted(set(rule) - RULE_KEYS):
            errors.append(f'CAP012 rule {rid}: unknown key {key}')
        for key in ('id', 'kind', 'scope', 'reason', 'owner', 'target', 'counts'):
            if key not in rule:
                errors.append(f'CAP012 rule {rid}: missing {key}')
        if rule.get('kind') not in KINDS:
            errors.append(f'CAP012 rule {rid}: kind must be one of {", ".join(KINDS)}')
        if rule.get('kind') == 'identifiers' and not rule.get('identifiers'):
            errors.append(f'CAP012 rule {rid}: an identifiers rule needs identifiers')
        for key in sorted(set(rule.get('scope', {})) - SCOPE_KEYS):
            errors.append(f'CAP012 rule {rid}: unknown scope key {key}')
    for area, entry in document.get('lines', {}).get('ceilings', {}).items():
        # A ceiling is a dict; one raised after the baseline carries its history.
        if not isinstance(entry, dict) or not isinstance(entry.get('ceiling'), int):
            errors.append(f'CAP013 area {area}: needs an integer ceiling')
    return errors


def compare(document, counts, lines):
    errors = []
    for rule in document.get('rules', []):
        recorded, current = rule.get('counts', {}), counts.get(rule['id'], {})
        for path in sorted(set(recorded) | set(current)):
            was, now = recorded.get(path, 0), current.get(path, 0)
            if now > was:
                where = 'new file' if was == 0 else f'was {was}'
                errors.append(f'CAP012 {rule["id"]} {path}: {now} ({where}); {rule["reason"]}')
            elif now < was:
                errors.append(f'CAP012 {rule["id"]} {path}: fell to {now} from {was}; record it with '
                              f'`archlint structure --write`')
    ceilings = document.get('lines', {}).get('ceilings', {})
    for area in sorted(set(lines) | set(ceilings)):
        now = lines.get(area, 0)
        entry = ceilings.get(area)
        if entry is None:
            errors.append(f'CAP013 {area}: new area with {now} code lines has no ceiling; record it with '
                          f'`archlint structure --raise {area} --reason ...`')
        elif now > entry['ceiling']:
            errors.append(f'CAP013 {area}: {now} code lines is above its ceiling {entry["ceiling"]} '
                          f'(+{now - entry["ceiling"]}); shrink it, or raise the ceiling with a reviewed '
                          f'`archlint structure --raise {area} --reason ...`')
    return errors


def lowered(document, counts, lines):
    """The document with counts and ceilings lowered, never raised: a new
    file or a higher count stays an error. Areas that no longer exist are
    dropped; new areas are left unrecorded."""
    out = json.loads(json.dumps(document))
    for rule in out.get('rules', []):
        current = counts.get(rule['id'], {})
        rule['counts'] = {p: min(n, rule['counts'][p]) for p, n in sorted(current.items())
                          if p in rule['counts']}
    ceilings = out.setdefault('lines', {}).setdefault('ceilings', {})
    for area in list(ceilings):
        if area not in lines:
            del ceilings[area]
        elif lines[area] < ceilings[area]['ceiling']:
            ceilings[area]['ceiling'] = lines[area]
    out['lines']['ceilings'] = dict(sorted(ceilings.items()))
    return out


def write(root, document):
    text = json.dumps(document, indent=2, sort_keys=False) + '\n'
    (Path(root) / STRUCTURE).write_text(text, encoding='utf-8')


def command(root, manifest, args, strip):
    block = manifest.get('capabilityModules')
    document = read_structure(root)
    errors = shape_errors(document)
    if errors:
        for error in errors:
            print(error)
        return 1
    counts, lines = measure(root, document, block, strip)
    if args.report:
        for rule in document['rules']:
            total = sum(counts[rule['id']].values())
            print(f'{rule["id"]}: {total} in {len(counts[rule["id"]])} files (target: {rule["target"]})')
        ceilings = document['lines']['ceilings']
        for area, n in sorted(lines.items(), key=lambda kv: -kv[1])[:args.top]:
            c = ceilings.get(area, {}).get('ceiling')
            print(f'{n:8d}  {area}' + (f'  (ceiling {c})' if c is not None else '  (no ceiling)'))
        print(f'{sum(lines.values()):8d}  total code lines in {len(lines)} areas')
        return 0
    if args.adopt:
        rule = next((r for r in document['rules'] if r['id'] == args.adopt), None)
        if rule is None:
            raise SystemExit(f'{args.adopt}: no such rule')
        if rule['counts']:
            raise SystemExit(f'{args.adopt}: already recorded; counts only fall (--write)')
        rule['counts'] = dict(sorted(counts[rule['id']].items()))
        write(root, document)
        print(f'archlint: structure: adopted {args.adopt} at {sum(rule["counts"].values())} in '
              f'{len(rule["counts"])} files')
        return 0
    if args.raise_area:
        if not args.reason:
            raise SystemExit('--raise needs --reason: growth is a recorded decision')
        if args.raise_area not in lines:
            raise SystemExit(f'{args.raise_area}: no such area in the tree')
        ceilings = document.setdefault('lines', {}).setdefault('ceilings', {})
        old = ceilings.get(args.raise_area, {}).get('ceiling')
        history = ceilings.get(args.raise_area, {}).get('raised', [])
        history.append({'date': datetime.date.today().isoformat(), 'from': old,
                        'to': lines[args.raise_area], 'reason': args.reason})
        ceilings[args.raise_area] = {'ceiling': lines[args.raise_area], 'raised': history}
        document['lines']['ceilings'] = dict(sorted(ceilings.items()))
        write(root, document)
        print(f'archlint: structure: {args.raise_area} ceiling {old} -> {lines[args.raise_area]}')
        return 0
    if args.write:
        updated = lowered(document, counts, lines)
        write(root, updated)
        remaining = compare(updated, counts, lines)
        for error in remaining:
            print(error)
        print(f'archlint: structure recorded; {len(remaining)} error(s) need a reviewed change')
        return 1 if remaining else 0
    errors = compare(document, counts, lines)
    for error in errors:
        print(error)
    print(f'archlint: structure: {len(document["rules"])} ratchets, {len(lines)} areas, '
          f'{len(errors)} error(s)')
    return 1 if errors else 0


def check_errors(root, manifest, strip):
    """For `archlint check --all`."""
    path = Path(root) / STRUCTURE
    if not path.is_file():
        return [f'CAP012 {STRUCTURE}: missing']
    document = read_structure(root)
    errors = shape_errors(document)
    if errors:
        return errors
    counts, lines = measure(root, document, manifest.get('capabilityModules'), strip)
    return compare(document, counts, lines)
