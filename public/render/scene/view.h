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

namespace render::scene
{

struct ViewDesc
{
	math::float4x4 view;
	math::float4x4 projection;
	std::uint32_t viewBit = 0; // matched against MeshInstanceDesc::viewMask
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
