//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.material.v1. ParseMaterial must return
//			nullopt for VMT text with no top-level shader block and for malformed
//			keyvalues, while a real material parses. Exit 0 when the good material
//			parses AND every bad input is rejected.
//
//=============================================================================//

#include "hammer/formats/material.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::ParseMaterial;

namespace
{
int g_checks = 0;
int g_failures = 0;

void ExpectParses( const char *text, const char *label )
{
	++g_checks;
	if ( !ParseMaterial( text ).has_value() )
	{
		std::printf( "FAIL: good material rejected (%s)\n", label );
		++g_failures;
	}
}

void ExpectRejected( const char *text, const char *label )
{
	++g_checks;
	if ( ParseMaterial( text ).has_value() )
	{
		std::printf( "FAIL: malformed material accepted (%s)\n", label );
		++g_failures;
	}
}

} // namespace

int main()
{
	ExpectParses( "\"UnlitGeneric\"\n{\n\t\"$basetexture\" \"dev/dev_measure\"\n}\n",
	    "well-formed material" );

	ExpectRejected( "", "empty text" );
	ExpectRejected( "   \n\t\n", "whitespace only" );
	ExpectRejected( "\"LightmappedGeneric\"\n{\n\t\"$basetexture\" \"x\"\n", "unterminated block" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.material negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.material negative: good material parses; empty, whitespace-only, and "
	             "unterminated-block all rejected\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
