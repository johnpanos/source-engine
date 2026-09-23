//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats VisGroups (formats.visgroups.v1).
//			Parses a nested visgroup tree and checks ApplyVisGroupFilter: hiding a
//			parent group hides objects in its descendant groups, other objects and
//			unassigned objects stay, and counts are right. Build/run via the
//			conformance manifest. Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/visgroups.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>
#include <vector>

using hammer::formats::ApplyVisGroupFilter;
using hammer::formats::ParseKeyValues;
using hammer::formats::ParseVisGroups;
using hammer::formats::VisGroup;
using hammer::formats::VisGroupFilterResult;

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

const char *kMap =
    "visgroups\n{\n"
    "\tvisgroup\n\t{\n\t\t\"name\" \"Detail\"\n\t\t\"visgroupid\" \"1\"\n"
    "\t\tvisgroup\n\t\t{\n\t\t\t\"name\" \"Trees\"\n\t\t\t\"visgroupid\" \"2\"\n\t\t}\n"
    "\t}\n"
    "\tvisgroup\n\t{\n\t\t\"name\" \"Lights\"\n\t\t\"visgroupid\" \"3\"\n\t}\n"
    "}\n"
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    // solid A in Trees (2), B in Lights (3), C no membership.
    "\tsolid\n\t{\n\t\t\"id\" \"10\"\n\t\teditor\n\t\t{\n\t\t\t\"visgroupid\" "
    "\"2\"\n\t\t}\n\t\tside"
    "\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) (16 16 0)\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"11\"\n\t\teditor\n\t\t{\n\t\t\t\"visgroupid\" "
    "\"3\"\n\t\t}\n\t\tside"
    "\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) (16 16 0)\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"12\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) "
    "(16 16 0)\"\n\t\t}\n\t}\n"
    "}\n"
    // entity in Lights (3).
    "entity\n{\n\t\"classname\" \"light\"\n\t\"origin\" \"0 0 0\"\n\teditor\n\t{\n\t\t"
    "\"visgroupid\" \"3\"\n\t}\n}\n";

void TestParseTree()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	Check( pr.ok, "map parses" );
	std::vector<VisGroup> tree = ParseVisGroups( pr.root );
	Check( tree.size() == 2, "two top-level visgroups (Detail, Lights)" );
	if ( tree.size() == 2 )
	{
		Check( tree[0].name == "Detail" && tree[0].id == 1, "Detail id 1" );
		Check( tree[0].children.size() == 1 && tree[0].children[0].id == 2 &&
		           tree[0].children[0].name == "Trees",
		    "Detail has child Trees id 2" );
		Check( tree[1].name == "Lights" && tree[1].id == 3, "Lights id 3" );
	}
}

void TestHideParentHidesChildMembers()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	// Hide "Detail" (1); its child "Trees" (2) is expanded in, so solid A (in 2) hides.
	VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, { 1 } );
	Check( r.hiddenObjects == 1, "hiding Detail hides the one Trees member (descendant expand)" );
	Check( r.visibleObjects == 3, "solid B, solid C, and the light stay visible" );
}

void TestHideLeaf()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	// Hide "Lights" (3): solid B and the light entity hide.
	VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, { 3 } );
	Check( r.hiddenObjects == 2, "hiding Lights hides solid B and the light entity" );
	Check( r.visibleObjects == 2, "solid A (Trees) and unassigned solid C stay" );
}

void TestEmptyKeepsAll()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, {} );
	Check( r.hiddenObjects == 0, "no hidden groups -> nothing hidden" );
	Check( r.visibleObjects == 4, "all four objects visible" );
}

} // namespace

int main()
{
	TestParseTree();
	TestHideParentHidesChildMembers();
	TestHideLeaf();
	TestEmptyKeepsAll();

	if ( g_failures != 0 )
	{
		std::printf( "formats.visgroups: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.visgroups: nested tree parses; hiding a parent hides descendant members; "
	             "unassigned objects stay; empty set keeps all\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
