//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::formats overlay (info_overlay) parsing
//			(formats.overlay.v1). Parses an info_overlay, checks its material,
//			target sides, basis, and the world-space quad WorldCorners() computes,
//			and confirms ExtractOverlays ignores non-overlay entities. Build/run via
//			the conformance manifest. Exit 0 on success.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/overlay.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <string>

using hammer::formats::ExtractOverlays;
using hammer::formats::Overlay;
using hammer::formats::ParseKeyValues;
using hammer::geometry::Vec3d;

namespace
{
int g_failures = 0;

void Check( bool ok, const char *label )
{
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

bool NearV( const Vec3d &a, double x, double y, double z )
{
	return Near( a.x, x ) && Near( a.y, y ) && Near( a.z, z );
}

const char *kMap = "versioninfo\n{\n\t\"editorversion\" \"400\"\n}\n"
                   "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
                   "entity\n{\n\t\"classname\" \"info_null\"\n\t\"origin\" \"0 0 0\"\n}\n"
                   "entity\n{\n"
                   "\t\"classname\" \"info_overlay\"\n"
                   "\t\"material\" \"decals/blood1\"\n"
                   "\t\"sides\" \"6 7 8\"\n"
                   "\t\"BasisOrigin\" \"10 0 0\"\n"
                   "\t\"BasisU\" \"1 0 0\"\n"
                   "\t\"BasisV\" \"0 1 0\"\n"
                   "\t\"BasisNormal\" \"0 0 1\"\n"
                   "\t\"uv0\" \"-16 -16 0\"\n"
                   "\t\"uv1\" \"16 -16 0\"\n"
                   "\t\"uv2\" \"16 16 0\"\n"
                   "\t\"uv3\" \"-16 16 0\"\n"
                   "\t\"StartU\" \"0\"\n"
                   "\t\"EndU\" \"1\"\n"
                   "}\n";

void TestParse()
{
	hammer::formats::ParseResult pr = ParseKeyValues( kMap );
	Check( pr.ok, "map parses" );

	std::vector<Overlay> overlays = ExtractOverlays( pr.root );
	Check( overlays.size() == 1, "exactly one info_overlay (info_null ignored)" );
	if ( overlays.empty() )
	{
		return;
	}
	const Overlay &o = overlays.front();
	Check( o.material == "decals/blood1", "material parsed" );
	Check( o.sides.size() == 3 && o.sides[0] == 6 && o.sides[2] == 8, "sides list parsed" );
	Check( NearV( o.basisOrigin, 10, 0, 0 ), "basis origin parsed" );

	// World corners: origin + U*uv.x + V*uv.y + N*uv.z.
	std::array<Vec3d, 4> corners = o.WorldCorners();
	Check( NearV( corners[0], -6, -16, 0 ), "corner0 = origin + U*-16 + V*-16" );
	Check( NearV( corners[2], 26, 16, 0 ), "corner2 = origin + U*16 + V*16" );
	Check( NearV( corners[3], -6, 16, 0 ), "corner3 = origin + U*-16 + V*16" );
}

} // namespace

int main()
{
	TestParse();

	if ( g_failures != 0 )
	{
		std::printf( "formats.overlay: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.overlay: material/sides/basis parse and WorldCorners project the quad "
	             "correctly; non-overlay entities ignored\n" );
	return 0;
}
