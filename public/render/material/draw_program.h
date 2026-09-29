//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What a pass needs to draw a material through its family (RFC
//			0016 K4, render.material): the family's pipeline for the
//			material, its material bind group (role kMaterial), the vertex
//			stride the pipeline reads, and the resources the group names, so
//			the pass can declare every access to the graph. Passes resolve a
//			material id through IDrawPrograms; MaterialPrograms
//			(material_programs.h) is the owner that builds them.
//
//			Some families also read a per-draw group (role kDraw): resources
//			that differ between draws of one material, such as the lightmap
//			page a surface sits on. Such a program names the layout it reads
//			(drawLayout); a scene instance names its group
//			(MeshInstanceDesc::drawGroup), resolved through IDrawGroups
//			(DrawGroups builds them), and the pass binds it when the layouts
//			match. Frame and view groups (roles kFrame, kView: a lookup
//			table, a view's lighting) come from the pass's owner in the same
//			shape and are checked against frameLayout and viewLayout.
//
//			Draw constants (D16) share one prefix in every family: the draw's
//			object-to-clip matrix (the view-projection times the instance's
//			world matrix), then its world matrix, both row-major with
//			column vectors as render.math stores them. A family reads the
//			first drawConstantBytes of FamilyDrawConstants (unlit reads only
//			toClip), and the pass writes exactly that many.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_DRAW_PROGRAM_H
#define RENDER_MATERIAL_DRAW_PROGRAM_H

#include "render/device/device.h"

#include <cstdint>
#include <vector>

namespace render::material
{

struct FamilyDrawConstants
{
	float toClip[16] = {};
	float world[16] = {};
};
static_assert( sizeof( FamilyDrawConstants ) == device::kMaxDrawConstantBytes );

struct SampledTexture
{
	device::TextureId texture;
	device::TextureDesc desc;
};

// A bind group and the resources it names, each resident in its usage.
struct ResidentGroup
{
	device::BindGroupId group;
	std::vector<SampledTexture> textures;   // in kSampled
	std::vector<device::BufferId> uniforms; // in kUniform
	std::vector<device::BufferId> storage;  // in kStorageRead
};

struct DrawProgram
{
	device::PipelineId pipeline;
	ResidentGroup material;               // role kMaterial
	std::uint32_t vertexStride = 0;       // the pipeline's vertex buffer 0
	std::uint32_t drawConstantBytes = 0;  // a prefix of FamilyDrawConstants
	device::BindGroupLayoutId drawLayout; // role kDraw; invalid when the family reads none
	// The frame and view groups the family reads (roles kFrame, kView), which
	// the pass's owner supplies per frame and view; invalid when it reads none.
	device::BindGroupLayoutId frameLayout;
	device::BindGroupLayoutId viewLayout;
	// The program's neutral view group (role kView, a view with nothing in
	// it), which a pass binds when the frame supplies no view group of
	// viewLayout; no group when the program declares none.
	ResidentGroup neutralView;
	bool hasNeutralView = false;
};

struct DrawGroup
{
	device::BindGroupLayoutId layout;
	ResidentGroup resident; // role kDraw
};

class IDrawPrograms
{
public:
	virtual ~IDrawPrograms() = default;
	// nullptr when the material has no program (unknown, or not resident yet).
	virtual const DrawProgram *Program( std::uint64_t material ) const = 0;
};

class IDrawGroups
{
public:
	virtual ~IDrawGroups() = default;
	// nullptr when the id names no group (unknown, or not resident yet).
	virtual const DrawGroup *Group( std::uint64_t drawGroup ) const = 0;
};

} // namespace render::material

#endif // RENDER_MATERIAL_DRAW_PROGRAM_H
