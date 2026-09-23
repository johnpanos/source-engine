//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats func_instance expansion
//			(formats.instancing.v1). Uses the in-memory IFileStore fake to hold
//			child VMFs, then checks ExpandInstances: instances are replaced by
//			their merged contents, world solids are transformed (translate + yaw
//			rotation) into the parent, nested instances compose, and the count is
//			right. Build/run via the conformance manifest. Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/instancing.h"
#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include "app/fake_file_store.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using hammer::formats::ExpandInstances;
using hammer::formats::InstanceExpandResult;
using hammer::formats::KeyValueNode;
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

bool Near( double a, double b )
{
	return std::fabs( a - b ) < 1e-4;
}

// A one-sided solid whose single plane's first point is 'px py pz' (enough to
// track how a solid's geometry is transformed; expansion does not require closure).
std::string Solid( int id, double px, double py, double pz )
{
	char buf[256];
	std::snprintf( buf, sizeof( buf ),
	    "\tsolid\n\t{\n\t\t\"id\" \"%d\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(%g %g %g) "
	    "(%g %g %g) (%g %g %g)\"\n\t\t\t\"material\" \"DEV/DEV\"\n\t\t}\n\t}\n",
	    id, px, py, pz, px + 16, py, pz, px + 16, py + 16, pz );
	return buf;
}

std::string WorldWith( const std::string &solids )
{
	return "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n" + solids + "}\n";
}

std::string Instance( int id, const char *file, const char *origin, const char *angles )
{
	std::string e = "entity\n{\n\t\"id\" \"" + std::to_string( id ) + "\"\n";
	e += "\t\"classname\" \"func_instance\"\n";
	e += std::string( "\t\"file\" \"" ) + file + "\"\n";
	e += std::string( "\t\"origin\" \"" ) + origin + "\"\n";
	e += std::string( "\t\"angles\" \"" ) + angles + "\"\n}\n";
	return e;
}

// Collects the first plane's first point of every world solid in a document.
struct P3
{
	double x, y, z;
};
std::vector<P3> WorldSolidFirstPoints( const KeyValueNode &doc )
{
	std::vector<P3> out;
	for ( const KeyValueNode &b : doc.children )
	{
		if ( b.name != "world" )
		{
			continue;
		}
		for ( const KeyValueNode &s : b.children )
		{
			if ( s.name != "solid" )
			{
				continue;
			}
			for ( const KeyValueNode &side : s.children )
			{
				if ( side.name != "side" )
				{
					continue;
				}
				if ( const std::string *plane = side.Find( "plane" ) )
				{
					double a[9] = { 0 };
					std::string clean = *plane;
					for ( char &c : clean )
					{
						if ( c == '(' || c == ')' )
						{
							c = ' ';
						}
					}
					if ( std::sscanf( clean.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &a[0],
					         &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8] ) == 9 )
					{
						out.push_back( { a[0], a[1], a[2] } );
					}
				}
				break; // first side only
			}
		}
	}
	return out;
}

bool HasFuncInstance( const KeyValueNode &doc )
{
	for ( const KeyValueNode &b : doc.children )
	{
		if ( b.name == "entity" )
		{
			if ( const std::string *cls = b.Find( "classname" ) )
			{
				if ( *cls == "func_instance" )
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool HasPoint( const std::vector<P3> &pts, double x, double y, double z )
{
	for ( const P3 &p : pts )
	{
		if ( Near( p.x, x ) && Near( p.y, y ) && Near( p.z, z ) )
		{
			return true;
		}
	}
	return false;
}

InstanceExpandResult Expand( const std::string &parentText, hammertest::InMemoryFileStore &store )
{
	hammer::formats::ParseResult pr = ParseKeyValues( parentText );
	if ( !pr.ok )
	{
		InstanceExpandResult bad;
		bad.ok = false;
		bad.error = "parent parse failed: " + pr.error;
		return bad;
	}
	return ExpandInstances( pr.root, store );
}

void TestTranslate()
{
	hammertest::InMemoryFileStore store;
	store.Write( "child.vmf", WorldWith( Solid( 5, 0, 0, 0 ) ) );

	const std::string parent =
	    WorldWith( Solid( 1, 0, 0, 0 ) ) + Instance( 2, "child.vmf", "64 0 0", "0 0 0" );
	InstanceExpandResult r = Expand( parent, store );

	Check( r.ok, "translate: ok" );
	Check( r.expanded == 1, "translate: expanded == 1" );
	Check( !HasFuncInstance( r.document ), "translate: func_instance removed" );
	auto pts = WorldSolidFirstPoints( r.document );
	Check( pts.size() == 2, "translate: two world solids merged" );
	Check( HasPoint( pts, 0, 0, 0 ), "translate: parent solid intact" );
	Check( HasPoint( pts, 64, 0, 0 ), "translate: child solid moved by origin" );
}

void TestYaw90()
{
	hammertest::InMemoryFileStore store;
	// Child solid's first plane point at (16,0,0).
	store.Write( "child.vmf", WorldWith( Solid( 5, 16, 0, 0 ) ) );

	const std::string parent =
	    WorldWith( Solid( 1, 0, 0, 0 ) ) + Instance( 2, "child.vmf", "0 0 0", "0 90 0" );
	InstanceExpandResult r = Expand( parent, store );

	Check( r.ok, "yaw: ok" );
	auto pts = WorldSolidFirstPoints( r.document );
	// Yaw 90 about Z: (16,0,0) -> (0,16,0).
	Check( HasPoint( pts, 0, 16, 0 ), "yaw: child point rotated 90 about Z" );
}

void TestNested()
{
	hammertest::InMemoryFileStore store;
	// b.vmf solid at origin; a.vmf places b at +100 x; parent places a at +5 x.
	store.Write( "b.vmf", WorldWith( Solid( 9, 0, 0, 0 ) ) );
	store.Write(
	    "a.vmf", WorldWith( Solid( 8, 0, 0, 0 ) ) + Instance( 7, "b.vmf", "100 0 0", "0 0 0" ) );

	const std::string parent =
	    WorldWith( Solid( 1, 0, 0, 0 ) ) + Instance( 2, "a.vmf", "5 0 0", "0 0 0" );
	InstanceExpandResult r = Expand( parent, store );

	Check( r.ok, "nested: ok" );
	Check( r.expanded == 2, "nested: expanded == 2 (a and b)" );
	Check( !HasFuncInstance( r.document ), "nested: all instances removed" );
	auto pts = WorldSolidFirstPoints( r.document );
	Check( pts.size() == 3, "nested: three solids (parent + a + b)" );
	// b placed at 100 in a's space, then a placed at +5: b -> 105.
	Check( HasPoint( pts, 105, 0, 0 ), "nested: composed transform 100+5" );
	Check( HasPoint( pts, 5, 0, 0 ), "nested: a's own solid at +5" );
}

} // namespace

int main()
{
	TestTranslate();
	TestYaw90();
	TestNested();

	if ( g_failures != 0 )
	{
		std::printf( "formats.instancing: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.instancing: translate + yaw + nested expansion merge world solids and "
	             "remove func_instance correctly\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
