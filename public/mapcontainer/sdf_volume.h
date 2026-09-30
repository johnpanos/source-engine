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
// Version 2 adds sphere and spot lights and the light cells; version 3 a
// sphere's or spot's Source falloff (SdfLight::attenuation). Version 1
// volumes (no cells: every light reaches every point) and 2 are still read.
static const uint32_t kSdfVolumeVersion = 3;
static const uint32_t kSdfVolumeHeaderBytes = 96;
static const uint32_t kSdfVolumeV1HeaderBytes = 64;
static const uint32_t kSdfVoxelBytes = 16;
static const uint32_t kSdfLightBytes = 80;   // version 3
static const uint32_t kSdfV2LightBytes = 64; // versions 1 and 2
static const uint32_t kSdfMaxVoxels = 1u << 24;
static const uint32_t kSdfMaxLights = 4096;
static const uint32_t kSdfMaxCells = 1u << 22;
static const uint16_t kSdfNoSource = 0xFFFF;

enum class SdfLightKind : uint32_t
{
	Rect = 0,
	Distant = 1,
	Dome = 2,
	Sphere = 3, // a centre, b[0] radius
	Spot = 4, // a centre, b emitting normal, c radius / cos inner / cos outer, reserved[0] exponent
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
	InvalidCells,
};

struct SdfLight
{
	uint32_t kind;
	int32_t style;
	float rgb[3];
	float a[3];
	float b[3];
	float c[3];
	float reserved[2]; // a spot's exponent in [0]; otherwise zero
	// Version 3: a sphere's or spot's Source falloff, vrad's constant, linear
	// and quadratic terms and hard radius (Source units; all zero, and always
	// in older versions: the inverse square). Its light at distance d from
	// its centre is the inverse-square light times d^2 / (c + l d + q d^2),
	// 0 at or beyond a positive radius (light_set::AttenuatedFalloff).
	float attenuation[4];
};
static_assert( sizeof( SdfLight ) == kSdfLightBytes, "SDFV light record" );

struct SdfVolumeLayout
{
	float origin[3];
	float voxel;
	uint32_t dims[3];
	uint32_t lightCount;
	float maxDistance;
	uint64_t voxelOffset;
	uint64_t lightOffset;
	uint32_t version;
	// The light cells (version 2): cellDims all zero in a version-1 volume.
	float cellOrigin[3];
	float cellSize;
	uint32_t cellDims[3];
	uint32_t cellEntries;
	uint64_t cellOffset;      // (cells + 1) u32 first entries
	uint64_t cellEntryOffset; // cellEntries u16 light indices
	uint32_t lightStride;     // kSdfLightBytes, or kSdfV2LightBytes before version 3
};

SdfVolumeError ValidateSdfVolume(
    const void *pData, size_t size, SdfVolumeLayout *pLayout = nullptr ) noexcept;
const char *SdfVolumeErrorName( SdfVolumeError error ) noexcept;

// Light `index` of a validated volume, its attenuation zero before version 3.
SdfLight ReadSdfLight( const void *pData, const SdfVolumeLayout &layout, uint32_t index ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_SDF_VOLUME_H
