//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 views (RFC 0016).
//
//=============================================================================//

#ifndef RENDER_SCENE_VIEW_H
#define RENDER_SCENE_VIEW_H

#include "render/math/frustum.h"
#include "render/math/matrix.h"

#include <cstdint>
#include <optional>

namespace render::scene
{

struct ViewDesc
{
	math::float4x4 view;
	math::float4x4 projection;
	std::uint32_t viewBit = 0; // matched against MeshInstanceDesc::viewMask
	// The view's culling planes when its owner has them (the legacy frontend
	// passes the engine's view frustum, so the core culls with the same
	// planes); otherwise MakeView extracts them from projection * view.
	std::optional<math::Frustum> frustum;
};

struct SceneView
{
	ViewDesc desc;
	math::float4x4 viewProjection;
	math::Frustum frustum;
};

SceneView MakeView( const ViewDesc &desc );

} // namespace render::scene

#endif // RENDER_SCENE_VIEW_H
