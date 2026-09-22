"""Strict, baseline-free checks for the bounded RFC 0001 composition modules."""
import re
from pathlib import Path

SOURCE = {'.h', '.cpp', '.cc', '.hpp', '.inl'}
OS_BRANCH = re.compile(r'^\s*#\s*(if|ifdef|ifndef|elif)\b.*\b(_WIN32|WIN32|_WIN64|POSIX|LINUX|OSX|__linux__|__APPLE__|__ANDROID__)\b', re.M)
INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*([<"])([^>"]+)[>"]', re.M)


def owner(path, block):
    # A private backend nested below a portable package owns only its more
    # specific subtree. Equally specific competing declarations remain errors.
    matches = [(len(prefix), module['id']) for module in block['modules']
               for prefix in module['paths']
               if (path.startswith(prefix) if prefix.endswith('/') else path == prefix)]
    if not matches:
        return None
    longest = max(length for length, _ in matches)
    owners = {mid for length, mid in matches if length == longest}
    return next(iter(owners)) if len(owners) == 1 else None


def target_errors(name, use, includes, block):
    target = block['targets'].get(name)
    if target is None:
        return [f'CAP003 unknown strict target {name}']
    errors = [f'CAP003 {name}: forbidden link dependency {dep}'
              for dep in use if dep not in target['allowedUse']]
    errors += [f'CAP003 {name}: non-hermetic include root {inc}'
               for inc in includes if inc != 'public']
    return errors


def check(root, block, strip):
    if not block:
        return []
    root = Path(root).resolve()
    errors = []
    modules = {m['id']: m for m in block['modules']}
    visited, active = set(), set()

    def visit(mid):
        if mid in active:
            errors.append(f'CAP004 capability permission cycle at {mid}')
            return
        if mid in visited:
            return
        visited.add(mid)
        active.add(mid)
        for dep in modules[mid]['allowedEdges']:
            if dep not in modules:
                errors.append(f'CAP004 unknown capability module {dep}')
            else:
                visit(dep)
        active.remove(mid)

    for mid in modules:
        visit(mid)
    paths = set()
    for module in modules.values():
        for prefix in module['paths']:
            path = root / prefix
            paths.update(path.rglob('*') if path.is_dir() else [path])
    for path in sorted(paths):
        if not path.is_file() or path.suffix not in SOURCE:
            continue
        relative = path.relative_to(root).as_posix()
        mid = owner(relative, block)
        if mid is None:
            errors.append(f'CAP001 ambiguous ownership: {relative}')
            continue
        text = path.read_text(encoding='utf-8')
        stripped = strip(text)
        module = modules[mid]
        native = module.get('kind') in {'backend', 'native-test', 'legacy-interop'}
        if not native:
            if module.get('externalHeaders') or module.get('legacyIncludes'):
                errors.append(f'CAP004 {mid}: portable modules cannot grant native/legacy include access')
            for match in OS_BRANCH.finditer(stripped):
                errors.append(f'CAP001 {relative}: platform identity branch in portable module')
        stripped_lines = stripped.splitlines()
        for match in INCLUDE.finditer(text):
            line = text.count('\n', 0, match.start())
            # Ignore commented-out includes; the main scanner blanks literals.
            if not re.search(r'#\s*include\b', stripped_lines[line]):
                continue
            name = match.group(2)
            candidate = next((p.resolve() for p in (path.parent / name, root / 'public' / name)
                              if p.is_file()), None)
            if candidate is None:
                if native and name in module.get('externalHeaders', []):
                    continue
                if match.group(1) == '<' and name in block['standardHeaders']:
                    continue
                errors.append(f'CAP002 {relative}: native, external, or unresolved include {name}')
                continue
            try:
                dependency = owner(candidate.relative_to(root).as_posix(), block)
            except ValueError:
                dependency = None
            if dependency is None and native and name in module.get('legacyIncludes', []):
                continue
            if dependency != mid and dependency not in modules[mid]['allowedEdges']:
                errors.append(f'CAP002 {relative}: forbidden include {name}; inject a narrow capability')
    return errors
