//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hammer's own render scene (RFC 0016 A.7; RFC 0002
//			hammer.adapters.render). The editor composes a render::scene of
//			its own, independent of any game's, and keeps it in step with the
//			document: each Sync diffs the render snapshot's solids
//			(presenters::EditorWorkspace's viewport::RenderSnapshot) against the
//			last one and commits a single change set (add, move or resize,
//			remove), or nothing when nothing changed. The viewports draw
//			through ViewportRenderer today; this scene is where the solids
//			move once the core's material families (RFC 0016 K4) draw them.
//
//			A solid is identified by its document ObjectId, which never
//			changes or is reused. Each instance is a unit box placed and
//			scaled onto the solid's bounds, so a move or resize is a transform
//			update, not a new instance.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_SCENE_PROJECTION_H
#define HAMMER_ADAPTERS_RENDER_SCENE_PROJECTION_H

#include "foundation/expected.h"
#include "hammer/viewport/extraction.h"
#include "render/math/bounds.h"
#include "render/scene/scene.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>

namespace hammer::render_adapter
{

struct SyncResult
{
	std::size_t added = 0;
	std::size_t updated = 0;
	std::size_t removed = 0;
	bool committed = false;
	std::uint64_t revision = 0;
};

class SceneProjection
{
public:
	// factory: how the editor's composition root makes scenes.
	explicit SceneProjection( render::scene::SceneFactory factory );

	foundation::Expected<SyncResult, render::scene::SceneError> Sync(
	    const viewport::RenderSnapshot &snapshot );

	const render::scene::IRenderScene &Scene() const { return *m_Scene; }
	std::size_t SolidCount() const { return m_Solids.size(); }

private:
	struct Tracked
	{
		render::scene::InstanceId instance;
		render::math::Aabb bounds;
	};

	std::unique_ptr<render::scene::IRenderScene> m_Scene;
	std::map<std::uint64_t, Tracked> m_Solids; // by ObjectId value
};

// The transform that places the unit box [-1, 1]^3 onto bounds.
render::math::float4x4 PlaceUnitBox( const render::math::Aabb &bounds );

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_SCENE_PROJECTION_H
