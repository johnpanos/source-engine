//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for the map container seam (RFC 0008 F1).
//
//  - BLAKE2b against the RFC 7693 test vector.
//  - One IMapContainer contract suite run against the legacy VBSP reader, the
//    BSP2 reader and deliberately bad providers (which must be caught).
//  - Legacy -> BSP2 -> legacy byte identity on synthetic maps with zero-length
//    lumps at arbitrary offsets, nonzero gaps and trailing bytes.
//  - Negative fixtures: each malformation fails with its structured error.
//  - Seeded mutation fuzzing of raw bytes and of rehashed directory fields.
//
//=============================================================================//

#include "mapcontainer/map_container.h"
#include "mapcontainer/map_container_builder.h"
#include "mapcontainer/blake2b.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace mapcontainer;

namespace
{
int g_Checks = 0;
int g_Failures = 0;

void Check( bool condition, const char *what, int line )
{
	++g_Checks;
	if ( !condition )
	{
		++g_Failures;
		std::printf( "FAIL line %d: %s\n", line, what );
	}
}
#define CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

using Bytes = std::vector<std::byte>;

uint32_t Get32( const Bytes &b, size_t at )
{
	uint32_t v = 0;
	for ( int i = 3; i >= 0; --i )
		v = ( v << 8 ) | uint32_t( b[at + i] );
	return v;
}
uint64_t Get64( const Bytes &b, size_t at )
{
	return uint64_t( Get32( b, at ) ) | ( uint64_t( Get32( b, at + 4 ) ) << 32 );
}
void Put32( Bytes &b, size_t at, uint32_t v )
{
	for ( int i = 0; i < 4; ++i )
		b[at + i] = std::byte( v >> ( 8 * i ) );
}
void Put64( Bytes &b, size_t at, uint64_t v )
{
	Put32( b, at, uint32_t( v ) );
	Put32( b, at + 4, uint32_t( v >> 32 ) );
}

//-----------------------------------------------------------------------------
// Synthetic legacy maps
//-----------------------------------------------------------------------------
struct LegacyLumpSpec
{
	int index;
	int version;
	std::vector<uint8_t> data;
};

// Lumps are placed in the given order with 4-byte alignment, as vbsp does.
// Every other legacy lump is zero-length at a nonzero, arbitrary offset.
Bytes MakeLegacyMap( int version, int32_t revision, const std::vector<LegacyLumpSpec> &lumps,
    bool bNonzeroGap, size_t trailing )
{
	Bytes file( kLegacyHeaderSize, std::byte{ 0 } );
	Put32( file, 0, kLegacyIdent );
	Put32( file, 4, uint32_t( version ) );
	for ( int i = 0; i < kLegacyLumpCount; ++i )
		Put32( file, 8 + 16 * i, uint32_t( 0x1000 + i * 3 ) ); // bogus zero-length offsets
	for ( const LegacyLumpSpec &lump : lumps )
	{
		while ( file.size() % 4 )
			file.push_back( std::byte{ 0 } );
		if ( bNonzeroGap )
		{
			for ( int k = 0; k < 4; ++k )
				file.push_back( std::byte( 0xA0 + k ) );
		}
		const size_t offset = file.size();
		for ( uint8_t value : lump.data )
			file.push_back( std::byte( value ) );
		Put32( file, 8 + 16 * lump.index, uint32_t( offset ) );
		Put32( file, 8 + 16 * lump.index + 4, uint32_t( lump.data.size() ) );
		Put32( file, 8 + 16 * lump.index + 8, uint32_t( lump.version ) );
	}
	for ( size_t i = 0; i < trailing; ++i )
		file.push_back( std::byte( i & 1 ? 0 : 0x5A ) );
	Put32( file, 8 + 16 * kLegacyLumpCount, uint32_t( revision ) );
	return file;
}

std::vector<uint8_t> Pattern( size_t size, uint8_t seed )
{
	std::vector<uint8_t> data( size );
	for ( size_t i = 0; i < size; ++i )
		data[i] = uint8_t( seed + i * 7 );
	return data;
}

Bytes StandardLegacyMap( bool bGap = true )
{
	return MakeLegacyMap( 20, 1234,
	    { { 0, 0, Pattern( 37, 1 ) },     // entities (odd size)
	        { 1, 0, Pattern( 200, 2 ) },  // planes
	        { 35, 0, Pattern( 64, 3 ) },  // game lumps
	        { 40, 1, Pattern( 513, 4 ) }, // pak
	        { 7, 0, Pattern( 1, 5 ) } },  // out-of-index-order placement
	    bGap, bGap ? 3 : 0 );
}

std::string Name( const MapContainerStatus &status )
{
	return MapContainerErrorName( status.code );
}

MapContainerStatus TryOpen(
    const Bytes &bytes, bool bVerify, const std::vector<uint32_t> &known = {} )
{
	MemoryByteSource source( bytes );
	MapContainerOpenOptions options{ bVerify, known.data(), known.size(), false };
	IMapContainer *pContainer = nullptr;
	const MapContainerStatus status = OpenMapContainer( source, options, &pContainer );
	if ( status.Ok() != ( pContainer != nullptr ) )
		return MapContainerStatus{ MapContainerError::InvalidArgument, 0xDEAD, 0 };
	DestroyMapContainer( pContainer );
	return status;
}

//-----------------------------------------------------------------------------
// Shared IMapContainer contract. Returns the number of violations so a bad
// provider can be shown to fail it without polluting the global counters.
//-----------------------------------------------------------------------------
int RunContainerContract(
    const IMapContainer &container, const Bytes &containerBytes, const Bytes &legacyReference )
{
	int violations = 0;
	auto expect = [&]( bool condition )
	{
		violations += condition ? 0 : 1;
	};

	const uint8_t *pLegacy = reinterpret_cast<const uint8_t *>( legacyReference.data() );
	const uint32_t referenceVersion = Get32( legacyReference, 4 );
	const int32_t referenceRevision =
	    int32_t( Get32( legacyReference, 8 + 16 * kLegacyLumpCount ) );
	expect( container.LegacyVersion() == int( referenceVersion ) );
	expect( container.MapRevision() == referenceRevision );

	MapLumpInfo info{};
	expect( !container.FindLegacyLump( -1, &info ) );
	expect( !container.FindLegacyLump( kLegacyLumpCount, &info ) );
	expect( !container.FindLegacyLump( 0, nullptr ) );

	for ( int i = 0; i < kLegacyLumpCount; ++i )
	{
		const size_t rec = 8 + 16 * size_t( i );
		const uint32_t fileofs = Get32( legacyReference, rec );
		const uint32_t filelen = Get32( legacyReference, rec + 4 );
		if ( !container.FindLegacyLump( i, &info ) )
		{
			expect( false );
			continue;
		}
		expect( info.fourcc == LegacyLumpFourCC( i ) );
		expect( info.version == Get32( legacyReference, rec + 8 ) );
		expect( info.legacyUncompressedSize == Get32( legacyReference, rec + 12 ) );
		expect( info.legacyOrigin == fileofs );
		expect( info.storedSize == filelen );
		if ( filelen == 0 )
			continue;
		const bool bInside = info.offset <= containerBytes.size() &&
		                     info.storedSize <= containerBytes.size() - info.offset;
		expect( bInside );
		if ( !bInside )
			continue;
		const void *pStored = containerBytes.data() + info.offset;
		expect( std::memcmp( pStored, pLegacy + fileofs, filelen ) == 0 );
		expect( container.VerifyContent( info, pStored, info.storedSize ).Ok() );

		MapLumpInfo byId{};
		expect( container.FindLump( LegacyLumpFourCC( i ), &byId ) && byId.offset == info.offset );

		// A hashed lump must reject altered content of the right size.
		if ( info.hasHash )
		{
			std::vector<uint8_t> altered( static_cast<const uint8_t *>( pStored ),
			    static_cast<const uint8_t *>( pStored ) + filelen );
			altered[filelen / 2] ^= 0x01;
			expect( container.VerifyContent( info, altered.data(), filelen ).code ==
			        MapContainerError::ContentHashMismatch );
			expect( container.VerifyContent( info, altered.data(), filelen - 1 ).code ==
			        MapContainerError::SizeMismatch );
		}
	}

	// Enumeration reaches every legacy lump with data.
	int enumerated = 0;
	for ( uint32_t i = 0; i < container.LumpCount(); ++i )
	{
		expect( container.LumpAt( i, &info ) );
		int index = -1;
		for ( int k = 0; k < kLegacyLumpCount; ++k )
			index = info.fourcc == LegacyLumpFourCC( k ) ? k : index;
		if ( index >= 0 && Get32( legacyReference, 8 + 16 * size_t( index ) + 4 ) != 0 )
			++enumerated;
	}
	int expectedLumps = 0;
	for ( int k = 0; k < kLegacyLumpCount; ++k )
		expectedLumps += Get32( legacyReference, 8 + 16 * size_t( k ) + 4 ) != 0;
	expect( enumerated == expectedLumps );
	expect( !container.LumpAt( container.LumpCount(), &info ) );
	return violations;
}

// Bad providers wrap a correct container and break one obligation each.
class ForwardingContainer : public IMapContainer
{
public:
	explicit ForwardingContainer( const IMapContainer &inner ) : m_Inner( inner ) {}
	MapContainerKind Kind() const override { return m_Inner.Kind(); }
	int LegacyVersion() const override { return m_Inner.LegacyVersion(); }
	int32_t MapRevision() const override { return m_Inner.MapRevision(); }
	bool FindLegacyLump( int index, MapLumpInfo *pInfo ) const override
	{
		return m_Inner.FindLegacyLump( index, pInfo );
	}
	bool FindLump( uint32_t fourcc, MapLumpInfo *pInfo ) const override
	{
		return m_Inner.FindLump( fourcc, pInfo );
	}
	uint32_t LumpCount() const override { return m_Inner.LumpCount(); }
	bool LumpAt( uint32_t index, MapLumpInfo *pInfo ) const override
	{
		return m_Inner.LumpAt( index, pInfo );
	}
	MapContainerStatus VerifyContent(
	    const MapLumpInfo &info, const void *pData, uint64_t size ) const override
	{
		return m_Inner.VerifyContent( info, pData, size );
	}

protected:
	const IMapContainer &m_Inner;
};

// Reports the container offset as the legacy origin (breaks game lump rebasing).
class LosesLegacyOrigin final : public ForwardingContainer
{
public:
	using ForwardingContainer::ForwardingContainer;
	bool FindLegacyLump( int index, MapLumpInfo *pInfo ) const override
	{
		const bool bFound = m_Inner.FindLegacyLump( index, pInfo );
		if ( bFound && pInfo )
			pInfo->legacyOrigin = pInfo->offset;
		return bFound;
	}
};

// Drops lump versions of empty lumps (the legacy loader reads them anyway).
class DropsEmptyVersions final : public ForwardingContainer
{
public:
	using ForwardingContainer::ForwardingContainer;
	bool FindLegacyLump( int index, MapLumpInfo *pInfo ) const override
	{
		const bool bFound = m_Inner.FindLegacyLump( index, pInfo );
		if ( bFound && pInfo && pInfo->storedSize == 0 )
			pInfo->version = 0x7777;
		return bFound;
	}
};

// Accepts any content.
class TrustsEverything final : public ForwardingContainer
{
public:
	using ForwardingContainer::ForwardingContainer;
	MapContainerStatus VerifyContent( const MapLumpInfo &, const void *, uint64_t ) const override
	{
		return MapContainerStatus{ MapContainerError::Ok, 0, 0 };
	}
};

//-----------------------------------------------------------------------------
// Directory mutation helpers (keep the directory hash valid so the targeted
// validation rule, not the directory hash, rejects the file).
//-----------------------------------------------------------------------------
size_t DirectoryOffset( const Bytes &b )
{
	return size_t( Get64( b, 24 ) );
}
uint32_t DirectoryCount( const Bytes &b )
{
	return Get32( b, 32 );
}
size_t EntryAt( const Bytes &b, uint32_t fourcc )
{
	for ( uint32_t i = 0; i < DirectoryCount( b ); ++i )
	{
		const size_t at = DirectoryOffset( b ) + i * kBsp2EntrySize;
		if ( Get32( b, at ) == fourcc )
			return at;
	}
	return 0;
}
void Rehash( Bytes &b )
{
	const size_t at = DirectoryOffset( b );
	const ContentHash digest = HashContent(
	    std::span<const std::byte>( b ).subspan( at, DirectoryCount( b ) * kBsp2EntrySize ) );
	std::memcpy( b.data() + 48, digest.data(), digest.size() );
}
void RehashEntryContent( Bytes &b, size_t entry )
{
	const ContentHash digest = HashContent( std::span<const std::byte>( b ).subspan(
	    size_t( Get64( b, entry + 16 ) ), size_t( Get64( b, entry + 24 ) ) ) );
	std::memcpy( b.data() + entry + 48, digest.data(), digest.size() );
}

//-----------------------------------------------------------------------------
// Sections
//-----------------------------------------------------------------------------
void TestBlake2b()
{
	// RFC 7693 Appendix A: BLAKE2b-512("abc").
	static const uint8_t kExpected[64] = {
	    0xBA,
	    0x80,
	    0xA5,
	    0x3F,
	    0x98,
	    0x1C,
	    0x4D,
	    0x0D,
	    0x6A,
	    0x27,
	    0x97,
	    0xB6,
	    0x9F,
	    0x12,
	    0xF6,
	    0xE9,
	    0x4C,
	    0x21,
	    0x2F,
	    0x14,
	    0x68,
	    0x5A,
	    0xC4,
	    0xB7,
	    0x4B,
	    0x12,
	    0xBB,
	    0x6F,
	    0xDB,
	    0xFF,
	    0xA2,
	    0xD1,
	    0x7D,
	    0x87,
	    0xC5,
	    0x39,
	    0x2A,
	    0xAB,
	    0x79,
	    0x2D,
	    0xC2,
	    0x52,
	    0xD5,
	    0xDE,
	    0x45,
	    0x33,
	    0xCC,
	    0x95,
	    0x18,
	    0xD3,
	    0x8A,
	    0xA8,
	    0xDB,
	    0xF1,
	    0x92,
	    0x5A,
	    0xB9,
	    0x23,
	    0x86,
	    0xED,
	    0xD4,
	    0x00,
	    0x99,
	    0x23,
	};
	detail::Blake2b hasher( 64 );
	hasher.Update( "abc", 3 );
	uint8_t digest[64];
	hasher.Final( digest );
	CHECK( std::memcmp( digest, kExpected, 64 ) == 0 );

	// Streaming split across the 128-byte block boundary equals one shot.
	std::vector<uint8_t> data = Pattern( 1000, 9 );
	uint8_t oneShot[16];
	uint8_t split[16];
	detail::Blake2b a( 16 );
	a.Update( data.data(), data.size() );
	a.Final( oneShot );
	detail::Blake2b b( 16 );
	b.Update( data.data(), 128 );
	b.Update( data.data() + 128, 1 );
	b.Update( data.data() + 129, data.size() - 129 );
	b.Final( split );
	CHECK( std::memcmp( oneShot, split, 16 ) == 0 );
}

void TestRoundTripAndContract()
{
	for ( int variant = 0; variant < 3; ++variant )
	{
		const Bytes legacy = variant == 0 ? StandardLegacyMap( true )
		                     : variant == 1
		                         ? StandardLegacyMap( false )
		                         : MakeLegacyMap( 21, -5, {}, false, 0 ); // no lumps at all
		auto converted = ConvertLegacyToBsp2( legacy );
		CHECK( !!converted );
		if ( !converted )
			continue;
		const Bytes &bsp2 = converted.Value();
		CHECK( IsBsp2Magic( bsp2.data(), bsp2.size() ) );
		CHECK( TryOpen( bsp2, true ).Ok() );

		auto exported = ExportLegacyFromBsp2( bsp2 );
		CHECK( !!exported && exported.Value() == legacy );

		// Deterministic output: converting twice gives the same bytes.
		auto again = ConvertLegacyToBsp2( legacy );
		CHECK( !!again && again.Value() == bsp2 );

		MemoryByteSource bsp2Source( bsp2 );
		auto bsp2Container = OpenMemoryContainer( bsp2Source, true );
		MemoryByteSource legacySource( legacy );
		auto legacyContainer = OpenMemoryContainer( legacySource, false );
		CHECK( !!bsp2Container && !!legacyContainer );
		if ( !bsp2Container || !legacyContainer )
			continue;
		CHECK( bsp2Container.Value()->Kind() == MapContainerKind::Bsp2 );
		CHECK( legacyContainer.Value()->Kind() == MapContainerKind::LegacyVbsp );
		CHECK( RunContainerContract( *bsp2Container.Value(), bsp2, legacy ) == 0 );
		CHECK( RunContainerContract( *legacyContainer.Value(), legacy, legacy ) == 0 );

		// LGAP exists exactly when the legacy file has nonzero gap bytes.
		CHECK( bsp2Container.Value()->FindLump( kLumpLegacyGaps, nullptr ) == ( variant == 0 ) );

		// Every payload is aligned to at least 16 bytes.
		for ( uint32_t i = 0; i < bsp2Container.Value()->LumpCount(); ++i )
		{
			MapLumpInfo info{};
			bsp2Container.Value()->LumpAt( i, &info );
			CHECK( info.offset % kBsp2MinAlignment == 0 );
		}

		if ( variant == 0 )
		{
			// The contract suite detects each deliberately bad provider.
			CHECK( RunContainerContract(
			           LosesLegacyOrigin( *bsp2Container.Value() ), bsp2, legacy ) > 0 );
			CHECK( RunContainerContract(
			           DropsEmptyVersions( *bsp2Container.Value() ), bsp2, legacy ) > 0 );
			CHECK( RunContainerContract(
			           TrustsEverything( *bsp2Container.Value() ), bsp2, legacy ) > 0 );
		}
	}
}

void TestExtraLumps()
{
	const Bytes legacy = StandardLegacyMap();
	const std::vector<uint8_t> mesh = Pattern( 5000, 11 );
	const Bsp2LumpInput extras[] = {
	    { MakeFourCC( 'W', 'M', 'S', 'H' ), 1, kBsp2FlagRequired, kBsp2BulkAlignment,
	        std::as_bytes( std::span( mesh ) ) },
	    { MakeFourCC( 'X', 'O', 'P', 'T' ), 3, 0, kBsp2MinAlignment,
	        std::as_bytes( std::span( mesh ).first( 10 ) ) },
	};
	auto converted = ConvertLegacyToBsp2( legacy, extras );
	CHECK( !!converted );
	if ( !converted )
		return;
	const Bytes &bsp2 = converted.Value();

	// A required lump the reader does not understand fails; declaring it works.
	CHECK( TryOpen( bsp2, true ).code == MapContainerError::UnknownRequiredLump );
	CHECK( TryOpen( bsp2, true ).fourcc == MakeFourCC( 'W', 'M', 'S', 'H' ) );
	CHECK( TryOpen( bsp2, true, { MakeFourCC( 'W', 'M', 'S', 'H' ) } ).Ok() );

	MemoryByteSource source( bsp2 );
	const uint32_t known[] = { MakeFourCC( 'W', 'M', 'S', 'H' ) };
	IMapContainer *pContainer = nullptr;
	CHECK( OpenMapContainer( source, MapContainerOpenOptions{ true, known, 1, false }, &pContainer )
	        .Ok() );
	if ( pContainer )
	{
		MapLumpInfo info{};
		CHECK( pContainer->FindLump( MakeFourCC( 'W', 'M', 'S', 'H' ), &info ) );
		CHECK( info.offset % kBsp2BulkAlignment == 0 && info.storedSize == mesh.size() );
		CHECK( std::memcmp( bsp2.data() + info.offset, mesh.data(), mesh.size() ) == 0 );
		DestroyMapContainer( pContainer );
	}
	// Legacy export drops lumps outside the legacy payload, even required ones.
	auto exported = ExportLegacyFromBsp2( bsp2 );
	CHECK( !!exported && exported.Value() == legacy );

	// Extras may not shadow reserved legacy lumps.
	const Bsp2LumpInput shadow[] = { { LegacyLumpFourCC( 3 ), 0, 0, kBsp2MinAlignment, {} } };
	auto bad = ConvertLegacyToBsp2( legacy, shadow );
	CHECK( !bad && bad.Error().code == MapContainerError::DuplicateLump );
}

struct NegativeCase
{
	const char *name;
	MapContainerError expected;
	bool bVerify;
	void ( *mutate )( Bytes & );
};

void TestNegativeFixtures()
{
	const Bytes legacy = StandardLegacyMap();
	auto converted = ConvertLegacyToBsp2( legacy );
	CHECK( !!converted );
	if ( !converted )
		return;
	const Bytes good = converted.Value();
	CHECK( TryOpen( good, true ).Ok() );

	static const NegativeCase kCases[] = {
	    { "empty file", MapContainerError::Truncated, false,
	        []( Bytes &b )
	        {
		        b.clear();
	        } },
	    { "bad magic", MapContainerError::BadMagic, false,
	        []( Bytes &b )
	        {
		        b[0] = std::byte{ 'X' };
	        } },
	    { "truncated header", MapContainerError::Truncated, false,
	        []( Bytes &b )
	        {
		        b.resize( 40 );
	        } },
	    { "truncated directory", MapContainerError::DirectoryOutOfBounds, false,
	        []( Bytes &b )
	        {
		        b.pop_back();
	        } },
	    { "future container version", MapContainerError::UnsupportedContainerVersion, false,
	        []( Bytes &b )
	        {
		        Put32( b, 8, 2 );
	        } },
	    { "header flags", MapContainerError::UnsupportedLayout, false,
	        []( Bytes &b )
	        {
		        Put32( b, 12, 1 );
	        } },
	    { "entry size", MapContainerError::UnsupportedLayout, false,
	        []( Bytes &b )
	        {
		        Put32( b, 20, 72 );
	        } },
	    { "hash algorithm", MapContainerError::UnsupportedHash, false,
	        []( Bytes &b )
	        {
		        Put32( b, 36, 2 );
	        } },
	    { "lump count cap", MapContainerError::TooManyLumps, false,
	        []( Bytes &b )
	        {
		        Put32( b, 32, kBsp2MaxLumps + 1 );
	        } },
	    { "directory inside header", MapContainerError::DirectoryOutOfBounds, false,
	        []( Bytes &b )
	        {
		        Put64( b, 24, 8 );
	        } },
	    { "directory offset overflow", MapContainerError::DirectoryOutOfBounds, false,
	        []( Bytes &b )
	        {
		        Put64( b, 24, ~uint64_t( 0 ) - 8 );
	        } },
	    { "stale directory hash", MapContainerError::DirectoryHashMismatch, false,
	        []( Bytes &b )
	        {
		        b[DirectoryOffset( b ) + 4] ^= std::byte{ 1 };
	        } },
	    { "stale content hash", MapContainerError::ContentHashMismatch, true,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 1 ) );
		        b[size_t( Get64( b, entry + 16 ) )] ^= std::byte{ 1 };
	        } },
	    { "misaligned lump", MapContainerError::LumpMisaligned, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 1 ) );
		        Put64( b, entry + 16, Get64( b, entry + 16 ) + 4 );
		        Rehash( b );
	        } },
	    { "non power of two alignment", MapContainerError::LumpMisaligned, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 1 ) ) + 12, 48 );
		        Rehash( b );
	        } },
	    { "lump overlaps directory", MapContainerError::LumpOutOfBounds, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 40 ) );
		        Put64( b, entry + 24, Get64( b, entry + 24 ) + 4096 );
		        Put64( b, entry + 32, Get64( b, entry + 32 ) + 4096 );
		        Rehash( b );
	        } },
	    { "lump size overflow", MapContainerError::LumpOutOfBounds, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 1 ) );
		        Put64( b, entry + 24, ~uint64_t( 0 ) );
		        Put64( b, entry + 32, ~uint64_t( 0 ) );
		        Rehash( b );
	        } },
	    { "overlapping lumps", MapContainerError::LumpOverlap, false,
	        []( Bytes &b )
	        {
		        const size_t first = EntryAt( b, LegacyLumpFourCC( 0 ) );
		        const size_t second = EntryAt( b, LegacyLumpFourCC( 1 ) );
		        Put64( b, second + 16, Get64( b, first + 16 ) );
		        Rehash( b );
	        } },
	    { "duplicate 4CC", MapContainerError::DuplicateLump, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 7 ) ), LegacyLumpFourCC( 1 ) );
		        Rehash( b );
	        } },
	    { "unknown flag", MapContainerError::UnknownFlags, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 1 ) ) + 8, kBsp2FlagRequired | 0x10000 );
		        Rehash( b );
	        } },
	    { "compressed known lump", MapContainerError::UnsupportedCompression, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 1 ) ) + 8,
		            kBsp2FlagRequired | kBsp2CompressionZstd );
		        Rehash( b );
	        } },
	    { "stored/uncompressed disagree", MapContainerError::SizeMismatch, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 1 ) );
		        Put64( b, entry + 32, Get64( b, entry + 32 ) + 1 );
		        Rehash( b );
	        } },
	    { "reserved entry field", MapContainerError::UnsupportedLayout, false,
	        []( Bytes &b )
	        {
		        Put64( b, EntryAt( b, LegacyLumpFourCC( 1 ) ) + 40, 1 );
		        Rehash( b );
	        } },
	    { "unknown required lump", MapContainerError::UnknownRequiredLump, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 7 ) ), MakeFourCC( 'Z', 'Z', 'Z', 'Z' ) );
		        Rehash( b );
	        } },
	    { "missing legacy header", MapContainerError::MissingLegacyHeader, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, kLumpLegacyHeader );
		        Put32( b, entry, MakeFourCC( 'O', 'P', 'T', '1' ) );
		        Put32( b, entry + 8, 0 ); // optional, so only the missing LHDR fails
		        Rehash( b );
	        } },
	    { "legacy lump dropped", MapContainerError::LegacyLumpMismatch, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, LegacyLumpFourCC( 7 ) );
		        Put32( b, entry, MakeFourCC( 'O', 'P', 'T', '2' ) );
		        Put32( b, entry + 8, 0 );
		        Rehash( b );
	        } },
	    { "legacy version disagrees", MapContainerError::LegacyLumpMismatch, false,
	        []( Bytes &b )
	        {
		        Put32( b, EntryAt( b, LegacyLumpFourCC( 40 ) ) + 4, 9 );
		        Rehash( b );
	        } },
	    { "legacy header lump out of legacy file", MapContainerError::LegacyHeaderInvalid, false,
	        []( Bytes &b )
	        {
		        const size_t entry = EntryAt( b, kLumpLegacyHeader );
		        const size_t payload = size_t( Get64( b, entry + 16 ) );
		        Put64( b, payload, 100 ); // legacy file size smaller than its lumps
		        RehashEntryContent( b, entry );
		        Rehash( b );
	        } },
	    { "legacy revision disagrees", MapContainerError::LegacyHeaderInvalid, false,
	        []( Bytes &b )
	        {
		        Put32( b, 40, 999 );
	        } },
	};

	for ( const NegativeCase &test : kCases )
	{
		Bytes mutated = good;
		test.mutate( mutated );
		const MapContainerStatus status = TryOpen( mutated, test.bVerify );
		const bool bMatched = status.code == test.expected;
		CHECK( bMatched );
		if ( !bMatched )
			std::printf( "  case '%s': expected %s, got %s\n", test.name,
			    MapContainerErrorName( test.expected ), Name( status ).c_str() );
	}

	// Stale content hash: accepted without eager verification, but the
	// per-read verification the engine performs rejects it.
	{
		Bytes mutated = good;
		const size_t entry = EntryAt( mutated, LegacyLumpFourCC( 1 ) );
		const size_t payload = size_t( Get64( mutated, entry + 16 ) );
		mutated[payload] ^= std::byte{ 1 };
		MemoryByteSource source( mutated );
		auto container = OpenMemoryContainer( source, false );
		CHECK( !!container );
		if ( container )
		{
			MapLumpInfo info{};
			container.Value()->FindLegacyLump( 1, &info );
			CHECK( container.Value()
			           ->VerifyContent( info, mutated.data() + payload, info.storedSize )
			           .code == MapContainerError::ContentHashMismatch );
		}
		CHECK( !ExportLegacyFromBsp2( mutated ) );
	}

	// An unknown optional lump in an unknown encoding is skipped.
	{
		const std::vector<uint8_t> blob = Pattern( 33, 3 );
		const Bsp2LumpInput extras[] = { { MakeFourCC( 'O', 'P', 'T', 'Z' ), 1, 0,
		    kBsp2MinAlignment, std::as_bytes( std::span( blob ) ) } };
		auto withExtra = ConvertLegacyToBsp2( legacy, extras );
		CHECK( !!withExtra );
		if ( withExtra )
		{
			Bytes mutated = withExtra.Value();
			Put32( mutated, EntryAt( mutated, MakeFourCC( 'O', 'P', 'T', 'Z' ) ) + 8,
			    kBsp2CompressionZstd );
			Rehash( mutated );
			CHECK( TryOpen( mutated, true ).Ok() );
			auto exported = ExportLegacyFromBsp2( mutated );
			CHECK( !!exported && exported.Value() == legacy );
		}
	}

	// LGAP records may not overwrite legacy lumps.
	{
		Bytes mutated = good;
		const size_t entry = EntryAt( mutated, kLumpLegacyGaps );
		CHECK( entry != 0 );
		if ( entry )
		{
			const size_t payload = size_t( Get64( mutated, entry + 16 ) );
			Put64( mutated, payload, Get32( legacy, 8 + 16 * 1 ) ); // into lump 1
			RehashEntryContent( mutated, entry );
			Rehash( mutated );
			CHECK( TryOpen( mutated, true ).Ok() );
			auto exported = ExportLegacyFromBsp2( mutated );
			CHECK( !exported && exported.Error().code == MapContainerError::LegacyGapsInvalid );
		}
	}

	// Legacy reader negatives.
	{
		Bytes bad = legacy;
		Put32( bad, 8 + 16 * 1 + 4, uint32_t( legacy.size() ) ); // lump runs past EOF
		CHECK( TryOpen( bad, false ).code == MapContainerError::LumpOutOfBounds );
		bad = legacy;
		Put32( bad, 8 + 16 * 1 + 4, 0x80000000u ); // negative length
		CHECK( TryOpen( bad, false ).code == MapContainerError::LegacyHeaderInvalid );
		bad = legacy;
		Put32( bad, 8 + 16 * 1, 12 ); // nonempty lump inside the header
		CHECK( TryOpen( bad, false ).code == MapContainerError::LumpOutOfBounds );
		bad = legacy;
		bad.resize( 100 );
		CHECK( TryOpen( bad, false ).code == MapContainerError::Truncated );
		bad = legacy;
		Put32( bad, 0, MakeFourCC( 'P', 'S', 'B', 'V' ) ); // big-endian console ident
		CHECK( TryOpen( bad, false ).code == MapContainerError::BadMagic );
		// Converting a BSP2 file again is refused rather than double-wrapped.
		auto twice = ConvertLegacyToBsp2( good );
		CHECK( !twice && twice.Error().code == MapContainerError::InvalidArgument );
	}

	// Writer refuses invalid input.
	{
		const std::vector<uint8_t> blob = Pattern( 4, 1 );
		const Bsp2LumpInput duplicate[] = {
		    { MakeFourCC( 'A', 'A', 'A', 'A' ), 1, 0, 16, std::as_bytes( std::span( blob ) ) },
		    { MakeFourCC( 'A', 'A', 'A', 'A' ), 1, 0, 16, std::as_bytes( std::span( blob ) ) },
		};
		CHECK( !WriteBsp2( 0, duplicate ) );
		const Bsp2LumpInput misaligned[] = { { MakeFourCC( 'A', 'A', 'A', 'A' ), 1, 0, 8, {} } };
		CHECK( !WriteBsp2( 0, misaligned ) );
		const Bsp2LumpInput compressed[] = {
		    { MakeFourCC( 'A', 'A', 'A', 'A' ), 1, kBsp2CompressionZstd, 16, {} } };
		CHECK( !WriteBsp2( 0, compressed ) );
	}
}

// Seeded mutation fuzzing: opening, verifying, reading every lump through the
// contract and exporting must never crash or read out of bounds (run under
// ASan/UBSan by the manifest). Accepted mutations must still export or fail
// with a structured error.
void TestMutationFuzz()
{
	const Bytes legacy = StandardLegacyMap();
	auto converted = ConvertLegacyToBsp2( legacy );
	CHECK( !!converted );
	if ( !converted )
		return;
	const Bytes good = converted.Value();

	uint64_t state = 0x9E3779B97F4A7C15ull;
	auto next = [&]()
	{
		state ^= state << 13;
		state ^= state >> 7;
		state ^= state << 17;
		return state;
	};

	int accepted = 0;
	int rejected = 0;
	int exportsIdentical = 0;
	const int kIterations = 20000;
	for ( int iteration = 0; iteration < kIterations; ++iteration )
	{
		Bytes mutated = good;
		const uint64_t mode = next() % 4;
		if ( mode == 0 )
		{
			// Raw byte flips anywhere.
			const int flips = 1 + int( next() % 4 );
			for ( int k = 0; k < flips; ++k )
				mutated[next() % mutated.size()] ^= std::byte( 1 + next() % 255 );
		}
		else if ( mode == 1 )
		{
			mutated.resize( size_t( next() % mutated.size() ) );
		}
		else
		{
			// Directory field mutation with a valid directory hash, so the
			// structural validation (not the hash) must catch it.
			const uint32_t count = DirectoryCount( mutated );
			const size_t entry =
			    DirectoryOffset( mutated ) + size_t( next() % count ) * kBsp2EntrySize;
			const size_t field = size_t( next() % 6 );
			const size_t fieldOffsets[] = { 0, 4, 8, 12, 16, 24 };
			if ( field < 4 )
				Put32(
				    mutated, entry + fieldOffsets[field], uint32_t( next() ) >> ( next() % 32 ) );
			else
				Put64( mutated, entry + fieldOffsets[field], next() >> ( next() % 64 ) );
			const uint64_t offset = Get64( mutated, entry + 16 );
			const uint64_t size = Get64( mutated, entry + 24 );
			if ( mode == 3 && offset <= mutated.size() && size <= mutated.size() - offset )
				RehashEntryContent( mutated, entry ); // content hash consistent too
			Rehash( mutated );
		}

		const bool bVerify = ( iteration & 1 ) != 0;
		MemoryByteSource source( mutated );
		IMapContainer *pContainer = nullptr;
		const MapContainerStatus status = OpenMapContainer(
		    source, MapContainerOpenOptions{ bVerify, nullptr, 0, false }, &pContainer );
		if ( !status.Ok() )
		{
			++rejected;
			continue;
		}
		++accepted;
		// Touch every lump the way the engine does: read, then verify.
		for ( int i = 0; i < kLegacyLumpCount; ++i )
		{
			MapLumpInfo info{};
			if ( !pContainer->FindLegacyLump( i, &info ) || info.storedSize == 0 )
				continue;
			std::vector<uint8_t> bytes( size_t( info.storedSize ) );
			if ( source.ReadAt( info.offset, bytes.data(), bytes.size() ) )
				(void)pContainer->VerifyContent( info, bytes.data(), bytes.size() );
		}
		DestroyMapContainer( pContainer );
		auto exported = ExportLegacyFromBsp2( mutated );
		if ( exported && exported.Value() == legacy )
			++exportsIdentical;
	}
	CHECK( rejected > kIterations / 2 );
	CHECK( accepted + rejected == kIterations );
	std::printf( "fuzz: %d iterations, %d rejected, %d accepted, %d accepted exports identical\n",
	    kIterations, rejected, accepted, exportsIdentical );
}
} // namespace

int main()
{
	TestBlake2b();
	TestRoundTripAndContract();
	TestExtraLumps();
	TestNegativeFixtures();
	TestMutationFuzz();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && g_Failures == 0 ? 0 : 1;
}
