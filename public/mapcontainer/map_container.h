//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Container-neutral map reader (RFC 0008, phase F1).
//
// IMapContainer answers "where is this lump and what is it" for a legacy VBSP
// file or a BSP2 container. It never owns file handles: callers provide an
// IMapByteSource and keep reading lump bytes through their own I/O path, so the
// engine's filesystem, lump override files and pak mounting keep working.
//
// This header is C++11-compatible for legacy consumers. The implementation is
// a strict C++20 target (mapcontainer/). No exceptions cross this boundary.
//
//=============================================================================//

#ifndef MAPCONTAINER_MAP_CONTAINER_H
#define MAPCONTAINER_MAP_CONTAINER_H

#include <stddef.h>
#include <stdint.h>

#include "mapcontainer/map_container_format.h"

namespace mapcontainer
{

enum class MapContainerKind
{
	LegacyVbsp,
	Bsp2,
};

enum class MapContainerError
{
	Ok = 0,
	ReadFailed,
	Truncated,
	BadMagic,
	UnsupportedLegacyIdent,
	UnsupportedContainerVersion,
	UnsupportedLayout,
	UnsupportedHash,
	TooManyLumps,
	DirectoryOutOfBounds,
	DirectoryHashMismatch,
	LumpOutOfBounds,
	LumpMisaligned,
	LumpOverlap,
	DuplicateLump,
	UnknownFlags,
	UnsupportedCompression,
	SizeMismatch,
	UnknownRequiredLump,
	ContentHashMismatch,
	MissingLegacyHeader,
	LegacyHeaderInvalid,
	LegacyLumpMismatch,
	LegacyGapsInvalid,
	OutOfMemory,
	InvalidArgument,
};

// Structured failure: the error plus the lump and file offset it concerns
// (zero when not applicable).
struct MapContainerStatus
{
	MapContainerError code;
	uint32_t fourcc;
	uint64_t offset;

	bool Ok() const { return code == MapContainerError::Ok; }
};

const char *MapContainerErrorName( MapContainerError error );

struct MapLumpInfo
{
	uint32_t fourcc;
	uint32_t version;
	uint32_t flags;
	uint32_t alignment;
	uint64_t offset;           // stored bytes start here in the container file
	uint64_t storedSize;
	uint64_t uncompressedSize; // equals storedSize when uncompressed
	// Legacy lumps only: the lump's offset in its legacy file (game lump
	// dictionaries hold absolute legacy offsets) and the legacy LZMA field.
	uint64_t legacyOrigin;
	uint32_t legacyUncompressedSize;
	bool hasHash;
	uint8_t hash[kBsp2HashSize];
};

// Random-access bytes of one map file. Must outlive the container.
class IMapByteSource
{
public:
	virtual uint64_t Size() const = 0;
	virtual bool ReadAt( uint64_t offset, void *pDest, size_t size ) = 0;

protected:
	~IMapByteSource() {}
};

struct MapContainerOpenOptions
{
	// Verify every lump's content hash while opening (tools, validation).
	// The engine instead verifies each lump after reading it.
	bool verifyContent;
	// Extra 4CCs the caller understands; unknown lumps flagged required fail.
	const uint32_t *pKnownFourCCs;
	size_t nKnownFourCCs;
	// Tools that only consume the legacy payload (the legacy exporter) may
	// skip required lumps they do not understand; runtime readers never do.
	bool allowUnknownRequired;
};

class IMapContainer
{
public:
	virtual ~IMapContainer() {}

	virtual MapContainerKind Kind() const = 0;
	virtual int LegacyVersion() const = 0;
	virtual int32_t MapRevision() const = 0;

	// Legacy lump 0..63. BSP2 containers answer from LHDR, so empty lumps keep
	// their legacy version. Returns false for an out-of-range index.
	virtual bool FindLegacyLump( int nLegacyIndex, MapLumpInfo *pInfo ) const = 0;
	virtual bool FindLump( uint32_t fourcc, MapLumpInfo *pInfo ) const = 0;

	// Directory enumeration (legacy containers enumerate Lnnn entries).
	virtual uint32_t LumpCount() const = 0;
	virtual bool LumpAt( uint32_t nIndex, MapLumpInfo *pInfo ) const = 0;

	// Checks bytes read for pInfo against the directory hash (no-op success
	// for lumps without a hash, which includes every legacy VBSP lump).
	virtual MapContainerStatus VerifyContent( const MapLumpInfo &info, const void *pData,
	                                          uint64_t size ) const = 0;
};

// Detects the container kind from the file's first bytes and validates its
// directory. On success *ppContainer is owned by the caller; release it with
// DestroyMapContainer.
MapContainerStatus OpenMapContainer( IMapByteSource &source, const MapContainerOpenOptions &options,
                                     IMapContainer **ppContainer );
void DestroyMapContainer( IMapContainer *pContainer );

// True when the first bytes look like a BSP2 container (cheap sniffing for
// callers that only need to route; OpenMapContainer still validates).
bool IsBsp2Magic( const void *pPrefix, size_t size );

} // namespace mapcontainer

#endif // MAPCONTAINER_MAP_CONTAINER_H
