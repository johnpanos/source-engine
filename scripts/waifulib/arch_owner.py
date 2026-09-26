#! /usr/bin/env python
# encoding: utf-8
"""RFC 0001 "Waf target checks" for targets that declare `arch_module`.

A task generator names its architectural owner beside `name` and `use`:

	bld.program(name = 'x', arch_module = 'render.vulkan.native-tests', ...)

When the target is posted, its sources, its declared owner and the targets it
uses are judged by the link-graph rules of tools/archlint/capabilities.py
(CAP006/CAP008), and a violation fails the build. The declaration is also
recorded in toolchain-invocations.json (toolchain_dialect) for the
`arch.targets` gate, which also requires every undeclared target to be in a
legacy group of architecture/modules.json `targetOwners`. That requirement is
the gate's alone, so this check never fails a product build for an
undeclared target.
"""

import json
import os
import sys

from waflib import Context, Errors, Utils
from waflib.TaskGen import feature, before_method

_BLOCKS = {}


def _capabilities(root):
	path = os.path.join(root, 'tools', 'archlint')
	if path not in sys.path:
		sys.path.insert(0, path)
	import capabilities
	return capabilities


def _block(root):
	if root not in _BLOCKS:
		with open(os.path.join(root, 'architecture', 'modules.json')) as stream:
			_BLOCKS[root] = json.load(stream)['capabilityModules']
	return _BLOCKS[root]


def _target_name(tg):
	return getattr(tg, 'name', None) or str(getattr(tg, 'target', ''))


def _entries(tg):
	"""Recorded-invocation-shaped entries for one task generator's sources."""
	name = _target_name(tg)
	use = Utils.to_list(getattr(tg, 'use', []))
	module = getattr(tg, 'arch_module', None)
	entries = []
	for source in Utils.to_list(getattr(tg, 'source', [])):
		try:
			nodes = tg.to_nodes([source])
		except Errors.WafError:
			# A generated source that does not exist yet has no owner to judge.
			continue
		for node in nodes:
			entry = {'target': name, 'source': node.path_from(tg.bld.srcnode), 'use': use}
			if module:
				entry['arch_module'] = module
			entries.append(entry)
	return entries


@feature('c', 'cxx')
@before_method('process_source')
def check_arch_module(self):
	if getattr(self, 'arch_module', None) is None or getattr(self.bld, 'kw', None) is not None:
		return
	root = Context.top_dir
	name = _target_name(self)
	entries = _entries(self)
	for dependency in Utils.to_list(getattr(self, 'use', [])):
		try:
			other = self.bld.get_tgen_by_name(dependency)
		except Errors.WafError:
			# A uselib variable (SDL3, VULKAN), judged by the owner's grants.
			continue
		entries.extend(_entries(other))
	errors, _, _ = _capabilities(root).link_graph_errors(_block(root), {'entries': entries})
	own = [error for error in errors
		if error.startswith(('CAP006 %s (' % name, 'CAP008 %s:' % name))]
	if own:
		self.bld.fatal('\n'.join(own))
