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
#include <fstream>
#include <limits>
#include <string>
#include <utility>

using namespace mapcontainer;

namespace
{
class FileByteSource final : public IMapByteSource
{
public:
	explicit FileByteSource( const char *pPath ) : m_File( pPath, std::ios::binary )
	{
		if ( !m_File )
			return;
		m_File.seekg( 0, std::ios::end );
		const std::streamoff end = m_File.tellg();
		if ( end < 0 )
			return;
		m_Size = uint64_t( end );
		m_Valid = true;
	}

	bool IsOpen() const { return m_Valid; }
	uint64_t Size() const override { return m_Size; }
	bool ReadAt( uint64_t offset, void *pDest, size_t size ) override
	{
		if ( !m_Valid || offset > m_Size || size > m_Size - offset ||
		     offset > uint64_t( std::numeric_limits<std::streamoff>::max() ) ||
		     size > size_t( std::numeric_limits<std::streamsize>::max() ) )
			return false;
		m_File.clear();
		m_File.seekg( std::streamoff( offset ), std::ios::beg );
		if ( !m_File )
			return false;
		m_File.read( static_cast<char *>( pDest ), std::streamsize( size ) );
		return m_File.gcount() == std::streamsize( size );
	}

private:
	std::ifstream m_File;
	uint64_t m_Size = 0;
	bool m_Valid = false;
};

class FileByteSink final : public IMapByteSink
{
public:
	explicit FileByteSink( std::string path ) : m_Path( std::move( path ) ) {}
	bool ResetToZeroes( uint64_t size ) override
	{
		m_File.close();
		m_File.open( m_Path, std::ios::binary | std::ios::trunc );
		if ( !m_File || size > uint64_t( std::numeric_limits<std::streamoff>::max() ) )
			return false;
		m_Size = size;
		if ( size == 0 )
			return true;
		m_File.seekp( std::streamoff( size - 1 ), std::ios::beg );
		m_File.put( '\0' );
		return !!m_File;
	}
	bool WriteAt( uint64_t offset, const void *pData, size_t size ) override
	{
		if ( !m_File || offset > m_Size || size > m_Size - offset ||
		     size > size_t( std::numeric_limits<std::streamsize>::max() ) )
			return false;
		m_File.seekp( std::streamoff( offset ), std::ios::beg );
		m_File.write( static_cast<const char *>( pData ), std::streamsize( size ) );
		return !!m_File;
	}
	bool Finish()
	{
		m_File.flush();
		m_File.close();
		return !m_File.fail();
	}

private:
	std::string m_Path;
	std::ofstream m_File;
	uint64_t m_Size = 0;
};

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
	std::fprintf( stderr, "%s: %s (lump %s, offset %llu)\n", pWhat,
	    MapContainerErrorName( status.code ),
	    status.fourcc ? FourCCText( status.fourcc ).c_str() : "-",
	    (unsigned long long)status.offset );
	return 1;
}

int Info( IMapByteSource &source, bool bVerify )
{
	IMapContainer *pContainer = nullptr;
	const MapContainerOpenOptions options{ bVerify, nullptr, 0, false };
	const MapContainerStatus status = OpenMapContainer( source, options, &pContainer );
	if ( !status.Ok() )
		return ReportError( "open", status );
	const ContainerPtr container( pContainer );
	const IMapContainer &map = *container;
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
		std::printf( "%s v%u flags=0x%x align=%u offset=%llu size=%llu",
		    FourCCText( info.fourcc ).c_str(), info.version, info.flags, info.alignment,
		    (unsigned long long)info.offset, (unsigned long long)info.storedSize );
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
	if ( !bTwoPaths )
	{
		FileByteSource source( argv[2] );
		if ( !source.IsOpen() )
		{
			std::fprintf( stderr, "bsp2tool: cannot read %s\n", argv[2] );
			return 2;
		}
		return Info( source, command == "verify" );
	}
	FileByteSource source( argv[2] );
	if ( !source.IsOpen() )
	{
		std::fprintf( stderr, "bsp2tool: cannot read %s\n", argv[2] );
		return 2;
	}
	const std::string temp = std::string( argv[3] ) + ".tmp";
	FileByteSink sink( temp );
	const MapContainerStatus status = command == "export" ? ExportLegacyFromBsp2( source, sink )
	                                                      : ConvertLegacyToBsp2( source, sink );
	if ( !status.Ok() )
	{
		sink.Finish();
		std::remove( temp.c_str() );
		ReportError( command.c_str(), status );
		return status.code == MapContainerError::WriteFailed ? 2 : 1;
	}
	if ( !sink.Finish() || std::rename( temp.c_str(), argv[3] ) != 0 )
	{
		std::remove( temp.c_str() );
		std::fprintf( stderr, "bsp2tool: cannot write %s\n", argv[3] );
		return 2;
	}
	return 0;
}
