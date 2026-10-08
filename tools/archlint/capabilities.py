"""Strict, baseline-free checks for the bounded RFC 0001 composition modules,
including the RFC 0016 layer contracts (CAP011, layer_contract_errors)."""
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


def generated_owner(name, block):
    """The module that owns a build-generated include (block['generatedHeaders']:
    each entry names a path prefix and its owner), or None."""
    for entry in block.get('generatedHeaders', []):
        if name.startswith(entry['prefix']):
            return entry['owner']
    return None


def generated_header_errors(block):
    modules = {m['id'] for m in block['modules']}
    errors = []
    for entry in block.get('generatedHeaders', []):
        missing = {'prefix', 'owner', 'producer'} - set(entry)
        if missing:
            errors.append(f'CAP004 generatedHeaders entry {entry.get("prefix")} lacks '
                          f'{", ".join(sorted(missing))}')
        elif entry['owner'] not in modules:
            errors.append(f'CAP004 generatedHeaders {entry["prefix"]}: unknown owner {entry["owner"]}')
    return errors


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
    errors += generated_header_errors(block)
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
            # Beside the file, under public/, or (a qualified name) from the
            # repository root, as the build's include roots find them.
            # Normalized without following links: a submodule linked into a
            # worktree must still resolve inside the root.
            roots = (path.parent / name, root / 'public' / name) + \
                ((root / name,) if '/' in name else ())
            candidate = next((Path(os.path.normpath(p)) for p in roots if p.is_file()), None)
            generated = generated_owner(name, block)
            if candidate is None and generated is not None:
                # A header the build writes (block['generatedHeaders']): owned
                # by its declared module, so the ordinary edge rule applies.
                if generated != mid and generated not in modules[mid]['allowedEdges']:
                    errors.append(f'CAP002 {relative}: forbidden include {name}; '
                                  f'generated headers belong to {generated}')
                continue
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
    if block.get('layerContracts') is not None:
        errors += layer_contract_errors(root, block, strip)
    return errors


# --- Layer contracts (RFC 0016 "Layer contract and import rules") -----------

LAYER_CONTRACT_KEYS = {'id', 'rfc', 'description', 'prefix', 'layers', 'externalBases', 'independent',
                       'adapters', 'adapterConsumers', 'outside', 'backendIdentity', 'planned',
                       'edgeCeilings', 'forbiddenLiterals'}
FORBIDDEN_LITERAL_KEYS = {'modules', 'literals', 'reason'}
LAYER_CONTRACT_REQUIRED = ('id', 'prefix', 'layers')
OUTSIDE_KEYS = {'modules', 'owner', 'reason'}
BACKEND_IDENTITY_KEYS = {'identifier', 'allowedModules'}


def matches(entry, mid):
    """A contract entry is an exact module id, or a `prefix.*` pattern that
    matches every module below that prefix."""
    return mid.startswith(entry[:-1]) if entry.endswith('.*') else mid == entry


def matching(entries, mid):
    return any(matches(entry, mid) for entry in entries)


def is_string_list(value):
    return isinstance(value, list) and all(isinstance(item, str) for item in value)


def layer_contract_shape_errors(contract):
    """Structural errors of one layerContracts entry (CAP011)."""
    name = contract.get('id', '?') if isinstance(contract, dict) else '?'
    label = f'CAP011 layerContracts {name}'
    if not isinstance(contract, dict):
        return [f'{label}: each layer contract is an object']
    errors = [f'{label}: missing key {key}' for key in LAYER_CONTRACT_REQUIRED if key not in contract]
    errors += [f'{label}: unknown key {key}' for key in sorted(set(contract) - LAYER_CONTRACT_KEYS)]
    layers = contract.get('layers', [])
    if not isinstance(layers, list) or not all(is_string_list(layer) and layer for layer in layers):
        errors.append(f'{label}: layers must be a list of non-empty module lists')
    for key in ('externalBases', 'adapterConsumers', 'planned'):
        if not is_string_list(contract.get(key, [])):
            errors.append(f'{label}: {key} must be a list of module ids')
    independent = contract.get('independent', [])
    if not isinstance(independent, list) or not all(is_string_list(group) for group in independent):
        errors.append(f'{label}: independent must be a list of module lists')
    adapters = contract.get('adapters', {})
    if not isinstance(adapters, dict) or not all(is_string_list(v) for v in adapters.values()):
        errors.append(f'{label}: adapters maps each port to a list of adapter modules')
    elif is_string_list_of_lists(layers):
        layered = [entry for layer in layers for entry in layer]
        errors += [f'{label}: adapter port {port} is not in a layer'
                   for port in sorted(adapters) if port not in layered]
    outside = contract.get('outside', [])
    if not isinstance(outside, list) or not all(isinstance(group, dict) for group in outside):
        errors.append(f'{label}: outside must be a list of groups')
    else:
        for index, group in enumerate(outside):
            where = f'{label} outside group {index + 1}'
            errors += [f'{where}: unknown key {key}' for key in sorted(set(group) - OUTSIDE_KEYS)]
            if not ROADMAP_ROW.match(str(group.get('owner', ''))):
                errors.append(f'{where}: owner must be a roadmap row')
            if not group.get('reason'):
                errors.append(f'{where}: needs a reason')
            modules = group.get('modules')
            if not is_string_list(modules) or not modules:
                errors.append(f'{where}: lists no modules')
            elif len(set(modules)) != len(modules):
                errors.append(f'{where}: modules must be unique')
    identity = contract.get('backendIdentity')
    if identity is not None:
        if not isinstance(identity, dict) or not isinstance(identity.get('identifier'), str) \
                or not re.fullmatch(r'\w+', identity.get('identifier', '')) \
                or not is_string_list(identity.get('allowedModules', [])):
            errors.append(f'{label}: backendIdentity needs an identifier and a list of allowedModules')
        else:
            errors += [f'{label}: backendIdentity unknown key {key}'
                       for key in sorted(set(identity) - BACKEND_IDENTITY_KEYS)]
    ceilings = contract.get('edgeCeilings', {})
    if not isinstance(ceilings, dict) or not all(is_string_list(v) for v in ceilings.values()):
        errors.append(f'{label}: edgeCeilings maps each module to the list of edges it may have')
    literals = contract.get('forbiddenLiterals')
    if literals is not None:
        if not isinstance(literals, dict) or not is_string_list(literals.get('modules')) \
                or not literals.get('modules') or not is_string_list(literals.get('literals')) \
                or not literals.get('literals') or not literals.get('reason'):
            errors.append(f'{label}: forbiddenLiterals needs modules, literals and a reason')
        else:
            errors += [f'{label}: forbiddenLiterals unknown key {key}'
                       for key in sorted(set(literals) - FORBIDDEN_LITERAL_KEYS)]
    return errors


def is_string_list_of_lists(value):
    return isinstance(value, list) and all(is_string_list(item) for item in value)


def classify_modules(contract, modules):
    """Place each capability module in the contract.

    Returns ({module: layer index}, {adapter: port}, outside set, errors).
    An `outside` exemption wins over a pattern match but not over an exact
    layer or adapter entry; any other second placement is an error (rule 4).
    """
    prefix, layers = contract['prefix'], contract['layers']
    adapters = contract.get('adapters', {})
    outside_groups = [group.get('modules', []) for group in contract.get('outside', [])]
    layer_of, port_of, outside, errors = {}, {}, set(), []
    for mid in sorted(modules):
        places, exact = [], False
        for index, layer in enumerate(layers):
            if matching(layer, mid):
                places.append(('layer', index))
                exact |= mid in layer
        for port, entries in sorted(adapters.items()):
            if matching(entries, mid):
                places.append(('adapter', port))
                exact |= mid in entries
        exempt = [index for index, group in enumerate(outside_groups) if matching(group, mid)]
        if len(exempt) > 1:
            errors.append(f'CAP011 rule 4 {mid}: listed in more than one outside group')
        if exempt:
            if exact:
                errors.append(f'CAP011 rule 4 {mid}: both in the layer contract and outside it')
            outside.add(mid)
            continue
        if len(places) > 1:
            where = ', '.join(f'layer {v}' if k == 'layer' else f'adapter of {v}' for k, v in places)
            errors.append(f'CAP011 rule 4 {mid}: declared in more than one place ({where})')
        if places:
            kind, value = places[0]
            if kind == 'layer':
                layer_of[mid] = value
            else:
                port_of[mid] = value
        elif mid.startswith(prefix):
            errors.append(f'CAP011 rule 4 {mid}: {prefix}* module has no layer, adapter column or '
                          f'outside group; place it in {contract["id"]} layerContracts')
    return layer_of, port_of, outside, errors


def declared_entry_errors(contract, modules):
    """Rule 4: exact contract entries name real modules, or planned ones."""
    planned = set(contract.get('planned', []))
    entries = [entry for layer in contract['layers'] for entry in layer]
    entries += [entry for group in contract.get('independent', []) for entry in group]
    entries += [entry for group in contract.get('adapters', {}).values() for entry in group]
    entries += contract.get('externalBases', []) + contract.get('adapterConsumers', [])
    entries += [entry for group in contract.get('outside', []) for entry in group.get('modules', [])]
    entries += list(contract.get('adapters', {}))
    entries += contract.get('backendIdentity', {}).get('allowedModules', [])
    errors = [f'CAP011 rule 4 {entry}: declared in the layer contract but not a capability module'
              for entry in sorted(set(entries))
              if not entry.endswith('.*') and entry not in modules and entry not in planned]
    errors += [f'CAP011 rule 4 {entry}: planned but already a capability module; remove it from planned'
               for entry in sorted(planned & set(modules))]
    return errors


def edge_errors(contract, modules, layer_of, port_of, outside):
    """Rules 1-3 over the declared allowedEdges."""
    prefix = contract['prefix']
    bases = set(contract.get('externalBases', []))
    consumers = contract.get('adapterConsumers', [])
    errors = []
    for mid in sorted(modules):
        if mid in outside:
            continue
        module = modules[mid]
        for dep in sorted(module['allowedEdges']):
            if dep in port_of and mid not in port_of and not matching(consumers, mid) \
                    and module.get('kind') != 'native-test':
                errors.append(f'CAP011 rule 3 {mid}: edge to adapter {dep}; only '
                              f'{", ".join(consumers) or "adapter consumers"} may name an adapter')
        if mid in layer_of:
            errors += layered_edge_errors(contract, modules, mid, layer_of, port_of, outside, bases)
        elif mid in port_of:
            port = port_of[mid]
            for dep in sorted(module['allowedEdges']):
                if dep == port or dep not in modules or layer_of.get(dep) == 0 \
                        or not dep.startswith(prefix):
                    continue
                what = 'adapter' if dep in port_of else f'layer {layer_of[dep]}' if dep in layer_of \
                    else 'module outside the layers'
                errors.append(f'CAP011 rule 3 {mid}: adapter of {port} depends on {dep} ({what}); '
                              f'an adapter depends only on its port, layer 0 and its native grants')
    for group in contract.get('independent', []):
        for mid in sorted(modules):
            if mid in outside or not matching(group, mid):
                continue
            for dep in sorted(modules[mid]['allowedEdges']):
                if dep != mid and dep in modules and matching(group, dep):
                    errors.append(f'CAP011 rule 2 {mid}: edge to independent sibling {dep}')
    return errors


def layered_edge_errors(contract, modules, mid, layer_of, port_of, outside, bases):
    """Rule 1 for one layered module. Modules borrowed into layer 0 from
    outside the family (foundation, jobs.graph) are only kept from depending
    on the family; family members may use them as bases. An edge between
    members of one independence group is reported once, as rule 2."""
    prefix, level, module = contract['prefix'], layer_of[mid], modules[mid]
    consumer = matching(contract.get('adapterConsumers', []), mid)
    family = mid.startswith(prefix)
    errors = []
    for dep in sorted(module['allowedEdges']):
        if dep in port_of or dep not in modules:
            continue  # rule 3; CAP004 reports unknown modules
        if dep in layer_of:
            below = layer_of[dep] < level or (level == 0 and not dep.startswith(prefix))
            siblings = any(matching(group, mid) and matching(group, dep)
                           for group in contract.get('independent', []))
            if not below and not siblings and (family or dep.startswith(prefix)):
                errors.append(f'CAP011 rule 1 {mid}: edge to {dep} (layer {layer_of[dep]}) from layer '
                              f'{level}; dependencies point down')
        elif consumer or dep in bases:
            continue
        elif dep in outside:
            errors.append(f'CAP011 rule 1 {mid}: edge to {dep}, which is outside the layer contract; '
                          f'layered modules do not depend on pre-core render code')
        elif not family and not dep.startswith(prefix):
            continue
        else:
            errors.append(f'CAP011 rule 1 {mid}: edge to undeclared base {dep}; add it to a layer '
                          f'or externalBases')
    return errors


def backend_identity_patterns(identifier):
    name = rf'\b{re.escape(identifier)}\b'
    return [re.compile(pattern) for pattern in (
        rf'{name}\s*(==|!=|<=>)',
        rf'(==|!=)\s*[\w.\->()]*{name}',
        rf'{name}\s*\.\s*(compare|starts_with|ends_with|find)\s*\(',
        rf'strcmp\s*\([^;]*{name}')]


def backend_identity_errors(root, block, contract, checked, strip):
    """Rule 5: portable code never compares the diagnostic backend identifier."""
    identity = contract.get('backendIdentity')
    if not identity:
        return []
    allowed = set(identity.get('allowedModules', []))
    patterns = backend_identity_patterns(identity['identifier'])
    modules = {m['id']: m for m in block['modules']}
    root = Path(root).resolve()
    errors = []
    for mid in sorted(checked - allowed):
        for prefix in modules[mid]['paths']:
            base = root / prefix
            for path in sorted(base.rglob('*') if base.is_dir() else [base]):
                if not path.is_file() or path.suffix not in SOURCE:
                    continue
                relative = path.relative_to(root).as_posix()
                if owner(relative, block) != mid:
                    continue
                lines = strip(path.read_text(encoding='utf-8', errors='replace')).splitlines()
                for number, line in enumerate(lines, 1):
                    if any(pattern.search(line) for pattern in patterns):
                        errors.append(f'CAP011 rule 5 {relative}:{number} ({mid}): compares the device\'s '
                                      f'diagnostic backend identifier; follow capabilities')
    return errors


def ceiling_errors(contract, modules):
    """Rule 6: a module listed in edgeCeilings declares no edge beyond its
    ceiling (for example a thin application that may use only the API and the
    composition), so widening it is a reviewed change to the contract."""
    errors = []
    for mid, ceiling in sorted(contract.get('edgeCeilings', {}).items()):
        if mid not in modules:
            errors.append(f'CAP011 rule 6 {mid}: has an edge ceiling but is not a capability module')
            continue
        for dep in sorted(set(modules[mid]['allowedEdges']) - set(ceiling)):
            errors.append(f'CAP011 rule 6 {mid}: edge to {dep} is above its ceiling '
                          f'({", ".join(ceiling)})')
    return errors


def string_literals(text):
    """(line, content) of each C/C++ string literal, skipping comments and
    character literals. Raw strings are read as ordinary ones, which is enough
    for a word scan."""
    i, line, n = 0, 1, len(text)
    while i < n:
        c = text[i]
        if c == '\n':
            line += 1
            i += 1
        elif text.startswith('//', i):
            end = text.find('\n', i)
            i = n if end < 0 else end
        elif text.startswith('/*', i):
            end = text.find('*/', i + 2)
            end = n if end < 0 else end + 2
            line += text.count('\n', i, end)
            i = end
        elif c in '"\'':
            start, i = i + 1, i + 1
            while i < n and text[i] != c and text[i] != '\n':
                i += 2 if text[i] == '\\' else 1
            if c == '"':
                yield line, text[start:i]
            i += 1
        else:
            i += 1


def forbidden_literal_errors(root, block, contract):
    """Rule 7: the listed modules' sources carry none of the listed literals
    inside a string (RFC 0027: kiln.core names no platform, SDK or package
    form; it asks the provider the profile names). Case-insensitive, whole
    words."""
    literals = contract.get('forbiddenLiterals')
    if not literals:
        return []
    words = [re.compile(rf'(?<![A-Za-z0-9_]){re.escape(word)}(?![A-Za-z0-9_])', re.IGNORECASE)
             for word in literals['literals']]
    modules = {m['id']: m for m in block['modules']}
    root = Path(root).resolve()
    errors = []
    for mid in literals['modules']:
        if mid not in modules:
            errors.append(f'CAP011 rule 7 {mid}: listed in forbiddenLiterals but not a capability module')
            continue
        for prefix in modules[mid]['paths']:
            base = root / prefix
            for path in sorted(base.rglob('*') if base.is_dir() else [base]):
                if not path.is_file() or path.suffix not in SOURCE:
                    continue
                relative = path.relative_to(root).as_posix()
                if owner(relative, block) != mid:
                    continue
                text = path.read_text(encoding='utf-8', errors='replace')
                for number, content in string_literals(text):
                    for word, pattern in zip(literals['literals'], words):
                        if pattern.search(content):
                            errors.append(f'CAP011 rule 7 {relative}:{number} ({mid}): string literal '
                                          f'names "{word}"; {literals["reason"]}')
    return errors


def layer_contract_errors(root, block, strip):
    """CAP011: the declared layer contracts (RFC 0016) over the module rows.

    1. down only: a layered module's edges target lower layers or declared
       external bases, never an outside (pre-core) module of the family;
    2. modules in one `independent` group have no edges among them;
    3. only `adapterConsumers` (and test fixtures) name an adapter, and an
       adapter depends only on its port, layer 0 and non-family modules;
    4. every module with the family prefix has one place: a layer, an adapter
       column or a row-owned `outside` group; exact entries name modules;
    5. no layered module or adapter outside `backendIdentity.allowedModules`
       compares the backend identifier in its sources;
    6. a module in `edgeCeilings` declares no edge beyond its ceiling;
    7. the `forbiddenLiterals` modules name none of the literals in a string.
    """
    contracts = block.get('layerContracts')
    if not isinstance(contracts, list):
        return ['CAP011 layerContracts must be a list']
    modules = {m['id']: m for m in block['modules']}
    errors = []
    for contract in contracts:
        shape = layer_contract_shape_errors(contract)
        if shape:
            errors += shape
            continue
        layer_of, port_of, outside, placed = classify_modules(contract, modules)
        errors += placed + declared_entry_errors(contract, modules)
        errors += edge_errors(contract, modules, layer_of, port_of, outside)
        errors += backend_identity_errors(root, block, contract, set(layer_of) | set(port_of), strip)
        errors += ceiling_errors(contract, modules)
        errors += forbidden_literal_errors(root, block, contract)
    return sorted(set(errors))


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
            # A header the build generated into a tree's generated/ root (a
            # legacy build*/ tree or kiln's out/<profile>/<flavor>/build) is
            # owned as block['generatedHeaders'] declares it.
            if relative is not None and '/generated/' in relative:
                produced = generated_owner(relative.split('/generated/', 1)[1], block)
                if produced is not None:
                    if produced != mid and produced not in modules[mid]['allowedEdges']:
                        errors.add(f'CAP005 {source} ({mid}): reaches {relative}, a generated '
                                   f'header of {produced}')
                    continue
            # A vendored file a module declares (external.bcdec) follows the
            # edge rule like any other; the rest of a vendored tree is external.
            if relative is None or relative.startswith('build') or (
                    relative.startswith(VENDORED_PREFIXES) and owner(relative, block) is None):
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


def hammer_strict_module(path, hammer_block):
    """RFC 0002: hammer/core/<area>/... and public/hammer/<area>/... belong to
    hammer.<area> (the one owner of that rule; archlint's HAM checks use it)."""
    for root in ( hammer_block or {} ).get('strictIncludeRoots', []):
        if path.startswith(root):
            rest = path[len(root):]
            return f'hammer.{rest.split("/", 1)[0]}' if '/' in rest else None
    return None


def combined_modules(block, hammer_block=None):
    """Capability modules plus the Hammer modules, which own no paths of their
    own here (their files map by directory, hammer_strict_module). A Hammer
    module of a native kind (a desktop host such as the GTK shell) grants its
    `uselib` native libraries to the targets it owns, as a native capability
    module does."""
    modules = {m['id']: m for m in block['modules']}
    for module in ( hammer_block or {} ).get('modules', []):
        entry = {'id': module['id'], 'paths': [], 'allowedEdges': module.get('allowedEdges', [])}
        if module.get('kind') in NATIVE_KINDS:
            entry['kind'] = module['kind']
            entry['uselib'] = list(module.get('uselib', []))
        modules.setdefault(module['id'], entry)
    return modules


def exception_grants(hammer_block, owner_closure, closure):
    """Modules a Hammer owner may also reach through its recorded
    includeExceptions: an exception lets a file of a module in the owner's
    closure depend on another module, so the target may link it."""
    granted = set()
    for entry in ( hammer_block or {} ).get('includeExceptions', []):
        module = hammer_strict_module(entry.get('path', ''), hammer_block)
        dependency = entry.get('dependency')
        if module in owner_closure and dependency in closure:
            granted |= closure[dependency]
    return granted


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


def link_graph_errors(block, record, root=None, hammer_block=None):
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

    With `hammer_block` (architecture/modules.json hammerModules), a target may
    also declare a Hammer module. Strict Hammer sources map to their module by
    directory, the closure spans both graphs, and the module's recorded
    includeExceptions also permit the dependency they name.
    Returns (errors, judged, legacy_owned).
    """
    modules = combined_modules(block, hammer_block)
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
        mid = owner(entry['source'], block) or hammer_strict_module(entry['source'], hammer_block)
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
            errors.add(f'CAP008 {target}: arch_module {value} is not a capability or Hammer module')
            continue
        judged += 1
        owner_ids = {value}
        label = f'{target} ({value})'
        allowed = closure[value] | exception_grants(hammer_block, closure[value], closure)
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
