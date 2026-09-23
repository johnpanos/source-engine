//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.cordon.v1. Proves ApplyCordon actually
//			discriminates by position: an all-encompassing box removes NOTHING, an
//			empty (inverted) box removes ALL positional geometry, and a far-away box
//			removes everything. A constant keep-all implementation fails the empty
//			box; a constant remove-all fails the universe box. Exit 0 when all hold.
//
//=============================================================================//

#include "hammer/formats/cordon.h"
#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::ApplyCordon;
using hammer::formats::CordonBox;
using hammer::formats::CordonResult;
using hammer::formats::ParseKeyValues;
using hammer::geometry::Vec3d;

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

std::string SolidAABB( int id, double x0, double y0, double z0, double x1, double y1, double z1 )
{
	char buf[256];
	std::snprintf( buf, sizeof( buf ),
	    "\tsolid\n\t{\n\t\t\"id\" \"%d\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(%g %g %g) "
	    "(%g %g %g) (%g %g %g)\"\n\t\t}\n\t}\n",
	    id, x0, y0, z0, x1, y0, z0, x1, y1, z1 );
	return buf;
}

CordonResult Run( const std::string &vmf, const CordonBox &box )
{
	hammer::formats::ParseResult pr = ParseKeyValues( vmf );
	CordonResult empty;
	++g_checks;
	if ( !pr.ok )
	{
		std::printf( "FAIL: fixture parse\n" );
		++g_failures;
		return empty;
	}
	return ApplyCordon( pr.root, box );
}

std::string Map()
{
	std::string world = "world\n{\n\t\"classname\" \"worldspawn\"\n";
	world += SolidAABB( 1, 0, 0, 0, 16, 16, 16 );
	world += SolidAABB( 2, 100, 100, 100, 116, 116, 116 );
	world += "}\n";
	world += "entity\n{\n\t\"classname\" \"info_target\"\n\t\"origin\" \"8 8 8\"\n}\n";
	return world;
}

} // namespace

int main()
{
	// Universe box: nothing is removed.
	{
		CordonBox box{ Vec3d( -100000, -100000, -100000 ), Vec3d( 100000, 100000, 100000 ) };
		CordonResult r = Run( Map(), box );
		Check( r.solidsRemoved == 0 && r.solidsKept == 2,
		    "universe box keeps all solids (remove-all impl would fail)" );
		Check( r.entitiesRemoved == 0, "universe box keeps all entities" );
	}

	// Empty (inverted) box: all positional geometry removed.
	{
		CordonBox box{ Vec3d( 64, 64, 64 ), Vec3d( 0, 0, 0 ) };
		CordonResult r = Run( Map(), box );
		Check( r.solidsKept == 0 && r.solidsRemoved == 2,
		    "inverted box removes all solids (keep-all impl would fail)" );
		Check( r.entitiesRemoved == 1, "inverted box removes the point entity" );
	}

	// Far-away small box: everything removed.
	{
		CordonBox box{ Vec3d( 9000, 9000, 9000 ), Vec3d( 9001, 9001, 9001 ) };
		CordonResult r = Run( Map(), box );
		Check( r.solidsKept == 0, "far box removes all solids" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.cordon negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.cordon negative: filter discriminates by position (universe keeps all, "
	             "empty/far box removes all)\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
