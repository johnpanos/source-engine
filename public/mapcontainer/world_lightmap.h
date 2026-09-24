//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded LMAP (BSP2 world lightmap) validation and layer layout for
//          runtime consumers and packaging tools (RFC 0008 F4, RFC 0011).
//
// An LMAP lump is one uncompressed linear RGBA16F KTX2 texture: 2D, one mip
// level, one face, no supercompression, texel rows top-left first.
//
//   version 1  a single page (KTX2 layerCount 0): the total baked diffuse
//              light (irradiance / pi), with the optional directional half,
//              reflection-probe band and sun texels of RFC 0008's preview
//   version 2  a layered page (KTX2 2D array); layer 0 is always the total
//              page above, and later layers are separated light of the same
//              dimensions, in the order the layer count fixes:
//                2 layers: total, indirect
//                3 layers: total, direct, indirect
//              Separated layers hold diffuse light only (no probe band or
//              marker texels) and, on a directional page, leave the
//              gradient half zero.
//
// The layer roles are fixed by the layer count so a consumer never parses
// key/value metadata to decide what it samples.
//
//=============================================================================//

#ifndef MAPCONTAINER_WORLD_LIGHTMAP_H
#define MAPCONTAINER_WORLD_LIGHTMAP_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

static const uint32_t kWorldLightmapMinVersion = 1;
static const uint32_t kWorldLightmapLayeredVersion = 2;
static const uint32_t kWorldLightmapMaxLayers = 3;
static const uint64_t kWorldLightmapMaxBytes = 256ull * 1024 * 1024;
static const uint32_t kWorldLightmapMaxDimension = 16384;
static const uint32_t kWorldLightmapTexelBytes = 8; // RGBA16F

enum class WorldLightmapLayer : uint32_t
{
	Total = 0,
	Direct = 1,
	Indirect = 2,
};

enum class WorldLightmapError
{
	Ok = 0,
	Truncated,
	BadIdentifier,
	UnsupportedFormat,
	UnsupportedTopology,
	InvalidLayerCount,
	InvalidDescriptor,
	InvalidLevelIndex,
	UnsupportedVersion,
	VersionMismatch,
};

struct WorldLightmapLayout
{
	uint32_t version; // 1 or 2, derived from the KTX2 layer count
	uint32_t width;
	uint32_t height;
	uint32_t layerCount; // 1 for version 1
	uint64_t layerBytes; // width * height * 8
	uint64_t layerOffset[kWorldLightmapMaxLayers];
	WorldLightmapLayer roles[kWorldLightmapMaxLayers];
};

// Validates complete LMAP bytes without retaining them or allocating.
// `expectedVersion` is the lump's version, or 0 for a tool deriving it. The
// caller keeps the bytes alive while it uses the layer offsets.
WorldLightmapError ValidateWorldLightmap( const void *pData, size_t size, uint32_t expectedVersion,
    WorldLightmapLayout *pLayout = nullptr ) noexcept;

// Index of `role` in a validated layout, or -1 when the map does not carry it.
int WorldLightmapLayerIndex( const WorldLightmapLayout &layout, WorldLightmapLayer role ) noexcept;

const char *WorldLightmapErrorName( WorldLightmapError error ) noexcept;
const char *WorldLightmapLayerName( WorldLightmapLayer role ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_WORLD_LIGHTMAP_H
