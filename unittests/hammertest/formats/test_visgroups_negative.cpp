//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.visgroups.v1. Pins the properties a naive
//			implementation gets wrong: hiding a PARENT group must hide members of
//			its child groups (descendant expansion -- a direct-only filter fails);
//			an empty hidden set and an unknown group id must hide NOTHING (a
//			hide-all filter fails); a non-numeric membership is treated as
//			unassigned (visible). Exit 0 when all hold.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/visgroups.h"

#include <cstdio>
#include <string>

using hammer::formats::ApplyVisGroupFilter;
using hammer::formats::ParseKeyValues;
using hammer::formats::VisGroupFilterResult;

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

// Parent group 1 -> child group 2; a solid in group 2; a solid with a non-numeric
// membership; an unassigned solid.
const char *kMap =
    "visgroups\n{\n"
    "\tvisgroup\n\t{\n\t\t\"name\" \"Parent\"\n\t\t\"visgroupid\" \"1\"\n"
    "\t\tvisgroup\n\t\t{\n\t\t\t\"name\" \"Child\"\n\t\t\t\"visgroupid\" \"2\"\n\t\t}\n"
    "\t}\n}\n"
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    "\tsolid\n\t{\n\t\t\"id\" \"10\"\n\t\teditor\n\t\t{\n\t\t\t\"visgroupid\" "
    "\"2\"\n\t\t}\n\t\tside"
    "\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) (16 16 0)\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"11\"\n\t\teditor\n\t\t{\n\t\t\t\"visgroupid\" \"notanumber\"\n\t\t}"
    "\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) (16 16 0)\"\n\t\t}\n\t}\n"
    "\tsolid\n\t{\n\t\t\"id\" \"12\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) "
    "(16 16 0)\"\n\t\t}\n\t}\n"
    "}\n";

} // namespace

int main()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	Check( pr.ok, "map parses" );

	// Hiding the PARENT (1) must hide the child(2)-member solid via descendant
	// expansion. A direct-membership-only implementation would keep it.
	{
		VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, { 1 } );
		Check( r.hiddenObjects == 1, "descendant expansion: parent hides child member" );
	}

	// Empty hidden set hides nothing (a hide-all implementation fails here).
	{
		VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, {} );
		Check( r.hiddenObjects == 0 && r.visibleObjects == 3, "empty set keeps all" );
	}

	// An unknown group id hides nothing.
	{
		VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, { 999 } );
		Check( r.hiddenObjects == 0, "unknown group id hides nothing" );
	}

	// A non-numeric membership is treated as unassigned: hiding group 2 hides only
	// the real member, never the malformed one.
	{
		VisGroupFilterResult r = ApplyVisGroupFilter( pr.root, { 2 } );
		Check( r.hiddenObjects == 1, "non-numeric membership is not hidden" );
		Check( r.visibleObjects == 2, "malformed-membership and unassigned solids stay" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.visgroups negative: ORACLE UNSOUND (%d)\n", g_failures );
		return 1;
	}
	std::printf( "formats.visgroups negative: descendant expansion, empty/unknown keep-all, and "
	             "non-numeric-membership handling all correct\n" );
	return 0;
}
