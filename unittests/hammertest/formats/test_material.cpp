//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the VMT material parser (formats.material.v1).
//			Parses a shader material and a patch material and checks the shader
//			name, parameter access (case-insensitive), the common accessors, and
//			proxy/patch detection. Build/run via the conformance manifest. Exit 0 on
//			success.
//
//=============================================================================//

#include "hammer/formats/material.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::Material;
using hammer::formats::ParseMaterial;

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

const char *kVmt =
    "\"LightmappedGeneric\"\n"
    "{\n"
    "\t\"$basetexture\" \"brick/brickwall001\"\n"
    "\t\"$surfaceprop\" \"brick\"\n"
    "\t\"$detail\" \"detail/noise\"\n"
    "\t\"Proxies\"\n"
    "\t{\n"
    "\t\t\"AnimatedTexture\"\n\t\t{\n\t\t\t\"animatedtexturevar\" \"$basetexture\"\n\t\t}\n"
    "\t}\n"
    "}\n";

const char *kPatch = "\"patch\"\n"
                     "{\n"
                     "\t\"include\" \"materials/brick/brickwall001.vmt\"\n"
                     "\t\"replace\"\n\t{\n\t\t\"$basetexture\" \"brick/brickwall002\"\n\t}\n"
                     "}\n";

void TestShaderMaterial()
{
	auto m = ParseMaterial( kVmt );
	Check( m.has_value(), "vmt parses" );
	if ( !m )
	{
		return;
	}
	Check( m->shader == "LightmappedGeneric", "shader name" );
	Check( m->BaseTexture() == "brick/brickwall001", "$basetexture accessor" );
	Check( m->SurfaceProp() == "brick", "$surfaceprop accessor" );
	Check( m->HasParam( "$detail" ), "$detail present" );
	Check( !m->HasParam( "$bumpmap" ), "$bumpmap absent" );
	Check( m->hasProxies, "Proxies block detected" );
	Check( !m->IsPatch(), "not a patch material" );

	// Parameter lookup is case-insensitive.
	const std::string *a = m->Param( "$basetexture" );
	const std::string *b = m->Param( "$BASETEXTURE" );
	Check( a != nullptr && b != nullptr && *a == *b, "case-insensitive parameter lookup" );
}

void TestPatchMaterial()
{
	auto m = ParseMaterial( kPatch );
	Check( m.has_value(), "patch parses" );
	if ( !m )
	{
		return;
	}
	Check( m->IsPatch(), "IsPatch true for patch shader" );
	Check( m->HasParam( "include" ), "patch include present" );
	// The $basetexture lives in the patch's "replace" block, not top-level.
	Check( m->Param( "$basetexture" ) == nullptr, "patch has no TOP-LEVEL $basetexture" );
	// ...but ResolvedParam / BaseTexture() resolve it through replace.
	const std::string *resolved = m->ResolvedParam( "$basetexture" );
	Check( resolved != nullptr && *resolved == "brick/brickwall002",
	    "ResolvedParam pulls $basetexture from the replace block" );
	Check( m->BaseTexture() == "brick/brickwall002", "BaseTexture() resolves the patched value" );
}

} // namespace

int main()
{
	TestShaderMaterial();
	TestPatchMaterial();

	if ( g_failures != 0 )
	{
		std::printf( "formats.material: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.material: shader/params/accessors, case-insensitive lookup, proxy and "
	             "patch detection all correct\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
