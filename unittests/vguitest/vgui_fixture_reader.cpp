//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Reads the generated VGUI fixture textures (RFC 0010 V0;
//          tools/vgui/vgui_fixture_content.py) through the engine's VTF
//          container reader (texturecontainer::vtf, which vtf/vtf.cpp uses
//          to load every game texture), and checks their probe texels.
//
//          Run by `vgui_fixture_content.py check`, which writes the fixture
//          files and a probe list: one line per probe,
//
//            <file> <width> <height> <frames> <frame> <x> <y> <r> <g> <b> <a>
//
//          Usage: vgui_fixture_reader <probe list>
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "texturecontainer/vtf_container.h"

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace
{
namespace vtf = texturecontainer::vtf;

struct Tally
{
	unsigned long checks = 0;
	unsigned long failures = 0;

	void Check( bool ok, const std::string &what )
	{
		++checks;
		if ( !ok )
			++failures;
		std::printf( "%s %s\n", ok ? "PASS" : "FAIL", what.c_str() );
	}
};

std::vector<std::byte> ReadFile( const std::string &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::vector<char> chars(
	    ( std::istreambuf_iterator<char>( stream ) ), std::istreambuf_iterator<char>() );
	std::vector<std::byte> bytes( chars.size() );
	for ( std::size_t i = 0; i < chars.size(); ++i )
		bytes[i] = std::byte( static_cast<unsigned char>( chars[i] ) );
	return bytes;
}

void CheckProbe( Tally &tally, const std::string &line )
{
	std::istringstream fields( line );
	std::string path;
	unsigned width = 0, height = 0, frames = 0, frame = 0, x = 0, y = 0;
	unsigned expected[4] = {};
	fields >> path >> width >> height >> frames >> frame >> x >> y >> expected[0] >> expected[1] >>
	    expected[2] >> expected[3];
	if ( !fields )
	{
		tally.Check( false, "probe line parses: " + line );
		return;
	}
	const std::string label = path + " frame " + std::to_string( frame ) + " texel (" +
	                          std::to_string( x ) + ", " + std::to_string( y ) + ")";
	const std::vector<std::byte> bytes = ReadFile( path );
	auto header = vtf::ReadHeader( bytes );
	if ( !header )
	{
		tally.Check( false, label + ": header: " + header.Error() );
		return;
	}
	const vtf::Header &h = header.Value();
	tally.Check( h.width == width && h.height == height && h.frames == frames && h.format == 0 &&
	                 h.mips == 1,
	    label + ": the reader sees the size, frames, RGBA8888 and one mip" );
	auto layout = vtf::ReadLayout( bytes, h );
	if ( !layout )
	{
		tally.Check( false, label + ": layout: " + layout.Error() );
		return;
	}
	const vtf::Subresource *image = nullptr;
	for ( const vtf::Subresource &candidate : layout.Value().images )
	{
		if ( candidate.mip == 0 && candidate.frame == frame && candidate.face == 0 )
			image = &candidate;
	}
	if ( !image )
	{
		tally.Check( false, label + ": the layout has the frame" );
		return;
	}
	auto texels = vtf::ReadImage( bytes, layout.Value(), *image );
	if ( !texels || x >= width || y >= height ||
	     texels.Value().size() != std::size_t( width ) * height * 4 )
	{
		tally.Check( false, label + ": image read" );
		return;
	}
	const std::byte *texel = texels.Value().data() + ( std::size_t( y ) * width + x ) * 4;
	unsigned actual[4];
	for ( int c = 0; c < 4; ++c )
		actual[c] = std::to_integer<unsigned>( texel[c] );
	char values[96];
	std::snprintf( values, sizeof( values ), " is (%u, %u, %u, %u) (read %u, %u, %u, %u)",
	    expected[0], expected[1], expected[2], expected[3], actual[0], actual[1], actual[2],
	    actual[3] );
	tally.Check( actual[0] == expected[0] && actual[1] == expected[1] && actual[2] == expected[2] &&
	                 actual[3] == expected[3],
	    label + values );
}
} // namespace

int main( int argc, char **argv )
{
	Tally tally;
	if ( argc != 2 )
	{
		std::printf( "usage: vgui_fixture_reader <probe list>\n" );
		return testing::ReportConformance( 0, 0 );
	}
	std::ifstream list( argv[1] );
	std::string line;
	while ( std::getline( list, line ) )
	{
		if ( !line.empty() )
			CheckProbe( tally, line );
	}
	return testing::ReportConformance( tally.checks, tally.failures );
}
