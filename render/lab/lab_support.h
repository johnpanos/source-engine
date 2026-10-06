//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shared support (RFC 0016 K11): game files, texture
//			staging, the lab's device and image files. Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_SUPPORT_H
#define RENDER_LAB_LAB_SUPPORT_H

#include "mapcontainer/probe_volume.h"
#include "content/asset_resolver.h"
#include "mapcontainer/reflection_probes.h"
#include "mapcontainer/world_lightmap.h"
#include "mdl/studio_model.h"
#include "render/device/device.h"
#include "render/pass/lights/cluster_pass.h"
#include "render/frame/debug_controls.h"
#include "render/pass/world/world_pass.h"
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

// Lab ownership for GPU lists shared by subsequent surface draws. Canvas renders
// wait for completion; destruction also drains error paths before retiring buffers.
class LabClusterLists
{
public:
	static std::unique_ptr<LabClusterLists> Create( device::IRenderDevice2 &device,
	    const pass::lights::ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights,
	    std::span<const area_light::AreaLight> areas = {} );
	~LabClusterLists();
	void Bind( material::GroupRequest &request ) const;

private:
	explicit LabClusterLists( device::IRenderDevice2 &device ) : m_Device( device ) {}
	device::IRenderDevice2 &m_Device;
	std::unique_ptr<pass::lights::ClusterKernel> m_Kernel;
	pass::lights::ClusterBuffers m_Buffers;
};

std::optional<std::string> ReadFile( const std::filesystem::path &path );

// A game directory's loose files, case-insensitively (Source content paths
// are case-insensitive and the tools lowercase them).
class GameFiles final : public mdl::IModelFiles
{
public:
	explicit GameFiles( std::filesystem::path root, std::filesystem::path package = {} );
	bool PackageReady() const noexcept
	{
		return m_Resolver.HasIndex() && !m_Resolver.InvalidIndex();
	}

	std::optional<std::filesystem::path> Resolve( const std::string &relative ) const;
	bool Exists( const std::string &path ) const override;
	bool Read( const std::string &path, std::string &out ) const override;

private:
	std::filesystem::path m_Root;
	content::AssetResolver m_Resolver;
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

// One LMAP layer as the lightmap basis reads it, and its split into pages:
// render.pass.world owns both (the product's world stage reads them too).
using LightmapLayerPages = pass::world::LightmapPages;
using pass::world::SplitLightmapLayer;

// The LMAP layer a world surface's baked diffuse light comes from under
// render.indirect-policy.v1's Baked policy (RFC 0011), with RFC 0016's rule
// that each light counts once per surface: the total layer, or the indirect
// layer when a runtime light the core evaluates owns the surface's direct
// light (the bake's direct share is then the core's to draw).
mapcontainer::WorldLightmapLayer BakedLightmapLayer( bool directOwnedByCore );

// A PRBV lump (RFC 0011 render.probe-volume.v1) staged as
// render/shaders/common/probe_volume.glsl reads it: the atlas (RGBA16F) as
// `<name>-atlas` and the grid table (RGBA32F, WriteProbeGridTable) as
// `<name>-grids`. The reason when it does not validate or is refused.
std::optional<std::string> StageProbeVolume( resources::TextureCache &cache,
    const std::string &name, std::span<const std::byte> lump,
    mapcontainer::ProbeVolumeLayout &layout );

// An RPRB lump (RFC 0008's reflection probes, R50) staged as
// render/shaders/common/reflection_probes.glsl reads it:
// WriteReflectionProbeTexture's GPU form (RGBA16F) in `mode`, relit when the
// probes carry relight bands and `relight` asks, as `name`. The reason when
// it does not validate or is refused.
// `layout` receives the decoded form's layout (RPRB v7's radiance blocks
// expanded, mapcontainer::DecodeReflectionProbes), and `decoded`, when given,
// its bytes, which a ReflectionProbesView takes with it.
std::optional<std::string> StageReflectionProbes( resources::TextureCache &cache,
    const std::string &name, std::span<const std::byte> lump,
    mapcontainer::ReflectionProbeMode mode, bool relight,
    mapcontainer::ReflectionProbesLayout &layout, std::vector<std::byte> *decoded = nullptr );

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
