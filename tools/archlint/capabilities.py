"""Strict, baseline-free checks for the bounded RFC 0001 composition modules."""
import os
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
    errors += target_owners(block)[1]
    errors += shared_library_groups(block)[1]
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
            if module.get('externalHeaders') or module.get('legacyIncludes') or module.get('uselib'):
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


# --- Preserved ABI declarations stay free of the C++20 vocabulary ------------

VOCABULARY_PREFIXES = ('foundation/', 'testing/')


def abi_vocabulary_errors(root, abi_paths, strip):
    """CAP010: RFC 0006 vocabulary types (Expected, StrongId, Error,
    ScopedResource, test matchers) never cross a preserved binary ABI. Every
    header listed in `legacyAbi.paths` is a preserved declaration, and none
    may include a vocabulary header. Adapters and tests on that list may
    convert at the boundary, so only headers are checked."""
    root = Path(root).resolve()
    errors = []
    for relative in sorted(abi_paths):
        path = root / relative
        if path.suffix not in {'.h', '.hpp'} or not path.is_file():
            continue
        text = path.read_text(encoding='utf-8', errors='replace')
        lines = strip(text).splitlines()
        for match in INCLUDE.finditer(text):
            if not re.search(r'#\s*include\b', lines[text.count('\n', 0, match.start())]):
                continue
            if match.group(2).startswith(VOCABULARY_PREFIXES):
                errors.append(f'CAP010 {relative}: preserved ABI header includes {match.group(2)}; '
                              f'convert to the ABI types at the boundary')
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


ROADMAP_ROW = re.compile(r'^R[0-9]{2}$')


def row_groups(groups, label, code):
    """Validate row-owned target groups: ({target: group}, errors).

    Each group has an id, the roadmap row that owns it, a reason and a
    sorted, unique, non-empty target list; a target is in one group only.
    """
    members, errors = {}, []
    for group in groups:
        name = group.get('id', '?')
        if not ROADMAP_ROW.match(group.get('owner', '')):
            errors.append(f'{code} {label} group {name}: owner must be a roadmap row')
        if not group.get('reason'):
            errors.append(f'{code} {label} group {name}: needs a reason')
        if not group.get('targets'):
            errors.append(f'{code} {label} group {name}: lists no targets')
        if group.get('targets') != sorted(set(group.get('targets', []))):
            errors.append(f'{code} {label} group {name}: targets must be sorted and unique')
        for target in group.get('targets', []):
            if target in members:
                errors.append(f'{code} target {target} is in more than one {label} group')
            members[target] = group
    return members, errors


def target_owners(block):
    """The RFC 0001 migration sidecar for targets that cannot declare an owner
    module yet. Returns ({target: owning roadmap row}, errors).

    Every recorded Waf target declares `arch_module` in its wscript, or is
    listed in exactly one `targetOwners.legacy` group: targets whose legacy
    sources still decide their links. Each group names the roadmap row that
    retires it and a reason, and the groups only shrink.
    """
    sidecar = block.get('targetOwners')
    if not sidecar:
        return {}, []
    errors = [f'CAP008 targetOwners.{key}: unknown section; owner modules are declared '
              f'with arch_module in the wscript'
              for key in sorted(set(sidecar) - {'description', 'legacy'})]
    members, group_errors = row_groups(sidecar.get('legacy', []), 'legacy target', 'CAP008')
    return {target: group.get('owner') for target, group in members.items()}, errors + group_errors


SHARED_FEATURES = {'cshlib', 'cxxshlib'}


def shared_library_groups(block):
    """The reviewed first-party shared libraries: ({target: group}, errors)."""
    section = block.get('sharedLibraries')
    if not section:
        return {}, []
    return row_groups(section.get('groups', []), 'shared library', 'CAP009')


def shared_library_errors(block, records):
    """CAP009: iOS links every first-party module statically (AGENTS.md), so
    each first-party shared library the declared trees build is reviewed debt
    or a declared desktop-only module. A new one fails; a group entry that no
    declared tree builds is stale. Returns (errors, built count)."""
    groups, _ = shared_library_groups(block)
    built = {entry['target'] for record in records for entry in record.get('entries', [])
             if SHARED_FEATURES & set(entry.get('features', []))}
    errors = [f'CAP009 {target}: new first-party shared library; link it statically (iOS composes '
              f'first-party modules statically) or record it in capabilityModules.sharedLibraries'
              for target in sorted(built - set(groups))]
    errors += [f'CAP009 stale shared library: {target} is built as a shared library by no declared tree'
               for target in sorted(set(groups) - built)]
    return errors, len(built)


INCLUDE_FLAGS = ('-isystem', '-iquote', '-idirafter', '-I')


def include_roots(entry):
    """Normalized absolute include directories of one recorded compile."""
    arguments, roots = entry.get('arguments', []), []
    index = 0
    while index < len(arguments):
        argument = arguments[index]
        for flag in INCLUDE_FLAGS:
            if argument == flag and index + 1 < len(arguments):
                roots.append(arguments[index + 1])
                index += 1
                break
            if argument.startswith(flag) and argument != flag:
                roots.append(argument[len(flag):])
                break
        index += 1
    directory = entry.get('directory', '.')
    return {os.path.normpath(os.path.join(directory, root)) for root in roots}


def foreign_include_root(path, directory, root):
    """Why a portable target may not have this include root, or None.

    Build-tree mirrors (`<tree>/public`) count as their source directory.
    Native SDK directories, vendored trees and anything outside the
    repository are foreign: portable code reaches only first-party headers.
    """
    if NATIVE_PATH.search(path.rstrip('/') + '/'):
        return 'native SDK'
    if root is None:
        return None
    root = os.path.normpath(root)
    if os.path.commonpath([root, path]) != root:
        return 'external'
    relative = os.path.relpath(path, root)
    tree = os.path.relpath(os.path.normpath(directory), root)
    if tree != '.' and (relative == tree or relative.startswith(tree + '/')):
        relative = os.path.relpath(relative, tree)
    if (relative + '/').startswith(VENDORED_PREFIXES):
        return 'vendored'
    return None


def use_cycles(uses):
    """Strongly connected groups (size > 1, or a self edge) of the recorded
    first-party target `use` graph."""
    graph = {target: sorted(dep for dep in deps if dep in uses) for target, deps in uses.items()}
    index, low, stack, on_stack, cycles = {}, {}, [], set(), []
    for start in sorted(graph):
        if start in index:
            continue
        # Iterative Tarjan: each frame is (node, next edge position).
        work = [(start, 0)]
        index[start] = low[start] = len(index)
        stack.append(start)
        on_stack.add(start)
        while work:
            node, position = work[-1]
            if position < len(graph[node]):
                work[-1] = (node, position + 1)
                dep = graph[node][position]
                if dep not in index:
                    index[dep] = low[dep] = len(index)
                    stack.append(dep)
                    on_stack.add(dep)
                    work.append((dep, 0))
                elif dep in on_stack:
                    low[node] = min(low[node], index[dep])
                continue
            work.pop()
            if work:
                low[work[-1][0]] = min(low[work[-1][0]], low[node])
            if low[node] == index[node]:
                group = []
                while True:
                    member = stack.pop()
                    on_stack.discard(member)
                    group.append(member)
                    if member == node:
                        break
                if len(group) > 1 or node in graph[node]:
                    cycles.append(sorted(group))
    return sorted(cycles)


def link_graph_errors(block, record, root=None):
    """CAP006/CAP008 over a tree's toolchain-invocations.json (target, sources, use).

    Every recorded target has one architectural owner (CAP008): the module
    its wscript declares with `arch_module` (recorded in its entries), or a
    `targetOwners.legacy` group, whose targets are counted but not judged. A
    target cannot have both, and a wholly strict target must declare.

    A judged target's owner bounds the whole target: every strict module it
    compiles lies in the owner's closure, every native SDK library it uses is
    granted by the owner's `uselib` (only native modules grant any), and every
    first-party target it uses carries strict code only inside that closure.
    A portable owner's target attaches no native SDK, vendored or external
    include directory. The first-party `use` graph has no cycle. `root` is
    the repository root that include directories are judged against.
    Returns (errors, judged, legacy_owned).
    """
    modules = {m['id']: m for m in block['modules']}
    closure = allowed_closure(modules)
    owners, _ = target_owners(block)
    owned, uses, unowned, roots, declared = {}, {}, set(), {}, {}
    for entry in record.get('entries', []):
        target = entry['target']
        uses.setdefault(target, set()).update(entry.get('use', []))
        for path in include_roots(entry):
            roots.setdefault(target, {})[path] = entry.get('directory', '.')
        if entry.get('arch_module'):
            declared.setdefault(target, set()).add(entry['arch_module'])
        mid = owner(entry['source'], block)
        if mid is None:
            unowned.add(target)
        else:
            owned.setdefault(target, set()).add(mid)
    errors, judged, legacy_owned = set(), 0, 0
    for target in sorted(uses):
        mids = owned.get(target, set())
        strict = bool(mids) and target not in unowned
        if target in owners:
            if target in declared:
                errors.add(f'CAP008 {target}: declares arch_module; remove it from its legacy group')
            elif strict:
                errors.add(f'CAP008 {target}: every source is strict; declare arch_module and remove it '
                           f'from its legacy group')
            legacy_owned += 1
            continue
        if target not in declared:
            errors.add(f'CAP008 {target}: no architectural owner; declare arch_module in its wscript '
                       f'(or, for unmigrated legacy code, list it in a targetOwners.legacy group)')
            continue
        if len(declared[target]) != 1:
            errors.add(f'CAP008 {target}: conflicting arch_module declarations '
                       f'{", ".join(sorted(declared[target]))}')
            continue
        value = next(iter(declared[target]))
        if value not in modules:
            errors.add(f'CAP008 {target}: arch_module {value} is not a capability module')
            continue
        judged += 1
        owner_ids = {value}
        label = f'{target} ({value})'
        allowed = closure[value]
        for mid in sorted(mids - allowed):
            errors.add(f'CAP006 {label}: compiles {mid}, which is outside its owner\'s closure')
        granted = set()
        for mid in owner_ids:
            if modules[mid].get('kind') in NATIVE_KINDS:
                granted.update(modules[mid].get('uselib', []))
        if not any(modules[mid].get('kind') in NATIVE_KINDS for mid in owner_ids):
            for path, directory in sorted(roots.get(target, {}).items()):
                reason = foreign_include_root(path, directory, root)
                if reason:
                    errors.add(f'CAP006 {label}: portable target attaches {reason} include directory {path}')
        for dep in sorted(uses[target]):
            if NATIVE_USELIB.match(dep) and dep not in granted:
                errors.add(f'CAP006 {label}: uses native library {dep}, which none of its modules grants')
            for dep_mid in sorted(owned.get(dep, ())):
                if dep_mid not in allowed:
                    errors.add(f'CAP006 {label}: links {dep} whose {dep_mid} is outside the allowed closure')
    for group in use_cycles(uses):
        errors.add(f'CAP006 target use cycle: {" -> ".join(group)}')
    return sorted(errors), judged, legacy_owned


def stale_target_owners(block, records):
    """CAP008: sidecar entries naming a target that no declared tree records."""
    owners, _ = target_owners(block)
    recorded = {entry['target'] for record in records for entry in record.get('entries', [])}
    return [f'CAP008 stale target owner: {target} is recorded in no declared build tree'
            for target in sorted(set(owners) - recorded)]


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
