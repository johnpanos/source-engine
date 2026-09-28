//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque (RFC 0016 K5, drawing through K4's material
//			families): draws a scene view's draw list as one graph pass.
//
//			AddOpaquePasses resolves each draw's mesh (render.resources) and
//			its material's family program (render.material IDrawPrograms),
//			then draws the list in its order into the color and depth
//			targets: per draw, the program's pipeline and material bind group,
//			and the draw constants' shared prefix (world-to-clip = the view's
//			view-projection times the instance's world matrix, then the world
//			matrix; FamilyDrawConstants), cut to the bytes the program reads.
//			A program that reads a draw group (drawLayout) gets the instance's
//			(MeshInstanceDesc::drawGroup through IDrawGroups), which must have
//			that layout; one that reads a frame or view group gets the
//			sources' frame or view group, which must have its layout. Depth test, blending and culling are the family
//			pipeline's.
//
//			A draw whose mesh, program or needed group does not resolve, or
//			whose mesh stride or group layout is not the program's, is not
//			drawn; it is counted in
//			OpaqueStats::unresolved, which the frame's owner checks (nothing
//			is dropped silently).
//
//			Mesh buffers, the groups' textures and uniform buffers are
//			imported in their residency usages (vertex, index, sampled,
//			uniform), so the graph sees every access. The pass creates no
//			device objects: pipelines and groups belong to the families,
//			MaterialPrograms and DrawGroups, which must outlive the graph's
//			execution.
//
//=============================================================================//

#ifndef RENDER_PASS_OPAQUE_OPAQUE_H
#define RENDER_PASS_OPAQUE_OPAQUE_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"
#include "render/material/draw_program.h"
#include "render/resources/mesh_cache.h"
#include "render/scene/draw_list.h"

#include <cstdint>

namespace render::pass::opaque
{

class IMeshResolver
{
public:
	virtual ~IMeshResolver() = default;
	// nullptr when the id names no resident mesh.
	virtual const resources::MeshEntry *Mesh( std::uint64_t mesh ) const = 0;
};

struct OpaqueTargets
{
	graph::ResourceRef color; // written as kColorAttachment
	graph::ResourceRef depth; // written as kDepthWrite
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::ClearColor clear;
	bool clearColor = true; // false: load the target's contents
	bool clearDepth = true;
};

struct OpaqueStats
{
	std::uint32_t drawn = 0;
	std::uint32_t unresolved = 0;
};

enum class OpaqueStatus : std::uint8_t
{
	kInvalidTargets = 1
};

struct OpaqueSources
{
	const IMeshResolver &meshes;
	const material::IDrawPrograms &programs;
	const material::IDrawGroups *drawGroups = nullptr; // for families that read one
	// The frame's and the view's groups (roles kFrame, kView), for families
	// that read them (a lookup table, a view's lighting).
	const material::DrawGroup *frame = nullptr;
	const material::DrawGroup *view = nullptr;
};

// Adds the draw pass. The snapshot, list and sources are read here; the
// meshes' buffers and the groups' objects must outlive the graph's
// execution.
foundation::Expected<OpaqueStats, OpaqueStatus> AddOpaquePasses( graph::GraphBuilder &builder,
    const scene::SceneSnapshot &snapshot, const scene::DrawList &list, const scene::SceneView &view,
    const OpaqueSources &sources, const OpaqueTargets &targets );

} // namespace render::pass::opaque

#endif // RENDER_PASS_OPAQUE_OPAQUE_H
