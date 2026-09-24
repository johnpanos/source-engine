//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: bsp2tool - inspect, verify, convert and export map containers
//          (RFC 0008, phase F1).
//
//   bsp2tool info    <map>                 directory listing
//   bsp2tool verify  <map>                 full validation incl. every hash
//   bsp2tool convert <legacy.bsp> <out>    legacy VBSP -> BSP2 (lossless)
//   bsp2tool export  <bsp2> <out>          BSP2 -> byte-identical legacy VBSP
//   bsp2tool pack-world <legacy.bsp> <world.wmsh> <out.bsp2>
//
// Exit status: 0 success, 1 container error, 2 usage or file I/O error.
//
//=============================================================================//

#include "mapcontainer/map_container.h"
#include "mapcontainer/map_container_builder.h"
#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh.h"
#include "mapcontainer/world_mesh_format.h"
#include "../common/map_file_io.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace mapcontainer;
using namespace mapcontainer::tooling;

namespace
{
uint32_t ReadU32( const std::byte *pBytes )
{
	uint32_t value = 0;
	for ( int i = 3; i >= 0; --i )
		value = ( value << 8 ) | std::to_integer<uint8_t>( pBytes[i] );
	return value;
}

uint64_t ReadU64( const std::byte *pBytes )
{
	uint64_t value = 0;
	for ( int i = 7; i >= 0; --i )
		value = ( value << 8 ) | std::to_integer<uint8_t>( pBytes[i] );
	return value;
}

bool ReadWorldMesh( const char *pPath, std::vector<std::byte> *pBytes )
{
	FileByteSource source( pPath );
	constexpr uint64_t kMaxWorldMeshBytes = 512ull * 1024 * 1024;
	if ( !source.IsOpen() || source.Size() < kWorldMeshHeaderSize ||
	     source.Size() > kMaxWorldMeshBytes )
		return false;
	std::array<std::byte, kWorldMeshHeaderSize> header{};
	if ( !source.ReadAt( 0, header.data(), header.size() ) )
		return false;
	const std::byte *pHeader = header.data();
	const uint32_t version = ReadU32( pHeader + 4 );
	if ( ReadU32( pHeader ) != kLumpWorldMesh || version < kWorldMeshMinVersion ||
	     version > kWorldMeshVersion ||
	     ReadU32( pHeader + 8 ) != kWorldMeshHeaderSize || ReadU32( pHeader + 12 ) != 0 ||
	     ReadU32( pHeader + 52 ) != 0 || ReadU64( pHeader + 120 ) != source.Size() )
		return false;
	pBytes->resize( size_t( source.Size() ) );
	return source.ReadAt( 0, pBytes->data(), pBytes->size() ) &&
	       ValidateWorldMesh( pBytes->data(), pBytes->size() ) == WorldMeshError::Ok;
}

// An LMAP v1 page or v2 layered page (world_lightmap.h owns the encoding);
// `pVersion` receives the lump version the payload's layer count fixes.
bool ReadWorldLightmap( const char *pPath, std::vector<std::byte> *pBytes, uint32_t *pVersion )
{
	FileByteSource source( pPath );
	if ( !source.IsOpen() || source.Size() < 80 || source.Size() > kWorldLightmapMaxBytes )
		return false;
	pBytes->resize( size_t( source.Size() ) );
	if ( !source.ReadAt( 0, pBytes->data(), pBytes->size() ) )
		return false;
	WorldLightmapLayout layout{};
	const WorldLightmapError error =
	    ValidateWorldLightmap( pBytes->data(), pBytes->size(), 0, &layout );
	if ( error != WorldLightmapError::Ok )
	{
		std::fprintf( stderr, "bsp2tool: LMAP %s\n", WorldLightmapErrorName( error ) );
		return false;
	}
	*pVersion = layout.version;
	return true;
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
		std::fprintf( stderr,
		    "usage: bsp2tool info|verify <map> | convert|export <in> <out> | "
		    "pack-world <legacy.bsp> <world.wmsh> <out.bsp2> | "
		    "pack-world-lit <legacy.bsp> <world.wmsh> <atlas.ktx2> <out.bsp2>\n" );
		return 2;
	}
	const std::string command = argv[1];
	const bool bTwoPaths = command == "convert" || command == "export";
	const bool bPackWorld = command == "pack-world";
	const bool bPackWorldLit = command == "pack-world-lit";
	if ( !bTwoPaths && !bPackWorld && !bPackWorldLit && command != "info" && command != "verify" )
	{
		std::fprintf( stderr, "bsp2tool: unknown command '%s'\n", command.c_str() );
		return 2;
	}
	if ( argc != ( bPackWorldLit ? 6 : bPackWorld ? 5 : bTwoPaths ? 4 : 3 ) )
	{
		std::fprintf( stderr, "bsp2tool: wrong argument count for '%s'\n", command.c_str() );
		return 2;
	}
	if ( !bTwoPaths && !bPackWorld && !bPackWorldLit )
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
	std::vector<std::byte> worldMesh;
	if ( ( bPackWorld || bPackWorldLit ) && !ReadWorldMesh( argv[3], &worldMesh ) )
	{
		std::fprintf( stderr, "bsp2tool: invalid WMSH file %s\n", argv[3] );
		return 2;
	}
	std::vector<std::byte> lightmap;
	uint32_t lightmapVersion = kWorldLightmapMinVersion;
	if ( bPackWorldLit && !ReadWorldLightmap( argv[4], &lightmap, &lightmapVersion ) )
	{
		std::fprintf( stderr, "bsp2tool: invalid LMAP (linear RGBA16F KTX2) file %s\n", argv[4] );
		return 2;
	}
	const char *pOutput = argv[bPackWorldLit ? 5 : bPackWorld ? 4 : 3];
	const std::string temp = std::string( pOutput ) + ".tmp";
	FileByteSink sink( temp );
	// The lump version repeats the validated payload's own version.
	const uint32_t worldMeshVersion =
	    worldMesh.empty() ? kWorldMeshVersion : ReadU32( worldMesh.data() + 4 );
	const Bsp2LumpInput worldLump{
	    kLumpWorldMesh, worldMeshVersion, 0, kBsp2BulkAlignment, worldMesh };
	const Bsp2LumpInput lightmapLump{
	    kLumpWorldLightmap, lightmapVersion, 0, kBsp2BulkAlignment, lightmap };
	const std::array<Bsp2LumpInput, 2> litLumps = { worldLump, lightmapLump };
	const MapContainerStatus status =
	    command == "export" ? ExportLegacyFromBsp2( source, sink )
	    : bPackWorldLit     ? ConvertLegacyToBsp2( source, sink, litLumps )
	    : bPackWorld        ? ConvertLegacyToBsp2( source, sink, std::span( &worldLump, 1 ) )
	                        : ConvertLegacyToBsp2( source, sink );
	if ( !status.Ok() )
	{
		sink.Finish();
		std::remove( temp.c_str() );
		ReportError( command.c_str(), status );
		return status.code == MapContainerError::WriteFailed ? 2 : 1;
	}
	if ( !sink.Finish() || std::rename( temp.c_str(), pOutput ) != 0 )
	{
		std::remove( temp.c_str() );
		std::fprintf( stderr, "bsp2tool: cannot write %s\n", pOutput );
		return 2;
	}
	return 0;
}
