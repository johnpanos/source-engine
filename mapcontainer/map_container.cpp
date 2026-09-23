//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy VBSP and BSP2 readers behind IMapContainer, BSP2 writer and
//          lossless legacy conversion (RFC 0008, phase F1).
//
//=============================================================================//

#include "mapcontainer/map_container.h"
#include "mapcontainer/map_container_builder.h"

#include "blake2b.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace mapcontainer
{

static MapContainerStatus WalkLegacyGaps( IMapByteSource &source, const MapLumpInfo &gapInfo,
    const std::array<std::pair<uint64_t, uint64_t>, kLegacyLumpCount + 1> &covered,
    size_t coveredCount, uint64_t legacySize, IMapByteSink *pSink );

namespace
{
using foundation::Expected;
using foundation::MakeUnexpected;

static_assert( kLegacyHeaderSize == 1036 );

MapContainerStatus Fail( MapContainerError code, uint32_t fourcc = 0, uint64_t offset = 0 ) noexcept
{
	return MapContainerStatus{ code, fourcc, offset };
}

constexpr MapContainerStatus kOk{ MapContainerError::Ok, 0, 0 };

uint32_t Load32( const uint8_t *p ) noexcept
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

uint64_t Load64( const uint8_t *p ) noexcept
{
	return uint64_t( Load32( p ) ) | ( uint64_t( Load32( p + 4 ) ) << 32 );
}

void Store32( uint8_t *p, uint32_t value ) noexcept
{
	for ( int i = 0; i < 4; ++i )
		p[i] = uint8_t( value >> ( 8 * i ) );
}

void Store64( uint8_t *p, uint64_t value ) noexcept
{
	Store32( p, uint32_t( value ) );
	Store32( p + 4, uint32_t( value >> 32 ) );
}

bool IsPowerOfTwo( uint64_t value ) noexcept
{
	return value != 0 && ( value & ( value - 1 ) ) == 0;
}

// a + b <= limit without overflow.
bool FitsWithin( uint64_t a, uint64_t b, uint64_t limit ) noexcept
{
	return a <= limit && b <= limit - a;
}

void Hash( const void *pData, uint64_t size, uint8_t *pDigest ) noexcept
{
	detail::Blake2b hasher( kBsp2HashSize );
	const uint8_t *pBytes = static_cast<const uint8_t *>( pData );
	// Update takes size_t; feed in bounded chunks for 32-bit hosts.
	while ( size > 0 )
	{
		const size_t chunk = size_t( std::min<uint64_t>( size, uint64_t( 1 ) << 30 ) );
		hasher.Update( pBytes, chunk );
		pBytes += chunk;
		size -= chunk;
	}
	hasher.Final( pDigest );
}

bool IsLegacyFourCC( uint32_t fourcc, int *pIndex ) noexcept
{
	const char c0 = char( fourcc & 0xFF );
	const char c1 = char( ( fourcc >> 8 ) & 0xFF );
	const char c2 = char( ( fourcc >> 16 ) & 0xFF );
	const char c3 = char( ( fourcc >> 24 ) & 0xFF );
	if ( c0 != 'L' || c1 < '0' || c1 > '9' || c2 < '0' || c2 > '9' || c3 < '0' || c3 > '9' )
		return false;
	const int index = ( c1 - '0' ) * 100 + ( c2 - '0' ) * 10 + ( c3 - '0' );
	if ( index >= kLegacyLumpCount )
		return false;
	if ( pIndex )
		*pIndex = index;
	return true;
}

struct LegacyLumpRecord
{
	int32_t fileofs;
	int32_t filelen;
	int32_t version;
	int32_t uncompressedSize;
};

struct LegacyHeader
{
	uint32_t ident = 0;
	int32_t version = 0;
	LegacyLumpRecord lumps[kLegacyLumpCount] = {};
	int32_t mapRevision = 0;
};

LegacyHeader DecodeLegacyHeader( const uint8_t *p ) noexcept
{
	LegacyHeader header;
	header.ident = Load32( p );
	header.version = int32_t( Load32( p + 4 ) );
	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		const uint8_t *pLump = p + 8 + 16 * i;
		header.lumps[i].fileofs = int32_t( Load32( pLump ) );
		header.lumps[i].filelen = int32_t( Load32( pLump + 4 ) );
		header.lumps[i].version = int32_t( Load32( pLump + 8 ) );
		header.lumps[i].uncompressedSize = int32_t( Load32( pLump + 12 ) );
	}
	header.mapRevision = int32_t( Load32( p + 8 + 16 * kLegacyLumpCount ) );
	return header;
}

// Nonempty lumps must lie inside the legacy file. Zero-length lumps may carry
// any offset (the corpus has hundreds); they are never read.
MapContainerStatus ValidateLegacyHeader( const LegacyHeader &header, uint64_t fileSize ) noexcept
{
	if ( header.ident != kLegacyIdent )
		return Fail( MapContainerError::UnsupportedLegacyIdent );
	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		const LegacyLumpRecord &lump = header.lumps[i];
		if ( lump.filelen < 0 || lump.uncompressedSize < 0 )
			return Fail( MapContainerError::LegacyHeaderInvalid, LegacyLumpFourCC( i ) );
		if ( lump.filelen == 0 )
			continue;
		if ( lump.fileofs < int32_t( kLegacyHeaderSize ) ||
		     !FitsWithin( uint64_t( lump.fileofs ), uint64_t( lump.filelen ), fileSize ) )
			return Fail( MapContainerError::LumpOutOfBounds, LegacyLumpFourCC( i ),
			    uint64_t( uint32_t( lump.fileofs ) ) );
	}
	return kOk;
}

MapLumpInfo LegacyInfo(
    const LegacyHeader &header, int index, uint64_t containerOffset, bool bPresent ) noexcept
{
	const LegacyLumpRecord &lump = header.lumps[index];
	MapLumpInfo info{};
	info.fourcc = LegacyLumpFourCC( index );
	info.version = uint32_t( lump.version );
	info.alignment = 1;
	info.offset = bPresent ? containerOffset : 0;
	info.storedSize = bPresent ? uint64_t( lump.filelen ) : 0;
	info.uncompressedSize = info.storedSize;
	info.legacyOrigin = uint64_t( uint32_t( lump.fileofs ) );
	info.legacyUncompressedSize = uint32_t( lump.uncompressedSize );
	return info;
}

//-----------------------------------------------------------------------------
// Legacy VBSP container
//-----------------------------------------------------------------------------
class LegacyVbspContainer final : public IMapContainer
{
public:
	explicit LegacyVbspContainer( const LegacyHeader &header ) : m_Header( header ) {}

	MapContainerKind Kind() const override { return MapContainerKind::LegacyVbsp; }
	int LegacyVersion() const override { return m_Header.version; }
	int32_t MapRevision() const override { return m_Header.mapRevision; }

	bool FindLegacyLump( int index, MapLumpInfo *pInfo ) const override
	{
		if ( index < 0 || index >= kLegacyLumpCount || !pInfo )
			return false;
		*pInfo = LegacyInfo(
		    m_Header, index, uint64_t( uint32_t( m_Header.lumps[index].fileofs ) ), true );
		return true;
	}

	bool FindLump( uint32_t fourcc, MapLumpInfo *pInfo ) const override
	{
		int index = 0;
		return IsLegacyFourCC( fourcc, &index ) && FindLegacyLump( index, pInfo );
	}

	uint32_t LumpCount() const override { return kLegacyLumpCount; }
	bool LumpAt( uint32_t index, MapLumpInfo *pInfo ) const override
	{
		return FindLegacyLump( int( index ), pInfo );
	}

	MapContainerStatus VerifyContent( const MapLumpInfo &, const void *, uint64_t ) const override
	{
		return kOk;
	}

private:
	LegacyHeader m_Header;
};

//-----------------------------------------------------------------------------
// BSP2 container
//-----------------------------------------------------------------------------
struct Bsp2Header
{
	uint32_t containerVersion = 0;
	uint32_t flags = 0;
	uint32_t headerSize = 0;
	uint32_t entrySize = 0;
	uint64_t directoryOffset = 0;
	uint32_t lumpCount = 0;
	uint32_t hashAlgorithm = 0;
	int32_t mapRevision = 0;
	uint32_t reserved = 0;
	uint8_t directoryHash[kBsp2HashSize] = {};
};

class Bsp2Container final : public IMapContainer
{
public:
	Bsp2Container( const Bsp2Header &header, std::vector<MapLumpInfo> &&entries ) noexcept
	    : m_Header( header ), m_Entries( std::move( entries ) )
	{
	}

	// Called once LHDR is decoded and cross-checked.
	void SetLegacy( const LegacyHeader &legacy ) noexcept
	{
		m_Legacy = legacy;
		m_bHasLegacy = true;
	}

	MapContainerKind Kind() const override { return MapContainerKind::Bsp2; }
	int LegacyVersion() const override { return m_bHasLegacy ? m_Legacy.version : 0; }
	int32_t MapRevision() const override { return m_Header.mapRevision; }

	bool FindLegacyLump( int index, MapLumpInfo *pInfo ) const override
	{
		if ( !m_bHasLegacy || index < 0 || index >= kLegacyLumpCount || !pInfo )
			return false;
		MapLumpInfo entry{};
		const bool bPresent = FindLump( LegacyLumpFourCC( index ), &entry );
		*pInfo = LegacyInfo( m_Legacy, index, entry.offset, bPresent );
		if ( bPresent )
		{
			pInfo->flags = entry.flags;
			pInfo->alignment = entry.alignment;
			pInfo->hasHash = true;
			std::memcpy( pInfo->hash, entry.hash, sizeof( entry.hash ) );
		}
		return true;
	}

	bool FindLump( uint32_t fourcc, MapLumpInfo *pInfo ) const override
	{
		for ( const MapLumpInfo &entry : m_Entries )
		{
			if ( entry.fourcc == fourcc )
			{
				if ( pInfo )
					*pInfo = entry;
				return true;
			}
		}
		return false;
	}

	uint32_t LumpCount() const override { return uint32_t( m_Entries.size() ); }
	bool LumpAt( uint32_t index, MapLumpInfo *pInfo ) const override
	{
		if ( index >= m_Entries.size() || !pInfo )
			return false;
		*pInfo = m_Entries[index];
		return true;
	}

	MapContainerStatus VerifyContent(
	    const MapLumpInfo &info, const void *pData, uint64_t size ) const override
	{
		if ( !info.hasHash )
			return kOk;
		if ( size != info.uncompressedSize || ( size && !pData ) )
			return Fail( MapContainerError::SizeMismatch, info.fourcc, info.offset );
		uint8_t digest[kBsp2HashSize];
		Hash( pData, size, digest );
		if ( std::memcmp( digest, info.hash, kBsp2HashSize ) != 0 )
			return Fail( MapContainerError::ContentHashMismatch, info.fourcc, info.offset );
		return kOk;
	}

private:
	Bsp2Header m_Header;
	std::vector<MapLumpInfo> m_Entries;
	LegacyHeader m_Legacy;
	bool m_bHasLegacy = false;
};

MapContainerStatus ReadExact(
    IMapByteSource &source, uint64_t offset, void *pDest, uint64_t size ) noexcept
{
	if ( !FitsWithin( offset, size, source.Size() ) )
		return Fail( MapContainerError::Truncated, 0, offset );
	uint8_t *pBytes = static_cast<uint8_t *>( pDest );
	while ( size > 0 )
	{
		const size_t chunk = size_t( std::min<uint64_t>( size, uint64_t( 1 ) << 30 ) );
		if ( !source.ReadAt( offset, pBytes, chunk ) )
			return Fail( MapContainerError::ReadFailed, 0, offset );
		pBytes += chunk;
		offset += chunk;
		size -= chunk;
	}
	return kOk;
}

bool IsKnownFourCC( uint32_t fourcc, const MapContainerOpenOptions &options ) noexcept
{
	if ( IsLegacyFourCC( fourcc, nullptr ) || fourcc == kLumpLegacyHeader ||
	     fourcc == kLumpLegacyGaps )
		return true;
	for ( size_t i = 0; i < options.nKnownFourCCs; ++i )
	{
		if ( options.pKnownFourCCs && options.pKnownFourCCs[i] == fourcc )
			return true;
	}
	return false;
}

MapContainerStatus ReadLumpBytes(
    IMapByteSource &source, const MapLumpInfo &info, std::vector<uint8_t> &bytes ) noexcept
{
	if ( info.storedSize > source.Size() )
		return Fail( MapContainerError::LumpOutOfBounds, info.fourcc, info.offset );
	if ( info.storedSize > std::numeric_limits<size_t>::max() )
		return Fail( MapContainerError::OutOfMemory, info.fourcc, info.offset );
	if ( info.storedSize > bytes.max_size() )
		return Fail( MapContainerError::OutOfMemory, info.fourcc, info.offset );
	try
	{
		bytes.resize( size_t( info.storedSize ) );
	}
	catch ( const std::bad_alloc & )
	{
		return Fail( MapContainerError::OutOfMemory, info.fourcc, info.offset );
	}
	const MapContainerStatus status = ReadExact( source, info.offset, bytes.data(), bytes.size() );
	return status.Ok() ? status : Fail( status.code, info.fourcc, info.offset );
}

MapContainerStatus HashLump(
    IMapByteSource &source, const MapLumpInfo &info, uint8_t *pDigest ) noexcept
{
	uint8_t bytes[64 * 1024];
	detail::Blake2b hasher( kBsp2HashSize );
	uint64_t consumed = 0;
	while ( consumed < info.storedSize )
	{
		const size_t chunk =
		    size_t( std::min<uint64_t>( sizeof( bytes ), info.storedSize - consumed ) );
		const MapContainerStatus status = ReadExact( source, info.offset + consumed, bytes, chunk );
		if ( !status.Ok() )
			return Fail( status.code, info.fourcc, info.offset + consumed );
		hasher.Update( bytes, chunk );
		consumed += chunk;
	}
	hasher.Final( pDigest );
	return kOk;
}

MapContainerStatus OpenBsp2( IMapByteSource &source, const MapContainerOpenOptions &options,
    IMapContainer **ppContainer ) noexcept
{
	const uint64_t fileSize = source.Size();
	uint8_t raw[kBsp2HeaderSize];
	MapContainerStatus status = ReadExact( source, 0, raw, sizeof( raw ) );
	if ( !status.Ok() )
		return status;

	Bsp2Header header;
	header.containerVersion = Load32( raw + 8 );
	header.flags = Load32( raw + 12 );
	header.headerSize = Load32( raw + 16 );
	header.entrySize = Load32( raw + 20 );
	header.directoryOffset = Load64( raw + 24 );
	header.lumpCount = Load32( raw + 32 );
	header.hashAlgorithm = Load32( raw + 36 );
	header.mapRevision = int32_t( Load32( raw + 40 ) );
	header.reserved = Load32( raw + 44 );
	std::memcpy( header.directoryHash, raw + 48, kBsp2HashSize );

	if ( header.containerVersion != kBsp2ContainerVersion )
		return Fail( MapContainerError::UnsupportedContainerVersion );
	if ( header.flags != 0 || header.headerSize != kBsp2HeaderSize ||
	     header.entrySize != kBsp2EntrySize || header.reserved != 0 )
		return Fail( MapContainerError::UnsupportedLayout );
	if ( header.hashAlgorithm != kBsp2HashBlake2b128 )
		return Fail( MapContainerError::UnsupportedHash );
	if ( header.lumpCount > kBsp2MaxLumps )
		return Fail( MapContainerError::TooManyLumps );

	const uint64_t directorySize = uint64_t( header.lumpCount ) * kBsp2EntrySize;
	if ( header.directoryOffset < kBsp2HeaderSize ||
	     !FitsWithin( header.directoryOffset, directorySize, fileSize ) )
		return Fail( MapContainerError::DirectoryOutOfBounds, 0, header.directoryOffset );

	std::vector<uint8_t> directory;
	try
	{
		directory.resize( size_t( directorySize ) );
	}
	catch ( const std::bad_alloc & )
	{
		return Fail( MapContainerError::OutOfMemory );
	}
	status = ReadExact( source, header.directoryOffset, directory.data(), directory.size() );
	if ( !status.Ok() )
		return status;

	uint8_t digest[kBsp2HashSize];
	Hash( directory.data(), directory.size(), digest );
	if ( std::memcmp( digest, header.directoryHash, kBsp2HashSize ) != 0 )
		return Fail( MapContainerError::DirectoryHashMismatch, 0, header.directoryOffset );

	std::vector<MapLumpInfo> entries;
	try
	{
		entries.resize( header.lumpCount );
	}
	catch ( const std::bad_alloc & )
	{
		return Fail( MapContainerError::OutOfMemory );
	}

	for ( uint32_t i = 0; i < header.lumpCount; ++i )
	{
		const uint8_t *p = directory.data() + size_t( i ) * kBsp2EntrySize;
		MapLumpInfo &entry = entries[i];
		entry.fourcc = Load32( p );
		entry.version = Load32( p + 4 );
		entry.flags = Load32( p + 8 );
		entry.alignment = Load32( p + 12 );
		entry.offset = Load64( p + 16 );
		entry.storedSize = Load64( p + 24 );
		entry.uncompressedSize = Load64( p + 32 );
		const uint64_t reserved = Load64( p + 40 );
		entry.hasHash = true;
		std::memcpy( entry.hash, p + 48, kBsp2HashSize );
		entry.legacyOrigin = entry.offset;

		if ( reserved != 0 )
			return Fail( MapContainerError::UnsupportedLayout, entry.fourcc );
		if ( entry.flags & ~kBsp2KnownFlags )
			return Fail( MapContainerError::UnknownFlags, entry.fourcc );
		if ( !IsPowerOfTwo( entry.alignment ) || entry.alignment < kBsp2MinAlignment )
			return Fail( MapContainerError::LumpMisaligned, entry.fourcc, entry.offset );
		if ( entry.offset % entry.alignment != 0 )
			return Fail( MapContainerError::LumpMisaligned, entry.fourcc, entry.offset );
		// Payloads live strictly between the header and the directory.
		if ( entry.offset < kBsp2HeaderSize ||
		     !FitsWithin( entry.offset, entry.storedSize, header.directoryOffset ) )
			return Fail( MapContainerError::LumpOutOfBounds, entry.fourcc, entry.offset );

		const bool bKnown = IsKnownFourCC( entry.fourcc, options );
		if ( !bKnown && ( entry.flags & kBsp2FlagRequired ) && !options.allowUnknownRequired )
			return Fail( MapContainerError::UnknownRequiredLump, entry.fourcc );
		const uint32_t compression = entry.flags & kBsp2CompressionMask;
		if ( compression != kBsp2CompressionNone )
		{
			// Unknown optional lumps are skipped whatever their encoding.
			if ( bKnown || ( entry.flags & kBsp2FlagRequired ) )
				return Fail( MapContainerError::UnsupportedCompression, entry.fourcc );
		}
		else if ( entry.storedSize != entry.uncompressedSize )
		{
			return Fail( MapContainerError::SizeMismatch, entry.fourcc, entry.offset );
		}
	}

	// Duplicates and overlaps, via an offset-sorted view.
	std::vector<uint32_t> order( entries.size() );
	for ( uint32_t i = 0; i < order.size(); ++i )
		order[i] = i;
	std::sort( order.begin(), order.end(),
	    [&]( uint32_t a, uint32_t b )
	    {
		    return entries[a].offset < entries[b].offset ||
		           ( entries[a].offset == entries[b].offset &&
		               entries[a].storedSize < entries[b].storedSize );
	    } );
	uint64_t previousEnd = kBsp2HeaderSize;
	for ( uint32_t index : order )
	{
		const MapLumpInfo &entry = entries[index];
		if ( entry.storedSize == 0 )
			continue;
		if ( entry.offset < previousEnd )
			return Fail( MapContainerError::LumpOverlap, entry.fourcc, entry.offset );
		previousEnd = entry.offset + entry.storedSize;
	}
	std::vector<uint32_t> ids( entries.size() );
	for ( size_t i = 0; i < entries.size(); ++i )
		ids[i] = entries[i].fourcc;
	std::sort( ids.begin(), ids.end() );
	const auto duplicate = std::adjacent_find( ids.begin(), ids.end() );
	if ( duplicate != ids.end() )
		return Fail( MapContainerError::DuplicateLump, *duplicate );

	if ( options.verifyContent )
	{
		for ( const MapLumpInfo &entry : entries )
		{
			if ( ( entry.flags & kBsp2CompressionMask ) != kBsp2CompressionNone )
				continue; // unknown lump in an encoding we cannot read
			status = HashLump( source, entry, digest );
			if ( !status.Ok() )
				return status;
			if ( std::memcmp( digest, entry.hash, kBsp2HashSize ) != 0 )
				return Fail( MapContainerError::ContentHashMismatch, entry.fourcc, entry.offset );
		}
	}

	Bsp2Container *pContainer = new ( std::nothrow ) Bsp2Container( header, std::move( entries ) );
	if ( !pContainer )
		return Fail( MapContainerError::OutOfMemory );

	// Legacy payload: LHDR is mandatory whenever Lnnn or LGAP is carried, and
	// the carried lumps and gaps must agree with it.
	MapLumpInfo legacyHeaderInfo{};
	bool bAnyLegacy = false;
	for ( uint32_t i = 0; i < pContainer->LumpCount(); ++i )
	{
		MapLumpInfo entry{};
		pContainer->LumpAt( i, &entry );
		bAnyLegacy = bAnyLegacy || IsLegacyFourCC( entry.fourcc, nullptr ) ||
		             entry.fourcc == kLumpLegacyGaps;
	}
	if ( pContainer->FindLump( kLumpLegacyHeader, &legacyHeaderInfo ) )
	{
		if ( legacyHeaderInfo.storedSize != kLegacyHeaderLumpSize )
		{
			delete pContainer;
			return Fail( MapContainerError::LegacyHeaderInvalid, kLumpLegacyHeader );
		}
		std::vector<uint8_t> bytes;
		status = ReadLumpBytes( source, legacyHeaderInfo, bytes );
		if ( status.Ok() )
			status = pContainer->VerifyContent( legacyHeaderInfo, bytes.data(), bytes.size() );
		LegacyHeader legacy;
		uint64_t legacyFileSize = 0;
		if ( status.Ok() )
		{
			legacyFileSize = Load64( bytes.data() );
			legacy = DecodeLegacyHeader( bytes.data() + 8 );
			status = ValidateLegacyHeader( legacy, legacyFileSize );
			if ( !status.Ok() )
				status =
				    Fail( MapContainerError::LegacyHeaderInvalid, status.fourcc, status.offset );
			if ( status.Ok() && legacyFileSize < kLegacyHeaderSize )
				status = Fail( MapContainerError::LegacyHeaderInvalid, kLumpLegacyHeader );
			if ( status.Ok() && legacy.mapRevision != header.mapRevision )
				status = Fail( MapContainerError::LegacyHeaderInvalid, kLumpLegacyHeader );
		}
		for ( int i = 0; status.Ok() && i < kLegacyLumpCount; ++i )
		{
			MapLumpInfo entry{};
			const bool bPresent = pContainer->FindLump( LegacyLumpFourCC( i ), &entry );
			const uint64_t legacyLength = uint64_t( legacy.lumps[i].filelen );
			if ( bPresent ? entry.storedSize != legacyLength : legacyLength != 0 )
				status = Fail( MapContainerError::LegacyLumpMismatch, LegacyLumpFourCC( i ) );
			else if ( bPresent && entry.version != uint32_t( legacy.lumps[i].version ) )
				status = Fail( MapContainerError::LegacyLumpMismatch, LegacyLumpFourCC( i ) );
		}
		if ( !status.Ok() )
		{
			delete pContainer;
			return status;
		}
		MapLumpInfo gapInfo{};
		if ( pContainer->FindLump( kLumpLegacyGaps, &gapInfo ) )
		{
			std::array<std::pair<uint64_t, uint64_t>, kLegacyLumpCount + 1> covered{};
			size_t coveredCount = 0;
			covered[coveredCount++] = { 0, kLegacyHeaderSize };
			for ( int i = 0; i < kLegacyLumpCount; ++i )
			{
				const LegacyLumpRecord &lump = legacy.lumps[i];
				if ( lump.filelen > 0 )
					covered[coveredCount++] = { uint64_t( lump.fileofs ),
					    uint64_t( lump.fileofs ) + uint64_t( lump.filelen ) };
			}
			status =
			    WalkLegacyGaps( source, gapInfo, covered, coveredCount, legacyFileSize, nullptr );
			if ( !status.Ok() )
			{
				delete pContainer;
				return status;
			}
		}
		pContainer->SetLegacy( legacy );
	}
	else if ( bAnyLegacy )
	{
		delete pContainer;
		return Fail( MapContainerError::MissingLegacyHeader, kLumpLegacyHeader );
	}

	*ppContainer = pContainer;
	return kOk;
}

MapContainerStatus OpenLegacy( IMapByteSource &source, IMapContainer **ppContainer ) noexcept
{
	uint8_t raw[kLegacyHeaderSize];
	const MapContainerStatus status = ReadExact( source, 0, raw, sizeof( raw ) );
	if ( !status.Ok() )
		return status;
	const LegacyHeader header = DecodeLegacyHeader( raw );
	const MapContainerStatus valid = ValidateLegacyHeader( header, source.Size() );
	if ( !valid.Ok() )
		return valid;
	IMapContainer *pContainer = new ( std::nothrow ) LegacyVbspContainer( header );
	if ( !pContainer )
		return Fail( MapContainerError::OutOfMemory );
	*ppContainer = pContainer;
	return kOk;
}

uint64_t AlignUp( uint64_t value, uint64_t alignment ) noexcept
{
	return ( value + alignment - 1 ) & ~( alignment - 1 );
}
} // namespace

//-----------------------------------------------------------------------------
// Public reader API
//-----------------------------------------------------------------------------
const char *MapContainerErrorName( MapContainerError error )
{
	switch ( error )
	{
	case MapContainerError::Ok:
		return "ok";
	case MapContainerError::ReadFailed:
		return "read-failed";
	case MapContainerError::Truncated:
		return "truncated";
	case MapContainerError::BadMagic:
		return "bad-magic";
	case MapContainerError::UnsupportedLegacyIdent:
		return "unsupported-legacy-ident";
	case MapContainerError::UnsupportedContainerVersion:
		return "unsupported-container-version";
	case MapContainerError::UnsupportedLayout:
		return "unsupported-layout";
	case MapContainerError::UnsupportedHash:
		return "unsupported-hash";
	case MapContainerError::TooManyLumps:
		return "too-many-lumps";
	case MapContainerError::DirectoryOutOfBounds:
		return "directory-out-of-bounds";
	case MapContainerError::DirectoryHashMismatch:
		return "directory-hash-mismatch";
	case MapContainerError::LumpOutOfBounds:
		return "lump-out-of-bounds";
	case MapContainerError::LumpMisaligned:
		return "lump-misaligned";
	case MapContainerError::LumpOverlap:
		return "lump-overlap";
	case MapContainerError::DuplicateLump:
		return "duplicate-lump";
	case MapContainerError::UnknownFlags:
		return "unknown-flags";
	case MapContainerError::UnsupportedCompression:
		return "unsupported-compression";
	case MapContainerError::SizeMismatch:
		return "size-mismatch";
	case MapContainerError::UnknownRequiredLump:
		return "unknown-required-lump";
	case MapContainerError::ContentHashMismatch:
		return "content-hash-mismatch";
	case MapContainerError::MissingLegacyHeader:
		return "missing-legacy-header";
	case MapContainerError::LegacyHeaderInvalid:
		return "legacy-header-invalid";
	case MapContainerError::LegacyLumpMismatch:
		return "legacy-lump-mismatch";
	case MapContainerError::LegacyGapsInvalid:
		return "legacy-gaps-invalid";
	case MapContainerError::OutOfMemory:
		return "out-of-memory";
	case MapContainerError::InvalidArgument:
		return "invalid-argument";
	case MapContainerError::WriteFailed:
		return "write-failed";
	}
	return "unknown";
}

bool IsBsp2Magic( const void *pPrefix, size_t size )
{
	return pPrefix && size >= sizeof( kBsp2Magic ) &&
	       std::memcmp( pPrefix, kBsp2Magic, sizeof( kBsp2Magic ) ) == 0;
}

MapContainerStatus OpenMapContainer(
    IMapByteSource &source, const MapContainerOpenOptions &options, IMapContainer **ppContainer )
{
	if ( !ppContainer )
		return Fail( MapContainerError::InvalidArgument );
	*ppContainer = nullptr;
	uint8_t prefix[8];
	const MapContainerStatus status = ReadExact( source, 0, prefix, sizeof( prefix ) );
	if ( !status.Ok() )
		return status;
	if ( IsBsp2Magic( prefix, sizeof( prefix ) ) )
		return OpenBsp2( source, options, ppContainer );
	if ( Load32( prefix ) == kLegacyIdent )
		return OpenLegacy( source, ppContainer );
	return Fail( MapContainerError::BadMagic );
}

void DestroyMapContainer( IMapContainer *pContainer )
{
	delete pContainer;
}

//-----------------------------------------------------------------------------
// Builder API
//-----------------------------------------------------------------------------
bool MemoryByteSource::ReadAt( uint64_t offset, void *pDest, size_t size )
{
	if ( !FitsWithin( offset, size, m_Bytes.size() ) )
		return false;
	if ( size )
		std::memcpy( pDest, m_Bytes.data() + offset, size );
	return true;
}

Expected<ContainerPtr, MapContainerStatus> OpenMemoryContainer(
    MemoryByteSource &source, bool verifyContent, bool allowUnknownRequired )
{
	MapContainerOpenOptions options{ verifyContent, nullptr, 0, allowUnknownRequired };
	IMapContainer *pContainer = nullptr;
	const MapContainerStatus status = OpenMapContainer( source, options, &pContainer );
	if ( !status.Ok() )
		return MakeUnexpected( status );
	return ContainerPtr( pContainer );
}

ContentHash HashContent( std::span<const std::byte> bytes ) noexcept
{
	ContentHash digest{};
	Hash( bytes.data(), bytes.size(), digest.data() );
	return digest;
}

namespace
{
struct LayoutLump
{
	uint32_t fourcc = 0;
	uint32_t version = 0;
	uint32_t flags = 0;
	uint32_t alignment = kBsp2MinAlignment;
	uint64_t size = 0;
	uint64_t offset = 0;
	uint8_t hash[kBsp2HashSize] = {};
};

MapContainerStatus PlaceLumps(
    std::span<LayoutLump> lumps, uint64_t &directoryOffset, uint64_t &totalSize ) noexcept
{
	if ( lumps.size() > kBsp2MaxLumps )
		return Fail( MapContainerError::TooManyLumps );
	uint64_t cursor = kBsp2HeaderSize;
	for ( size_t i = 0; i < lumps.size(); ++i )
	{
		LayoutLump &lump = lumps[i];
		if ( !IsPowerOfTwo( lump.alignment ) || lump.alignment < kBsp2MinAlignment )
			return Fail( MapContainerError::LumpMisaligned, lump.fourcc );
		if ( lump.flags & ~kBsp2KnownFlags )
			return Fail( MapContainerError::UnknownFlags, lump.fourcc );
		if ( ( lump.flags & kBsp2CompressionMask ) != kBsp2CompressionNone )
			return Fail( MapContainerError::UnsupportedCompression, lump.fourcc );
		for ( size_t j = 0; j < i; ++j )
		{
			if ( lumps[j].fourcc == lump.fourcc )
				return Fail( MapContainerError::DuplicateLump, lump.fourcc );
		}
		if ( cursor > std::numeric_limits<uint64_t>::max() - ( lump.alignment - 1 ) )
			return Fail( MapContainerError::UnsupportedLayout, lump.fourcc );
		cursor = AlignUp( cursor, lump.alignment );
		lump.offset = cursor;
		if ( lump.size > std::numeric_limits<uint64_t>::max() - cursor )
			return Fail( MapContainerError::UnsupportedLayout, lump.fourcc );
		cursor += lump.size;
	}
	if ( cursor > std::numeric_limits<uint64_t>::max() - ( kBsp2MinAlignment - 1 ) )
		return Fail( MapContainerError::UnsupportedLayout );
	directoryOffset = AlignUp( cursor, kBsp2MinAlignment );
	const uint64_t directorySize = uint64_t( lumps.size() ) * kBsp2EntrySize;
	if ( directorySize > std::numeric_limits<uint64_t>::max() - directoryOffset )
		return Fail( MapContainerError::UnsupportedLayout );
	totalSize = directoryOffset + directorySize;
	return kOk;
}

void EncodeHeaderDirectory( int32_t mapRevision, std::span<const LayoutLump> lumps,
    uint64_t directoryOffset, uint8_t *pHeader, uint8_t *pDirectory ) noexcept
{
	for ( size_t i = 0; i < lumps.size(); ++i )
	{
		const LayoutLump &lump = lumps[i];
		uint8_t *p = pDirectory + i * kBsp2EntrySize;
		Store32( p, lump.fourcc );
		Store32( p + 4, lump.version );
		Store32( p + 8, lump.flags );
		Store32( p + 12, lump.alignment );
		Store64( p + 16, lump.offset );
		Store64( p + 24, lump.size );
		Store64( p + 32, lump.size );
		Store64( p + 40, 0 );
		std::memcpy( p + 48, lump.hash, kBsp2HashSize );
	}
	std::memcpy( pHeader, kBsp2Magic, sizeof( kBsp2Magic ) );
	Store32( pHeader + 8, kBsp2ContainerVersion );
	Store32( pHeader + 12, 0 );
	Store32( pHeader + 16, kBsp2HeaderSize );
	Store32( pHeader + 20, kBsp2EntrySize );
	Store64( pHeader + 24, directoryOffset );
	Store32( pHeader + 32, uint32_t( lumps.size() ) );
	Store32( pHeader + 36, kBsp2HashBlake2b128 );
	Store32( pHeader + 40, uint32_t( mapRevision ) );
	Store32( pHeader + 44, 0 );
	Hash( pDirectory, lumps.size() * kBsp2EntrySize, pHeader + 48 );
}
} // namespace

Expected<std::vector<std::byte>, MapContainerStatus> WriteBsp2(
    int32_t mapRevision, std::span<const Bsp2LumpInput> lumps )
{
	if ( lumps.size() > kBsp2MaxLumps )
		return MakeUnexpected( Fail( MapContainerError::TooManyLumps ) );
	std::vector<LayoutLump> layout;
	try
	{
		layout.resize( lumps.size() );
		for ( size_t i = 0; i < lumps.size(); ++i )
		{
			const Bsp2LumpInput &lump = lumps[i];
			layout[i].fourcc = lump.fourcc;
			layout[i].version = lump.version;
			layout[i].flags = lump.flags;
			layout[i].alignment = lump.alignment;
			layout[i].size = lump.data.size();
		}
	}
	catch ( const std::bad_alloc & )
	{
		return MakeUnexpected( Fail( MapContainerError::OutOfMemory ) );
	}
	uint64_t directoryOffset = 0;
	uint64_t totalSize = 0;
	const MapContainerStatus placed = PlaceLumps( layout, directoryOffset, totalSize );
	if ( !placed.Ok() )
		return MakeUnexpected( placed );
	for ( size_t i = 0; i < lumps.size(); ++i )
		Hash( lumps[i].data.data(), lumps[i].data.size(), layout[i].hash );

	std::vector<std::byte> file;
	if ( totalSize > file.max_size() )
		return MakeUnexpected( Fail( MapContainerError::OutOfMemory ) );
	try
	{
		file.assign( size_t( totalSize ), std::byte{ 0 } );
	}
	catch ( const std::bad_alloc & )
	{
		return MakeUnexpected( Fail( MapContainerError::OutOfMemory ) );
	}
	uint8_t *pFile = reinterpret_cast<uint8_t *>( file.data() );

	for ( size_t i = 0; i < lumps.size(); ++i )
	{
		const Bsp2LumpInput &lump = lumps[i];
		if ( !lump.data.empty() )
			std::memcpy( pFile + layout[i].offset, lump.data.data(), lump.data.size() );
	}
	EncodeHeaderDirectory( mapRevision, layout, directoryOffset, pFile, pFile + directoryOffset );
	return file;
}

namespace
{
class VectorByteSink final : public IMapByteSink
{
public:
	bool ResetToZeroes( uint64_t size ) override
	{
		if ( size > m_Bytes.max_size() )
			return false;
		try
		{
			m_Bytes.assign( size_t( size ), std::byte{ 0 } );
		}
		catch ( const std::bad_alloc & )
		{
			return false;
		}
		return true;
	}
	bool WriteAt( uint64_t offset, const void *pData, size_t size ) override
	{
		if ( offset > m_Bytes.size() || size > m_Bytes.size() - offset )
			return false;
		if ( size )
			std::memcpy( m_Bytes.data() + size_t( offset ), pData, size );
		return true;
	}
	std::vector<std::byte> m_Bytes;
};
} // namespace

Expected<std::vector<std::byte>, MapContainerStatus> ConvertLegacyToBsp2(
    std::span<const std::byte> legacyFile, std::span<const Bsp2LumpInput> extraLumps )
{
	MemoryByteSource source( legacyFile );
	VectorByteSink sink;
	MapContainerStatus status = ConvertLegacyToBsp2( source, sink, extraLumps );
	if ( status.code == MapContainerError::WriteFailed )
		status = Fail( MapContainerError::OutOfMemory, status.fourcc, status.offset );
	if ( !status.Ok() )
		return MakeUnexpected( status );
	return std::move( sink.m_Bytes );
}

MapContainerStatus CopyToSink( IMapByteSource &source, IMapByteSink &sink, uint64_t sourceOffset,
    uint64_t outputOffset, uint64_t size, uint32_t fourcc,
    detail::Blake2b *pHasher = nullptr ) noexcept
{
	uint8_t bytes[64 * 1024];
	uint64_t copied = 0;
	while ( copied < size )
	{
		const size_t chunk = size_t( std::min<uint64_t>( sizeof( bytes ), size - copied ) );
		const MapContainerStatus status = ReadExact( source, sourceOffset + copied, bytes, chunk );
		if ( !status.Ok() )
			return Fail( status.code, fourcc, sourceOffset + copied );
		if ( !sink.WriteAt( outputOffset + copied, bytes, chunk ) )
			return Fail( MapContainerError::WriteFailed, fourcc, outputOffset + copied );
		if ( pHasher )
			pHasher->Update( bytes, chunk );
		copied += chunk;
	}
	return kOk;
}

namespace
{
template <typename RunFn>
MapContainerStatus ScanLegacyGapRuns( IMapByteSource &source,
    std::span<const std::pair<uint64_t, uint64_t>> covered, uint64_t legacySize, RunFn &&onRun )
{
	uint8_t bytes[64 * 1024];
	auto scanRange = [&]( uint64_t begin, uint64_t end ) -> MapContainerStatus
	{
		bool inRun = false;
		uint64_t runStart = 0;
		for ( uint64_t cursor = begin; cursor < end; )
		{
			const size_t chunk = size_t( std::min<uint64_t>( sizeof( bytes ), end - cursor ) );
			const MapContainerStatus read = ReadExact( source, cursor, bytes, chunk );
			if ( !read.Ok() )
				return read;
			for ( size_t i = 0; i < chunk; ++i )
			{
				if ( bytes[i] != 0 && !inRun )
				{
					inRun = true;
					runStart = cursor + i;
				}
				else if ( bytes[i] == 0 && inRun )
				{
					const MapContainerStatus status = onRun( runStart, cursor + i - runStart );
					if ( !status.Ok() )
						return status;
					inRun = false;
				}
			}
			cursor += chunk;
		}
		return inRun ? onRun( runStart, end - runStart ) : kOk;
	};

	uint64_t coveredEnd = 0;
	for ( const auto &range : covered )
	{
		if ( range.first > coveredEnd )
		{
			const MapContainerStatus status = scanRange( coveredEnd, range.first );
			if ( !status.Ok() )
				return status;
		}
		coveredEnd = std::max( coveredEnd, range.second );
	}
	return coveredEnd < legacySize ? scanRange( coveredEnd, legacySize ) : kOk;
}
} // namespace

MapContainerStatus ConvertLegacyToBsp2(
    IMapByteSource &source, IMapByteSink &sink, std::span<const Bsp2LumpInput> extraLumps )
{
	const uint64_t legacySize = source.Size();
	if ( legacySize < kLegacyHeaderSize )
		return Fail( MapContainerError::Truncated );
	uint8_t header[kLegacyHeaderSize];
	MapContainerStatus status = ReadExact( source, 0, header, sizeof( header ) );
	if ( !status.Ok() )
		return status;
	if ( IsBsp2Magic( header, sizeof( header ) ) )
		return Fail( MapContainerError::InvalidArgument );
	const LegacyHeader legacy = DecodeLegacyHeader( header );
	status = ValidateLegacyHeader( legacy, legacySize );
	if ( !status.Ok() )
		return status;

	std::array<std::pair<uint64_t, uint64_t>, kLegacyLumpCount + 1> covered{};
	size_t coveredCount = 0;
	covered[coveredCount++] = { 0, kLegacyHeaderSize };
	for ( const LegacyLumpRecord &lump : legacy.lumps )
	{
		if ( lump.filelen > 0 )
			covered[coveredCount++] = {
			    uint64_t( lump.fileofs ), uint64_t( lump.fileofs ) + uint64_t( lump.filelen ) };
	}
	std::sort( covered.begin(), covered.begin() + coveredCount );
	const std::span<const std::pair<uint64_t, uint64_t>> ranges( covered.data(), coveredCount );
	uint64_t gapSize = 0;
	status = ScanLegacyGapRuns( source, ranges, legacySize,
	    [&]( uint64_t, uint64_t length )
	    {
		    if ( length > std::numeric_limits<uint64_t>::max() - 7 )
			    return Fail( MapContainerError::UnsupportedLayout, kLumpLegacyGaps );
		    const uint64_t padded = AlignUp( length, 8 );
		    if ( gapSize > std::numeric_limits<uint64_t>::max() - 16 ||
		         padded > std::numeric_limits<uint64_t>::max() - gapSize - 16 )
			    return Fail( MapContainerError::UnsupportedLayout, kLumpLegacyGaps );
		    gapSize += 16 + padded;
		    return kOk;
	    } );
	if ( !status.Ok() )
		return status;

	const size_t baseLumpCount = coveredCount + ( gapSize != 0 );
	if ( extraLumps.size() > kBsp2MaxLumps - baseLumpCount )
		return Fail( MapContainerError::TooManyLumps );
	std::vector<LayoutLump> layout;
	try
	{
		layout.reserve( baseLumpCount + extraLumps.size() );
		layout.push_back(
		    { kLumpLegacyHeader, 1, kBsp2FlagRequired, kBsp2MinAlignment, kLegacyHeaderLumpSize } );
		if ( gapSize )
			layout.push_back(
			    { kLumpLegacyGaps, 1, kBsp2FlagRequired, kBsp2MinAlignment, gapSize } );
		for ( int i = 0; i < kLegacyLumpCount; ++i )
		{
			const LegacyLumpRecord &lump = legacy.lumps[i];
			if ( lump.filelen > 0 )
				layout.push_back( { LegacyLumpFourCC( i ), uint32_t( lump.version ),
				    kBsp2FlagRequired, kBsp2MinAlignment, uint64_t( lump.filelen ) } );
		}
		for ( const Bsp2LumpInput &extra : extraLumps )
		{
			if ( IsKnownFourCC(
			         extra.fourcc, MapContainerOpenOptions{ false, nullptr, 0, false } ) )
				return Fail( MapContainerError::DuplicateLump, extra.fourcc );
			layout.push_back(
			    { extra.fourcc, extra.version, extra.flags, extra.alignment, extra.data.size() } );
		}
	}
	catch ( const std::bad_alloc & )
	{
		return Fail( MapContainerError::OutOfMemory );
	}
	uint64_t directoryOffset = 0;
	uint64_t totalSize = 0;
	status = PlaceLumps( layout, directoryOffset, totalSize );
	if ( !status.Ok() )
		return status;
	std::vector<uint8_t> directory;
	try
	{
		directory.resize( layout.size() * kBsp2EntrySize );
	}
	catch ( const std::bad_alloc & )
	{
		return Fail( MapContainerError::OutOfMemory );
	}
	if ( !sink.ResetToZeroes( totalSize ) )
		return Fail( MapContainerError::WriteFailed );

	uint8_t headerLump[kLegacyHeaderLumpSize];
	Store64( headerLump, legacySize );
	std::memcpy( headerLump + 8, header, sizeof( header ) );
	if ( !sink.WriteAt( layout[0].offset, headerLump, sizeof( headerLump ) ) )
		return Fail( MapContainerError::WriteFailed, kLumpLegacyHeader, layout[0].offset );
	Hash( headerLump, sizeof( headerLump ), layout[0].hash );

	size_t next = 1;
	if ( gapSize )
	{
		LayoutLump &gap = layout[next++];
		detail::Blake2b hasher( kBsp2HashSize );
		uint64_t written = 0;
		status = ScanLegacyGapRuns( source, ranges, legacySize,
		    [&]( uint64_t origin, uint64_t length )
		    {
			    if ( written > gap.size || gap.size - written < 16 ||
			         length > gap.size - written - 16 )
				    return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps );
			    uint8_t record[16];
			    Store64( record, origin );
			    Store64( record + 8, length );
			    if ( !sink.WriteAt( gap.offset + written, record, sizeof( record ) ) )
				    return Fail(
				        MapContainerError::WriteFailed, kLumpLegacyGaps, gap.offset + written );
			    hasher.Update( record, sizeof( record ) );
			    written += sizeof( record );
			    const MapContainerStatus copied = CopyToSink(
			        source, sink, origin, gap.offset + written, length, kLumpLegacyGaps, &hasher );
			    if ( !copied.Ok() )
				    return copied;
			    written += length;
			    const uint8_t zeroes[7] = {};
			    const size_t padding = size_t( AlignUp( written, 8 ) - written );
			    if ( padding > gap.size - written )
				    return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps );
			    if ( padding && !sink.WriteAt( gap.offset + written, zeroes, padding ) )
				    return Fail(
				        MapContainerError::WriteFailed, kLumpLegacyGaps, gap.offset + written );
			    hasher.Update( zeroes, padding );
			    written += padding;
			    return kOk;
		    } );
		if ( !status.Ok() )
			return status;
		if ( written != gap.size )
			return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps );
		hasher.Final( gap.hash );
	}
	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		const LegacyLumpRecord &lump = legacy.lumps[i];
		if ( lump.filelen == 0 )
			continue;
		LayoutLump &out = layout[next++];
		detail::Blake2b hasher( kBsp2HashSize );
		status = CopyToSink(
		    source, sink, uint64_t( lump.fileofs ), out.offset, out.size, out.fourcc, &hasher );
		if ( !status.Ok() )
			return status;
		hasher.Final( out.hash );
	}
	for ( const Bsp2LumpInput &extra : extraLumps )
	{
		LayoutLump &out = layout[next++];
		const uint8_t *pData = reinterpret_cast<const uint8_t *>( extra.data.data() );
		for ( size_t copied = 0; copied < extra.data.size(); )
		{
			const size_t chunk = std::min<size_t>( 64 * 1024, extra.data.size() - copied );
			if ( !sink.WriteAt( out.offset + copied, pData + copied, chunk ) )
				return Fail( MapContainerError::WriteFailed, out.fourcc, out.offset + copied );
			copied += chunk;
		}
		Hash( pData, extra.data.size(), out.hash );
	}
	uint8_t bsp2Header[kBsp2HeaderSize] = {};
	EncodeHeaderDirectory(
	    legacy.mapRevision, layout, directoryOffset, bsp2Header, directory.data() );
	if ( !sink.WriteAt( directoryOffset, directory.data(), directory.size() ) ||
	     !sink.WriteAt( 0, bsp2Header, sizeof( bsp2Header ) ) )
		return Fail( MapContainerError::WriteFailed );
	return kOk;
}

static MapContainerStatus WalkLegacyGaps( IMapByteSource &source, const MapLumpInfo &gapInfo,
    const std::array<std::pair<uint64_t, uint64_t>, kLegacyLumpCount + 1> &covered,
    size_t coveredCount, uint64_t legacySize, IMapByteSink *pSink )
{
	uint64_t cursor = 0;
	uint64_t previousEnd = 0;
	while ( cursor < gapInfo.storedSize )
	{
		if ( gapInfo.storedSize - cursor < 16 )
			return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps, cursor );
		uint8_t record[16];
		MapContainerStatus status =
		    ReadExact( source, gapInfo.offset + cursor, record, sizeof( record ) );
		if ( !status.Ok() )
			return Fail( status.code, kLumpLegacyGaps, gapInfo.offset + cursor );
		const uint64_t offset = Load64( record );
		const uint64_t length = Load64( record + 8 );
		cursor += sizeof( record );
		if ( length == 0 || length > gapInfo.storedSize - cursor ||
		     !FitsWithin( offset, length, legacySize ) || offset < previousEnd ||
		     cursor + length > std::numeric_limits<uint64_t>::max() - 7 )
			return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps, cursor );
		for ( size_t i = 0; i < coveredCount; ++i )
		{
			if ( offset < covered[i].second && covered[i].first < offset + length )
				return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps, cursor );
		}
		const uint64_t next = AlignUp( cursor + length, 8 );
		if ( next > gapInfo.storedSize )
			return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps, cursor );
		uint8_t padding[7];
		const size_t padSize = size_t( next - cursor - length );
		status = ReadExact( source, gapInfo.offset + cursor + length, padding, padSize );
		if ( !status.Ok() )
			return Fail( status.code, kLumpLegacyGaps, gapInfo.offset + cursor + length );
		if ( std::any_of( padding, padding + padSize,
		         []( uint8_t byte )
		         {
			         return byte != 0;
		         } ) )
			return Fail( MapContainerError::LegacyGapsInvalid, kLumpLegacyGaps, cursor );
		if ( pSink )
		{
			status = CopyToSink(
			    source, *pSink, gapInfo.offset + cursor, offset, length, kLumpLegacyGaps );
			if ( !status.Ok() )
				return status;
		}
		previousEnd = offset + length;
		cursor = next;
	}
	return kOk;
}

MapContainerStatus ExportLegacyFromBsp2( IMapByteSource &source, IMapByteSink &sink )
{
	IMapContainer *pRaw = nullptr;
	// Only the legacy payload is exported; newer lumps are intentionally dropped.
	const MapContainerOpenOptions options{ true, nullptr, 0, true };
	MapContainerStatus status = OpenMapContainer( source, options, &pRaw );
	if ( !status.Ok() )
		return status;
	const ContainerPtr container( pRaw );
	if ( container->Kind() != MapContainerKind::Bsp2 )
		return Fail( MapContainerError::InvalidArgument );

	MapLumpInfo headerInfo{};
	if ( !container->FindLump( kLumpLegacyHeader, &headerInfo ) )
		return Fail( MapContainerError::MissingLegacyHeader, kLumpLegacyHeader );
	uint8_t headerLump[kLegacyHeaderLumpSize];
	status = ReadExact( source, headerInfo.offset, headerLump, sizeof( headerLump ) );
	if ( !status.Ok() )
		return Fail( status.code, kLumpLegacyHeader, headerInfo.offset );
	const uint64_t legacySize = Load64( headerLump );

	std::array<std::pair<uint64_t, uint64_t>, kLegacyLumpCount + 1> covered{};
	size_t coveredCount = 0;
	covered[coveredCount++] = { 0, kLegacyHeaderSize };
	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		MapLumpInfo info{};
		container->FindLegacyLump( i, &info );
		if ( info.storedSize == 0 )
			continue;
		// Open validated legacyOrigin + size against the legacy file size.
		covered[coveredCount++] = { info.legacyOrigin, info.legacyOrigin + info.storedSize };
	}

	MapLumpInfo gapInfo{};
	const bool hasGaps = container->FindLump( kLumpLegacyGaps, &gapInfo );
	if ( hasGaps )
	{
		status = WalkLegacyGaps( source, gapInfo, covered, coveredCount, legacySize, nullptr );
		if ( !status.Ok() )
			return status;
	}
	if ( !sink.ResetToZeroes( legacySize ) ||
	     !sink.WriteAt( 0, headerLump + 8, kLegacyHeaderSize ) )
		return Fail( MapContainerError::WriteFailed, kLumpLegacyHeader );
	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		MapLumpInfo info{};
		container->FindLegacyLump( i, &info );
		if ( info.storedSize == 0 )
			continue;
		status = CopyToSink(
		    source, sink, info.offset, info.legacyOrigin, info.storedSize, info.fourcc );
		if ( !status.Ok() )
			return status;
	}
	if ( hasGaps )
		return WalkLegacyGaps( source, gapInfo, covered, coveredCount, legacySize, &sink );
	return kOk;
}

Expected<std::vector<std::byte>, MapContainerStatus> ExportLegacyFromBsp2(
    std::span<const std::byte> bsp2File )
{
	MemoryByteSource source( bsp2File );
	VectorByteSink sink;
	MapContainerStatus status = ExportLegacyFromBsp2( source, sink );
	if ( status.code == MapContainerError::WriteFailed )
		status = Fail( MapContainerError::OutOfMemory, status.fourcc, status.offset );
	if ( !status.Ok() )
		return MakeUnexpected( status );
	return std::move( sink.m_Bytes );
}

} // namespace mapcontainer
