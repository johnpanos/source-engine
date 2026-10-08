# encoding: utf-8
# wasm_localize.py -- `objcopy --localize-hidden` for WebAssembly objects
#
# The static composition (static_composition.py) partially links each module
# with `wasm-ld --relocatable`, then needs the module's hidden symbols made
# local so each module keeps its own copy of its private libraries, as it does
# on ELF and Mach-O. No objcopy reads wasm, so this rewrites the object's
# "linking" custom section (tool-conventions/Linking.md, version 2):
#
#  - every defined symbol with VISIBILITY_HIDDEN gets BINDING_LOCAL (and loses
#    BINDING_WEAK): references inside the object resolve by symbol index, so
#    they are unchanged, and the final link no longer sees the name;
#  - the COMDAT_INFO subsection is dropped, as --force-group-allocation
#    dissolves ELF groups: the final link must not fold one module's inline
#    functions (and their static locals) into another module's.
#
# Every other section and subsection is copied byte for byte.

import sys

WASM_MAGIC = b'\0asm\x01\0\0\0'
SUB_COMDAT_INFO = 7
SUB_SYMBOL_TABLE = 8

SYM_FUNCTION, SYM_DATA, SYM_GLOBAL, SYM_SECTION, SYM_EVENT, SYM_TABLE = 0, 1, 2, 3, 4, 5
BINDING_WEAK = 0x1
BINDING_LOCAL = 0x2
VISIBILITY_HIDDEN = 0x4
UNDEFINED = 0x10
EXPLICIT_NAME = 0x40


class WasmError(Exception):
	pass


def read_leb(data, at):
	value = shift = 0
	while True:
		if at >= len(data):
			raise WasmError('truncated LEB128 at %d' % at)
		byte = data[at]
		at += 1
		value |= (byte & 0x7f) << shift
		shift += 7
		if not byte & 0x80:
			return value, at


def leb(value):
	out = bytearray()
	while True:
		byte = value & 0x7f
		value >>= 7
		if value:
			out.append(byte | 0x80)
		else:
			out.append(byte)
			return bytes(out)


def read_name(data, at):
	length, at = read_leb(data, at)
	return data[at:at + length], at + length


def rewrite_symbols(payload):
	"""The symbol table with hidden definitions made local; how many changed."""
	count, at = read_leb(payload, 0)
	out = bytearray(leb(count))
	changed = 0
	for _ in range(count):
		kind = payload[at]
		at += 1
		flags, at = read_leb(payload, at)
		start = at
		defined = not flags & UNDEFINED
		if kind in (SYM_FUNCTION, SYM_GLOBAL, SYM_EVENT, SYM_TABLE):
			_, at = read_leb(payload, at)
			if defined or flags & EXPLICIT_NAME:
				_, at = read_name(payload, at)
		elif kind == SYM_DATA:
			_, at = read_name(payload, at)
			if defined:
				for _ in range(3):
					_, at = read_leb(payload, at)
		elif kind == SYM_SECTION:
			_, at = read_leb(payload, at)
		else:
			raise WasmError('unknown symbol kind %d' % kind)
		if defined and flags & VISIBILITY_HIDDEN and not flags & BINDING_LOCAL:
			flags = (flags & ~BINDING_WEAK) | BINDING_LOCAL
			changed += 1
		out.append(kind)
		out += leb(flags)
		out += payload[start:at]
	if at != len(payload):
		raise WasmError('symbol table has %d trailing bytes' % (len(payload) - at))
	return bytes(out), changed


def rewrite_linking(content):
	version, at = read_leb(content, 0)
	if version != 2:
		raise WasmError('linking section version %d (expected 2)' % version)
	out = bytearray(leb(version))
	changed = 0
	while at < len(content):
		kind = content[at]
		size, body = read_leb(content, at + 1)
		payload = content[body:body + size]
		at = body + size
		if kind == SUB_COMDAT_INFO:
			continue
		if kind == SUB_SYMBOL_TABLE:
			payload, changed = rewrite_symbols(payload)
		out.append(kind)
		out += leb(len(payload))
		out += payload
	return bytes(out), changed


def localize_hidden(data):
	"""The object with its hidden symbols local; (bytes, symbols changed)."""
	if data[:8] != WASM_MAGIC:
		raise WasmError('not a WebAssembly object')
	out = bytearray(data[:8])
	at = 8
	changed = 0
	seen = False
	while at < len(data):
		section = data[at]
		size, body = read_leb(data, at + 1)
		end = body + size
		if section == 0:
			name, content_at = read_name(data, body)
			if name == b'linking':
				content, changed = rewrite_linking(data[content_at:end])
				seen = True
				payload = leb(len(name)) + name + content
				out.append(0)
				out += leb(len(payload))
				out += payload
				at = end
				continue
		out += data[at:end]
		at = end
	if not seen:
		raise WasmError('no linking section (not a relocatable object)')
	return bytes(out), changed


def main(argv):
	if len(argv) != 3:
		print('usage: wasm_localize.py <input.o> <output.o>', file=sys.stderr)
		return 2
	with open(argv[1], 'rb') as source:
		data, changed = localize_hidden(source.read())
	with open(argv[2], 'wb') as target:
		target.write(data)
	print('%s: %d hidden symbols made local' % (argv[2], changed))
	return 0


if __name__ == '__main__':
	sys.exit(main(sys.argv))
