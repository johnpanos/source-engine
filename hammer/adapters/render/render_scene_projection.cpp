//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hammer's own render scene; see render_scene_projection.h.
//
//=============================================================================//

#include "hammer/adapters/render/render_scene_projection.h"

#include <algorithm>

namespace hammer::render_adapter
{

render::math::float4x4 PlaceUnitBox( const render::math::Aabb &bounds )
{
	const render::math::float3 half = bounds.Extents();
	// A flat solid still gets a box that contains it.
	const render::math::float3 scale = {
	    std::max( half.x, 1e-3f ), std::max( half.y, 1e-3f ), std::max( half.z, 1e-3f ) };
	return render::math::Multiply(
	    render::math::Translation( bounds.Center() ), render::math::Scale( scale ) );
}

SceneProjection::SceneProjection( render::scene::SceneFactory factory )
    : m_Scene( factory.create ? factory.create() : render::scene::CreateRenderScene() )
{
}

foundation::Expected<SyncResult, render::scene::SceneError> SceneProjection::Sync(
    const viewport::RenderSnapshot &snapshot )
{
	std::map<std::uint64_t, render::math::Aabb> current;
	for ( const viewport::SolidDraw &solid : snapshot.solids )
	{
		const scene::Box &box = solid.bounds;
		current[solid.id.value] = {
		    { static_cast<float>( box.mins.x ), static_cast<float>( box.mins.y ),
		        static_cast<float>( box.mins.z ) },
		    { static_cast<float>( box.maxs.x ), static_cast<float>( box.maxs.y ),
		        static_cast<float>( box.maxs.z ) } };
	}

	SyncResult result;
	render::scene::ChangeSet changes;
	std::map<std::uint64_t, Tracked> next;
	for ( const auto &[key, bounds] : current )
	{
		const auto found = m_Solids.find( key );
		if ( found == m_Solids.end() )
		{
			render::scene::MeshInstanceDesc desc;
			desc.world = PlaceUnitBox( bounds );
			desc.localBounds = { { -1, -1, -1 }, { 1, 1, 1 } };
			const render::scene::InstanceId instance = m_Scene->Reserve();
			changes.Add( instance, desc );
			next[key] = { instance, bounds };
			++result.added;
			continue;
		}
		if ( !( found->second.bounds == bounds ) )
		{
			changes.UpdateTransform( found->second.instance, PlaceUnitBox( bounds ) );
			++result.updated;
		}
		next[key] = { found->second.instance, bounds };
	}
	for ( const auto &[key, tracked] : m_Solids )
	{
		if ( !current.count( key ) )
		{
			changes.Remove( tracked.instance );
			++result.removed;
		}
	}
	result.revision = m_Scene->Revision();
	if ( changes.Empty() )
		return result;
	auto committed = m_Scene->Commit( changes );
	if ( !committed )
		return foundation::MakeUnexpected( committed.Error() );
	m_Solids = std::move( next );
	result.committed = true;
	result.revision = committed.Value();
	return result;
}

} // namespace hammer::render_adapter
