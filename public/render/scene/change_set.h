//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 change sets (RFC 0016). The scene's owner gathers
//			changes into a change set and commits it; a commit applies all of
//			it or none of it.
//
//=============================================================================//

#ifndef RENDER_SCENE_CHANGE_SET_H
#define RENDER_SCENE_CHANGE_SET_H

#include "render/scene/objects.h"

#include <vector>

namespace render::scene
{

class ChangeSet
{
public:
	enum class Op : std::uint8_t
	{
		kAdd,
		kUpdateTransform,
		kRemove
	};

	struct Change
	{
		Op op = Op::kAdd;
		InstanceId id;
		MeshInstanceDesc desc;
	};

	// id comes from IRenderScene::Reserve.
	void Add( InstanceId id, const MeshInstanceDesc &desc )
	{
		m_Changes.push_back( { Op::kAdd, id, desc } );
	}
	void UpdateTransform( InstanceId id, const math::float4x4 &world )
	{
		Change change{ Op::kUpdateTransform, id, {} };
		change.desc.world = world;
		m_Changes.push_back( change );
	}
	void Remove( InstanceId id ) { m_Changes.push_back( { Op::kRemove, id, {} } ); }

	const std::vector<Change> &Changes() const { return m_Changes; }

private:
	std::vector<Change> m_Changes;
};

} // namespace render::scene

#endif // RENDER_SCENE_CHANGE_SET_H
