//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Engine bridge from IFileSystem handles to the map container seam
//          (RFC 0008, phase F1). Every engine reader of .bsp headers goes
//          through here so legacy VBSP and BSP2 maps are handled alike.
//
//=============================================================================//

#ifndef MAP_CONTAINER_FILE_H
#define MAP_CONTAINER_FILE_H

#include "filesystem.h"
#include "mapcontainer/map_container.h"

// Reads through an open filesystem handle. The handle and this object must
// outlive any container opened over it. ReadAt moves the file position.
class CMapFileByteSource : public mapcontainer::IMapByteSource
{
public:
	CMapFileByteSource() : m_hFile( FILESYSTEM_INVALID_HANDLE ), m_nBase( 0 ), m_nSize( 0 ) {}

	// nBase is the map's first byte within the handle (COM_OpenFile can
	// return handles into pack files); nSize is the map's byte length.
	void Attach( FileHandle_t hFile, uint64_t nBase, uint64_t nSize );
	void Detach();

	virtual uint64_t Size() const { return m_nSize; }
	virtual bool ReadAt( uint64_t offset, void *pDest, size_t size );

private:
	FileHandle_t m_hFile;
	uint64_t m_nBase;
	uint64_t m_nSize;
};

// Opens a container over hFile (whole file from its current size). Reports
// failures with the map name and structured error; returns NULL on failure.
mapcontainer::IMapContainer *OpenMapContainerForFile( CMapFileByteSource &source, FileHandle_t hFile,
                                                      const char *pMapName, bool bQuiet = false );

#endif // MAP_CONTAINER_FILE_H
