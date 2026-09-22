//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.instancing.v1. ExpandInstances must FAIL
//			(not loop, not silently drop) on a missing instance file, a cyclic
//			reference, and a func_instance with no "file", while a well-formed
//			instance expands. Otherwise a broken map would flatten to garbage or
//			hang. Exit 0 when the good case expands AND every bad case is reported
//			as an error; exit 1 otherwise.
//
//=============================================================================//

#include "hammer/formats/instancing.h"
#include "hammer/formats/keyvalues.h"

#include "app/fake_file_store.h"

#include <cstdio>
#include <string>

using hammer::formats::ExpandInstances;
using hammer::formats::InstanceExpandResult;
using hammer::formats::ParseKeyValues;

namespace
{
int g_failures = 0;

const char *kMinimalWorld = "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n}\n";

std::string Instance( const char *file, bool withFile )
{
	std::string e = "entity\n{\n\t\"classname\" \"func_instance\"\n";
	if ( withFile )
	{
		e += std::string( "\t\"file\" \"" ) + file + "\"\n";
	}
	e += "\t\"origin\" \"0 0 0\"\n\t\"angles\" \"0 0 0\"\n}\n";
	return e;
}

InstanceExpandResult Expand(
    const std::string &parentText, hammertest::InMemoryFileStore &store, int maxDepth )
{
	hammer::formats::ParseResult pr = ParseKeyValues( parentText );
	InstanceExpandResult r;
	if ( !pr.ok )
	{
		r.ok = false;
		return r;
	}
	return ExpandInstances( pr.root, store, maxDepth );
}

void ExpectOk( const InstanceExpandResult &r, const char *label )
{
	if ( !r.ok )
	{
		std::printf( "FAIL: good case reported error (%s): %s\n", label, r.error.c_str() );
		++g_failures;
	}
}

void ExpectError( const InstanceExpandResult &r, const char *label )
{
	if ( r.ok )
	{
		std::printf( "FAIL: broken case accepted (%s)\n", label );
		++g_failures;
	}
}

} // namespace

int main()
{
	// Soundness: a well-formed instance expands.
	{
		hammertest::InMemoryFileStore store;
		store.Write( "child.vmf", kMinimalWorld );
		std::string parent = std::string( kMinimalWorld ) + Instance( "child.vmf", true );
		ExpectOk( Expand( parent, store, 16 ), "well-formed instance" );
	}

	// Missing file: referenced VMF is not in the store.
	{
		hammertest::InMemoryFileStore store;
		std::string parent = std::string( kMinimalWorld ) + Instance( "nope.vmf", true );
		ExpectError( Expand( parent, store, 16 ), "missing instance file" );
	}

	// Cyclic reference: child.vmf instances itself. Must hit the depth guard, not
	// loop forever.
	{
		hammertest::InMemoryFileStore store;
		store.Write( "child.vmf", std::string( kMinimalWorld ) + Instance( "child.vmf", true ) );
		std::string parent = std::string( kMinimalWorld ) + Instance( "child.vmf", true );
		ExpectError( Expand( parent, store, 4 ), "cyclic instance reference" );
	}

	// func_instance with no "file".
	{
		hammertest::InMemoryFileStore store;
		std::string parent = std::string( kMinimalWorld ) + Instance( "", false );
		ExpectError( Expand( parent, store, 16 ), "func_instance without file" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.instancing negative: ORACLE UNSOUND (%d)\n", g_failures );
		return 1;
	}
	std::printf( "formats.instancing negative: good expands; missing-file, cyclic, and no-file all "
	             "reported as errors\n" );
	return 0;
}
