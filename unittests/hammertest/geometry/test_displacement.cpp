//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::geometry displacement parsing + surface
//			construction (geometry.displacement.v1). Drives ParseDispInfo through
//			the real hammer::formats keyvalues codec and checks BuildDisplacementSurface
//			against analytical expectations: grid sizes, flat-surface bilinear
//			interpolation, uniform normal displacement, and startposition-driven
//			grid orientation. Build/run via the conformance manifest
//			(linux-headless-core). Exit 0 on success.
//
//=============================================================================//

#include "hammer/geometry/displacement.h"
#include "hammer/formats/keyvalues.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <string>

using hammer::geometry::BuildDisplacementSurface;
using hammer::geometry::DispInfo;
using hammer::geometry::DisplacementSurface;
using hammer::geometry::ParseDispInfo;
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
	return std::fabs( a - b ) < 1e-6;
}

bool NearV( const Vec3d &a, const Vec3d &b )
{
	return Near( a.x, b.x ) && Near( a.y, b.y ) && Near( a.z, b.z );
}

// Builds dispinfo VMF text of a given power. 'nz' is the constant per-vertex
// normal z (normals are (0,0,nz)); 'dist' is the constant per-vertex distance;
// 'start' is the startposition string.
std::string MakeDispInfo( int power, const char *start, double nz, double dist )
{
	const int side = ( 1 << power ) + 1;
	std::string text = "dispinfo\n{\n";
	text += "\t\"power\" \"" + std::to_string( power ) + "\"\n";
	text += std::string( "\t\"startposition\" \"" ) + start + "\"\n";
	text += "\t\"elevation\" \"0\"\n";

	auto grid = [&]( const char *name, const std::string &rowVals )
	{
		std::string block = std::string( "\t" ) + name + "\n\t{\n";
		for ( int r = 0; r < side; ++r )
		{
			block += "\t\t\"row" + std::to_string( r ) + "\" \"" + rowVals + "\"\n";
		}
		block += "\t}\n";
		return block;
	};

	std::string normRow;
	std::string distRow;
	std::string offRow;
	for ( int c = 0; c < side; ++c )
	{
		normRow += ( c ? " " : "" ) + std::string( "0 0 " ) + std::to_string( nz );
		distRow += ( c ? " " : "" ) + std::to_string( dist );
		offRow += ( c ? " " : "" ) + std::string( "0 0 0" );
	}
	text += grid( "normals", normRow );
	text += grid( "distances", distRow );
	text += grid( "offsets", offRow );
	text += "}\n";
	return text;
}

// Parses text holding one dispinfo block and returns the parsed DispInfo.
bool ParseText( const std::string &text, DispInfo &out )
{
	hammer::formats::ParseResult pr = hammer::formats::ParseKeyValues( text );
	if ( !pr.ok || pr.root.children.empty() )
	{
		return false;
	}
	auto info = ParseDispInfo( pr.root.children.front() );
	if ( !info )
	{
		return false;
	}
	out = *info;
	return true;
}

// A 64x64 quad in the z=0 plane, wound CCW from the origin corner.
std::array<Vec3d, 4> UnitQuad()
{
	return { Vec3d( 0, 0, 0 ), Vec3d( 64, 0, 0 ), Vec3d( 64, 64, 0 ), Vec3d( 0, 64, 0 ) };
}

void TestGridSizes()
{
	for ( int power = 2; power <= 4; ++power )
	{
		DispInfo info;
		Check( ParseText( MakeDispInfo( power, "[0 0 0]", 1.0, 0.0 ), info ),
		    "grid: dispinfo parses" );
		const int side = ( 1 << power ) + 1;
		Check( info.power == power, "grid: power preserved" );
		Check( info.Side() == side, "grid: side == 2^power + 1" );

		DisplacementSurface s = BuildDisplacementSurface( UnitQuad(), Vec3d( 0, 0, 1 ), info );
		Check( s.side == side, "grid: surface side" );
		Check( static_cast<int>( s.VertexCount() ) == side * side, "grid: vertex count side^2" );
		Check( static_cast<int>( s.triangles.size() ) == 2 * ( side - 1 ) * ( side - 1 ),
		    "grid: triangle count 2*(side-1)^2" );
	}
}

void TestFlatBilinear()
{
	// distances 0, offsets 0, elevation 0 -> vertices lie on the base quad exactly.
	DispInfo info;
	Check( ParseText( MakeDispInfo( 2, "[0 0 0]", 1.0, 0.0 ), info ), "flat: parses" );
	DisplacementSurface s = BuildDisplacementSurface( UnitQuad(), Vec3d( 0, 0, 1 ), info );
	const int side = s.side; // 5

	// (0,0) == origin corner A == (0,0,0).
	Check( NearV( s.vertices[0], Vec3d( 0, 0, 0 ) ), "flat: (0,0) at origin corner" );
	// Center vertex == quad centroid (32,32,0).
	const int mid = ( side - 1 ) / 2;
	Check( NearV( s.vertices[static_cast<std::size_t>( mid ) * side + mid], Vec3d( 32, 32, 0 ) ),
	    "flat: center at centroid" );
	// Far corner (r=side-1, c=side-1) == C == (64,64,0).
	Check( NearV( s.vertices[static_cast<std::size_t>( side - 1 ) * side + ( side - 1 )],
	           Vec3d( 64, 64, 0 ) ),
	    "flat: far corner at C" );
	// Everything stays in-plane (z == 0).
	bool planar = true;
	for ( const Vec3d &v : s.vertices )
	{
		planar = planar && Near( v.z, 0.0 );
	}
	Check( planar, "flat: all vertices in z=0 plane" );
}

void TestUniformDisplacement()
{
	// Constant normal (0,0,1) and distance 10 -> every vertex pushed +10 in z; the
	// x/y stay on the base grid.
	DispInfo info;
	Check( ParseText( MakeDispInfo( 3, "[0 0 0]", 1.0, 10.0 ), info ), "uniform: parses" );
	DisplacementSurface s = BuildDisplacementSurface( UnitQuad(), Vec3d( 0, 0, 1 ), info );

	bool allPushed = true;
	for ( const Vec3d &v : s.vertices )
	{
		allPushed = allPushed && Near( v.z, 10.0 );
	}
	Check( allPushed, "uniform: every vertex z == +distance" );
	// Origin corner x/y unchanged.
	Check( Near( s.vertices[0].x, 0.0 ) && Near( s.vertices[0].y, 0.0 ),
	    "uniform: base x/y preserved at origin" );
}

void TestStartPositionOrientation()
{
	// Same quad, but startposition names corner (64,64,0). The grid origin must
	// rotate to that corner: vertex (0,0) becomes (64,64,0).
	DispInfo info;
	Check( ParseText( MakeDispInfo( 2, "[64 64 0]", 1.0, 0.0 ), info ), "start: parses" );
	Check( NearV( info.startPosition, Vec3d( 64, 64, 0 ) ), "start: startposition parsed" );
	DisplacementSurface s = BuildDisplacementSurface( UnitQuad(), Vec3d( 0, 0, 1 ), info );
	Check( NearV( s.vertices[0], Vec3d( 64, 64, 0 ) ),
	    "start: (0,0) rotates to startposition corner" );
}

// Builds a power-2 dispinfo that also carries alphas, subdiv, and triangle_tags,
// so the optional attributes can be exercised.
std::string MakeDispInfoWithExtras( double alpha )
{
	const int side = 5; // power 2
	std::string text = "dispinfo\n{\n";
	text += "\t\"power\" \"2\"\n";
	text += "\t\"startposition\" \"[0 0 0]\"\n";
	text += "\t\"elevation\" \"0\"\n";
	text += "\t\"subdiv\" \"1\"\n";

	auto rows = [&]( const char *name, int count, const std::string &rowVals )
	{
		std::string block = std::string( "\t" ) + name + "\n\t{\n";
		for ( int r = 0; r < count; ++r )
		{
			block += "\t\t\"row" + std::to_string( r ) + "\" \"" + rowVals + "\"\n";
		}
		block += "\t}\n";
		return block;
	};

	std::string normRow;
	std::string distRow;
	std::string alphaRow;
	std::string tagRow;
	for ( int c = 0; c < side; ++c )
	{
		normRow += ( c ? " " : "" ) + std::string( "0 0 1" );
		distRow += ( c ? " " : "" ) + std::string( "0" );
		alphaRow += ( c ? " " : "" ) + std::to_string( alpha );
	}
	for ( int c = 0; c < ( side - 1 ) * 2; ++c )
	{
		tagRow += ( c ? " " : "" ) + std::string( "9" );
	}
	text += rows( "normals", side, normRow );
	text += rows( "distances", side, distRow );
	text += rows( "alphas", side, alphaRow );
	text += rows( "triangle_tags", side - 1, tagRow );
	text += "}\n";
	return text;
}

void TestAlphasTagsSubdiv()
{
	DispInfo info;
	Check( ParseText( MakeDispInfoWithExtras( 128.0 ), info ), "extras: parses" );
	const int side = info.Side(); // 5

	Check( static_cast<int>( info.alphas.size() ) == side * side, "extras: alphas grid size" );
	bool alphaOk = !info.alphas.empty();
	for ( double a : info.alphas )
	{
		alphaOk = alphaOk && Near( a, 128.0 );
	}
	Check( alphaOk, "extras: per-vertex alpha value" );
	Check( info.subdiv == 1, "extras: subdiv flag parsed" );
	Check( static_cast<int>( info.triangleTags.size() ) == 2 * ( side - 1 ) * ( side - 1 ),
	    "extras: triangle_tags count 2*(side-1)^2" );

	DisplacementSurface s = BuildDisplacementSurface( UnitQuad(), Vec3d( 0, 0, 1 ), info );
	Check( static_cast<int>( s.vertexAlphas.size() ) == side * side,
	    "extras: surface carries per-vertex alphas" );
	Check( Near( s.vertexAlphas[0], 128.0 ), "extras: surface alpha value preserved" );

	// A dispinfo without alphas defaults every vertex weight to zero.
	DispInfo plain;
	Check( ParseText( MakeDispInfo( 2, "[0 0 0]", 1.0, 0.0 ), plain ), "extras: plain parses" );
	Check( static_cast<int>( plain.alphas.size() ) == side * side && Near( plain.alphas[0], 0.0 ),
	    "extras: absent alphas default to zero" );
}

} // namespace

int main()
{
	TestGridSizes();
	TestFlatBilinear();
	TestUniformDisplacement();
	TestStartPositionOrientation();
	TestAlphasTagsSubdiv();

	if ( g_failures != 0 )
	{
		std::printf( "geometry.displacement: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "geometry.displacement: dispinfo parse + bilinear surface + uniform push + "
	             "startposition orientation all correct\n" );
	return 0;
}
