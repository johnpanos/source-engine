//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.visibility.v1 (RFC 0016), a port of render.scene. After
//			frustum culling, a provider may remove instances it proves hidden
//			(BSP leaves and area portals, WMSH clusters, occlusion). It may only
//			remove: an instance it keeps must have been a candidate.
//
//=============================================================================//

#ifndef RENDER_SCENE_VISIBILITY_H
#define RENDER_SCENE_VISIBILITY_H

#include "render/scene/snapshot.h"
#include "render/scene/view.h"

#include <cstdint>
#include <vector>

namespace render::scene
{

class IVisibilityProvider
{
public:
	virtual ~IVisibilityProvider() = default;
	// candidates holds indices into snapshot.instances; remove the hidden ones.
	virtual void Filter( const SceneSnapshot &snapshot, const SceneView &view,
	    std::vector<std::uint32_t> &candidates ) = 0;
};

} // namespace render::scene

#endif // RENDER_SCENE_VISIBILITY_H
