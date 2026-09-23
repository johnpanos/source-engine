//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats prefab instantiation
//			(formats.prefab.v1). Instantiates a prefab fragment at a translation and
//			a yaw rotation and checks the placed solid/entity coordinates and that
//			counts are preserved. Build/run via the conformance manifest. Exit 0 on
//			success.
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

// First plane point of the first solid.
bool FirstSolidPoint( const PrefabInstance &inst, double &x, double &y, double &z )
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
				y = a[1];
				z = a[2];
				return true;
			}
		}
	}
	return false;
}

bool FirstEntityOrigin( const PrefabInstance &inst, double &x, double &y, double &z )
{
	if ( inst.entities.empty() )
	{
		return false;
	}
	if ( const std::string *o = inst.entities.front().Find( "origin" ) )
	{
		return std::sscanf( o->c_str(), "%lf %lf %lf", &x, &y, &z ) == 3;
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
	++g_checks;
	if ( !pr.ok )
	{
		std::printf( "FAIL: prefab fixture parse\n" );
		++g_failures;
		return PrefabInstance{};
	}
	return InstantiatePrefab( pr.root, at );
}

void TestTranslate()
{
	PrefabInstance inst = Instantiate( { { 100, 0, 0 }, 0, 0, 0 } );
	Check( inst.solids.size() == 1 && inst.entities.size() == 1, "translate: counts preserved" );
	double x = 0, y = 0, z = 0;
	Check( FirstSolidPoint( inst, x, y, z ) && Near( x, 100 ) && Near( y, 0 ) && Near( z, 0 ),
	    "translate: solid point (0,0,0) -> (100,0,0)" );
	Check( FirstEntityOrigin( inst, x, y, z ) && Near( x, 108 ) && Near( y, 8 ) && Near( z, 8 ),
	    "translate: entity origin (8,8,8) -> (108,8,8)" );
}

void TestYaw90()
{
	PrefabInstance inst = Instantiate( { { 0, 0, 0 }, 0, 90, 0 } );
	double x = 0, y = 0, z = 0;
	// Plane point (16,0,0) rotated yaw 90 about Z -> (0,16,0).
	Check( FirstSolidPoint( inst, x, y, z ) && Near( x, 0 ) && Near( y, 0 ) && Near( z, 0 ),
	    "yaw: solid first point (0,0,0) stays at origin" );
	// Entity (8,8,8) -> (-8,8,8).
	Check( FirstEntityOrigin( inst, x, y, z ) && Near( x, -8 ) && Near( y, 8 ) && Near( z, 8 ),
	    "yaw: entity (8,8,8) -> (-8,8,8)" );
}

} // namespace

int main()
{
	TestTranslate();
	TestYaw90();

	if ( g_failures != 0 )
	{
		std::printf( "formats.prefab: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.prefab: prefab instantiation places solids + entities by translate and "
	             "yaw through the shared VMF transform; counts preserved\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
