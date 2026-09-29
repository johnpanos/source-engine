//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shared support (RFC 0016 K11): game files, texture
//			staging, the lab's device and image files. Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_SUPPORT_H
#define RENDER_LAB_LAB_SUPPORT_H

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

float HalfToFloat( std::uint16_t half );
// Round to nearest even; NaN stays NaN and out-of-range values become
// infinities.
std::uint16_t FloatToHalf( float value );

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
