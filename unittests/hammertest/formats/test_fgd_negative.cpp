//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.fgd.v1. ParseFgd must REPORT malformed
//			schema -- a property with no "(type)", an unterminated class body, a
//			missing class name, and a stray top-level token -- while a well-formed
//			FGD parses. Otherwise the editor would build entity property sheets
//			from silently-corrupt schema. Exit 0 when the good FGD parses AND every
//			bad one is reported as an error; exit 1 otherwise.
//
//=============================================================================//

#include "hammer/formats/fgd.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::ParseFgd;

namespace
{
int g_checks = 0;
int g_failures = 0;

void ExpectOk( const char *text, const char *label )
{
	++g_checks;
	if ( !ParseFgd( text ).ok )
	{
		std::printf( "FAIL: good FGD rejected (%s)\n", label );
		++g_failures;
	}
}

void ExpectError( const char *text, const char *label )
{
	++g_checks;
	if ( ParseFgd( text ).ok )
	{
		std::printf( "FAIL: malformed FGD accepted (%s)\n", label );
		++g_failures;
	}
}

} // namespace

int main()
{
	ExpectOk( "@PointClass = info_null : \"Nothing.\"\n[\n]\n", "well-formed empty-body class" );
	ExpectOk( "@BaseClass = B\n[\n\tk(string) : \"K\" : \"d\"\n]\n"
	          "@PointClass base(B) = e\n[\n]\n",
	    "well-formed class with base + property" );

	ExpectError( "@PointClass = e\n[\n\tbadkey : \"no type\"\n]\n", "property missing (type)" );
	ExpectError( "@PointClass = e\n[\n\tk(string) : \"x\"\n", "unterminated class body" );
	ExpectError( "@PointClass = \n[\n]\n", "missing class name after '='" );
	ExpectError( "garbage_at_top_level\n", "stray top-level token" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.fgd negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.fgd negative: good parses; missing-type, unterminated-body, "
	             "missing-name, and stray-token all reported as errors\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
