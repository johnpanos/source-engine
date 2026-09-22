//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the VMF/keyvalues parser, writer, and semantic
//			comparator (RFC 0002, HAM-CORPUS-001 / H2). Pins parse structure,
//			round-trip stability, unknown-chunk preservation, malformed-input
//			diagnostics, and the comparator's declared normalization.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"

#include <cstdio>
#include <string>

using hammer::formats::CompareKeyValues;
using hammer::formats::KeyValueNode;
using hammer::formats::ParseKeyValues;
using hammer::formats::ParseResult;
using hammer::formats::WriteKeyValues;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_keyvalues.cpp", line, expression );
		++g_failures;
	}
}

const char *kSampleVmf = "world\n"
                         "{\n"
                         "\t\"id\" \"1\"\n"
                         "\t\"classname\" \"worldspawn\"\n"
                         "\tsolid\n"
                         "\t{\n"
                         "\t\t\"id\" \"2\"\n"
                         "\t\tside\n"
                         "\t\t{\n"
                         "\t\t\t\"id\" \"3\"\n"
                         "\t\t\t\"material\" \"BRICK/BRICKWALL\"\n"
                         "\t\t}\n"
                         "\t}\n"
                         "}\n"
                         "customchunk\n" // an unknown top-level block
                         "{\n"
                         "\t\"opaque\" \"keepme\"\n"
                         "}\n";

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

void TestParseStructure()
{
	const ParseResult parsed = ParseKeyValues( kSampleVmf );
	CHECK( parsed.ok );
	CHECK( parsed.root.children.size() == 2 ); // world + customchunk

	const KeyValueNode &world = parsed.root.children[0];
	CHECK( world.name == "world" );
	CHECK( world.Find( "classname" ) != nullptr && *world.Find( "classname" ) == "worldspawn" );
	CHECK( world.children.size() == 1 ); // solid
	const KeyValueNode &solid = world.children[0];
	CHECK( solid.name == "solid" );
	CHECK( solid.children.size() == 1 ); // side
	CHECK( solid.children[0].Find( "material" ) != nullptr );
}

void TestRoundTripAndUnknownPreserved()
{
	const ParseResult first = ParseKeyValues( kSampleVmf );
	CHECK( first.ok );
	const std::string written = WriteKeyValues( first.root );
	const ParseResult second = ParseKeyValues( written );
	CHECK( second.ok );

	// Semantic round trip is stable.
	CHECK( CompareKeyValues( first.root, second.root ).equal );

	// The unknown block and its key survive the round trip (nothing dropped).
	bool foundUnknown = false;
	for ( const KeyValueNode &child : second.root.children )
	{
		if ( child.name == "customchunk" && child.Find( "opaque" ) &&
		     *child.Find( "opaque" ) == "keepme" )
		{
			foundUnknown = true;
		}
	}
	CHECK( foundUnknown );
}

void TestMalformedInput()
{
	const ParseResult unterminated = ParseKeyValues( "world\n{\n\t\"id\" \"1\"\n" ); // no closing }
	CHECK( !unterminated.ok );
	CHECK( !unterminated.error.empty() );

	const ParseResult badQuote = ParseKeyValues( "world\n{\n\t\"id\" \"oops\n}\n" );
	CHECK( !badQuote.ok );
}

void TestComparatorNormalization()
{
	// Key order within a block is not semantically significant.
	const ParseResult a = ParseKeyValues( "ent\n{\n\t\"a\" \"1\"\n\t\"b\" \"2\"\n}\n" );
	const ParseResult b = ParseKeyValues( "ent\n{\n\t\"b\" \"2\"\n\t\"a\" \"1\"\n}\n" );
	CHECK( a.ok && b.ok );
	CHECK( CompareKeyValues( a.root, b.root ).equal );

	// A changed value is detected.
	const ParseResult c = ParseKeyValues( "ent\n{\n\t\"a\" \"1\"\n\t\"b\" \"CHANGED\"\n}\n" );
	CHECK( c.ok );
	CHECK( !CompareKeyValues( a.root, c.root ).equal );

	// A duplicate key is significant (multiset, not set).
	const ParseResult d = ParseKeyValues( "ent\n{\n\t\"o\" \"x\"\n}\n" );
	const ParseResult e = ParseKeyValues( "ent\n{\n\t\"o\" \"x\"\n\t\"o\" \"x\"\n}\n" );
	CHECK( d.ok && e.ok );
	CHECK( !CompareKeyValues( d.root, e.root ).equal );
}

int main()
{
	TestParseStructure();
	TestRoundTripAndUnknownPreserved();
	TestMalformedInput();
	TestComparatorNormalization();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.formats keyvalues: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.formats keyvalues: all checks passed\n" );
	return 0;
}
