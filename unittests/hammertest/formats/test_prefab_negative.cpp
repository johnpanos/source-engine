//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.prefab.v1. Pins that instantiation
//			actually applies its placement (a no-op implementation fails the
//			translate check), an identity placement changes nothing (a spurious
//			transform fails), and solid/entity counts are always preserved (a
//			dropping implementation fails). Exit 0 when all hold.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/prefab.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <string>

using hammer::formats::InstantiatePrefab;
using hammer::formats::KeyValueNode;
using hammer::formats::ParseKeyValues;
using hammer::formats::Placement;
using hammer::formats::PrefabInstance;

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

bool Near( double a, double b )
{
	return std::fabs( a - b ) < 1e-4;
}

bool FirstSolidX( const PrefabInstance &inst, double &x )
{
	if ( inst.solids.empty() )
	{
		return false;
	}
	for ( const KeyValueNode &side : inst.solids.front().children )
	{
		if ( side.name != "side" )
		{
			continue;
		}
		if ( const std::string *plane = side.Find( "plane" ) )
		{
			std::string clean = *plane;
			for ( char &c : clean )
			{
				if ( c == '(' || c == ')' )
				{
					c = ' ';
				}
			}
			double a[9] = { 0 };
			if ( std::sscanf( clean.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &a[0], &a[1],
			         &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8] ) == 9 )
			{
				x = a[0];
				return true;
			}
		}
	}
	return false;
}

const char *kPrefab =
    "world\n{\n\t\"classname\" \"worldspawn\"\n"
    "\tsolid\n\t{\n\t\t\"id\" \"1\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) "
    "(16 16 0)\"\n\t\t}\n\t}\n"
    "}\n"
    "entity\n{\n\t\"classname\" \"prop_static\"\n\t\"origin\" \"8 8 8\"\n}\n";

PrefabInstance Instantiate( const Placement &at )
{
	hammer::formats::ParseResult pr = ParseKeyValues( kPrefab );
	Check( pr.ok, "prefab fixture parses" );
	return InstantiatePrefab( pr.root, at );
}

} // namespace

int main()
{
	// Identity placement changes nothing: the solid's first point stays (0,0,0).
	{
		PrefabInstance inst = Instantiate( { { 0, 0, 0 }, 0, 0, 0 } );
		Check( inst.solids.size() == 1 && inst.entities.size() == 1, "identity: counts preserved" );
		double x = 999;
		Check( FirstSolidX( inst, x ) && Near( x, 0 ), "identity leaves the solid point at 0" );
	}

	// A translated placement actually moves geometry (a no-op impl fails here).
	{
		PrefabInstance inst = Instantiate( { { 100, 0, 0 }, 0, 0, 0 } );
		Check(
		    inst.solids.size() == 1 && inst.entities.size() == 1, "translate: counts preserved" );
		double x = 0;
		Check( FirstSolidX( inst, x ) && Near( x, 100 ), "translate moves the solid point to 100" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.prefab negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf(
	    "formats.prefab negative: identity preserves, translate moves, counts preserved\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
