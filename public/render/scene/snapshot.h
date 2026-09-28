//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 snapshots (RFC 0016): the scene as of one commit,
//			immutable. A frame renders one snapshot while the next frame's
//			changes commit.
//
//=============================================================================//

#ifndef RENDER_SCENE_SNAPSHOT_H
#define RENDER_SCENE_SNAPSHOT_H

#include "render/scene/objects.h"

#include <cstdint>
#include <vector>

namespace render::scene
{

struct SceneSnapshot
{
	std::uint64_t revision = 0;
	std::vector<MeshInstance> instances; // ordered by id
};

} // namespace render::scene

#endif // RENDER_SCENE_SNAPSHOT_H
