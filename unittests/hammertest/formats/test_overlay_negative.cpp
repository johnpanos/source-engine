//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.overlay.v1. ParseOverlay must REJECT an
//			info_overlay missing a required field (material, sides, a basis vector,
//			a uv corner) and a non-info_overlay entity, while a complete overlay
//			parses. Otherwise the editor/renderer would project from corrupt data.
//			Exit 0 when the good overlay parses AND every bad one is rejected.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/overlay.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::ParseKeyValues;
using hammer::formats::ParseOverlay;

namespace
{
int g_checks = 0;
int g_failures = 0;

// Parses a single-entity document and runs ParseOverlay on that entity.
bool Parses( const std::string &entityText )
{
	hammer::formats::ParseResult pr = ParseKeyValues( entityText );
	if ( !pr.ok || pr.root.children.empty() )
	{
		return false;
	}
	return ParseOverlay( pr.root.children.front() ).has_value();
}

void ExpectParses( const std::string &text, const char *label )
{
	++g_checks;
	if ( !Parses( text ) )
	{
		std::printf( "FAIL: good overlay rejected (%s)\n", label );
		++g_failures;
	}
}

void ExpectRejected( const std::string &text, const char *label )
{
	++g_checks;
	if ( Parses( text ) )
	{
		std::printf( "FAIL: malformed overlay accepted (%s)\n", label );
		++g_failures;
	}
}

// Builds an info_overlay entity, optionally omitting one key or corrupting one.
std::string Overlay( const char *omit, const char *badBasisU )
{
	auto kv = [&]( const char *k, const char *v )
	{
		if ( omit != nullptr && std::string( k ) == omit )
		{
			return std::string();
		}
		return std::string( "\t\"" ) + k + "\" \"" + v + "\"\n";
	};
	std::string e = "entity\n{\n\t\"classname\" \"info_overlay\"\n";
	e += kv( "material", "decals/blood1" );
	e += kv( "sides", "6 7" );
	e += kv( "BasisOrigin", "10 0 0" );
	e += kv( "BasisU", badBasisU != nullptr ? badBasisU : "1 0 0" );
	e += kv( "BasisV", "0 1 0" );
	e += kv( "BasisNormal", "0 0 1" );
	e += kv( "uv0", "-16 -16 0" );
	e += kv( "uv1", "16 -16 0" );
	e += kv( "uv2", "16 16 0" );
	e += kv( "uv3", "-16 16 0" );
	e += "}\n";
	return e;
}

} // namespace

int main()
{
	ExpectParses( Overlay( nullptr, nullptr ), "complete overlay" );

	ExpectRejected( Overlay( "material", nullptr ), "missing material" );
	ExpectRejected( Overlay( "sides", nullptr ), "missing sides" );
	ExpectRejected( Overlay( "BasisU", nullptr ), "missing BasisU" );
	ExpectRejected( Overlay( "uv2", nullptr ), "missing uv2 corner" );
	ExpectRejected( Overlay( nullptr, "1 0" ), "BasisU with only two components" );
	ExpectRejected( "entity\n{\n\t\"classname\" \"info_null\"\n}\n", "not an info_overlay" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.overlay negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.overlay negative: complete overlay parses; missing material/sides/basis/"
	             "corner and non-overlay all rejected\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
