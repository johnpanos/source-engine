//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.groups.v1. Pins the properties a naive
//			implementation gets wrong: selecting a solid in a NESTED group pulls in
//			siblings under the shared TOP-LEVEL group (a direct-group-only expand
//			fails); an ungrouped solid selects only itself (a pull-all expand
//			fails); and a cyclic group parent chain terminates. Exit 0 when all hold.
//
//=============================================================================//

#include "hammer/formats/groups.h"
#include "hammer/formats/keyvalues.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using hammer::formats::ExpandGroupSelection;
using hammer::formats::ParseKeyValues;

namespace
{
int g_failures = 0;

void Check( bool ok, const char *label )
{
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

bool Has( const std::vector<int> &v, int x )
{
	return std::find( v.begin(), v.end(), x ) != v.end();
}

// group 1 (top) -> group 2 (child). solid 10 in group 2, solid 11 in group 1,
// solid 12 ungrouped.
const char *kNested =
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    "\tgroup\n\t{\n\t\t\"id\" \"1\"\n\t}\n"
    "\tgroup\n\t{\n\t\t\"id\" \"2\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"10\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"2\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"11\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"12\"\n\t}\n"
    "}\n";

// Cyclic parent chain: group 1 -> 2 -> 1. Solid 20 in group 1.
const char *kCyclic =
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    "\tgroup\n\t{\n\t\t\"id\" \"1\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"2\"\n\t\t}\n\t}\n"
    "\tgroup\n\t{\n\t\t\"id\" \"2\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"20\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "}\n";

} // namespace

int main()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kNested );
	Check( pr.ok, "nested map parses" );

	// NESTED top-level resolution: selecting 10 (direct group 2) must pull in 11
	// (direct group 1) because both resolve to top-level group 1. A direct-group-
	// only implementation returns just {10} and fails here.
	{
		std::vector<int> r = ExpandGroupSelection( pr.root, { 10 } );
		Check( r.size() == 2 && Has( r, 10 ) && Has( r, 11 ),
		    "nested: select 10 pulls in sibling 11 via shared top-level group" );
	}

	// Ungrouped solid selects only itself (a pull-all implementation fails).
	{
		std::vector<int> r = ExpandGroupSelection( pr.root, { 12 } );
		Check( r.size() == 1 && Has( r, 12 ), "ungrouped 12 selects only itself" );
	}

	// Cyclic parent chain must terminate (no infinite loop) and stay consistent.
	{
		hammer::formats::ParseResult pc = ParseKeyValues( kCyclic );
		Check( pc.ok, "cyclic map parses" );
		std::vector<int> r = ExpandGroupSelection( pc.root, { 20 } );
		Check( Has( r, 20 ), "cyclic group chain terminates and keeps the selection" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.groups negative: ORACLE UNSOUND (%d)\n", g_failures );
		return 1;
	}
	std::printf( "formats.groups negative: nested top-level expansion, ungrouped singularity, and "
	             "cyclic-chain termination all correct\n" );
	return 0;
}
