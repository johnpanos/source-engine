//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats map export/compile preparation
//			(formats.map_export.v1). Verifies PrepareForCompile composes the cores
//			in order: func_instances expand, hidden-VisGroup objects drop, and the
//			cordon clips the rest -- with the right counts and surviving solids.
//			Build/run via the conformance manifest. Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/map_export.h"
#include "testing/conformance_result.h"

#include "app/fake_file_store.h"

#include <cstdio>
#include <string>

using hammer::formats::CompileOptions;
using hammer::formats::CompilePrepResult;
using hammer::formats::CordonBox;
using hammer::formats::KeyValueNode;
using hammer::formats::ParseKeyValues;
using hammer::formats::PrepareForCompile;
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

std::string SolidAt( int id, int visgroup, double x0 )
{
	std::string s = "\tsolid\n\t{\n\t\t\"id\" \"" + std::to_string( id ) + "\"\n";
	if ( visgroup >= 0 )
	{
		s += "\t\teditor\n\t\t{\n\t\t\t\"visgroupid\" \"" + std::to_string( visgroup ) +
		     "\"\n\t\t}\n";
	}
	char plane[160];
	std::snprintf( plane, sizeof( plane ),
	    "\t\tside\n\t\t{\n\t\t\t\"plane\" \"(%g 0 0) (%g 0 0) "
	    "(%g 16 0)\"\n\t\t}\n",
	    x0, x0 + 16, x0 + 16 );
	s += plane;
	s += "\t}\n";
	return s;
}

std::size_t WorldSolids( const KeyValueNode &doc )
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

// Parent: visgroup 5 "Hidden"; solids 1 (visible, inside), 2 (visgroup 5), 3
// (visible, far outside); a func_instance of child.vmf at the origin.
std::string ParentMap()
{
	std::string m = "visgroups\n{\n\tvisgroup\n\t{\n\t\t\"name\" \"Hidden\"\n\t\t\"visgroupid\" "
	                "\"5\"\n\t}\n}\n";
	m += "world\n{\n\t\"classname\" \"worldspawn\"\n";
	m += SolidAt( 1, -1, 0 );   // inside cordon, visible
	m += SolidAt( 2, 5, 0 );    // hidden visgroup
	m += SolidAt( 3, -1, 500 ); // outside cordon
	m += "}\n";
	m += "entity\n{\n\t\"classname\" \"func_instance\"\n\t\"file\" \"child.vmf\"\n"
	     "\t\"origin\" \"0 0 0\"\n\t\"angles\" \"0 0 0\"\n}\n";
	return m;
}

const char *kChild = "world\n{\n\t\"classname\" \"worldspawn\"\n"
                     "\tsolid\n\t{\n\t\t\"id\" \"9\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) "
                     "(16 0 0) (16 16 0)\"\n\t\t}\n\t}\n}\n";

void TestFullPipeline()
{
	hammertest::InMemoryFileStore store;
	store.Write( "child.vmf", kChild );
	hammer::formats::ParseResult pr = ParseKeyValues( ParentMap() );
	Check( pr.ok, "parent map parses" );

	CompileOptions opt;
	opt.hiddenVisGroups = { 5 };
	opt.useCordon = true;
	opt.cordon = CordonBox{ Vec3d( -8, -8, -8 ), Vec3d( 64, 64, 64 ) };

	CompilePrepResult r = PrepareForCompile( pr.root, store, opt );
	Check( r.ok, "prepare ok" );
	Check( r.instancesExpanded == 1, "one func_instance expanded" );
	Check( !HasFuncInstance( r.document ), "no func_instance remains" );
	Check( r.solidsRemovedByVisGroup == 1, "hidden-visgroup solid (2) dropped" );
	Check( r.solidsRemovedByCordon == 1, "out-of-cordon solid (3) clipped" );
	// Survivors: solid 1 (visible, inside) + child solid 9 (merged, inside) = 2.
	Check( WorldSolids( r.document ) == 2, "two solids survive (1 and merged 9)" );
}

void TestNoCordon()
{
	hammertest::InMemoryFileStore store;
	store.Write( "child.vmf", kChild );
	hammer::formats::ParseResult pr = ParseKeyValues( ParentMap() );

	CompileOptions opt;
	opt.hiddenVisGroups = { 5 };
	opt.useCordon = false; // keep the far solid

	CompilePrepResult r = PrepareForCompile( pr.root, store, opt );
	Check( r.ok, "no-cordon prepare ok" );
	Check( r.solidsRemovedByCordon == 0, "no cordon -> nothing clipped" );
	// Survivors: solids 1, 3, and merged 9 (only 2 dropped by visgroup).
	Check( WorldSolids( r.document ) == 3, "three solids survive without cordon" );
}

} // namespace

int main()
{
	TestFullPipeline();
	TestNoCordon();

	if ( g_failures != 0 )
	{
		std::printf( "formats.map_export: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.map_export: instance-expand + visgroup-drop + cordon-clip compose in "
	             "order with correct counts and survivors\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
