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


# --- Compiler-grounded transitive include check (RFC 0001 "full mode") -------

NATIVE_KINDS = {'backend', 'native-test', 'legacy-interop'}
# Vendored and pinned third-party trees inside the repository count as external.
VENDORED_PREFIXES = ('thirdparty/', 'dependencies/', 'external/', 'box3d/', 'ivp/')
# -MMD omits headers found in system directories, where the SDL, Vulkan and
# windowing SDKs live. Every first-party file a unit really reached is known
# from its .d file, so their own include lines expose such a native SDK.
NATIVE_FAMILY = re.compile(
    r'^(SDL[0-9]?[/_.]|SDL$|vulkan/|X11/|xcb/|wayland-|EGL/|GL/|GLES[0-9]*/|windows\.h$|'
    r'd3d|dxgi|Metal/|QuartzCore/|UIKit/|AppKit/|android/|jni\.h$|gtk/|gdk/)', re.I)


def native_family_includes(path, cache, strip=None):
    if path not in cache:
        try:
            text = path.read_text(encoding='utf-8', errors='replace')
        except OSError:
            text = ''
        lines = strip(text).splitlines() if strip else None
        names = set()
        for match in INCLUDE.finditer(text):
            if not NATIVE_FAMILY.match(match.group(2)):
                continue
            # As in the direct check, an include inside a comment is not one.
            if lines is not None and not re.search(r'#\s*include\b', lines[text.count('\n', 0, match.start())]):
                continue
            names.add(match.group(2))
        cache[path] = sorted(names)
    return cache[path]


def parse_depfile(text):
    """(object, [prerequisites]) from a gcc/clang -MMD dependency file."""
    joined = text.replace('\\\n', ' ')
    target, _, rest = joined.partition(': ')
    tokens, current, escaped = [], '', False
    for ch in rest:
        if escaped:
            current += ch
            escaped = False
        elif ch == '\\':
            escaped = True
        elif ch.isspace():
            if current:
                tokens.append(current)
            current = ''
        else:
            current += ch
    if current:
        tokens.append(current)
    return target.strip(), tokens


def allowed_closure(modules):
    closure = {}

    def reach(mid, seen):
        for dep in modules[mid]['allowedEdges']:
            if dep in modules and dep not in seen:
                seen.add(dep)
                reach(dep, seen)
        return seen

    for mid in modules:
        closure[mid] = reach(mid, {mid})
    return closure


def compile_dep_errors(root, block, depfiles, strip=None):
    """CAP005: check each strict translation unit's resolved include closure.

    A unit owned by module M may reach headers of M and of every module in M's
    transitive allowlist. A portable (non-native) M may not reach external,
    vendored, or unowned first-party headers at all. `depfiles` are
    (tree_directory, path_to_d_file) pairs. Returns (errors, units_checked).
    """
    root = Path(root).resolve()
    modules = {m['id']: m for m in block['modules']}
    closure = allowed_closure(modules)
    errors, checked = set(), 0
    scanned = {}
    for tree, depfile in depfiles:
        _, prerequisites = parse_depfile(Path(depfile).read_text(encoding='utf-8', errors='replace'))
        if not prerequisites:
            continue
        resolved = [(Path(tree) / p).resolve() for p in prerequisites]
        try:
            source = resolved[0].relative_to(root).as_posix()
        except ValueError:
            continue
        mid = owner(source, block)
        if mid is None:
            continue
        checked += 1
        portable = modules[mid].get('kind') not in NATIVE_KINDS
        if portable:
            for reached in resolved:
                if reached.is_file() and str(reached).startswith(str(root)):
                    for name in native_family_includes(reached, scanned, strip):
                        where = reached.relative_to(root).as_posix()
                        errors.add(f'CAP005 {source} ({mid}): portable module reaches native SDK '
                                   f'header <{name}> through {where}')
        for header in resolved[1:]:
            try:
                relative = header.relative_to(root).as_posix()
            except ValueError:
                relative = None
            if relative is None or relative.startswith(VENDORED_PREFIXES) or relative.startswith('build'):
                if portable:
                    errors.add(f'CAP005 {source} ({mid}): portable module reaches external header {header}')
                continue
            dependency = owner(relative, block)
            if dependency is None:
                if portable:
                    errors.add(f'CAP005 {source} ({mid}): portable module reaches unowned header {relative}')
                continue
            if dependency not in closure[mid]:
                errors.add(f'CAP005 {source} ({mid}): transitive include {relative} ({dependency}) '
                           f'is outside the allowed closure')
    return sorted(errors), checked


# --- Recorded Waf link graph (RFC 0001 "Waf target checks", bounded) ---------

NATIVE_USELIB = re.compile(r'^(SDL[0-9]*|VULKAN|X11|XCB|WAYLAND.*|EGL|GL|GLES.*|GTK.*|GDK.*|ADW.*|EPOXY|D3D.*|DXVK.*|METAL.*|MOLTENVK)$')


def link_graph_errors(block, record):
    """CAP006 over a tree's toolchain-invocations.json (target, sources, use).

    A target whose strict sources are all portable may not use a native SDK
    library, nor a first-party target whose strict code lies outside its
    modules' allowed closure. Targets that mix portable and native modules, or
    carry no strict source, are counted but not judged until targets declare
    their architectural owner. Returns (errors, judged, skipped).
    """
    modules = {m['id']: m for m in block['modules']}
    closure = allowed_closure(modules)
    owned, uses = {}, {}
    for entry in record.get('entries', []):
        target = entry['target']
        uses.setdefault(target, set()).update(entry.get('use', []))
        mid = owner(entry['source'], block)
        if mid is not None:
            owned.setdefault(target, set()).add(mid)
    errors, judged, skipped = set(), 0, 0
    for target, mids in sorted(owned.items()):
        if any(modules[m].get('kind') in NATIVE_KINDS for m in mids):
            skipped += 1
            continue
        judged += 1
        allowed = set().union(*(closure[m] for m in mids))
        for dep in sorted(uses.get(target, ())):
            if NATIVE_USELIB.match(dep):
                errors.add(f'CAP006 {target} ({", ".join(sorted(mids))}): portable target uses native library {dep}')
            for dep_mid in sorted(owned.get(dep, ())):
                if dep_mid not in allowed:
                    errors.add(f'CAP006 {target} ({", ".join(sorted(mids))}): links {dep} whose {dep_mid} '
                               f'is outside the allowed closure')
    return sorted(errors), judged, skipped


# --- Hermetic contract-header compiles (RFC 0001 "Source dependency checks") --

NATIVE_PATH = re.compile(r'/(SDL[0-9]*|vulkan|X11|xcb|wayland[^/]*|EGL|GL|GLES[0-9]*|gtk-[0-9.]+|gdk|'
                         r'libadwaita[^/]*|d3d[^/]*|Metal|MoltenVK|android)/', re.I)


def portable_public_headers(root, block):
    root = Path(root).resolve()
    headers = []
    for module in block['modules']:
        if module.get('kind') in NATIVE_KINDS:
            continue
        for prefix in module['paths']:
            base = root / prefix
            for path in (base.rglob('*.h') if base.is_dir() else [base]):
                if path.suffix != '.h' or not path.is_file():
                    continue
                relative = path.relative_to(root).as_posix()
                if relative.startswith('public/') and owner(relative, block) == module['id']:
                    headers.append((module['id'], relative))
    return sorted(headers)


def hermetic_errors(root, block, compile_deps):
    """CAP007: each portable module's public header compiles alone from the
    `public/` root, and its full resolved closure (system headers included)
    reaches no native SDK and no repository header outside `public/`.

    `compile_deps(include_line) -> (ok, diagnostics, [resolved paths])` runs
    the compiler; it is injected so fixtures need no host SDK.
    """
    root = Path(root).resolve()
    errors, headers = [], portable_public_headers(root, block)
    for mid, relative in headers:
        ok, diagnostics, resolved = compile_deps('#include "%s"\n' % relative[len('public/'):])
        if not ok:
            errors.append(f'CAP007 {relative} ({mid}): does not compile alone: {diagnostics}')
            continue
        for dependency in resolved:
            path = Path(dependency).resolve()
            if NATIVE_PATH.search(path.as_posix()):
                errors.append(f'CAP007 {relative} ({mid}): reaches native SDK header {path}')
            elif path.is_relative_to(root) and not path.relative_to(root).as_posix().startswith('public/'):
                errors.append(f'CAP007 {relative} ({mid}): reaches non-public repository header '
                              f'{path.relative_to(root).as_posix()}')
    return errors, len(headers)


def compiler_deps(root, cxx):
    """A compile_deps callable for hermetic_errors using a real compiler."""
    import subprocess

    def run(include_line):
        result = subprocess.run(
            [cxx, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-I', str(Path(root) / 'public'),
             '-x', 'c++', '-M', '-'], input=include_line, text=True, capture_output=True, cwd=root)
        if result.returncode:
            lines = result.stderr.strip().splitlines()
            return False, lines[0] if lines else 'compiler failed', []
        _, prerequisites = parse_depfile(result.stdout)
        return True, '', [p for p in prerequisites if p != '-']
    return run
