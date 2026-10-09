"""`archlint graph`: draw the capability-module dependency graph from
architecture/modules.json, so the contract the checks enforce can be seen.

A layer contract family (`--family render`, `product`, `kiln`) is drawn as
its layers, top to bottom, with each module colored by its role in the
contract (CAP011): core, translator (the anti-corruption layer), adapter,
fixture, or outside the contract (pre-core modules that retire). Modules
outside the family that it depends on are drawn as plain bases. Without a
family every capability module is drawn, grouped by its id's first segment.

Edges are the declared `allowedEdges`. By default the graph is transitively
reduced (an edge implied by a longer path is left out), so the picture shows
structure rather than every permission; `--all-edges` draws them all. The
recorded CAP011 `pending` violations are always drawn, in red, labeled with
their owning row.

    python3 tools/archlint/archlint.py graph --family render
    python3 tools/archlint/archlint.py graph --focus render.pass.world --depth 1
    python3 tools/archlint/archlint.py graph --format svg -o render.svg

Rendering uses Graphviz's `dot` (PNG or SVG); `--format dot` writes the
source and needs nothing installed.
"""

from __future__ import annotations

import json
import shutil
import subprocess
from collections import deque
from pathlib import Path

from capabilities import classify_modules, group_cycles, is_test_module, matching, translation_roles

OUT_DIR = 'out/archlint'
ROLE_STYLE = {
    'core': ('#dbe9f6', '#2b5d8a'),
    'translator': ('#fde3c8', '#b5651d'),
    'adapter': ('#d9f0d3', '#2e7d32'),
    'composition': ('#e8dcf3', '#5e3c8f'),
    'fixture': ('#eeeeee', '#777777'),
    'outside': ('#f8d7da', '#a33a45'),
    'base': ('#ffffff', '#999999'),
    'module': ('#f4f4f4', '#555555'),
}
LEGEND = (('core', 'core'), ('translator', 'translator (anti-corruption)'), ('adapter', 'adapter'),
          ('composition', 'composition root'), ('fixture', 'fixture / test'),
          ('outside', 'outside the contract'), ('base', 'base outside the family'))


def quote(text):
    return '"' + str(text).replace('\\', '\\\\').replace('"', '\\"') + '"'


def contract_named(block, family):
    for contract in block.get('layerContracts', []):
        if contract.get('id') == family:
            return contract
    raise SystemExit(f'no layer contract {family!r}; known: '
                     f'{", ".join(c.get("id", "?") for c in block.get("layerContracts", []))}')


def roles(contract, modules):
    """(module -> role, module -> layer index) for one layer contract."""
    layer_of, port_of, outside, _ = classify_modules(contract, modules)
    translators, _, fixtures, _ = translation_roles(contract)
    consumers = contract.get('adapterConsumers', [])
    role = {}
    for mid in set(layer_of) | set(port_of) | set(outside):
        if mid in outside:
            role[mid] = 'outside'
        elif matching(fixtures, mid) or is_test(mid, {}) or 'fixture' in mid or '.lab' in mid:
            role[mid] = 'fixture'
        elif matching(translators, mid):
            role[mid] = 'translator'
        elif mid in port_of:
            role[mid] = 'adapter'
        elif matching(consumers, mid):
            role[mid] = 'composition'
        else:
            role[mid] = 'core'
    return role, layer_of


def reduce_edges(nodes, edges):
    """Transitive reduction: drop u->v when v is reachable from u through
    another of u's targets. Cycles are kept as they are."""
    targets = {n: set() for n in nodes}
    for u, v in edges:
        if u in targets and v in targets:
            targets[u].add(v)

    def reaches(start, goal, skip):
        seen, queue = {start}, deque(t for t in targets[start] if (start, t) != skip)
        while queue:
            node = queue.popleft()
            if node == goal:
                return True
            if node in seen:
                continue
            seen.add(node)
            queue.extend(targets.get(node, ()))
        return False

    return [(u, v) for u, v in edges
            if not (u in targets and v in targets and reaches(u, v, (u, v)))]


def neighborhood(focus, edges, depth):
    keep, frontier = {focus}, {focus}
    for _ in range(depth):
        nxt = set()
        for u, v in edges:
            if u in frontier and v not in keep:
                nxt.add(v)
            if v in frontier and u not in keep:
                nxt.add(u)
        keep |= nxt
        frontier = nxt
    return keep


def group_of(mid):
    return mid.split('.')[0]


ROW = 8  # modules per row inside a layer before it wraps


def is_test(mid, role):
    return role.get(mid) == 'fixture' or is_test_module(mid)


SCHEMA = 'archlint-graph/v1'
FORMATS = ('png', 'svg', 'dot', 'json', 'graphml', 'mermaid')
EXTENSIONS = {'mermaid': 'mmd'}


def cycle_records(block):
    """Group cycles as CAP014 sees them before its pending edges excuse them."""
    pending = {(e.get('module'), e.get('edge')): e for e in block.get('groupCycles', {}).get('pending', [])
               if isinstance(e, dict)}
    out = []
    for groups, edges in group_cycles(block):
        out.append({'groups': groups,
                    'edges': [{'from': m, 'to': d, 'pending': (m, d) in pending,
                               'owner': pending.get((m, d), {}).get('owner')} for m, d in edges],
                    # CAP014 passes only when the pending edges break the cycle.
                    'recordedDebt': any((m, d) in pending for m, d in edges)})
    return out


def build_model(block, family=None, focus=None, depth=1, all_edges=False, fixtures=False):
    """The module graph as data: every format is written from this."""
    modules = {m['id']: m for m in block['modules']}
    if focus is not None and focus not in modules:
        raise SystemExit(f'no capability module {focus!r}')
    contract = contract_named(block, family) if family else None
    role, layer_of = roles(contract, modules) if contract else ({}, {})
    members = set(role) if contract else set(modules)
    if not fixtures and focus is None:
        # Fixtures and tests depend on everything; they hide the structure.
        members = {m for m in members if not is_test(m, role)}
    edges = sorted({(mid, dep) for mid in members for dep in modules[mid]['allowedEdges']
                    if dep in modules and dep != mid})
    pending = []
    if contract:
        for entry in translation_roles(contract)[3]:
            if 'edge' in entry:
                pending.append((entry['module'], entry['edge'], entry.get('owner', ''),
                                entry.get('reason', '')))
    nodes = members | {v for _, v in edges if fixtures or focus is not None or not is_test(v, role)}
    edges = [(u, v) for u, v in edges if v in nodes]
    if focus is not None:
        nodes = neighborhood(focus, edges, depth) | {focus}
        edges = [(u, v) for u, v in edges if u in nodes and v in nodes]
        pending = [p for p in pending if p[0] in nodes and p[1] in nodes]
    pending_pairs = {(u, v) for u, v, _, _ in pending}
    plain = [e for e in edges if e not in pending_pairs]
    shown = set(plain if all_edges else reduce_edges(nodes, plain))

    title = f'{family} layer contract' if family else 'capability modules'
    if focus:
        title += f', around {focus} (depth {depth})'
    title += '' if all_edges else ' (transitively reduced)'
    layers = []
    if contract:
        for index, entries in enumerate(contract['layers']):
            members_of = sorted(m for m in nodes if layer_of.get(m) == index and role.get(m) != 'outside')
            layers.append({'index': index, 'declared': entries, 'modules': members_of})
    return {
        'schema': SCHEMA, 'view': 'modules', 'title': title, 'family': family, 'focus': focus,
        'depth': depth if focus else None, 'reduced': not all_edges, 'fixtures': fixtures,
        'nodes': [{'id': mid, 'group': group_of(mid),
                   'role': role.get(mid, 'base' if contract else 'module'),
                   'layer': layer_of.get(mid) if role.get(mid) != 'outside' else None,
                   'focus': mid == focus} for mid in sorted(nodes)],
        'edges': [{'from': u, 'to': v, 'kind': 'dependency', 'shown': (u, v) in shown}
                  for u, v in plain]
                 + [{'from': u, 'to': v, 'kind': 'pending', 'shown': True, 'owner': owner,
                     'reason': reason} for u, v, owner, reason in pending],
        'layers': layers,
        'cycles': cycle_records(block),
    }


def build_group_model(block, fixtures=False):
    """Every capability module collapsed into its group (the id's first
    segment): one node per group with its module count, one edge per group
    dependency weighted by the module edges it stands for."""
    modules = {m['id']: m for m in block['modules']}
    keep = {m for m in modules if fixtures or not is_test(m, {})}
    sizes, weights = {}, {}
    for mid in keep:
        sizes[group_of(mid)] = sizes.get(group_of(mid), 0) + 1
        for dep in modules[mid]['allowedEdges']:
            if dep in keep and group_of(dep) != group_of(mid):
                key = (group_of(mid), group_of(dep))
                weights[key] = weights.get(key, 0) + 1
    cycles = cycle_records(block)
    in_cycle = {(group_of(e['from']), group_of(e['to'])) for c in cycles for e in c['edges']}
    return {
        'schema': SCHEMA, 'view': 'groups', 'fixtures': fixtures,
        'title': 'capability module groups (edge label: module dependencies; orange: a group cycle)',
        'nodes': [{'id': g, 'group': g, 'role': 'group', 'modules': n} for g, n in sorted(sizes.items())],
        'edges': [{'from': u, 'to': v, 'kind': 'dependency', 'shown': True, 'weight': n,
                   'cycle': (u, v) in in_cycle} for (u, v), n in sorted(weights.items())],
        'layers': [], 'cycles': cycles,
    }


def stats(model):
    shown = [e for e in model['edges'] if e['kind'] == 'dependency' and e['shown']]
    return {'nodes': len(model['nodes']), 'edges': len(shown),
            'all': sum(1 for e in model['edges'] if e['kind'] == 'dependency'),
            'pending': sum(1 for e in model['edges'] if e['kind'] == 'pending'),
            'cycles': len(model['cycles'])}


# --- Writers ------------------------------------------------------------------

def write_dot(model):
    lines = ['digraph archlint {',
             '  graph [rankdir=TB, newrank=true, concentrate=true, nodesep=0.25, ranksep=0.7, '
             f'fontname="Helvetica", fontsize=20, labelloc=t, label={quote(model["title"])}];',
             '  node [shape=box, style="rounded,filled", fontname="Helvetica", fontsize=11];',
             '  edge [color="#88888899", arrowsize=0.6];']
    if model['view'] == 'groups':
        for n in model['nodes']:
            label = quote(f'{n["id"]} ({n["modules"]})')
            lines.append(f'  {quote(n["id"])} [label={label}, fillcolor="#dbe9f6", color="#2b5d8a", '
                         'fontsize=13];')
        for e in model['edges']:
            color = '"#e07b00"' if e['cycle'] else '"#88888899"'
            lines.append(f'  {quote(e["from"])} -> {quote(e["to"])} [label="{e["weight"]}", color={color}, '
                         f'fontcolor="#666666", fontsize=10, penwidth={min(1 + e["weight"] / 4, 5):.1f}];')
        lines.append('}')
        return '\n'.join(lines) + '\n'

    def node_line(node):
        fill, line = ROLE_STYLE[node['role']]
        extra = ', style="rounded,dashed"' if node['role'] == 'base' else ''
        width = ', penwidth=2.5' if node['focus'] else ''
        return f'    {quote(node["id"])} [fillcolor="{fill}", color="{line}"{extra}{width}];'

    by_id = {n['id']: n for n in model['nodes']}
    if model['layers']:
        # Each layer is a cluster of rows (rank=same, at most ROW wide), and
        # invisible edges stack the rows and the layers: the highest layer on
        # top, dependencies pointing down.
        anchors = []
        for layer in reversed(model['layers']):
            if not layer['modules']:
                continue
            names = ', '.join(layer['declared'][:3]) + (', ...' if len(layer['declared']) > 3 else '')
            lines.append(f'  subgraph cluster_layer{layer["index"]} {{')
            label = quote(f'layer {layer["index"]}: {names}')
            lines.append(f'    label={label}; style="rounded"; color="#cccccc"; fontsize=12; '
                         'fontcolor="#666666";')
            lines += [node_line(by_id[m]) for m in layer['modules']]
            for i in range(0, len(layer['modules']), ROW):
                row = layer['modules'][i:i + ROW]
                lines.append('    { rank=same; ' + ' '.join(quote(m) for m in row) + ' }')
                anchors.append(row[0])
            lines.append('  }')
        lines += [f'  {quote(a)} -> {quote(b)} [style=invis, weight=10];' for a, b in zip(anchors, anchors[1:])]
        for kind, label, style in (('adapter', 'adapters', 'rounded'),
                                   ('outside', 'outside the contract (retiring)', 'rounded,dashed')):
            cluster = 'adapters' if kind == 'adapter' else 'outside'
            members = [n for n in model['nodes'] if n['role'] == kind]
            if members:
                color = '#a33a45' if kind == 'outside' else '#cccccc'
                lines.append(f'  subgraph cluster_{cluster} {{')
                lines.append(f'    label={quote(label)}; style="{style}"; color="{color}"; fontsize=12; '
                             f'fontcolor="{"#a33a45" if kind == "outside" else "#666666"}";')
                lines += [node_line(n) for n in members]
                lines.append('  }')
        lines += [node_line(n) for n in model['nodes'] if n['role'] == 'base']
    else:
        groups = {}
        for node in model['nodes']:
            groups.setdefault(node['group'], []).append(node)
        for index, (name, members) in enumerate(sorted(groups.items())):
            lines.append(f'  subgraph cluster_group{index} {{')
            lines.append(f'    label={quote(name)}; style="rounded"; color="#cccccc"; fontsize=12;')
            lines += [node_line(n) for n in members]
            lines.append('  }')
    for e in model['edges']:
        if e['kind'] == 'dependency' and e['shown']:
            lines.append(f'  {quote(e["from"])} -> {quote(e["to"])};')
        elif e['kind'] == 'pending':
            lines.append(f'  {quote(e["from"])} -> {quote(e["to"])} [color="#d32f2f", penwidth=2, '
                         f'style=dashed, fontcolor="#d32f2f", fontsize=9, '
                         f'label={quote("pending " + e["owner"])}, constraint=false];')
    if model['family']:
        lines.append('  subgraph cluster_legend {')
        lines.append('    label="legend"; style="rounded"; color="#cccccc"; fontsize=12; rank=sink;')
        for kind, text in LEGEND:
            fill, line = ROLE_STYLE[kind]
            style = 'rounded,dashed,filled' if kind == 'base' else 'rounded,filled'
            lines.append(f'    {quote("legend:" + kind)} [label={quote(text)}, fillcolor="{fill}", '
                         f'color="{line}", style="{style}", fontsize=10];')
        lines.append('    "legend:pending" [label="pending violation (red edge)", shape=plaintext, '
                     'style="", fontcolor="#d32f2f", fontsize=10];')
        lines.append('  }')
    lines.append('}')
    return '\n'.join(lines) + '\n'


def write_json(model):
    return json.dumps(model, indent=2, sort_keys=False) + '\n'


def write_graphml(model):
    """GraphML for yEd, Gephi, networkx and other graph tools: every node and
    edge with its attributes (hidden reduced edges included, `shown` false)."""
    import xml.etree.ElementTree as ET
    ns = 'http://graphml.graphdrawing.org/xmlns'
    root = ET.Element('graphml', xmlns=ns)
    keys = (('node', 'group', 'string'), ('node', 'role', 'string'), ('node', 'layer', 'int'),
            ('node', 'modules', 'int'), ('edge', 'kind', 'string'), ('edge', 'shown', 'boolean'),
            ('edge', 'owner', 'string'), ('edge', 'weight', 'int'), ('edge', 'cycle', 'boolean'))
    for target, name, kind in keys:
        ET.SubElement(root, 'key', {'id': f'{target}_{name}', 'for': target, 'attr.name': name,
                                    'attr.type': kind})
    g = ET.SubElement(root, 'graph', id=model['view'], edgedefault='directed')
    for node in model['nodes']:
        element = ET.SubElement(g, 'node', id=node['id'])
        for name in ('group', 'role', 'layer', 'modules'):
            if node.get(name) is not None:
                ET.SubElement(element, 'data', key=f'node_{name}').text = str(node[name])
    for index, edge in enumerate(model['edges']):
        element = ET.SubElement(g, 'edge', id=f'e{index}', source=edge['from'], target=edge['to'])
        for name in ('kind', 'shown', 'owner', 'weight', 'cycle'):
            if edge.get(name) is not None:
                value = edge[name]
                ET.SubElement(element, 'data', key=f'edge_{name}').text = (
                    str(value).lower() if isinstance(value, bool) else str(value))
    ET.indent(root)
    return '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root, encoding='unicode') + '\n'


def write_mermaid(model):
    """A Mermaid flowchart (renders in Markdown on GitHub and most viewers)."""
    ids = {n['id']: f'n{i}' for i, n in enumerate(model['nodes'])}
    lines = ['---', f'title: {model["title"]}', '---', 'flowchart TB']
    for kind, (fill, line) in ROLE_STYLE.items():
        lines.append(f'  classDef {kind} fill:{fill},stroke:{line}')
    lines.append('  classDef group fill:#dbe9f6,stroke:#2b5d8a')

    def node(n):
        label = f'{n["id"]} ({n["modules"]})' if 'modules' in n else n['id']
        return f'{ids[n["id"]]}["{label}"]:::{n["role"]}'
    by_layer = {}
    for n in model['nodes']:
        by_layer.setdefault(n.get('layer'), []).append(n)
    if model['layers']:
        for layer in reversed(model['layers']):
            members = [n for n in by_layer.get(layer['index'], [])]
            if members:
                lines.append(f'  subgraph layer{layer["index"]}["layer {layer["index"]}"]')
                lines += [f'    {node(n)}' for n in members]
                lines.append('  end')
        lines += [f'  {node(n)}' for n in by_layer.get(None, [])]
    else:
        lines += [f'  {node(n)}' for n in model['nodes']]
    links, styles = [], []
    for e in model['edges']:
        if not e['shown']:
            continue
        if e['kind'] == 'pending':
            links.append(f'  {ids[e["from"]]} -. "pending {e["owner"]}" .-> {ids[e["to"]]}')
            styles.append((len(links) - 1, 'stroke:#d32f2f,stroke-width:2px'))
        elif 'weight' in e:
            links.append(f'  {ids[e["from"]]} -- "{e["weight"]}" --> {ids[e["to"]]}')
            if e.get('cycle'):
                styles.append((len(links) - 1, 'stroke:#e07b00,stroke-width:2px'))
        else:
            links.append(f'  {ids[e["from"]]} --> {ids[e["to"]]}')
    lines += links
    lines += [f'  linkStyle {i} {style}' for i, style in styles]
    return '\n'.join(lines) + '\n'


WRITERS = {'dot': write_dot, 'json': write_json, 'graphml': write_graphml, 'mermaid': write_mermaid}


def build_dot(block, family=None, focus=None, depth=1, all_edges=False, fixtures=False):
    """The graph as Graphviz DOT text, and its counts."""
    model = build_model(block, family, focus, depth, all_edges, fixtures)
    return write_dot(model), stats(model)


def build_group_dot(block, fixtures=False):
    model = build_group_model(block, fixtures)
    return write_dot(model), stats(model)


def command(root, manifest, args):
    block = manifest['capabilityModules']
    if args.family is None and args.focus is None and not args.modules:
        model = build_group_model(block, args.fixtures)
    else:
        model = build_model(block, args.family, args.focus, args.depth, args.all_edges, args.fixtures)
    counts = stats(model)
    name = args.focus or args.family or ('modules' if args.modules else 'all')
    extension = EXTENSIONS.get(args.format, args.format)
    out = Path(args.output) if args.output else Path(root) / OUT_DIR / f'graph-{name}.{extension}'
    out.parent.mkdir(parents=True, exist_ok=True)
    if args.format in WRITERS:
        out.write_text(WRITERS[args.format](model), encoding='utf-8')
    else:
        dot = shutil.which('dot')
        if dot is None:
            raise SystemExit('Graphviz `dot` is not installed; use --format dot, json, graphml or mermaid')
        rendered = subprocess.run([dot, f'-T{args.format}', '-Gdpi=' + str(args.dpi), '-o', str(out)],
                                  input=write_dot(model), text=True, capture_output=True)
        if rendered.returncode != 0:
            raise SystemExit(f'dot failed: {rendered.stderr.strip()}')
    print(f'archlint: graph: {counts["nodes"]} nodes, {counts["edges"]} of {counts["all"]} edges'
          f'{" after reduction" if model.get("reduced") else ""}, {counts["pending"]} pending, '
          f'{counts["cycles"]} group cycle(s) -> {out}')
    return 0
