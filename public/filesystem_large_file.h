//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Reads past 2 GiB in files opened through IFileSystem.
//
// IFileSystem's Seek, Tell and Size are 32-bit, and its layout is a frozen
// ABI, so files larger than that (a BSP2 map with its lighting lumps passes
// 2 GiB) are read through this side interface, reached with
// IFileSystem::QueryInterface( FILESYSTEM_LARGE_FILE_INTERFACE_VERSION ). A
// file system that does not return it reads only the first 2 GiB.
//
//===========================================================================//

#ifndef FILESYSTEM_LARGE_FILE_H
#define FILESYSTEM_LARGE_FILE_H
#ifdef _WIN32
#pragma once
#endif

#include <stddef.h>

#include "tier0/platform.h"

typedef void *FileHandle_t;

#define FILESYSTEM_LARGE_FILE_INTERFACE_VERSION "FileSystemLargeFile001"

abstract_class IFileSystemLargeFile
{
public:
	// The open file's length in bytes, 0 for an invalid handle.
	virtual uint64 Size64( FileHandle_t file ) = 0;

	// Reads `size` bytes at `offset` from the start of the open file into
	// pDest, moving its position. False when the range is not inside the
	// file or the read is short; pDest's contents are then unspecified.
	virtual bool ReadAt( FileHandle_t file, uint64 offset, void *pDest, size_t size ) = 0;
};

#endif // FILESYSTEM_LARGE_FILE_H
