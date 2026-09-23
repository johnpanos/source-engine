//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: On-disk layout of the BSP2 map container (RFC 0008, phase F1).
//
// Every multi-byte field is little-endian. Readers decode fields byte by byte;
// the structs below document the layout and are never read by casting file
// bytes. This header is C++11-compatible so legacy consumers can include it.
//
//   [header, 64 bytes] [lump payloads, each aligned] [directory, count * 64]
//
// Header (offset: field)
//   0  magic "SRCBSP2\x1A"          8 bytes; distinct from "VBSP"
//   8  container version            uint32, currently 1
//  12  flags                        uint32, must be zero in version 1
//  16  header size                  uint32, 64
//  20  directory entry size         uint32, 64
//  24  directory offset             uint64
//  32  lump count                   uint32
//  36  hash algorithm               uint32, 1 = BLAKE2b-128 (RFC 7693)
//  40  map revision                 int32
//  44  reserved                     uint32, zero
//  48  directory hash               16 bytes over the directory bytes
//
// Directory entry
//   0  4CC id                       uint32 ('L','0','0','0' reads "L000")
//   4  lump version                 uint32
//   8  flags                        uint32 (compression in bits 0-3, required bit 8)
//  12  alignment                    uint32, power of two >= 16
//  16  offset                       uint64, multiple of alignment
//  24  stored size                  uint64
//  32  uncompressed size            uint64
//  40  reserved                     uint64, zero
//  48  content hash                 16 bytes over the uncompressed content
//
// Legacy carriage: legacy lump N is stored byte-identically as 4CC "Lnnn".
// "LHDR" holds the legacy file size (uint64) followed by the verbatim legacy
// dheader_t, so legacy lump origins, versions and the legacy LZMA size field
// are preserved. "LGAP" records any nonzero bytes between legacy lumps as
// {uint64 offset, uint64 length, bytes, zero padding to 8}.
//
//=============================================================================//

#ifndef MAPCONTAINER_MAP_CONTAINER_FORMAT_H
#define MAPCONTAINER_MAP_CONTAINER_FORMAT_H

#include <cstdint>

namespace mapcontainer
{

inline uint32_t MakeFourCC( char a, char b, char c, char d )
{
	return (uint32_t)(uint8_t)a | ( (uint32_t)(uint8_t)b << 8 ) | ( (uint32_t)(uint8_t)c << 16 ) |
	       ( (uint32_t)(uint8_t)d << 24 );
}

// "Lnnn" for legacy lump index 0..999.
inline uint32_t LegacyLumpFourCC( int nLegacyIndex )
{
	return MakeFourCC( 'L', (char)( '0' + ( nLegacyIndex / 100 ) % 10 ),
	    (char)( '0' + ( nLegacyIndex / 10 ) % 10 ), (char)( '0' + nLegacyIndex % 10 ) );
}

static const uint8_t kBsp2Magic[8] = { 'S', 'R', 'C', 'B', 'S', 'P', '2', 0x1A };
static const uint32_t kBsp2ContainerVersion = 1;
static const uint32_t kBsp2HeaderSize = 64;
static const uint32_t kBsp2EntrySize = 64;
static const uint32_t kBsp2HashBlake2b128 = 1;
static const uint32_t kBsp2HashSize = 16;
static const uint32_t kBsp2MinAlignment = 16;
static const uint32_t kBsp2BulkAlignment = 4096;
static const uint32_t kBsp2MaxLumps = 65536;

static const uint32_t kBsp2CompressionMask = 0x0000000Fu;
static const uint32_t kBsp2CompressionNone = 0;
static const uint32_t kBsp2CompressionZstd = 1; // reserved; not yet supported
static const uint32_t kBsp2CompressionLzma = 2; // reserved; not yet supported
static const uint32_t kBsp2FlagRequired = 0x00000100u;
static const uint32_t kBsp2KnownFlags = kBsp2CompressionMask | kBsp2FlagRequired;

// Legacy (VBSP) layout facts the container needs; public/bspfile.h stays the
// owner of the legacy structures themselves.
static const uint32_t kLegacyIdent = 0x50534256u; // "VBSP"
static const int kLegacyLumpCount = 64;
static const uint32_t kLegacyHeaderSize = 8 + 16 * 64 + 4;

static const uint32_t kLumpLegacyHeader = 0x5244484Cu; // "LHDR"
static const uint32_t kLumpLegacyGaps = 0x5041474Cu;   // "LGAP"
static const uint32_t kLegacyHeaderLumpSize = 8 + kLegacyHeaderSize;

} // namespace mapcontainer

#endif // MAPCONTAINER_MAP_CONTAINER_FORMAT_H
