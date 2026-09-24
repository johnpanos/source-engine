#!/usr/bin/env python3
"""Render declaration skeletons for Portal 2 source paths from a Mach-O dSYM.

The Steam2 dSYM bundles (see README.md) carry DWARF type, member, function,
parameter and local-variable metadata with declaration lines, but no source
text. For each requested .cpp this reads only that compile unit and prints:

* classes, structs, unions, enums and typedefs declared in the .cpp or in a
  header that does not exist in this checkout, in declaration-line order;
* file-scope variables (ConVars, statics, static member definitions);
* every function defined by the unit, with parameter names, local variables,
  lexical-block nesting and inlined callees.

The output is a reconstruction aid, not source. Layouts are for the 2010 i386
build; this fork is 64-bit and its engine headers differ.

  python3 tools/portal2/dsym/dwarf_skeleton.py --dsym <DWARF file> \
      --out external/portal2_steam2_decompiled/skeleton/852_3/server \
      game/server/portal2/paint_sphere.cpp ...
"""

import argparse
import mmap
import os
import struct
import sys
from collections import defaultdict
from pathlib import Path

from elftools.dwarf.dwarfinfo import DebugSectionDescriptor, DwarfConfig, DWARFInfo

ROOT = Path(__file__).resolve().parents[3]

SECTIONS = {
	'__debug_info': 'debug_info_sec',
	'__debug_abbrev': 'debug_abbrev_sec',
	'__debug_str': 'debug_str_sec',
	'__debug_line': 'debug_line_sec',
	'__debug_ranges': 'debug_ranges_sec',
	'__debug_loc': 'debug_loc_sec',
	'__debug_frame': 'debug_frame_sec',
	'__debug_aranges': 'debug_aranges_sec',
	'__debug_pubnames': 'debug_pubnames_sec',
	'__debug_pubtypes': 'debug_pubtypes_sec',
}


def load_dwarf(path):
	handle = open(path, 'rb')
	data = mmap.mmap(handle.fileno(), 0, access=mmap.ACCESS_READ)
	magic, _cpu, _sub, _ftype, ncmds, _size, _flags = struct.unpack_from('<7I', data, 0)
	if magic != 0xfeedface:
		raise SystemExit(f'{path}: expected a 32-bit little-endian Mach-O dSYM')
	offset = 28
	found = {}
	for _ in range(ncmds):
		cmd, cmdsize = struct.unpack_from('<2I', data, offset)
		if cmd == 1:  # LC_SEGMENT
			nsects = struct.unpack_from('<I', data, offset + 48)[0]
			for index in range(nsects):
				base = offset + 56 + index * 68
				name = data[base:base + 16].rstrip(b'\0').decode()
				_addr, size, fileoff = struct.unpack_from('<3I', data, base + 32)
				if name in SECTIONS and fileoff:
					found[SECTIONS[name]] = (fileoff, size)
		offset += cmdsize

	import io

	def section(key):
		if key not in found:
			return None
		fileoff, size = found[key]
		return DebugSectionDescriptor(io.BytesIO(data[fileoff:fileoff + size]), key, None, size, 0)

	config = DwarfConfig(little_endian=True, machine_arch='x86', default_address_size=4)
	return DWARFInfo(
		config=config,
		debug_info_sec=section('debug_info_sec'),
		debug_aranges_sec=None,
		debug_abbrev_sec=section('debug_abbrev_sec'),
		debug_frame_sec=None,
		eh_frame_sec=None,
		debug_str_sec=section('debug_str_sec'),
		debug_loc_sec=section('debug_loc_sec'),
		debug_ranges_sec=section('debug_ranges_sec'),
		debug_line_sec=section('debug_line_sec'),
		debug_pubtypes_sec=None,
		debug_types_sec=None,
		debug_pubnames_sec=None,
		debug_addr_sec=None,
		debug_str_offsets_sec=None,
		debug_line_str_sec=None,
		debug_loclists_sec=None,
		debug_rnglists_sec=None,
		debug_sup_sec=None,
		gnu_debugaltlink_sec=None,
	)


def text(value):
	return value.decode('utf-8', 'replace') if isinstance(value, bytes) else str(value)


def source_relative(path):
	path = path.replace('\\', '/')
	marker = '/src/'
	index = path.find(marker)
	if index >= 0:
		path = path[index + len(marker):]
	parts = []
	for part in path.split('/'):
		if part == '..' and parts:
			parts.pop()
		elif part not in ('', '.'):
			parts.append(part)
	return '/'.join(parts)


class Unit:
	def __init__(self, dwarf, cu):
		self.dwarf = dwarf
		self.cu = cu
		self.top = cu.get_top_DIE()
		self.files = [None]
		program = dwarf.line_program_for_CU(cu)
		directories = [text(d) for d in program['include_directory']]
		comp_dir = text(self.top.attributes['DW_AT_comp_dir'].value) if 'DW_AT_comp_dir' in self.top.attributes else ''
		for entry in program['file_entry']:
			name = text(entry.name)
			directory = directories[entry.dir_index - 1] if entry.dir_index else comp_dir
			full = name if name.startswith('/') else f'{directory}/{name}'
			self.files.append(source_relative(full))
		self.cache = {}

	def die(self, attribute_value_die_ref):
		return attribute_value_die_ref

	def ref(self, die, name):
		if name not in die.attributes:
			return None
		return die.get_DIE_from_attribute(name)

	def attr(self, die, name, default=None):
		attribute = die.attributes.get(name)
		if attribute is None:
			return default
		value = attribute.value
		return text(value) if isinstance(value, bytes) else value

	def decl(self, die):
		index = self.attr(die, 'DW_AT_decl_file')
		line = self.attr(die, 'DW_AT_decl_line', 0)
		if index is None or index >= len(self.files):
			return None, line
		return self.files[index], line

	def name(self, die):
		name = self.attr(die, 'DW_AT_name')
		if name is None:
			for key in ('DW_AT_specification', 'DW_AT_abstract_origin'):
				other = self.ref(die, key)
				if other is not None:
					return self.name(other)
		return name

	def qualified(self, die):
		parts = []
		name = self.name(die)
		scope = die
		for key in ('DW_AT_specification', 'DW_AT_abstract_origin'):
			other = self.ref(die, key)
			if other is not None:
				scope = other
				break
		parent = scope.get_parent()
		while parent is not None and parent.tag in ('DW_TAG_structure_type', 'DW_TAG_class_type',
													 'DW_TAG_union_type', 'DW_TAG_namespace'):
			parts.append(self.name(parent) or '<anon>')
			parent = parent.get_parent()
		return '::'.join(list(reversed(parts)) + [name or '<anon>'])

	def type_name(self, die, inner=''):
		"""Return a C++ spelling of the type with `inner` as the declarator."""
		if die is None:
			return ('void ' + inner).strip()
		tag = die.tag
		if tag in ('DW_TAG_base_type', 'DW_TAG_structure_type', 'DW_TAG_class_type', 'DW_TAG_union_type',
				   'DW_TAG_enumeration_type', 'DW_TAG_typedef', 'DW_TAG_unspecified_type'):
			name = self.qualified(die) if tag != 'DW_TAG_base_type' else self.name(die)
			if name is None or '<anon>' in name:
				name = {'DW_TAG_union_type': 'union', 'DW_TAG_enumeration_type': 'enum'}.get(tag, 'struct') + ' <anonymous>'
			return (name + ' ' + inner).strip() if inner else name
		sub = self.ref(die, 'DW_AT_type')
		if tag == 'DW_TAG_pointer_type':
			return self.type_name(sub, self._wrap(sub, '*' + inner))
		if tag == 'DW_TAG_reference_type':
			return self.type_name(sub, self._wrap(sub, '&' + inner))
		if tag == 'DW_TAG_const_type':
			if sub is not None and sub.tag == 'DW_TAG_reference_type':
				return self.type_name(sub, inner)
			return self.type_name(sub, ('const ' + inner).strip()) if sub is not None and sub.tag == \
				'DW_TAG_pointer_type' else 'const ' + self.type_name(sub, inner)
		if tag == 'DW_TAG_volatile_type':
			return 'volatile ' + self.type_name(sub, inner)
		if tag == 'DW_TAG_array_type':
			dims = ''
			for child in die.iter_children():
				if child.tag == 'DW_TAG_subrange_type':
					upper = self.attr(child, 'DW_AT_upper_bound')
					dims += f'[{upper + 1}]' if isinstance(upper, int) else '[]'
			return self.type_name(sub, inner + dims)
		if tag == 'DW_TAG_subroutine_type':
			params = [self.type_name(self.ref(c, 'DW_AT_type')) for c in die.iter_children()
					  if c.tag == 'DW_TAG_formal_parameter']
			return self.type_name(sub, f'{inner}({", ".join(params)})')
		if tag == 'DW_TAG_ptr_to_member_type':
			owner = self.ref(die, 'DW_AT_containing_type')
			return self.type_name(sub, f'({self.qualified(owner) if owner else "?"}::*{inner})')
		return f'/*{tag}*/ {inner}'.strip()

	@staticmethod
	def _wrap(sub, inner):
		if sub is not None and sub.tag in ('DW_TAG_subroutine_type', 'DW_TAG_array_type'):
			return f'({inner})'
		return inner


def uleb(data):
	value = 0
	for shift, byte in enumerate(data):
		value |= (byte & 0x7f) << (7 * shift)
		if not byte & 0x80:
			break
	return value


def resolve_origin(unit, die):
	"""Follow abstract-origin/specification links to the DIE that has names and types."""
	seen = 0
	while seen < 8 and 'DW_AT_type' not in die.attributes and 'DW_AT_name' not in die.attributes:
		other = unit.ref(die, 'DW_AT_abstract_origin') or unit.ref(die, 'DW_AT_specification')
		if other is None:
			break
		die = other
		seen += 1
	return die


def network_var(unit, member, type_die):
	"""Spell CNetworkVar-family members as their declaring macro."""
	name = unit.type_name(type_die)
	member_name = unit.name(member)
	marker = f'::NetworkVar_{member_name}>'
	if not name.endswith(marker):
		return None
	inner = name[name.index('<') + 1:-len(marker)]
	template, _, _ = name.partition('<')
	value_type, _, _owner = inner.rpartition(',')
	value_type = value_type.strip()
	macro = {
		'CNetworkVarBase': 'CNetworkVar',
		'CNetworkHandleBase': 'CNetworkHandle',
		'CNetworkVectorBase': 'CNetworkVector',
		'CNetworkVarEmbedded': 'CNetworkVarEmbedded',
		'CNetworkColor32Base': 'CNetworkColor32',
		'CNetworkStringTBase': 'CNetworkString',
	}.get(template, template)
	if macro == 'CNetworkVector' and value_type == 'QAngle':
		macro = 'CNetworkQAngle'
	if macro == 'CNetworkHandle' and ',' in value_type:
		value_type = value_type.split(',')[0]
	return f'{macro}( {value_type}, {member_name} );'


def signature(unit, die, qualified=False, definition=None):
	name = unit.qualified(die) if qualified else unit.name(die)
	params = []
	is_const = False
	has_this = False
	source = definition if definition is not None else die
	for child in source.iter_children():
		if child.tag == 'DW_TAG_formal_parameter':
			child = resolve_origin(unit, child)
			type_die = unit.ref(child, 'DW_AT_type')
			if unit.attr(child, 'DW_AT_artificial'):
				has_this = True
				pointee = unit.ref(type_die, 'DW_AT_type') if type_die is not None else None
				is_const = pointee is not None and pointee.tag == 'DW_TAG_const_type'
				continue
			pname = unit.attr(child, 'DW_AT_name') or ''
			params.append(unit.type_name(type_die, pname))
		elif child.tag == 'DW_TAG_unspecified_parameters':
			params.append('...')
	returns = unit.ref(die, 'DW_AT_type')
	prefix = ''
	if not qualified:
		virtuality = unit.attr(die, 'DW_AT_virtuality', 0)
		if virtuality:
			prefix = 'virtual '
		elif not has_this and die.get_parent() is not None and die.get_parent().tag in (
				'DW_TAG_structure_type', 'DW_TAG_class_type'):
			prefix = 'static '
	owner = die.get_parent()
	ctor = owner is not None and unit.name(owner) is not None and name.split('::')[-1] in (
		unit.name(owner), '~' + (unit.name(owner) or ''))
	decl = f'{name}( {", ".join(params)} )' if params else f'{name}()'
	if not ctor and returns is None and name.split('::')[-1].startswith('~') is False and not qualified:
		decl = 'void ' + decl
	elif returns is not None:
		decl = unit.type_name(returns, decl)
	elif qualified and not name.split('::')[-1].startswith('~') and '::' in name and \
			name.split('::')[-1] != name.split('::')[-2]:
		decl = 'void ' + decl
	return prefix + decl + (' const' if is_const else '')


ACCESS = {1: 'public', 2: 'protected', 3: 'private'}


def render_record(unit, die, indent=''):
	tag = die.tag
	name = unit.name(die) or '<anonymous>'
	file, line = unit.decl(die)
	size = unit.attr(die, 'DW_AT_byte_size')
	keyword = {'DW_TAG_structure_type': 'struct', 'DW_TAG_class_type': 'class', 'DW_TAG_union_type': 'union'}[tag]
	bases = []
	for child in die.iter_children():
		if child.tag == 'DW_TAG_inheritance':
			access = ACCESS.get(unit.attr(child, 'DW_AT_accessibility', 1), 'public')
			virtual = 'virtual ' if unit.attr(child, 'DW_AT_virtuality') else ''
			bases.append(f'{access} {virtual}{unit.type_name(unit.ref(child, "DW_AT_type"))}')
	out = [f'{indent}// {file}:{line}' + (f' sizeof=0x{size:x} (i386)' if isinstance(size, int) else '')]
	head = f'{indent}{keyword} {name}' + (' : ' + ', '.join(bases) if bases else '')
	if unit.attr(die, 'DW_AT_declaration'):
		return [f'{indent}{keyword} {name}; // declaration only in this unit']
	out.append(head)
	out.append(indent + '{')
	entries = []
	for child in die.iter_children():
		ctag = child.tag
		cname = unit.name(child)
		_cfile, cline = unit.decl(child)
		access = ACCESS.get(unit.attr(child, 'DW_AT_accessibility', 1 if tag != 'DW_TAG_class_type' else 3))
		if ctag == 'DW_TAG_member':
			type_die = unit.ref(child, 'DW_AT_type')
			location = unit.attr(child, 'DW_AT_data_member_location')
			offset = ''
			if isinstance(location, list) and len(location) >= 2:
				offset = f' // +0x{uleb(location[1:]):x}'
			elif isinstance(location, int):
				offset = f' // +0x{location:x}'
			bits = unit.attr(child, 'DW_AT_bit_size')
			spelled = network_var(unit, child, type_die) if type_die is not None else None
			if spelled is None:
				spelled = unit.type_name(type_die, cname or '') + (f' : {bits}' if bits else '') + ';'
			entries.append((cline, access, spelled + offset))
		elif ctag == 'DW_TAG_variable':
			if cname in ('m_DataMap', 'm_pClassSendTable', 'm_pClassRecvTable', 'm_PredMap'):
				continue
			entries.append((cline, access, 'static ' + unit.type_name(unit.ref(child, 'DW_AT_type'), cname or '') + ';'))
		elif ctag == 'DW_TAG_subprogram':
			if unit.attr(child, 'DW_AT_artificial'):
				continue
			if cname and (cname.startswith('NetworkStateChanged') or cname in (
					'GetDataDescMap', 'GetBaseMap', 'GetSendTable', 'GetClientClass', 'GetServerClass',
					'GetRecvTable', 'GetPredDescMap', 'YouForgotToImplementOrDeclareServerClass',
					'YouForgotToImplementOrDeclareClientClass', 'GetClassName', 'ShouldPredict',
					'GetPredictionDescMap', 'DataMapAccess', 'DataMapInit', 'PredMapInit',
					'GetClientClassStatic', 'GetServerClassStatic')):
				continue
			entries.append((cline, access, signature(unit, child) + ';'))
		elif ctag in ('DW_TAG_structure_type', 'DW_TAG_class_type', 'DW_TAG_union_type'):
			if cname and (cname.startswith('NetworkVar_') or cname in ('NetworkVar_m_Local',)):
				continue
			entries.append((cline, access, '\n'.join(render_record(unit, child, indent + '\t')).lstrip('\t')))
		elif ctag == 'DW_TAG_enumeration_type':
			entries.append((cline, access, '\n'.join(render_enum(unit, child, indent + '\t')).lstrip('\t')))
		elif ctag == 'DW_TAG_typedef':
			if cname in ('BaseClass', 'ThisClass'):
				entries.append((cline, access, f'DECLARE_CLASS-like typedef {unit.type_name(unit.ref(child, "DW_AT_type"))} {cname};'))
				continue
			entries.append((cline, access, f'typedef {unit.type_name(unit.ref(child, "DW_AT_type"), cname)};'))
	current = None
	for cline, access, body in sorted(entries, key=lambda e: e[0] or 0):
		if access != current:
			out.append(f'{indent}{access}:')
			current = access
		out.append(f'{indent}\t{body}  // line {cline}')
	out.append(indent + '};')
	return out


def render_enum(unit, die, indent=''):
	file, line = unit.decl(die)
	out = [f'{indent}// {file}:{line}', f'{indent}enum {unit.name(die) or ""}'.rstrip(), indent + '{']
	for child in die.iter_children():
		if child.tag == 'DW_TAG_enumerator':
			out.append(f'{indent}\t{unit.name(child)} = {unit.attr(child, "DW_AT_const_value")},')
	out.append(indent + '};')
	return out


def render_function(unit, die):
	spec = unit.ref(die, 'DW_AT_specification')
	origin = unit.ref(die, 'DW_AT_abstract_origin')
	decl_die = spec or origin or die
	file, line = unit.decl(die)
	target = decl_die
	while file is None and target is not None:
		file, line = unit.decl(target)
		target = unit.ref(target, 'DW_AT_abstract_origin') or unit.ref(target, 'DW_AT_specification')
	low = unit.attr(die, 'DW_AT_low_pc')
	linkage = unit.attr(decl_die, 'DW_AT_MIPS_linkage_name') or unit.attr(die, 'DW_AT_MIPS_linkage_name')
	while 'DW_AT_name' not in decl_die.attributes:
		other = unit.ref(decl_die, 'DW_AT_abstract_origin') or unit.ref(decl_die, 'DW_AT_specification')
		if other is None:
			break
		decl_die = other
	head = signature(unit, decl_die, qualified=True, definition=die)
	out = [f'// {file}:{line}' + (f' @0x{low:x}' if isinstance(low, int) else '') + (f' {linkage}' if linkage else ''),
		   head, '{']
	walk_body(unit, die, out, '\t')
	out.append('}')
	return file, line, out


def walk_body(unit, die, out, indent):
	for child in die.iter_children():
		tag = child.tag
		if tag == 'DW_TAG_variable':
			child = resolve_origin(unit, child)
			_f, line = unit.decl(child)
			out.append(f'{indent}{unit.type_name(unit.ref(child, "DW_AT_type"), unit.name(child) or "")};  // line {line}')
		elif tag == 'DW_TAG_lexical_block':
			out.append(indent + '{')
			walk_body(unit, child, out, indent + '\t')
			out.append(indent + '}')
		elif tag == 'DW_TAG_inlined_subroutine':
			origin = unit.ref(child, 'DW_AT_abstract_origin')
			call_line = unit.attr(child, 'DW_AT_call_line')
			label = unit.qualified(origin) if origin is not None else '?'
			out.append(f'{indent}// inlined {label}()' + (f' at line {call_line}' if call_line else ''))


RECORD_TAGS = ('DW_TAG_structure_type', 'DW_TAG_class_type', 'DW_TAG_union_type')


def collect(unit, wanted_files, blocks, seen):
	def visit(die, depth):
		for child in die.iter_children():
			tag = child.tag
			file, line = unit.decl(child)
			if tag == 'DW_TAG_namespace':
				visit(child, depth + 1)
				continue
			if tag == 'DW_TAG_subprogram' and ('DW_AT_low_pc' in child.attributes):
				if (unit.name(child) or '').startswith(('__tcf_', '_GLOBAL__')):
					continue
				ffile, fline, body = render_function(unit, child)
				owner = ffile if ffile in wanted_files else unit.files_cu
				key = ('fn', owner, unit.attr(child, 'DW_AT_low_pc'))
				if owner in wanted_files and key not in seen:
					seen.add(key)
					blocks[owner].append((fline, '\n'.join(body)))
				continue
			if file not in wanted_files:
				continue
			key = (tag, file, line, unit.name(child))
			if key in seen:
				continue
			if tag in RECORD_TAGS:
				if unit.attr(child, 'DW_AT_declaration'):
					continue
				if (unit.name(child) or '').startswith('NetworkVar_'):
					continue
				seen.add(key)
				blocks[file].append((line, '\n'.join(render_record(unit, child))))
			elif tag == 'DW_TAG_enumeration_type':
				seen.add(key)
				blocks[file].append((line, '\n'.join(render_enum(unit, child))))
			elif tag == 'DW_TAG_typedef':
				seen.add(key)
				blocks[file].append((line, f'// {file}:{line}\ntypedef {unit.type_name(unit.ref(child, "DW_AT_type"), unit.name(child))};'))
			elif tag == 'DW_TAG_variable':
				seen.add(key)
				spec = unit.ref(child, 'DW_AT_specification')
				name = unit.qualified(spec) if spec is not None else unit.name(child)
				storage = '' if unit.attr(child, 'DW_AT_external') or spec is not None else 'static '
				blocks[file].append((line, f'// {file}:{line}\n{storage}{unit.type_name(unit.ref(child, "DW_AT_type"), name)};'))
			elif tag == 'DW_TAG_subprogram' and depth == 0:
				seen.add(key)
				blocks[file].append((line, f'// {file}:{line} (declaration)\n{signature(unit, child)};'))

	visit(unit.top, 0)


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--dsym', required=True, help='DWARF file inside the .dSYM bundle')
	parser.add_argument('--out', required=True, help='output directory (source paths are appended)')
	parser.add_argument('--label', default='', help='provenance label written into each file')
	parser.add_argument('paths', nargs='+', help='repository-relative .cpp paths whose units to read')
	args = parser.parse_args()

	requested = {p.replace('\\', '/') for p in args.paths}
	dwarf = load_dwarf(args.dsym)
	blocks = defaultdict(list)
	seen = set()
	matched = set()
	for cu in dwarf.iter_CUs():
		top = cu.get_top_DIE()
		name = top.attributes.get('DW_AT_name')
		if name is None:
			continue
		comp_dir = top.attributes.get('DW_AT_comp_dir')
		raw = text(name.value)
		if not raw.startswith('/') and comp_dir is not None:
			raw = text(comp_dir.value) + '/' + raw
		path = source_relative(raw)
		if path not in requested:
			continue
		matched.add(path)
		unit = Unit(dwarf, cu)
		unit.files_cu = path
		wanted = {f for f in unit.files if f and (f == path or (
			f.startswith('game/') and not (ROOT / f).exists()))}
		collect(unit, wanted, blocks, seen)
		print(f'read {path}: {len(wanted)} target files', file=sys.stderr)

	for missing in sorted(requested - matched):
		print(f'warning: no compile unit for {missing}', file=sys.stderr)
	out_root = Path(args.out)
	for file, entries in sorted(blocks.items()):
		target = out_root / (file + '.skeleton.h')
		target.parent.mkdir(parents=True, exist_ok=True)
		header = [
			f'// DWARF declaration skeleton for {file}',
			f'// Source: {args.label or args.dsym}',
			'// Generated reconstruction aid; not original source and not buildable.',
			'// Layouts and offsets are for the 2010 macOS i386 build.',
			'',
		]
		body = [block for _line, block in sorted(entries, key=lambda e: (e[0] or 0))]
		target.write_text('\n'.join(header) + '\n\n'.join(body) + '\n')
		print(f'wrote {target} ({len(entries)} entries)', file=sys.stderr)
	return 0


if __name__ == '__main__':
	sys.exit(main())
