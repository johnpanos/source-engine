//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shared support (RFC 0016 K11): game files, texture
//			staging, the lab's device and image files. Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_SUPPORT_H
#define RENDER_LAB_LAB_SUPPORT_H

#include "mapcontainer/world_lightmap.h"
#include "mdl/studio_model.h"
#include "render/device/device.h"
#include "render/frame/debug_controls.h"
#include "render/resources/texture_cache.h"
#include "texturecontainer/texture_image.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::lab
{

std::optional<std::string> ReadFile( const std::filesystem::path &path );

// A game directory's loose files, case-insensitively (Source content paths
// are case-insensitive and the tools lowercase them).
class GameFiles final : public mdl::IModelFiles
{
public:
	explicit GameFiles( std::filesystem::path root ) : m_Root( std::move( root ) ) {}

	std::optional<std::filesystem::path> Resolve( const std::string &relative ) const;
	bool Exists( const std::string &path ) const override;
	bool Read( const std::string &path, std::string &out ) const override;

private:
	std::filesystem::path m_Root;
};

std::optional<device::Format> PortFormat( texturecontainer::PixelFormat format, bool srgb );

// Stages a decoded image with its mips; the name is the importer's texture
// reference ("materials/..."), which the programs' groups look up. The
// reason when it cannot.
std::optional<std::string> StageImage( resources::TextureCache &cache, const std::string &name,
    const texturecontainer::TextureImage &image, bool srgb );

// A linear RGB image as a PFM (portable float map, bottom row first); rgba
// holds width x height RGBA texels, row 0 at the top.
bool WritePfm( const std::filesystem::path &path, std::uint32_t width, std::uint32_t height,
    const std::vector<float> &rgba );

// Stages a 4x4 texture (a cube's six faces with `cube`) of one texel.
bool StageConstant( resources::TextureCache &cache, const std::string &name, device::Format format,
    std::span<const std::byte> texel, bool cube = false );
// An RGBA8 texel.
std::vector<std::byte> ByteTexel( int r, int g, int b, int a );

float HalfToFloat( std::uint16_t half );
// Round to nearest even; NaN stays NaN and out-of-range values become
// infinities.
std::uint16_t FloatToHalf( float value );

// One LMAP layer as the lightmap basis reads it (RFC 0008's LMAP;
// tools/quality/lightmap_directional.py writes the directional form, and the
// native backend's world_pbr.frag reads it). A flat page holds the diffuse
// light (irradiance / pi) at every texel. A directional page is twice as wide
// as it is tall: its left half is that flat light E0, baked on the smooth
// normal N, and its right half holds at the same texel the signed world-space
// luminance gradient beta of E(n) = a + g . n, relative to E0. The world
// mesh's lightmap coordinates span the flat half, so the halves become two
// pages of the same size; sampled whole, a directional page reads beta as
// light.
struct LightmapLayerPages
{
	std::uint32_t width = 0; // of each page
	std::uint32_t height = 0;
	std::vector<std::byte> flat;     // RGBA16F texels, row 0 at the top
	std::vector<std::byte> gradient; // RGBA16F beta; empty for a flat page

	bool Directional() const { return !gradient.empty(); }
};

// Splits one layer of `width` x `height` RGBA16F texels; a page whose width
// is twice its height is directional. No page (flat empty) when `layer` does
// not hold exactly that many texels.
LightmapLayerPages SplitLightmapLayer(
    std::span<const std::byte> layer, std::uint32_t width, std::uint32_t height );

// The LMAP layer a world surface's baked diffuse light comes from under
// render.indirect-policy.v1's Baked policy (RFC 0011), with RFC 0016's rule
// that each light counts once per surface: the total layer, or the indirect
// layer when a runtime light the core evaluates owns the surface's direct
// light (the bake's direct share is then the core's to draw).
mapcontainer::WorldLightmapLayer BakedLightmapLayer( bool directOwnedByCore );

// The lab's device: the Vulkan adapter (RENDER_VK_ADAPTER picks the physical
// device), with the Khronos validation layer and synchronization validation
// when asked, counting its messages into `messages`. The reason when there is
// none, or when validation is asked and the layer is not installed.
std::optional<std::string> CreateLabDevice( bool validate, std::atomic<std::uint64_t> &messages,
    std::unique_ptr<device::IRenderDevice2> &out );

// One --debug-* option with its value into the frame's debug controls (the
// lab's twin of the product's cl_render_debug_* ConVars): --debug-view n,
// --debug-program name, --debug-scale, --debug-range, --debug-threshold,
// --debug-brdf n, --debug-term name[,name...], --debug-force-roughness,
// --debug-force-metalness, --debug-legacy n. False when the option is not
// one of them or its value does not parse; render.frame validates the rest.
bool ParseDebugOption( const std::string &option, const char *value, frame::DebugControls &debug );

} // namespace render::lab

#endif // RENDER_LAB_LAB_SUPPORT_H
