//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: PRBV probe volume (RFC 0011 render.probe-volume.v1): bounded
//          validation, layout and the C++ reference sampler.
//
// The encoding is documented once, in tools/quality/probe_volume.py (the
// pipeline writer and independent reader); this reader must accept exactly
// what that writer produces and reject the malformations it lists. The
// sampler here is the oracle for shaders/probe_volume.glsl and the engine's
// source of ambient cubes for models on BSP2 maps that carry a volume.
//
// Units: irradiance texels are irradiance / pi (the lightmap's diffuse light
// unit); positions, spacing and distances are Source units, and visibility
// moments are stored as fractions of the grid's max distance.
//
//=============================================================================//

#ifndef MAPCONTAINER_PROBE_VOLUME_H
#define MAPCONTAINER_PROBE_VOLUME_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

static const uint32_t kLumpProbeVolume = 0x56425250u; // "PRBV"
static const uint32_t kProbeVolumeVersion = 1;
static const uint32_t kProbeVolumeHeaderBytes = 64;
static const uint32_t kProbeVolumeGridBytes = 96;
static const uint32_t kProbeVolumeMaxGrids = 16;
static const uint32_t kProbeVolumeMaxLayers = 2;
static const uint32_t kProbeIrradianceTile = 8;
static const uint32_t kProbeVisibilityTile = 16;
static const uint32_t kProbeVolumeMaxAtlas = 16384;
static const uint32_t kProbeVolumeMaxProbes = 1u << 20;
static const uint64_t kProbeVolumeMaxBytes = 512ull * 1024 * 1024;
// Sampling constants shared with probe_volume.py and probe_volume.glsl.
static const float kProbeNormalBias = 0.1f; // times the smallest spacing
static const float kProbeCrushThreshold = 0.2f;

enum class ProbeVolumeLayer : uint32_t
{
	Total = 0,
	Indirect = 1,
};

enum class ProbeVolumeError
{
	Ok = 0,
	Truncated,
	BadMagic,
	UnsupportedVersion,
	InvalidCounts,
	InvalidAtlas,
	InvalidGrid,
	SectionOutsideAtlas,
	InvalidState,
	RelocationOutOfBounds,
	InvalidIrradiance,
	InvalidVisibility,
};

struct ProbeGridLayout
{
	float origin[3];
	float spacing[3];
	uint32_t dims[3];
	uint32_t probeCount;
	uint32_t tilesPerRow;
	float maxRelocation;
	float maxDistance;
	uint32_t irradianceOrigin[kProbeVolumeMaxLayers][2];
	uint32_t visibilityOrigin[2];
	uint32_t stateOrigin[2];
};

struct ProbeVolumeLayout
{
	uint32_t gridCount;
	uint32_t layerCount;
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint64_t atlasOffset; // RGBA16F texels, rows top-left first
	uint64_t atlasBytes;
	uint32_t activeProbes;
	ProbeGridLayout grids[kProbeVolumeMaxGrids];
};

// Validates complete PRBV bytes (header, grids, section bounds, every probe's
// state, and finite non-negative irradiance and distance moments) without
// retaining them or allocating.
ProbeVolumeError ValidateProbeVolume(
    const void *pData, size_t size, ProbeVolumeLayout *pLayout = nullptr ) noexcept;
const char *ProbeVolumeErrorName( ProbeVolumeError error ) noexcept;

// A read-only view of validated bytes; the caller keeps them alive.
class ProbeVolumeView
{
public:
	ProbeVolumeView( const void *pData, const ProbeVolumeLayout &layout ) noexcept;
	// Irradiance / pi arriving at a surface point with unit normal `normal`
	// from `layer`; false when the point lies outside every grid. With
	// `useVisibility` false the Chebyshev test is skipped (the leak-control
	// sensitivity build).
	bool Sample( const float position[3], const float normal[3], ProbeVolumeLayer layer,
	    bool useVisibility, float outIrradiance[3] ) const noexcept;
	// Source's six-direction ambient cube (+X -X +Y -Y +Z -Z) at a point.
	bool AmbientCube( const float position[3], ProbeVolumeLayer layer, bool useVisibility,
	    float outCube[6][3] ) const noexcept;
	const ProbeVolumeLayout &Layout() const noexcept { return m_layout; }

private:
	void Texel( uint32_t x, uint32_t y, float out[4] ) const noexcept;
	void Bilinear( float x, float y, float out[4] ) const noexcept;
	void TileSample( const uint32_t origin[2], uint32_t tile, uint32_t probe, uint32_t tilesPerRow,
	    const float direction[3], float out[4] ) const noexcept;
	bool SampleGrid( const ProbeGridLayout &grid, const float position[3], const float normal[3],
	    ProbeVolumeLayer layer, bool useVisibility, float out[3] ) const noexcept;

	const unsigned char *m_texels;
	ProbeVolumeLayout m_layout;
};

float HalfToFloat( uint16_t half ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_PROBE_VOLUME_H
