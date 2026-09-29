//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5, the K5 plan's step 4): the BSP
//			world's surfaces drawn by the core, at a core-pass slot of the
//			legacy stream (render/legacy/core_passes.h).
//
//			At level load the engine hands the pass the world's vertices,
//			triangle indices, surfaces (an index range, a material and a
//			lightmap page each) and materials (shader name, variables, and
//			the material system handles of their textures). The pass names no
//			material family: it maps each material (MapVariables) and asks the
//			one resolver (render/material/program_resolver.h) whether the
//			model draws it, opaque, from the world's vertex; a surface of such
//			a material is the pass's, and the engine stops drawing it through
//			the legacy stream while the pass is on. A material the model does
//			not draw yet stays legacy, with the reason in WorldStats.
//
//			Per view, the engine queues the visible surfaces the pass draws
//			and the view's world-to-clip (QueueView); the returned tag names
//			the slot to mark at that point of the stream. When the scene pass
//			records the slot, Record draws the view's surfaces into the slot's
//			target: the back buffer (its sRGB view, or its unorm view with the
//			shader encoding sRGB) and its depth, loaded and
//			stored, with the resolved programs, and the materials' textures
//			and the lightmap pages imported from the backend (IWorldTextures)
//			with the backend's samplers.
//			Device objects are made on the render sequence at the first Record
//			after SetWorld, and the previous world's are released behind the
//			frames that used them.
//
//			Nothing is dropped silently: a view the pass cannot draw (no sRGB
//			target, several samples, a texture that does not import) counts in
//			WorldStats with the reason.
//
//			Threads: SetWorld, ClearWorld, Draws and QueueView on the main
//			thread; Record on the render sequence. The queue between them is
//			locked.
//
//=============================================================================//

#ifndef RENDER_PASS_WORLD_WORLD_PASS_H
#define RENDER_PASS_WORLD_WORLD_PASS_H

#include "render/device/device.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace render::pass::world
{

// The `lightmapped` family's vertex (material::LightmappedVertex).
struct WorldVertex
{
	float position[3] = {};
	float uv[2] = {};
	float lightmapUv[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};

struct WorldMaterial
{
	std::string name;
	std::string shader;
	std::vector<std::pair<std::string, std::string>> variables; // "$key", value
	// The material system handle of each texture variable ("$key", handle).
	std::vector<std::pair<std::string, int>> textures;
};

struct WorldSurface
{
	std::uint32_t material = 0; // into WorldData::materials
	int lightmapPage = 0;       // the material system handle of its lightmap page
	std::uint32_t firstIndex = 0;
	std::uint32_t indexCount = 0;
};

struct WorldData
{
	std::vector<WorldVertex> vertices;
	std::vector<std::uint32_t> indices; // triangle lists, into vertices
	std::vector<WorldSurface> surfaces;
	std::vector<WorldMaterial> materials;
};

// The backend's textures (render/legacy/core_passes.h ICoreTextures).
class IWorldTextures
{
public:
	virtual device::TextureId Import( int handle, bool srgb ) = 0;
	virtual device::SamplerDesc Sampler( int handle ) = 0;

protected:
	~IWorldTextures() = default;
};

struct WorldTarget
{
	// The device the slot records on (the legacy backend's); the pass's
	// device objects live on it.
	device::IRenderDevice2 *device = nullptr;
	// The color target (home kColorAttachment): its sRGB view, or, when it has
	// none, its unorm view with the shader encoding sRGB (encodeOutput).
	device::TextureId color;
	device::Format colorFormat = device::Format::kUnknown;
	bool encodeOutput = false;
	device::TextureId depth; // home kDepthWrite
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	IWorldTextures *textures = nullptr;
	// A token no earlier than every submission made so far: the previous
	// world's objects are released behind it.
	device::CompletionToken submitted;
	// The frame's light terms at the slot (material::FrameTerms).
	float lightmapScale = 1.0f;
	float outputScale = 1.0f;
};

struct WorldView
{
	std::vector<std::uint32_t> surfaces; // into WorldData::surfaces
	float toClip[16] = {};               // world to clip, row-major, D3D9 conventions
	device::Viewport viewport;
};

struct WorldStats
{
	std::uint32_t materials = 0;
	std::uint32_t claimedMaterials = 0; // opaque materials the model draws
	std::uint32_t surfaces = 0;
	std::uint32_t claimedSurfaces = 0;
	std::uint64_t viewsQueued = 0;
	std::uint64_t viewsDrawn = 0;
	std::uint64_t viewsFailed = 0;
	std::uint64_t surfacesDrawn = 0;
	std::string lastFailure;
	// Why materials stay legacy: reason and count, most frequent first.
	std::vector<std::pair<std::string, std::uint32_t>> gaps;
	// The materials the pass draws, and their surface counts.
	std::vector<std::pair<std::string, std::uint32_t>> claimed;
};

// A world tag: the high bit set, then the view's serial.
inline constexpr std::uint32_t kWorldTag = 0x80000000u;
inline bool IsWorldTag( std::uint32_t tag )
{
	return ( tag & kWorldTag ) != 0;
}

class WorldPass
{
public:
	WorldPass();
	~WorldPass();
	WorldPass( const WorldPass & ) = delete;
	WorldPass &operator=( const WorldPass & ) = delete;

	// Main thread.
	void SetWorld( WorldData data );
	void ClearWorld();
	// Whether the pass draws the material's surfaces (valid after SetWorld).
	bool Draws( std::uint32_t material ) const;
	// The tag of the slot to mark for the view; 0 when there is nothing to draw.
	std::uint32_t QueueView( WorldView view );
	WorldStats Stats() const;

	// Render sequence: draws the view a slot's tag names.
	void Record( std::uint32_t tag, device::CommandEncoder &encoder, const WorldTarget &target );
	// The device is about to go (after an idle wait): its objects are released.
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::world

#endif // RENDER_PASS_WORLD_WORLD_PASS_H
