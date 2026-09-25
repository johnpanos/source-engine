//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDFV signed distance volume (RFC 0011 G6, the SDF-traced
//          producer's input): bounded validation and a read-only view.
//
// The model and encoding are documented once, in tools/quality/sdf_volume.py
// (the pipeline writer and independent reader); this reader accepts exactly
// what that writer produces and rejects its malformations.
//
//=============================================================================//

#ifndef MAPCONTAINER_SDF_VOLUME_H
#define MAPCONTAINER_SDF_VOLUME_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

static const uint32_t kLumpSdfVolume = 0x56464453u; // "SDFV"
static const uint32_t kSdfVolumeVersion = 1;
static const uint32_t kSdfVolumeHeaderBytes = 64;
static const uint32_t kSdfVoxelBytes = 16;
static const uint32_t kSdfLightBytes = 64;
static const uint32_t kSdfMaxVoxels = 1u << 24;
static const uint32_t kSdfMaxLights = 64;
static const uint16_t kSdfNoSource = 0xFFFF;

enum class SdfLightKind : uint32_t
{
	Rect = 0,
	Distant = 1,
	Dome = 2,
};

enum class SdfVolumeError
{
	Ok = 0,
	Truncated,
	BadMagic,
	UnsupportedVersion,
	InvalidGrid,
	SizeMismatch,
	InvalidVoxel,
	InvalidLight,
};

struct SdfLight
{
	uint32_t kind;
	int32_t style;
	float rgb[3];
	float a[3];
	float b[3];
	float c[3];
	float reserved[2];
};
static_assert( sizeof( SdfLight ) == 64, "SDFV light record" );

struct SdfVolumeLayout
{
	float origin[3];
	float voxel;
	uint32_t dims[3];
	uint32_t lightCount;
	float maxDistance;
	uint64_t voxelOffset;
	uint64_t lightOffset;
};

SdfVolumeError ValidateSdfVolume(
    const void *pData, size_t size, SdfVolumeLayout *pLayout = nullptr ) noexcept;
const char *SdfVolumeErrorName( SdfVolumeError error ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_SDF_VOLUME_H
