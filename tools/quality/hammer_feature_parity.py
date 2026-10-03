#!/usr/bin/env python3
"""Legacy Hammer feature parity (RFC 0002, R08/R43).

The MFC editor's user-visible functionality is enumerated from its own
sources at a pinned git revision (so the list survives retirement of the
legacy code):

  cmd:<ID>   every command ID the editor reaches from a menu, accelerator,
             toolbar button or a frame/document/view message map
             (ON_COMMAND, ON_COMMAND_EX, ON_COMMAND_RANGE), with its menu
             paths, accelerators, toolbars and handlers;
  dlg:<IDD>  every dialog template, with its caption and the classes that
             use it;
  tool:<ID>  every interactive tool (ToolID_t), with its class.

architecture/hammer_feature_parity.json maps each feature to the new stack:

  status      replicated | partial | missing | not-applicable
  actions     presenters::ActionCatalog ids that reach it
  commands    app::SessionCommands names that perform it
  host        host-side entry points (GTK windows, host requests)
  tests       conformance suite ids (or "suite:case" for UI cases)
  note        what is missing (partial/missing) or why it does not apply

  hammer_feature_parity.py extract            # print the enumerated features
  hammer_feature_parity.py verify             # mapping is complete and its
                                              # references exist (checks-v1)
  hammer_feature_parity.py verify --complete  # also: nothing partial/missing
  hammer_feature_parity.py report             # counts and the open features
  hammer_feature_parity.py selftest           # negative fixtures (checks-v1)

'verify' fails on an unmapped or unknown feature, an invalid status, a
replicated feature without an action or command and a test, a reference to
an action, command or suite that does not exist, and a not-applicable
feature without a reason.
"""

import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path( __file__ ).resolve().parents[2]
MAPPING = ROOT / "architecture" / "hammer_feature_parity.json"
STATUSES = ( "replicated", "partial", "missing", "not-applicable" )


# ---- Extraction -----------------------------------------------------------------------

def git_text( revision, path ):
	return subprocess.run( [ "git", "-C", str( ROOT ), "show", "%s:%s" % ( revision, path ) ],
		check=True, capture_output=True ).stdout.decode( "latin-1" )


def git_files( revision, directory ):
	out = subprocess.run( [ "git", "-C", str( ROOT ), "ls-tree", "--name-only", revision,
		directory + "/" ], check=True, capture_output=True ).stdout.decode()
	return [ line for line in out.splitlines() if line ]


def clean_label( text ):
	text = text.split( "\\t" )[ 0 ].replace( "&", "" ).strip()
	return text


def parse_menus( rc ):
	"""{id: [menu path]} from every MENU resource."""
	found = {}
	lines = rc.splitlines()
	i = 0
	while i < len( lines ):
		m = re.match( r"^(\w+)\s+MENU\b", lines[ i ] )
		if not m:
			i += 1
			continue
		resource = m.group( 1 )
		stack = [ resource ]
		depth = 0
		i += 1
		pending = None
		while i < len( lines ):
			line = lines[ i ].strip()
			i += 1
			popup = re.match( r'^POPUP\s+"([^"]*)"', line )
			item = re.match( r'^MENUITEM\s+"([^"]*)"\s*,?\s*(\w+)', line )
			if popup:
				pending = clean_label( popup.group( 1 ) )
			elif line == "BEGIN":
				depth += 1
				if pending is not None:
					stack.append( pending )
					pending = None
			elif line == "END":
				depth -= 1
				if depth == 0:
					break
				stack.pop()
			elif item:
				found.setdefault( item.group( 2 ), [] ).append(
					" > ".join( stack[ 1: ] + [ clean_label( item.group( 1 ) ) ] ) )
	return found


def parse_accelerators( rc ):
	found = {}
	for block in re.finditer( r"^(\w+)\s+ACCELERATORS\b.*?\nBEGIN\n(.*?)\nEND", rc, re.M | re.S ):
		for line in block.group( 2 ).splitlines():
			parts = [ p.strip() for p in line.split( "," ) ]
			if len( parts ) < 3:
				continue
			key, cid = parts[ 0 ].strip( '"' ), parts[ 1 ]
			mods = [ p for p in parts[ 2: ] if p in ( "CONTROL", "SHIFT", "ALT" ) ]
			chord = "+".join( [ { "CONTROL": "Ctrl", "SHIFT": "Shift", "ALT": "Alt" }[ m ]
				for m in mods ] + [ key.replace( "VK_", "" ) ] )
			found.setdefault( cid, [] )
			if chord not in found[ cid ]:
				found[ cid ].append( chord )
	return found


def parse_toolbars( rc ):
	found = {}
	for block in re.finditer( r"^(\w+)\s+TOOLBAR\b.*?\nBEGIN\n(.*?)\nEND", rc, re.M | re.S ):
		for cid in re.findall( r"BUTTON\s+(\w+)", block.group( 2 ) ):
			found.setdefault( cid, [] ).append( block.group( 1 ) )
	return found


def parse_dialogs( rc ):
	found = {}
	for m in re.finditer( r'^(IDD_\w+)\s+DIALOG(?:EX)?\b[^\n]*\n(?:[^\n]*\n){0,4}?CAPTION\s+"([^"]*)"',
			rc, re.M ):
		found[ m.group( 1 ) ] = m.group( 2 )
	for m in re.finditer( r"^(IDD_\w+)\s+DIALOG(?:EX)?\b", rc, re.M ):
		found.setdefault( m.group( 1 ), "" )
	return found


def parse_sources( revision ):
	"""Command handlers per ID and dialog/tool owners, from hammer/*.cpp and *.h."""
	handlers, dialog_classes, tool_classes = {}, {}, {}
	for path in git_files( revision, "hammer" ):
		if not path.endswith( ( ".cpp", ".h" ) ):
			continue
		text = git_text( revision, path )
		for m in re.finditer( r"BEGIN_MESSAGE_MAP\s*\(\s*(\w+)\s*,\s*\w+\s*\)(.*?)END_MESSAGE_MAP",
				text, re.S ):
			cls = m.group( 1 )
			for entry in re.finditer( r"ON_COMMAND(?:_EX)?\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)", m.group( 2 ) ):
				handlers.setdefault( entry.group( 1 ), [] ).append( "%s::%s" % ( cls, entry.group( 2 ) ) )
			for entry in re.finditer(
					r"ON_COMMAND(?:_EX)?_RANGE\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)", m.group( 2 ) ):
				handlers.setdefault( entry.group( 1 ), [] ).append(
					"%s::%s (range to %s)" % ( cls, entry.group( 3 ), entry.group( 2 ) ) )
		for m in re.finditer( r"class\s+(\w+)\s*:[^{]*\{(.*?)\n\};", text, re.S ):
			for idd in re.findall( r"IDD\s*=\s*(IDD_\w+)", m.group( 2 ) ):
				dialog_classes.setdefault( idd, [] ).append( m.group( 1 ) )
			tool = re.search( r"GetToolID\s*\(\s*(?:void)?\s*\)\s*\{\s*return\s*\(?\s*(TOOL_\w+)", m.group( 2 ) )
			if tool:
				tool_classes.setdefault( tool.group( 1 ), [] ).append( m.group( 1 ) )
	return handlers, dialog_classes, tool_classes


def extract( revision ):
	rc = git_text( revision, "hammer/hammer.rc" )
	menus, accels, toolbars = parse_menus( rc ), parse_accelerators( rc ), parse_toolbars( rc )
	dialogs = parse_dialogs( rc )
	handlers, dialog_classes, tool_classes = parse_sources( revision )
	features = {}
	ids = set( menus ) | set( accels ) | set( toolbars ) | { i for i in handlers if i.startswith( "ID_" ) }
	for cid in sorted( ids ):
		features[ "cmd:" + cid ] = {
			"menus": menus.get( cid, [] ), "accelerators": accels.get( cid, [] ),
			"toolbars": toolbars.get( cid, [] ), "handlers": handlers.get( cid, [] ) }
	for idd in sorted( dialogs ):
		features[ "dlg:" + idd ] = { "caption": dialogs[ idd ], "classes": dialog_classes.get( idd, [] ) }
	enum = re.search( r"enum ToolID_t\s*\{(.*?)\};", git_text( revision, "hammer/toolinterface.h" ), re.S )
	for tid in re.findall( r"(TOOL_\w+)", enum.group( 1 ) ):
		if tid != "TOOL_NONE":
			features[ "tool:" + tid ] = { "classes": tool_classes.get( tid, [] ) }
	return features


# ---- Verification ----------------------------------------------------------------------

def new_stack_names( root ):
	catalog = ( root / "hammer/core/presenters/action_catalog.cpp" ).read_text()
	actions = set( re.findall( r'(?:Cmd|HostAction|ToolAction)\(\s*"([a-z0-9_.]+)"', catalog ) )
	commands_text = ( root / "hammer/core/app/session_commands.cpp" ).read_text()
	commands = set( re.findall( r'\{\s*\{\s*"([a-z0-9_]+)"', commands_text ) )
	manifest = json.loads( ( root / "quality/conformance.manifest.json" ).read_text() )
	suites = { s[ "id" ] for s in manifest[ "suites" ] }
	return actions, commands, suites


def verify_mapping( features, mapping, names, complete=False ):
	actions, commands, suites = names
	errors = []
	mapped = mapping.get( "features", {} )
	for key in features:
		if key not in mapped:
			errors.append( "%s: not mapped" % key )
	for key, entry in mapped.items():
		if key not in features:
			errors.append( "%s: not a legacy feature at the pinned revision" % key )
			continue
		status = entry.get( "status" )
		if status not in STATUSES:
			errors.append( "%s: invalid status %r" % ( key, status ) )
			continue
		for action in entry.get( "actions", [] ):
			if action not in actions:
				errors.append( "%s: unknown action %r" % ( key, action ) )
		for command in entry.get( "commands", [] ):
			if command not in commands:
				errors.append( "%s: unknown command %r" % ( key, command ) )
		for test in entry.get( "tests", [] ):
			if test.split( ":" )[ 0 ] not in suites:
				errors.append( "%s: unknown suite %r" % ( key, test ) )
		if status == "replicated":
			if not ( entry.get( "actions" ) or entry.get( "commands" ) or entry.get( "host" ) ):
				errors.append( "%s: replicated without an action, command or host entry" % key )
			if not entry.get( "tests" ):
				errors.append( "%s: replicated without a test" % key )
		if status in ( "partial", "missing", "not-applicable" ) and not entry.get( "note" ):
			errors.append( "%s: %s without a note" % ( key, status ) )
		if complete and status in ( "partial", "missing" ):
			errors.append( "%s: %s" % ( key, status ) )
	return errors


def load_mapping( path=MAPPING ):
	return json.loads( pathlib.Path( path ).read_text() )


def counts( mapping ):
	result = { s: 0 for s in STATUSES }
	for entry in mapping.get( "features", {} ).values():
		result[ entry.get( "status" ) ] = result.get( entry.get( "status" ), 0 ) + 1
	return result


# ---- Self-test ---------------------------------------------------------------------------

def selftest():
	features = { "cmd:ID_A": {}, "dlg:IDD_B": {}, "tool:TOOL_C": {} }
	names = ( { "edit.a" }, { "do_a" }, { "suite.a" } )
	good = { "features": {
		"cmd:ID_A": { "status": "replicated", "actions": [ "edit.a" ], "commands": [ "do_a" ],
			"tests": [ "suite.a" ] },
		"dlg:IDD_B": { "status": "not-applicable", "note": "Windows only" },
		"tool:TOOL_C": { "status": "missing", "note": "not started" } } }
	cases = [ ( "good mapping passes", good, False, True ),
		( "complete flags a missing feature", good, True, False ) ]

	def bad( mutate ):
		data = json.loads( json.dumps( good ) )
		mutate( data[ "features" ] )
		return data
	cases += [
		( "unmapped feature", bad( lambda f: f.pop( "tool:TOOL_C" ) ), False, False ),
		( "unknown feature", bad( lambda f: f.update( { "cmd:ID_Z": { "status": "missing",
			"note": "x" } } ) ), False, False ),
		( "invalid status", bad( lambda f: f[ "tool:TOOL_C" ].update( status="done" ) ), False, False ),
		( "unknown action", bad( lambda f: f[ "cmd:ID_A" ].update( actions=[ "edit.z" ] ) ), False, False ),
		( "unknown command", bad( lambda f: f[ "cmd:ID_A" ].update( commands=[ "do_z" ] ) ), False, False ),
		( "unknown suite", bad( lambda f: f[ "cmd:ID_A" ].update( tests=[ "suite.z" ] ) ), False, False ),
		( "replicated without a test", bad( lambda f: f[ "cmd:ID_A" ].pop( "tests" ) ), False, False ),
		( "replicated without a target", bad( lambda f: [ f[ "cmd:ID_A" ].pop( k ) for k in
			( "actions", "commands" ) ] ), False, False ),
		( "not-applicable without a reason", bad( lambda f: f[ "dlg:IDD_B" ].pop( "note" ) ),
			False, False ) ]
	failures = 0
	for name, data, complete, should_pass in cases:
		passed = not verify_mapping( features, data, names, complete )
		if passed != should_pass:
			failures += 1
			print( "FAIL %s" % name )
	# The extractor reads the pinned legacy sources.
	mapping = load_mapping()
	extracted = extract( mapping[ "revision" ] )
	checks = [ ( "the extractor finds the File menu's Save", "cmd:ID_FILE_SAVE" in extracted and
		extracted[ "cmd:ID_FILE_SAVE" ][ "menus" ] ),
		( "the extractor finds accelerators", "Ctrl+S" in extracted.get( "cmd:ID_FILE_SAVE", {} ).get(
			"accelerators", [] ) ),
		( "the extractor finds handlers", any( "CMapDoc::" in h for h in extracted.get(
			"cmd:ID_TOOLS_HOLLOW", {} ).get( "handlers", [] ) ) ),
		( "the extractor finds dialogs", extracted.get( "dlg:IDD_REPLACETEX", {} ).get( "caption" )
			== "Replace Textures" ),
		( "the extractor finds tools", "tool:TOOL_MORPH" in extracted ) ]
	for name, ok in checks:
		if not ok:
			failures += 1
			print( "FAIL %s" % name )
	print( "CONFORMANCE %d %d" % ( len( cases ) + len( checks ), failures ) )
	return 1 if failures else 0


def main():
	parser = argparse.ArgumentParser( description=__doc__.splitlines()[ 0 ] )
	parser.add_argument( "action", choices=[ "extract", "verify", "report", "selftest" ] )
	parser.add_argument( "--complete", action="store_true" )
	parser.add_argument( "--revision" )
	args = parser.parse_args()
	if args.action == "selftest":
		return selftest()
	mapping = load_mapping() if MAPPING.exists() else { "features": {} }
	revision = args.revision or mapping.get( "revision" )
	features = extract( revision )
	if args.action == "extract":
		print( json.dumps( features, indent=1 ) )
		return 0
	if args.action == "report":
		print( json.dumps( counts( mapping ), indent=1 ) )
		for key, entry in sorted( mapping.get( "features", {} ).items() ):
			if entry.get( "status" ) in ( "partial", "missing" ):
				print( "%-12s %-50s %s" % ( entry[ "status" ], key, entry.get( "note", "" )[ :90 ] ) )
		return 0
	errors = verify_mapping( features, mapping, new_stack_names( ROOT ), args.complete )
	for error in errors:
		print( "FAIL %s" % error )
	summary = counts( mapping )
	print( "hammer feature parity: %d features; %s" % ( len( features ), ", ".join(
		"%s %d" % ( k, v ) for k, v in summary.items() ) ) )
	print( "CONFORMANCE %d %d" % ( len( features ), len( errors ) ) )
	return 1 if errors else 0


if __name__ == "__main__":
	sys.exit( main() )
