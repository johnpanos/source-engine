//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded LMAP (BSP2 world lightmap) validation, decoding and layer
//          layout for runtime consumers and packaging tools (RFC 0008 F3/F4,
//          RFC 0011).
//
// LMAP v3 is the one version (2026-10-05; v1 and v2 are refused). The
// encoding is owned by tools/quality/world_lightmap_v3.py: a 64-byte header
// ("LMP3", 3, page width, height, layer count 1..3, flags with bit 0 = every
// layer's gradient alpha is the sun's visibility, the irradiance and gradient
// VkFormats BC6H_UFLOAT_BLOCK and BC7_UNORM_BLOCK, each page's bytes, the data
// offset 64, 8 zero bytes), then per layer its BC6H irradiance blocks and its
// BC7 gradient blocks. Every layer is directional: the gradient's RGB is the
// world-space luminance gradient beta * 0.5 + 0.5 (beta in [-1, 1]).
//
// The layer roles are fixed by the layer count so a consumer never parses
// metadata to decide what it samples: 1 total; 2 total, indirect; 3 total,
// direct, indirect.
//
// GPU consumers upload the blocks. CPU consumers (direct occlusion, the
// legacy backend) decode them with DecodeWorldLightmap into the RGBA16F
// form below: each layer a page twice as wide as it is tall, the irradiance
// on the left (alpha: the sun's visibility on layer 0, else 1) and beta on
// the right (alpha 1).
//
//=============================================================================//
#ifndef MAPCONTAINER_WORLD_LIGHTMAP_H
#define MAPCONTAINER_WORLD_LIGHTMAP_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mapcontainer
{

static const uint32_t kWorldLightmapVersion = 3;
static const uint32_t kWorldLightmapMagic = 0x33504D4Cu; // "LMP3"
static const uint32_t kWorldLightmapHeaderBytes = 64;
static const uint32_t kWorldLightmapFlagSun = 1;
static const uint32_t kWorldLightmapIrradianceVkFormat = 143; // BC6H_UFLOAT_BLOCK
static const uint32_t kWorldLightmapGradientVkFormat = 145;   // BC7_UNORM_BLOCK
static const uint32_t kWorldLightmapMaxLayers = 3;
// A sanity bound on map input, not a budget.
static const uint64_t kWorldLightmapMaxBytes = 1024ull * 1024 * 1024;
static const uint32_t kWorldLightmapMaxDimension = 16384;
static const uint32_t kWorldLightmapTexelBytes = 8; // the decoded form's RGBA16F
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
	InvalidLayerCount,
	InvalidDescriptor,
	InvalidLevelIndex,
	UnsupportedVersion,
};

// A validated v3 lump: page size and each layer's blocks (offsets into the
// lump's bytes).
struct WorldLightmapBlocks
{
	uint32_t width = 0; // of each page
	uint32_t height = 0;
	uint32_t layerCount = 0;
	bool sun = false;             // every layer's gradient alpha is the sun's visibility
	uint64_t irradianceBytes = 0; // BC6H, per layer
	uint64_t gradientBytes = 0;   // BC7, per layer
	uint64_t irradianceOffset[kWorldLightmapMaxLayers] = {};
	uint64_t gradientOffset[kWorldLightmapMaxLayers] = {};
	WorldLightmapLayer roles[kWorldLightmapMaxLayers] = {};
};
// The decoded RGBA16F form (DecodeWorldLightmap): offsets into its buffer.
struct WorldLightmapLayout
{
	uint32_t version; // 3
	uint32_t width;   // 2 * the page width
	uint32_t height;
	uint32_t layerCount;
	uint64_t layerBytes; // width * height * 8
	uint64_t layerOffset[kWorldLightmapMaxLayers];
	WorldLightmapLayer roles[kWorldLightmapMaxLayers];
};
// Validates complete LMAP bytes without retaining them or allocating.
// `expectedVersion` is the lump's version, or 0 for a tool.
WorldLightmapError ValidateWorldLightmap( const void *pData, size_t size, uint32_t expectedVersion,
    WorldLightmapBlocks *pBlocks = nullptr ) noexcept;
// Decodes validated bytes into the RGBA16F form; `pOut` receives every layer,
// and false when the buffer cannot be allocated.
bool DecodeWorldLightmap( const void *pData, const WorldLightmapBlocks &blocks,
    std::vector<std::byte> *pOut, WorldLightmapLayout *pLayout );
// Index of `role` in a layout, or -1 when the map does not carry it.
int WorldLightmapLayerIndex( const WorldLightmapLayout &layout, WorldLightmapLayer role ) noexcept;
int WorldLightmapLayerIndex( const WorldLightmapBlocks &blocks, WorldLightmapLayer role ) noexcept;
const char *WorldLightmapErrorName( WorldLightmapError error ) noexcept;
const char *WorldLightmapLayerName( WorldLightmapLayer role ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_WORLD_LIGHTMAP_H
