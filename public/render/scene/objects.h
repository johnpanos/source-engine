//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 objects (RFC 0016). Engine-facing: no std::string
//			or other dual-ABI library type appears here, because the engine
//			builds with the legacy libstdc++ ABI and the core does not.
//
//=============================================================================//

#ifndef RENDER_SCENE_OBJECTS_H
#define RENDER_SCENE_OBJECTS_H

#include "foundation/strong_id.h"
#include "render/math/bounds.h"
#include "render/math/matrix.h"

#include <cstdint>

namespace render::scene
{

using InstanceId = foundation::StrongId<struct InstanceTag, std::uint64_t>;

struct MeshInstanceDesc
{
	std::uint64_t mesh = 0;     // a render.resources mesh the owner resolved
	std::uint64_t material = 0; // a render.material MaterialId value
	// A render.material draw group (per-draw resources such as a lightmap
	// page) for families that read one; 0 for none.
	std::uint64_t drawGroup = 0;
	math::float4x4 world;
	math::Aabb localBounds;
	std::uint32_t viewMask = ~0u; // views (by bit) the instance may appear in
};

struct MeshInstance
{
	InstanceId id;
	MeshInstanceDesc desc;
	math::Aabb worldBounds;
};

} // namespace render::scene

#endif // RENDER_SCENE_OBJECTS_H
