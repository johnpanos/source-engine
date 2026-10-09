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
import hashlib
import json
import re
import subprocess
from pathlib import Path

from capabilities import ROADMAP_ROW, matches, owner

STRUCTURE = 'architecture/structure.json'
SCHEMA = 'archlint-structure/v1'
RULE_KEYS = {'id', 'kind', 'scope', 'identifiers', 'patterns', 'headers', 'reason', 'owner', 'target',
             'counts', 'declared'}
SCOPE_KEYS = {'modules', 'exceptModules', 'paths', 'exceptPaths', 'named', 'reason'}
KINDS = ('identifiers', 'patterns', 'includes', 'platform-branches')
CODE_SUFFIXES = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hh', '.hpp', '.inl', '.mm'}
INCLUDE_LINE = re.compile(r'^[ \t]*#[ \t]*include\b[ \t]*[<"]([^>"\n]+)[>"]', re.M)
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
    root = Path(root)
    if (root / '.git').exists():
        out = subprocess.run(['git', '-C', str(root), 'ls-files', '--cached', '--others', '--exclude-standard'],
                             capture_output=True, text=True, check=True).stdout.splitlines()
    else:  # a fixture tree
        out = [path.relative_to(root).as_posix() for path in root.rglob('*') if path.is_file()]
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


def resolve_scope(scope, document):
    """A rule's scope, with `named` replaced by the document's named scope
    (one declaration of where providers or format owners live) merged with
    the rule's own keys."""
    if 'named' not in scope:
        return scope
    base = dict(document.get('scopes', {}).get(scope['named'], {}))
    for key in ('modules', 'exceptModules', 'paths', 'exceptPaths'):
        base[key] = list(base.get(key, [])) + list(scope.get(key, []))
    return base


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


def required_literals(pattern):
    """Literal words a match of `pattern` must contain one of, for a fast
    substring prefilter, or None when no such set is known. Each top-level
    alternative contributes its most selective required word: the longest
    literal outside any group, else the words its groups' alternatives
    require."""
    words = set()
    for alternative in split_top_level(pattern):
        found = alternative_literals(alternative)
        if not found:
            return None
        words |= found
    return words


def alternative_literals(alternative):
    outside, groups, depth, start, i = [], [], 0, 0, 0
    while i < len(alternative):
        c = alternative[i]
        if c == '\\':
            if depth == 0:
                outside.append(alternative[i:i + 2])
            i += 2
            continue
        if c == '[':
            close = alternative.index(']', i + 1)
            if depth == 0:
                outside.append(' ')
            i = close + 1
            continue
        if c == '(':
            if depth == 0:
                start = i
                outside.append(' ')
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                groups.append(alternative[start + 1:i])
        elif depth == 0:
            outside.append(c)
        i += 1
    # An item quantified by ? or * may be absent, so it splits words; escapes
    # (\\w, \\s, \\b) are not literals.
    text = re.sub(r'(?:\\.|.)[?*]', ' ', ''.join(outside))
    text = re.sub(r'\\[A-Za-z]|\{[^}]*\}|[+^$.]', ' ', text)
    candidates = re.findall(r'[A-Za-z0-9_:]{2,}', text)
    best = max(candidates, key=len, default='')
    if len(best) >= 3:
        return {best}
    for group in groups:
        if group.startswith(('?<', '?=', '?!')):
            continue
        body = group[2:] if group.startswith('?:') else group
        found = required_literals(body)
        if found and all(len(w) >= 3 for w in found):
            return found
    return {best} if best else None


def split_top_level(pattern):
    depth, start, parts = 0, 0, []
    i = 0
    while i < len(pattern):
        c = pattern[i]
        if c == '\\':
            i += 2
            continue
        if c == '[':
            i = pattern.index(']', i + 1) + 1
            continue
        depth += c == '('
        depth -= c == ')'
        if c == '|' and depth == 0:
            parts.append(pattern[start:i])
            start = i + 1
        i += 1
    parts.append(pattern[start:])
    return parts


def prefiltered(regex, literals):
    if literals is None:
        return lambda stripped: len(regex.findall(stripped))
    return lambda stripped: len(regex.findall(stripped)) if any(w in stripped for w in literals) else 0


def rule_matcher(rule):
    """A function of (stripped text, raw text) giving the rule's count."""
    if rule['kind'] == 'platform-branches':
        return lambda stripped, raw: len(PLATFORM_BRANCH.findall(PRAGMA_ONCE_GUARD.sub('', stripped)))
    if rule['kind'] == 'patterns':
        # One pass per pattern behind its literal prefilter: Python's engine
        # cannot prefix-scan a combined alternation, which made this the
        # slowest rule by an order of magnitude.
        parts = [prefiltered(re.compile(p), required_literals(p)) for p in rule['patterns']]
        return lambda stripped, raw: sum(part(stripped) for part in parts)
    if rule['kind'] == 'includes':
        headers = re.compile('|'.join(f'(?:{h})' for h in rule['headers']))

        def includes(stripped, raw):
            # The stripped text keeps the directive but blanks its quoted path,
            # so the line decides whether it is code and the raw line gives the path.
            code = stripped.splitlines()
            count = 0
            for match in INCLUDE_LINE.finditer(raw):
                line = raw.count('\n', 0, match.start())
                if line < len(code) and re.match(r'[ \t]*#[ \t]*include\b', code[line]) \
                        and headers.fullmatch(match.group(1).strip()):
                    count += 1
            return count
        return includes
    words = re.compile(r'(?<![A-Za-z0-9_])(?:' + '|'.join(map(re.escape, rule['identifiers'])) +
                       r')(?![A-Za-z0-9_])')
    count = prefiltered(words, set(rule['identifiers']))
    return lambda stripped, raw: count(stripped)


def declared_prefix(rule, relative):
    return next((prefix for prefix in rule.get('declared', {}) if relative.startswith(prefix)), None)


CACHE = 'out/archlint/structure-cache.json'


def cache_key(document, block):
    """Everything a file's result depends on besides the file: the rules and
    scopes, the module paths that decide owners, and this checker's code."""
    digest = hashlib.sha256(Path(__file__).read_bytes())
    digest.update(json.dumps([document.get('rules', []), document.get('scopes', {})], sort_keys=True,
                             default=str).encode())
    digest.update(json.dumps([[m['id'], m['paths']] for m in (block or {}).get('modules', [])]).encode())
    return digest.hexdigest()


def measure(root, document, block, strip, files=None, cache=True):
    """(rule id -> {file: count}, area -> code lines) for the current tree.
    Sites under a rule's `declared` prefixes are counted per prefix under the
    key `declared:<prefix>` instead, so a stale declaration can be found.

    Per-file results are cached in out/ (ignored), keyed by the file's
    modification time and size and by cache_key(); a fixture tree without
    a git checkout is never cached."""
    root = Path(root)
    files = listed_files(root) if files is None else files
    rules = document.get('rules', [])
    matchers = [(rule, resolve_scope(rule['scope'], document), rule_matcher(rule))
                for rule in rules if rule.get('kind') in KINDS]
    key = cache_key(document, block)
    cache_path = root / CACHE
    use_cache = cache and (root / '.git').exists()
    stored = {}
    if use_cache:
        try:
            loaded = json.loads(cache_path.read_text(encoding='utf-8'))
            stored = loaded['files'] if loaded.get('key') == key else {}
        except (OSError, ValueError, KeyError):
            stored = {}
    fresh = {}
    counts = {rule['id']: {} for rule in rules}
    lines = {}
    for relative in files:
        path = root / relative
        stat = path.stat()
        entry = stored.get(relative)
        if entry is None or entry[0] != stat.st_mtime_ns or entry[1] != stat.st_size:
            raw = path.read_text(encoding='utf-8', errors='replace')
            stripped = fast_strip(raw)
            module = owner(relative, block) if block else None
            hits = {}
            if Path(relative).suffix.lower() in CODE_SUFFIXES:
                for rule, scope, matcher in matchers:
                    if in_scope(relative, scope, module):
                        n = matcher(stripped, raw)
                        if n:
                            hits[rule['id']] = n
            entry = [stat.st_mtime_ns, stat.st_size, area_of(relative, module), code_lines(stripped), hits]
        fresh[relative] = entry
        area, count, hits = entry[2], entry[3], entry[4]
        lines[area] = lines.get(area, 0) + count
        for rule in rules:
            n = hits.get(rule['id'])
            if n:
                prefix = declared_prefix(rule, relative)
                where = f'declared:{prefix}' if prefix is not None else relative
                counts[rule['id']][where] = counts[rule['id']].get(where, 0) + n
    if use_cache and fresh != stored:
        try:
            cache_path.parent.mkdir(parents=True, exist_ok=True)
            temporary = cache_path.with_suffix('.tmp')
            temporary.write_text(json.dumps({'key': key, 'files': fresh}), encoding='utf-8')
            temporary.replace(cache_path)
        except OSError:
            pass  # a read-only tree still checks; it only loses the cache
    return counts, lines


def counted(counts):
    return {path: n for path, n in counts.items() if not path.startswith('declared:')}


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
        for kind, field in (('identifiers', 'identifiers'), ('patterns', 'patterns'), ('includes', 'headers')):
            if rule.get('kind') == kind and not rule.get(field):
                errors.append(f'CAP012 rule {rid}: a {kind} rule needs {field}')
        for key in sorted(set(rule.get('scope', {})) - SCOPE_KEYS):
            errors.append(f'CAP012 rule {rid}: unknown scope key {key}')
        named = rule.get('scope', {}).get('named')
        if named is not None and named not in document.get('scopes', {}):
            errors.append(f'CAP012 rule {rid}: no named scope {named}')
        for prefix, entry in rule.get('declared', {}).items():
            # A declared consumer: code that needs what the rule forbids, and why.
            if not isinstance(entry, dict) or not entry.get('reason') \
                    or not ROADMAP_ROW.match(str(entry.get('owner', ''))):
                errors.append(f'CAP012 rule {rid}: declared {prefix} needs a reason and an owning row')
    for name, scope in document.get('scopes', {}).items():
        for key in sorted(set(scope) - SCOPE_KEYS):
            errors.append(f'CAP012 scope {name}: unknown key {key}')
        if not scope.get('reason'):
            errors.append(f'CAP012 scope {name}: needs a reason')
    for area, entry in document.get('lines', {}).get('ceilings', {}).items():
        # A ceiling is a dict; one raised after the baseline carries its history.
        if not isinstance(entry, dict) or not isinstance(entry.get('ceiling'), int):
            errors.append(f'CAP013 area {area}: needs an integer ceiling')
    return errors


def compare(document, counts, lines):
    errors = []
    for rule in document.get('rules', []):
        recorded, current = rule.get('counts', {}), counted(counts.get(rule['id'], {}))
        for prefix in sorted(rule.get('declared', {})):
            if not counts.get(rule['id'], {}).get(f'declared:{prefix}'):
                errors.append(f'CAP012 {rule["id"]} declared {prefix}: no site left; remove the declaration')
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
        current = counted(counts.get(rule['id'], {}))
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
            current = counted(counts[rule['id']])
            declared = sum(counts[rule['id']].values()) - sum(current.values())
            print(f'{rule["id"]}: {sum(current.values())} in {len(current)} files'
                  + (f', {declared} declared' if declared else '') + f' (target: {rule["target"]})')
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
        rule['counts'] = dict(sorted(counted(counts[rule['id']]).items()))
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
