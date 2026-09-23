//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats object groups
//			(formats.groups.v1). Checks group-hierarchy parsing and that
//			ExpandGroupSelection pulls in top-level group siblings (following nested
//			groups), while ungrouped solids select only themselves. Build/run via
//			the conformance manifest. Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/groups.h"
#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using hammer::formats::ExpandGroupSelection;
using hammer::formats::GroupTable;
using hammer::formats::ParseGroups;
using hammer::formats::ParseKeyValues;

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *label )
{
	++g_checks;
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

// group 1 (top), group 2 (child of 1), group 3 (top).
// solids: 10 in group 2, 11 in group 1, 14 in group 3, 13 ungrouped.
const char *kMap =
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    "\tgroup\n\t{\n\t\t\"id\" \"1\"\n\t}\n"
    "\tgroup\n\t{\n\t\t\"id\" \"2\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "\tgroup\n\t{\n\t\t\"id\" \"3\"\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"10\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"2\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"11\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"1\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"14\"\n\t\teditor\n\t\t{\n\t\t\t\"groupid\" \"3\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"13\"\n\t}\n"
    "}\n";

void TestTable()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	Check( pr.ok, "map parses" );
	GroupTable t = ParseGroups( pr.root );
	// Nested: solid 10 is in group 2, whose top-level ancestor is group 1.
	Check( t.TopLevelGroupOfSolid( 10 ) == 1, "solid 10 -> top-level group 1 (via nested 2)" );
	Check( t.TopLevelGroupOfSolid( 11 ) == 1, "solid 11 -> group 1" );
	Check( t.TopLevelGroupOfSolid( 14 ) == 3, "solid 14 -> group 3" );
	Check( t.TopLevelGroupOfSolid( 13 ) == 0, "solid 13 ungrouped" );
}

void TestExpand()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );

	// Selecting 10 (nested group 2) pulls in 11 (group 1) via the shared top-level.
	std::vector<int> a = ExpandGroupSelection( pr.root, { 10 } );
	Check( a.size() == 2 && Has( a, 10 ) && Has( a, 11 ), "select 10 -> {10,11}" );

	// Selecting the lone group-3 solid brings only itself.
	std::vector<int> b = ExpandGroupSelection( pr.root, { 14 } );
	Check( b.size() == 1 && Has( b, 14 ), "select 14 -> {14}" );

	// Ungrouped solid selects only itself.
	std::vector<int> c = ExpandGroupSelection( pr.root, { 13 } );
	Check( c.size() == 1 && Has( c, 13 ), "select 13 (ungrouped) -> {13}" );

	// Multi-select across groups unions the members.
	std::vector<int> d = ExpandGroupSelection( pr.root, { 10, 14 } );
	Check( d.size() == 3 && Has( d, 10 ) && Has( d, 11 ) && Has( d, 14 ),
	    "select {10,14} -> {10,11,14}" );
}

} // namespace

int main()
{
	TestTable();
	TestExpand();

	if ( g_failures != 0 )
	{
		std::printf( "formats.groups: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.groups: nested group hierarchy resolves to top-level; selection expands "
	             "to group siblings; ungrouped stays singular\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
