import argparse
import io
import shutil
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import graph
import test_capabilities as fixtures_of


class GraphTest(unittest.TestCase):
    """`archlint graph`: the drawing follows the contract the checks use."""

    def setUp(self):
        fixtures_of.TranslationContractTest.setUp(self)

    module = fixtures_of.LayerContractTest.module
    add = fixtures_of.LayerContractTest.add
    write = fixtures_of.LayerContractTest.write
    EDGES = fixtures_of.LayerContractTest.EDGES

    def dot(self, **options):
        return graph.build_dot(self.block, **options)

    def test_layers_roles_and_pending(self):
        text, stats = self.dot(family='render')
        self.assertIn('subgraph cluster_layer0', text)
        self.assertIn('subgraph cluster_adapters', text)
        self.assertIn('subgraph cluster_outside', text)
        # Roles: the frontend is a translator, the composition the root.
        self.assertIn('"render.legacy-frontend" [fillcolor="#fde3c8"', text)
        self.assertIn('"render.composition" [fillcolor="#e8dcf3"', text)
        self.assertIn('"render.device.vulkan" [fillcolor="#d9f0d3"', text)
        # A recorded pending violation is drawn red with its row.
        self.assertIn('"render.material" -> "content.keyvalues-text" [color="#d32f2f"', text)
        self.assertIn('label="pending R96"', text)
        self.assertEqual(stats['pending'], 2)

    def test_reduction_drops_implied_edges(self):
        # graph -> device -> math, and graph -> math directly: the direct edge is implied.
        reduced, _ = self.dot(family='render')
        full, _ = self.dot(family='render', all_edges=True)
        self.assertIn('"render.graph" -> "render.math";', full)
        self.assertNotIn('"render.graph" -> "render.math";', reduced)
        self.assertIn('"render.graph" -> "render.device";', reduced)

    def test_reduce_keeps_cycles_and_unrelated_edges(self):
        edges = [('a', 'b'), ('b', 'c'), ('a', 'c'), ('c', 'a'), ('d', 'e')]
        kept = graph.reduce_edges({'a', 'b', 'c', 'd', 'e'}, edges)
        self.assertIn(('d', 'e'), kept)
        self.assertIn(('a', 'b'), kept)
        self.assertNotIn(('a', 'c'), kept)

    def test_focus_limits_to_the_neighborhood(self):
        text, stats = self.dot(family='render', focus='render.scene', depth=1)
        self.assertIn('"render.scene"', text)
        self.assertIn('"render.material"', text)  # its dependency
        self.assertIn('"render.frame"', text)     # its dependent
        self.assertNotIn('"render.device.vulkan"', text)
        self.assertIn('penwidth=2.5', text)

    def test_fixtures_hidden_by_default(self):
        self.add('render.core-tests', ['render.composition'])
        self.contract['layers'].append(['render.core-tests'])
        self.contract['translation']['fixtures'] = ['render.core-tests']
        self.assertNotIn('"render.core-tests"', self.dot(family='render')[0])
        self.assertIn('"render.core-tests"', self.dot(family='render', fixtures=True)[0])

    def test_unknown_family_and_module(self):
        with self.assertRaises(SystemExit):
            self.dot(family='nope')
        with self.assertRaises(SystemExit):
            self.dot(focus='render.nope')

    def test_groups_view_counts_module_edges(self):
        text, stats = graph.build_group_dot(self.block)
        self.assertIn('"render" -> "content"', text)
        self.assertIn('"content" [label="content (2)"', text)
        self.assertGreater(stats['edges'], 0)

    def test_json_model(self):
        import json
        model = json.loads(graph.write_json(graph.build_model(self.block, family='render')))
        self.assertEqual(model['schema'], 'archlint-graph/v1')
        roles = {n['id']: n['role'] for n in model['nodes']}
        self.assertEqual(roles['render.legacy-frontend'], 'translator')
        self.assertEqual(roles['content.keyvalues-text'], 'base')
        layers = {n['id']: n['layer'] for n in model['nodes']}
        self.assertEqual(layers['render.scene'], 4)
        pending = [e for e in model['edges'] if e['kind'] == 'pending']
        self.assertEqual({(e['from'], e['to'], e['owner']) for e in pending},
                         {('render.material', 'content.keyvalues-text', 'R96'),
                          ('render.resources', 'content.texture-contract', 'R96')})
        # Reduced edges stay in the data, marked hidden.
        hidden = [e for e in model['edges'] if not e['shown']]
        self.assertIn({'from': 'render.graph', 'to': 'render.math', 'kind': 'dependency', 'shown': False},
                      hidden)

    def test_graphml_parses(self):
        import xml.etree.ElementTree as ET
        root = ET.fromstring(graph.write_graphml(graph.build_model(self.block, family='render')))
        ns = {'g': 'http://graphml.graphdrawing.org/xmlns'}
        nodes = root.findall('.//g:node', ns)
        edges = root.findall('.//g:edge', ns)
        self.assertEqual(len(nodes), len(graph.build_model(self.block, family='render')['nodes']))
        kinds = {d.text for e in edges for d in e.findall('g:data', ns) if d.get('key') == 'edge_kind'}
        self.assertEqual(kinds, {'dependency', 'pending'})

    def test_mermaid(self):
        text = graph.write_mermaid(graph.build_model(self.block, family='render'))
        self.assertTrue(text.startswith('---\ntitle: render layer contract'))
        self.assertIn('flowchart TB', text)
        self.assertIn('"pending R96"', text)
        self.assertIn('stroke:#d32f2f', text)
        self.assertNotIn('render.graph"] --> ', text)  # labels never used as ids

    def test_group_cycles_marked(self):
        self.module('content.texture-contract')['allowedEdges'] = ['render.math']
        model = graph.build_group_model(self.block)
        self.assertTrue(any(c['groups'] == ['content', 'render'] for c in model['cycles']))
        self.assertTrue(any(e['cycle'] for e in model['edges']))
        self.assertIn('color="#e07b00"', graph.write_dot(model))

    @unittest.skipUnless(shutil.which('dot'), 'Graphviz is not installed')
    def test_png_renders(self):
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory) / 'render.png'
            args = argparse.Namespace(family='render', focus=None, depth=1, all_edges=False,
                                      fixtures=False, modules=False, format='png', dpi=60,
                                      output=str(out))
            with redirect_stdout(io.StringIO()):
                self.assertEqual(graph.command(self.root, {'capabilityModules': self.block}, args), 0)
            self.assertEqual(out.read_bytes()[:8], b'\x89PNG\r\n\x1a\n')


if __name__ == '__main__':
    unittest.main()
