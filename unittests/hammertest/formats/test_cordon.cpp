//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats cordon filtering
//			(formats.cordon.v1). Builds a VMF with solids inside/outside/straddling
//			a cordon box and point/brush/positionless entities, then checks
//			ApplyCordon keeps exactly the geometry that belongs in the box.
//			Build/run via the conformance manifest. Exit 0 on success.
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
using hammer::formats::KeyValueNode;
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

// A solid with one side whose three plane points span the AABB (x0,y0,z0)..(x1,y1,z1).
std::string SolidAABB( int id, double x0, double y0, double z0, double x1, double y1, double z1 )
{
	char buf[256];
	std::snprintf( buf, sizeof( buf ),
	    "\tsolid\n\t{\n\t\t\"id\" \"%d\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(%g %g %g) "
	    "(%g %g %g) (%g %g %g)\"\n\t\t}\n\t}\n",
	    id, x0, y0, z0, x1, y0, z0, x1, y1, z1 );
	return buf;
}

std::string PointEntity( const char *classname, const char *origin )
{
	std::string e = "entity\n{\n\t\"classname\" \"";
	e += classname;
	e += "\"\n\t\"origin\" \"";
	e += origin;
	e += "\"\n}\n";
	return e;
}

CordonResult Run( const std::string &vmf, const CordonBox &box )
{
	hammer::formats::ParseResult pr = ParseKeyValues( vmf );
	++g_checks;
	if ( !pr.ok )
	{
		std::printf( "FAIL: fixture parse: %s\n", pr.error.c_str() );
		++g_failures;
		return CordonResult{};
	}
	return ApplyCordon( pr.root, box );
}

std::size_t WorldSolidCount( const KeyValueNode &doc )
{
	std::size_t n = 0;
	for ( const KeyValueNode &b : doc.children )
	{
		if ( b.name == "world" )
		{
			for ( const KeyValueNode &c : b.children )
			{
				if ( c.name == "solid" )
				{
					++n;
				}
			}
		}
	}
	return n;
}

std::string BuildMap()
{
	// world: one inside solid, one outside, one straddling.
	std::string world = "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n";
	world += SolidAABB( 10, 16, 16, 16, 48, 48, 48 );       // inside 0..64
	world += SolidAABB( 11, 100, 100, 100, 120, 120, 120 ); // outside
	world += SolidAABB( 12, -10, -10, -10, 10, 10, 10 );    // straddling the min corner
	world += "}\n";

	std::string doc = "versioninfo\n{\n\t\"editorversion\" \"400\"\n}\n";
	doc += world;
	doc += PointEntity( "info_player_start", "32 32 32" );   // inside
	doc += PointEntity( "info_target", "200 0 0" );          // outside
	doc += "entity\n{\n\t\"classname\" \"logic_auto\"\n}\n"; // positionless -> kept
	return doc;
}

void TestFilter()
{
	CordonBox box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) };
	CordonResult r = Run( BuildMap(), box );

	Check( r.solidsKept == 2, "inside + straddling solids kept" );
	Check( r.solidsRemoved == 1, "outside solid removed" );
	Check( WorldSolidCount( r.document ) == 2, "world has 2 solids after cordon" );
	// info_player_start (inside) + logic_auto (positionless) kept; info_target removed.
	Check( r.entitiesKept == 2, "inside point entity + positionless entity kept" );
	Check( r.entitiesRemoved == 1, "outside point entity removed" );
	// versioninfo preserved.
	bool haveVersion = false;
	for ( const KeyValueNode &b : r.document.children )
	{
		haveVersion = haveVersion || b.name == "versioninfo";
	}
	Check( haveVersion, "versioninfo preserved verbatim" );
}

void TestBoundaryTouch()
{
	// A solid whose max corner exactly touches the box min is kept (inclusive edge).
	std::string doc = "world\n{\n\t\"classname\" \"worldspawn\"\n" +
	                  SolidAABB( 1, -20, -20, -20, 0, 0, 0 ) + "}\n";
	CordonBox box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) };
	CordonResult r = Run( doc, box );
	Check( r.solidsKept == 1 && r.solidsRemoved == 0, "edge-touching solid kept (inclusive)" );
}

} // namespace

int main()
{
	TestFilter();
	TestBoundaryTouch();

	if ( g_failures != 0 )
	{
		std::printf( "formats.cordon: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.cordon: solids/entities inside the box are kept, outside removed, "
	             "straddling/edge-touching kept, metadata preserved\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
