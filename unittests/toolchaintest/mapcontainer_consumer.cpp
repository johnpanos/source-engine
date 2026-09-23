//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frozen external consumer of the mapcontainer C++11 facade
//          (toolchain.abi.v1). Built in the fixture-only legacy-cxx11 dialect
//          and linked with the real mapcontainer sources built as C++20. It
//          implements IMapByteSource (called from C++20), opens legacy and BSP2
//          containers through IMapContainer (C++20 vtable called from C++11),
//          and exchanges MapLumpInfo / MapContainerStatus values.
//
//=============================================================================//

#include "abi_checks.h"
#include "mapcontainer_bsp2_helper.h"

#include "mapcontainer/map_container.h"

#include <cstring>
#include <vector>

#if __cplusplus != 201103L
#error "the frozen consumer must be compiled as C++11"
#endif

int toolchaintest::g_checks = 0;
int toolchaintest::g_failures = 0;

using namespace mapcontainer;

namespace
{

class VectorSource : public IMapByteSource
{
public:
	explicit VectorSource( const std::vector<unsigned char> &bytes ) : m_bytes( bytes ), m_reads( 0 ) {}
	virtual ~VectorSource() {}

	virtual uint64_t Size() const { return m_bytes.size(); }
	virtual bool ReadAt( uint64_t offset, void *pDest, size_t size )
	{
		++m_reads;
		if ( offset > m_bytes.size() || size > m_bytes.size() - offset )
			return false;
		if ( size )
			std::memcpy( pDest, &m_bytes[static_cast<size_t>( offset )], size );
		return true;
	}
	int Reads() const { return m_reads; }

private:
	const std::vector<unsigned char> &m_bytes;
	int m_reads;
};

void Put32( std::vector<unsigned char> &file, size_t offset, uint32_t value )
{
	for ( int i = 0; i < 4; ++i )
		file[offset + i] = static_cast<unsigned char>( value >> ( 8 * i ) );
}

// A version-20 legacy map: entities (lump 0) and planes (lump 1) hold data,
// every other lump is empty.
std::vector<unsigned char> MakeLegacyMap( std::vector<unsigned char> &entities,
    std::vector<unsigned char> &planes )
{
	std::vector<unsigned char> file( kLegacyHeaderSize, 0 );
	Put32( file, 0, kLegacyIdent );
	Put32( file, 4, 20 );
	for ( int i = 0; i < kLegacyLumpCount; ++i )
		Put32( file, 8 + 16 * i, static_cast<uint32_t>( kLegacyHeaderSize ) );
	entities.assign( 37, 0 );
	planes.assign( 200, 0 );
	for ( size_t i = 0; i < entities.size(); ++i )
		entities[i] = static_cast<unsigned char>( 1 + i * 7 );
	for ( size_t i = 0; i < planes.size(); ++i )
		planes[i] = static_cast<unsigned char>( 2 + i * 7 );
	const std::vector<unsigned char> *lumps[2] = { &entities, &planes };
	for ( int index = 0; index < 2; ++index )
	{
		while ( file.size() % 4 )
			file.push_back( 0 );
		const size_t offset = file.size();
		file.insert( file.end(), lumps[index]->begin(), lumps[index]->end() );
		Put32( file, 8 + 16 * index, static_cast<uint32_t>( offset ) );
		Put32( file, 8 + 16 * index + 4, static_cast<uint32_t>( lumps[index]->size() ) );
	}
	Put32( file, 8 + 16 * kLegacyLumpCount, 1234 );
	return file;
}

bool ReadLump( IMapByteSource &source, const MapLumpInfo &info, std::vector<unsigned char> &out )
{
	out.assign( static_cast<size_t>( info.storedSize ), 0 );
	return out.empty() || source.ReadAt( info.offset, &out[0], out.size() );
}

void CheckContainer( const std::vector<unsigned char> &file, MapContainerKind kind,
    const std::vector<unsigned char> &entities, const std::vector<unsigned char> &planes )
{
	VectorSource source( file );
	MapContainerOpenOptions options = {};
	options.verifyContent = true;
	IMapContainer *pContainer = NULL;
	const MapContainerStatus status = OpenMapContainer( source, options, &pContainer );
	ABI_CHECK( status.Ok() );
	ABI_CHECK( pContainer != NULL );
	if ( !pContainer )
		return;
	ABI_CHECK( source.Reads() > 0 );
	ABI_CHECK( pContainer->Kind() == kind );
	ABI_CHECK( pContainer->LegacyVersion() == 20 );
	ABI_CHECK( pContainer->MapRevision() == 1234 );

	MapLumpInfo info;
	std::vector<unsigned char> bytes;
	ABI_CHECK( pContainer->FindLegacyLump( 0, &info ) );
	ABI_CHECK( info.fourcc == LegacyLumpFourCC( 0 ) && info.storedSize == entities.size() );
	ABI_CHECK( ReadLump( source, info, bytes ) && bytes == entities );
	ABI_CHECK( pContainer->VerifyContent( info, bytes.empty() ? NULL : &bytes[0], bytes.size() ).Ok() );
	ABI_CHECK( pContainer->FindLegacyLump( 1, &info ) );
	ABI_CHECK( ReadLump( source, info, bytes ) && bytes == planes );
	ABI_CHECK( !pContainer->FindLegacyLump( kLegacyLumpCount, &info ) );
	ABI_CHECK( pContainer->LumpCount() > 0 );

	if ( kind == MapContainerKind::Bsp2 )
	{
		// A hashed lump rejects corrupted bytes: the structured status crosses back.
		ABI_CHECK( info.hasHash );
		bytes[0] ^= 0xFF;
		const MapContainerStatus bad = pContainer->VerifyContent( info, &bytes[0], bytes.size() );
		ABI_CHECK( bad.code == MapContainerError::ContentHashMismatch );
		ABI_CHECK( bad.fourcc == info.fourcc );
	}
	DestroyMapContainer( pContainer );
}

} // namespace

int main()
{
	toolchaintest::CheckBoundaryLayouts();

	std::vector<unsigned char> entities, planes;
	const std::vector<unsigned char> legacy = MakeLegacyMap( entities, planes );
	CheckContainer( legacy, MapContainerKind::LegacyVbsp, entities, planes );

	unsigned char *pBsp2 = NULL;
	size_t bsp2Size = 0;
	ABI_CHECK( ToolchainFixtureConvertToBsp2( &legacy[0], legacy.size(), &pBsp2, &bsp2Size ) );
	if ( pBsp2 )
	{
		ABI_CHECK( IsBsp2Magic( pBsp2, bsp2Size ) );
		const std::vector<unsigned char> bsp2( pBsp2, pBsp2 + bsp2Size );
		ToolchainFixtureFreeBsp2( pBsp2 );
		CheckContainer( bsp2, MapContainerKind::Bsp2, entities, planes );
	}

	// Structured failure crosses the boundary: a truncated file.
	std::vector<unsigned char> truncated( legacy.begin(), legacy.begin() + 16 );
	VectorSource source( truncated );
	MapContainerOpenOptions options = {};
	IMapContainer *pContainer = NULL;
	const MapContainerStatus status = OpenMapContainer( source, options, &pContainer );
	ABI_CHECK( !status.Ok() && pContainer == NULL );
	ABI_CHECK( std::strlen( MapContainerErrorName( status.code ) ) > 0 );

	return toolchaintest::Finish();
}
