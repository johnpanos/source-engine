//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RTRN radiosity transfer (RFC 0011, the precomputed radiosity
//          producer's input): bounded validation and a read-only view.
//
// The model and encoding are documented once, in
// tools/quality/radiosity_transfer.py (the pipeline writer and independent
// reader); this reader must accept exactly what that writer produces and
// reject the malformations it lists (radiosity_transfer.malformations).
//
// Units: light is the lightmap's diffuse light (irradiance / pi); positions
// and areas are Source units.
//
//=============================================================================//

#ifndef MAPCONTAINER_RADIOSITY_TRANSFER_H
#define MAPCONTAINER_RADIOSITY_TRANSFER_H

#include "mapcontainer/probe_volume.h"

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

static const uint32_t kLumpRadiosityTransfer = 0x4E525452u; // "RTRN"
static const uint32_t kRadiosityTransferVersion = 1;
static const uint32_t kRadiosityTransferHeaderBytes = 64;
static const uint32_t kRadiosityMaxSources = 64;
static const uint32_t kRadiosityMaxPatches = 1u << 22;
static const uint32_t kRadiosityMaxLinks = 1u << 28;
static const uint32_t kRadiosityProbeTexels = 36; // the PRBV 6 x 6 irradiance interior
static const uint32_t kRadiositySHCoefficients = 9;
static const int32_t kRadiosityMaxStyle = 63;
static const uint32_t kRadiositySourceNameBytes = 48;
static const uint64_t kRadiosityTransferMaxBytes = 1024ull * 1024 * 1024;

enum class RadiositySourceKind : uint32_t
{
	Light = 0,
	Sky = 1,
	Emissive = 2,
};

enum class RadiosityTransferError
{
	Ok = 0,
	Truncated,
	BadMagic,
	UnsupportedVersion,
	InvalidCounts,
	SizeMismatch,
	InvalidSource,
	InvalidPatch,
	InvalidRows,
	IndexOutOfBounds,
	InvalidWeight,
	TransferNotNormalized,
	GatherNotNormalized,
	TopologyMismatch,
};

struct RadiositySource
{
	uint32_t kind;
	int32_t style; // -1: fixed
	uint32_t flags;
	uint32_t reserved;
	char name[kRadiositySourceNameBytes];
};

struct RadiosityPatch
{
	float position[3];
	float normal[3];
	float area;
	float albedo[3];
	uint32_t reserved[2];
};

struct RadiosityTransferLink
{
	uint32_t patch;
	float factor;
};

struct RadiosityInjectionLink
{
	uint32_t patch;
	float light[3];
};

struct RadiosityGatherLink
{
	uint32_t patch;
	float sh[kRadiositySHCoefficients];
};

static_assert( sizeof( RadiositySource ) == 64, "RTRN source record" );
static_assert( sizeof( RadiosityPatch ) == 48, "RTRN patch record" );
static_assert( sizeof( RadiosityTransferLink ) == 8, "RTRN transfer link" );
static_assert( sizeof( RadiosityInjectionLink ) == 16, "RTRN injection link" );
static_assert( sizeof( RadiosityGatherLink ) == 40, "RTRN gather link" );

struct RadiosityTransferLayout
{
	uint32_t sourceCount;
	uint32_t patchCount;
	uint32_t probeCount;
	uint32_t transferLinks;
	uint32_t injectionLinks;
	uint32_t gatherLinks;
	uint64_t prbvHash;
	// Byte offsets of the nine sections, in the encoding's order.
	uint64_t sources, patches, transferRows, transfer, injectionRows, injection, gatherRows, gather,
	    probeDirect;
};

// Validates complete RTRN bytes: the header, the exact layout, every record,
// row, index and weight, and the two normalization bounds. With `prbv`
// (validated PRBV bytes and layout), also that the transfer was baked for
// that volume. Retains nothing and allocates nothing. The bytes must be
// 4-byte aligned (map lumps are).
RadiosityTransferError ValidateRadiosityTransfer( const void *pData, size_t size,
    RadiosityTransferLayout *pLayout = nullptr, const void *prbv = nullptr,
    const ProbeVolumeLayout *prbvLayout = nullptr ) noexcept;
const char *RadiosityTransferErrorName( RadiosityTransferError error ) noexcept;

// The FNV-1a 64 topology hash of validated PRBV bytes: its header and grid
// records, then every grid's probe state texels in probe order.
uint64_t ProbeVolumeTopologyHash( const void *prbv, const ProbeVolumeLayout &layout ) noexcept;

// A read-only view of validated, 4-byte aligned bytes; the caller keeps them
// alive.
class RadiosityTransferView
{
public:
	RadiosityTransferView( const void *pData, const RadiosityTransferLayout &layout ) noexcept;
	const RadiosityTransferLayout &Layout() const noexcept { return m_layout; }
	const RadiositySource &Source( uint32_t s ) const noexcept;
	const RadiosityPatch &Patch( uint32_t p ) const noexcept;
	// Row r's links as [begin, end).
	const RadiosityTransferLink *TransferBegin( uint32_t patch ) const noexcept;
	const RadiosityTransferLink *TransferEnd( uint32_t patch ) const noexcept;
	const RadiosityInjectionLink *InjectionBegin( uint32_t source ) const noexcept;
	const RadiosityInjectionLink *InjectionEnd( uint32_t source ) const noexcept;
	const RadiosityGatherLink *GatherBegin( uint32_t probe ) const noexcept;
	const RadiosityGatherLink *GatherEnd( uint32_t probe ) const noexcept;
	// Source s's direct light at probe i's interior texels (36 rgb).
	const float *ProbeDirect( uint32_t source, uint32_t probe ) const noexcept;

private:
	const uint32_t *Rows( uint64_t offset ) const noexcept;

	const unsigned char *m_bytes;
	RadiosityTransferLayout m_layout;
};

} // namespace mapcontainer

#endif // MAPCONTAINER_RADIOSITY_TRANSFER_H
