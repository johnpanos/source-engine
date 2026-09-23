//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for geometry.displacement.v1. ParseDispInfo must
//			REJECT malformed/inconsistent dispinfo blocks -- a bad power, a wrong
//			row count, a wrong-length row, or a missing required grid -- while a
//			well-formed block parses. Otherwise the parser would silently accept
//			corrupt terrain data. Exit 0 when the good block parses AND every bad
//			block is rejected; exit 1 otherwise.
//
//=============================================================================//

#include "hammer/geometry/displacement.h"
#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::geometry::ParseDispInfo;

namespace
{
int g_failures = 0;
int g_checks = 0;

// Returns true iff 'text' (one dispinfo block) PARSES.
bool Parses( const std::string &text )
{
	hammer::formats::ParseResult pr = hammer::formats::ParseKeyValues( text );
	if ( !pr.ok || pr.root.children.empty() )
	{
		return false;
	}
	return ParseDispInfo( pr.root.children.front() ).has_value();
}

void ExpectParses( const std::string &text, const char *label )
{
	++g_checks;
	if ( !Parses( text ) )
	{
		std::printf( "FAIL: good block rejected (%s)\n", label );
		++g_failures;
	}
}

void ExpectRejected( const std::string &text, const char *label )
{
	++g_checks;
	if ( Parses( text ) )
	{
		std::printf( "FAIL: malformed block accepted (%s)\n", label );
		++g_failures;
	}
}

// power 2 -> side 5. A well-formed block for reference.
const char *kGood = "dispinfo\n{\n"
                    "\t\"power\" \"2\"\n"
                    "\t\"startposition\" \"[0 0 0]\"\n"
                    "\tnormals\n\t{\n"
                    "\t\t\"row0\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                    "\t\t\"row1\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                    "\t\t\"row2\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                    "\t\t\"row3\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                    "\t\t\"row4\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                    "\t}\n"
                    "\tdistances\n\t{\n"
                    "\t\t\"row0\" \"0 0 0 0 0\"\n"
                    "\t\t\"row1\" \"0 0 0 0 0\"\n"
                    "\t\t\"row2\" \"0 0 0 0 0\"\n"
                    "\t\t\"row3\" \"0 0 0 0 0\"\n"
                    "\t\t\"row4\" \"0 0 0 0 0\"\n"
                    "\t}\n"
                    "}\n";

// power 5 is out of the supported [1,4] range.
const char *kBadPower = "dispinfo\n{\n"
                        "\t\"power\" \"5\"\n"
                        "\t\"startposition\" \"[0 0 0]\"\n"
                        "\tnormals\n\t{\n\t\t\"row0\" \"0 0 1\"\n\t}\n"
                        "\tdistances\n\t{\n\t\t\"row0\" \"0\"\n\t}\n"
                        "}\n";

// distances row0 has 4 values (should be 5 for side 5).
const char *kShortRow = "dispinfo\n{\n"
                        "\t\"power\" \"2\"\n"
                        "\t\"startposition\" \"[0 0 0]\"\n"
                        "\tnormals\n\t{\n"
                        "\t\t\"row0\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row1\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row2\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row3\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row4\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t}\n"
                        "\tdistances\n\t{\n"
                        "\t\t\"row0\" \"0 0 0 0\"\n"
                        "\t\t\"row1\" \"0 0 0 0 0\"\n"
                        "\t\t\"row2\" \"0 0 0 0 0\"\n"
                        "\t\t\"row3\" \"0 0 0 0 0\"\n"
                        "\t\t\"row4\" \"0 0 0 0 0\"\n"
                        "\t}\n"
                        "}\n";

// Only 4 distance rows (need 5 for side 5).
const char *kMissingRow = "dispinfo\n{\n"
                          "\t\"power\" \"2\"\n"
                          "\t\"startposition\" \"[0 0 0]\"\n"
                          "\tnormals\n\t{\n"
                          "\t\t\"row0\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                          "\t\t\"row1\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                          "\t\t\"row2\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                          "\t\t\"row3\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                          "\t\t\"row4\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                          "\t}\n"
                          "\tdistances\n\t{\n"
                          "\t\t\"row0\" \"0 0 0 0 0\"\n"
                          "\t\t\"row1\" \"0 0 0 0 0\"\n"
                          "\t\t\"row2\" \"0 0 0 0 0\"\n"
                          "\t\t\"row3\" \"0 0 0 0 0\"\n"
                          "\t}\n"
                          "}\n";

// No normals grid at all.
const char *kMissingNormals = "dispinfo\n{\n"
                              "\t\"power\" \"2\"\n"
                              "\t\"startposition\" \"[0 0 0]\"\n"
                              "\tdistances\n\t{\n"
                              "\t\t\"row0\" \"0 0 0 0 0\"\n"
                              "\t\t\"row1\" \"0 0 0 0 0\"\n"
                              "\t\t\"row2\" \"0 0 0 0 0\"\n"
                              "\t\t\"row3\" \"0 0 0 0 0\"\n"
                              "\t\t\"row4\" \"0 0 0 0 0\"\n"
                              "\t}\n"
                              "}\n";

// Valid normals + distances, but an alphas grid whose row0 has 4 values (needs 5).
const char *kBadAlpha = "dispinfo\n{\n"
                        "\t\"power\" \"2\"\n"
                        "\t\"startposition\" \"[0 0 0]\"\n"
                        "\tnormals\n\t{\n"
                        "\t\t\"row0\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row1\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row2\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row3\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t\t\"row4\" \"0 0 1 0 0 1 0 0 1 0 0 1 0 0 1\"\n"
                        "\t}\n"
                        "\tdistances\n\t{\n"
                        "\t\t\"row0\" \"0 0 0 0 0\"\n"
                        "\t\t\"row1\" \"0 0 0 0 0\"\n"
                        "\t\t\"row2\" \"0 0 0 0 0\"\n"
                        "\t\t\"row3\" \"0 0 0 0 0\"\n"
                        "\t\t\"row4\" \"0 0 0 0 0\"\n"
                        "\t}\n"
                        "\talphas\n\t{\n"
                        "\t\t\"row0\" \"0 0 0 0\"\n"
                        "\t\t\"row1\" \"0 0 0 0 0\"\n"
                        "\t\t\"row2\" \"0 0 0 0 0\"\n"
                        "\t\t\"row3\" \"0 0 0 0 0\"\n"
                        "\t\t\"row4\" \"0 0 0 0 0\"\n"
                        "\t}\n"
                        "}\n";

} // namespace

int main()
{
	ExpectParses( kGood, "well-formed power-2 dispinfo" );
	ExpectRejected( kBadPower, "power out of range" );
	ExpectRejected( kShortRow, "distances row too short" );
	ExpectRejected( kMissingRow, "distances missing a row" );
	ExpectRejected( kMissingNormals, "normals grid absent" );
	ExpectRejected( kBadAlpha, "alphas row too short" );

	if ( g_failures != 0 )
	{
		std::printf( "geometry.displacement negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "geometry.displacement negative: good block parses; bad power/row-length/"
	             "row-count/missing-grid all rejected\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
