//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: bsp2tool - inspect, verify, convert and export map containers
//          (RFC 0008, phase F1).
//
//   bsp2tool info    <map>                 directory listing
//   bsp2tool verify  <map>                 full validation incl. every hash
//   bsp2tool convert <legacy.bsp> <out>    legacy VBSP -> BSP2 (lossless)
//   bsp2tool export  <bsp2> <out>          BSP2 -> byte-identical legacy VBSP
//
// Exit status: 0 success, 1 container error, 2 usage or file I/O error.
//
//=============================================================================//

#include "mapcontainer/map_container.h"
#include "mapcontainer/map_container_builder.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

using namespace mapcontainer;

namespace
{
std::optional<std::vector<std::byte>> ReadFile( const char *pPath )
{
	std::ifstream in( pPath, std::ios::binary );
	if ( !in )
		return std::nullopt;
	std::vector<char> raw( ( std::istreambuf_iterator<char>( in ) ), std::istreambuf_iterator<char>() );
	if ( in.bad() )
		return std::nullopt;
	std::vector<std::byte> bytes( raw.size() );
	if ( !raw.empty() )
		std::memcpy( bytes.data(), raw.data(), raw.size() );
	return bytes;
}

bool WriteFile( const char *pPath, const std::vector<std::byte> &bytes )
{
	// Write beside the target and rename, so a failure never leaves a torn map.
	const std::string temp = std::string( pPath ) + ".tmp";
	{
		std::ofstream out( temp, std::ios::binary | std::ios::trunc );
		if ( !out )
			return false;
		out.write( reinterpret_cast<const char *>( bytes.data() ), std::streamsize( bytes.size() ) );
		if ( !out )
			return false;
	}
	return std::rename( temp.c_str(), pPath ) == 0;
}

std::string FourCCText( uint32_t fourcc )
{
	std::string text;
	for ( int i = 0; i < 4; ++i )
	{
		const char c = char( ( fourcc >> ( 8 * i ) ) & 0xFF );
		text += ( c >= 32 && c < 127 ) ? c : '?';
	}
	return text;
}

int ReportError( const char *pWhat, const MapContainerStatus &status )
{
	std::fprintf( stderr, "%s: %s (lump %s, offset %llu)\n", pWhat, MapContainerErrorName( status.code ),
	              status.fourcc ? FourCCText( status.fourcc ).c_str() : "-", (unsigned long long)status.offset );
	return 1;
}

int Info( const std::vector<std::byte> &bytes, bool bVerify )
{
	MemoryByteSource source( bytes );
	auto container = OpenMemoryContainer( source, bVerify );
	if ( !container )
		return ReportError( "open", container.Error() );
	const IMapContainer &map = *container.Value();
	std::printf( "kind %s\nlegacy-version %d\nrevision %d\nlumps %u\n",
	             map.Kind() == MapContainerKind::Bsp2 ? "bsp2" : "legacy-vbsp", map.LegacyVersion(),
	             map.MapRevision(), map.LumpCount() );
	if ( bVerify )
		return 0;
	for ( uint32_t i = 0; i < map.LumpCount(); ++i )
	{
		MapLumpInfo info{};
		map.LumpAt( i, &info );
		if ( map.Kind() == MapContainerKind::LegacyVbsp && info.storedSize == 0 )
			continue;
		std::printf( "%s v%u flags=0x%x align=%u offset=%llu size=%llu", FourCCText( info.fourcc ).c_str(), info.version,
		             info.flags, info.alignment, (unsigned long long)info.offset, (unsigned long long)info.storedSize );
		if ( info.hasHash )
		{
			std::printf( " blake2b128=" );
			for ( uint8_t byte : info.hash )
				std::printf( "%02x", byte );
		}
		std::printf( "\n" );
	}
	return 0;
}
} // namespace

int main( int argc, char **argv )
{
	if ( argc < 3 )
	{
		std::fprintf( stderr, "usage: bsp2tool info|verify <map> | convert|export <in> <out>\n" );
		return 2;
	}
	const std::string command = argv[1];
	const bool bTwoPaths = command == "convert" || command == "export";
	if ( !bTwoPaths && command != "info" && command != "verify" )
	{
		std::fprintf( stderr, "bsp2tool: unknown command '%s'\n", command.c_str() );
		return 2;
	}
	if ( argc != ( bTwoPaths ? 4 : 3 ) )
	{
		std::fprintf( stderr, "bsp2tool: wrong argument count for '%s'\n", command.c_str() );
		return 2;
	}
	const auto input = ReadFile( argv[2] );
	if ( !input )
	{
		std::fprintf( stderr, "bsp2tool: cannot read %s\n", argv[2] );
		return 2;
	}

	if ( !bTwoPaths )
		return Info( *input, command == "verify" );

	auto output = command == "convert" ? ConvertLegacyToBsp2( *input ) : ExportLegacyFromBsp2( *input );
	if ( !output )
		return ReportError( command.c_str(), output.Error() );
	if ( !WriteFile( argv[3], output.Value() ) )
	{
		std::fprintf( stderr, "bsp2tool: cannot write %s\n", argv[3] );
		return 2;
	}
	return 0;
}
