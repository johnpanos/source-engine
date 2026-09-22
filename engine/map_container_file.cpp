//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Engine bridge from IFileSystem handles to the map container seam.
//
//=============================================================================//

#include "map_container_file.h"

#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IFileSystem *g_pFileSystem;

void CMapFileByteSource::Attach( FileHandle_t hFile, uint64_t nBase, uint64_t nSize )
{
	m_hFile = hFile;
	m_nBase = nBase;
	m_nSize = nSize;
}

void CMapFileByteSource::Detach()
{
	m_hFile = FILESYSTEM_INVALID_HANDLE;
	m_nBase = 0;
	m_nSize = 0;
}

bool CMapFileByteSource::ReadAt( uint64_t offset, void *pDest, size_t size )
{
	if ( m_hFile == FILESYSTEM_INVALID_HANDLE || offset > m_nSize || size > m_nSize - offset )
		return false;
	// IFileSystem positions are 32-bit; legacy-sized maps only until the
	// filesystem gains 64-bit seeks.
	const uint64_t position = m_nBase + offset;
	if ( position > 0x7FFFFFFFu || size > 0x7FFFFFFFu )
		return false;
	g_pFileSystem->Seek( m_hFile, (int)position, FILESYSTEM_SEEK_HEAD );
	return g_pFileSystem->Read( pDest, (int)size, m_hFile ) == (int)size;
}

mapcontainer::IMapContainer *OpenMapContainerForFile( CMapFileByteSource &source, FileHandle_t hFile,
                                                      const char *pMapName, bool bQuiet )
{
	const unsigned int nBase = g_pFileSystem->Tell( hFile );
	const unsigned int nFileSize = g_pFileSystem->Size( hFile );
	source.Attach( hFile, nBase, nFileSize >= nBase ? nFileSize - nBase : 0 );

	mapcontainer::MapContainerOpenOptions options = {};
	mapcontainer::IMapContainer *pContainer = NULL;
	const mapcontainer::MapContainerStatus status = mapcontainer::OpenMapContainer( source, options, &pContainer );
	if ( !status.Ok() )
	{
		if ( !bQuiet )
		{
			const uint32_t fourcc = status.fourcc;
			const char name[5] = { (char)( fourcc & 0xFF ), (char)( ( fourcc >> 8 ) & 0xFF ),
			                       (char)( ( fourcc >> 16 ) & 0xFF ), (char)( ( fourcc >> 24 ) & 0xFF ), 0 };
			Warning( "Map %s: container error %s (lump %s, offset %llu)\n", pMapName,
			         mapcontainer::MapContainerErrorName( status.code ), fourcc ? name : "-",
			         (unsigned long long)status.offset );
		}
		source.Detach();
		return NULL;
	}
	return pContainer;
}
