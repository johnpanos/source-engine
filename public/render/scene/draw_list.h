//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 draw lists (RFC 0016): the visible instances of
//			one view, culled against the frustum (conservatively: an instance
//			whose bounds reach the frustum is never dropped), filtered by an
//			optional visibility provider, and sorted by material, mesh, then
//			front to back.
//
//=============================================================================//

#ifndef RENDER_SCENE_DRAW_LIST_H
#define RENDER_SCENE_DRAW_LIST_H

#include "foundation/expected.h"
#include "render/scene/snapshot.h"
#include "render/scene/view.h"
#include "render/scene/visibility.h"

#include <cstdint>
#include <vector>

namespace jobsystem
{
class IGraphExecutor;
}

namespace render::scene
{

struct DrawItem
{
	std::uint32_t instance = 0; // index into the snapshot
	std::uint64_t material = 0;
	std::uint64_t mesh = 0;
	float depth = 0.0f; // view-space distance to the bounds center
};

struct DrawList
{
	std::uint64_t revision = 0; // the snapshot's
	std::vector<DrawItem> items;
	std::uint32_t frustumCulled = 0;
	std::uint32_t providerCulled = 0;
};

DrawList BuildDrawList(
    const SceneSnapshot &snapshot, const SceneView &view, IVisibilityProvider *provider = nullptr );

enum class CullStatus : std::uint8_t
{
	kJobs = 1 // the job graph did not run to completion
};

// The same list, with the frustum pass split into `chunk`-instance jobs on
// `jobs` (a jobs.graph executor the caller owns). Chunks merge in index
// order and the provider pass and sort run on the caller, so the result
// equals BuildDrawList's for every input; the serial function is the oracle.
foundation::Expected<DrawList, CullStatus> BuildDrawListPooled( const SceneSnapshot &snapshot,
    const SceneView &view, jobsystem::IGraphExecutor &jobs, IVisibilityProvider *provider = nullptr,
    std::uint32_t chunk = 256 );

} // namespace render::scene

#endif // RENDER_SCENE_DRAW_LIST_H
