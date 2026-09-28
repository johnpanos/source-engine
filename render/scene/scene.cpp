//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 (RFC 0016): the scene, snapshots and draw lists.
//
//			This first slice copies the instance table into each snapshot
//			(O(instances) per commit); RFC 0016 K5 replaces it with persistent
//			GPU-resident storage and measures submission cost.
//
//=============================================================================//

#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "render/scene/draw_list.h"
#include "render/scene/scene.h"

#include <algorithm>
#include <atomic>
#include <map>
#include <mutex>
#include <set>

namespace render::scene
{

namespace
{

class RenderScene final : public IRenderScene
{
public:
	RenderScene() : m_Snapshot( std::make_shared<const SceneSnapshot>() ) {}

	InstanceId Reserve() override { return InstanceId{ ++m_NextId }; }

	foundation::Expected<std::uint64_t, SceneError> Commit( const ChangeSet &changes ) override
	{
		// Validate against the table as the changes would leave it.
		std::set<std::uint64_t> live;
		for ( const auto &[id, instance] : m_Instances )
			live.insert( id );
		const auto &list = changes.Changes();
		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			const ChangeSet::Change &change = list[i];
			const std::uint32_t index = static_cast<std::uint32_t>( i );
			if ( !change.id.IsValid() || change.id.value > m_NextId )
				return foundation::MakeUnexpected( SceneError{ SceneStatus::kInvalidId, index } );
			const bool present = live.count( change.id.value ) != 0;
			if ( change.op == ChangeSet::Op::kAdd )
			{
				if ( present )
					return foundation::MakeUnexpected(
					    SceneError{ SceneStatus::kDuplicateInstance, index } );
				live.insert( change.id.value );
			}
			else
			{
				if ( !present )
					return foundation::MakeUnexpected(
					    SceneError{ SceneStatus::kUnknownInstance, index } );
				if ( change.op == ChangeSet::Op::kRemove )
					live.erase( change.id.value );
			}
		}
		for ( const ChangeSet::Change &change : list )
		{
			switch ( change.op )
			{
			case ChangeSet::Op::kAdd:
				m_Instances[change.id.value] = { change.id, change.desc,
				    math::TransformBounds( change.desc.world, change.desc.localBounds ) };
				break;
			case ChangeSet::Op::kUpdateTransform:
			{
				MeshInstance &instance = m_Instances[change.id.value];
				instance.desc.world = change.desc.world;
				instance.worldBounds =
				    math::TransformBounds( instance.desc.world, instance.desc.localBounds );
				break;
			}
			case ChangeSet::Op::kRemove:
				m_Instances.erase( change.id.value );
				break;
			}
		}
		const std::uint64_t revision = m_Revision.load( std::memory_order_relaxed ) + 1;
		auto snapshot = std::make_shared<SceneSnapshot>();
		snapshot->revision = revision;
		snapshot->instances.reserve( m_Instances.size() );
		for ( const auto &[id, instance] : m_Instances )
			snapshot->instances.push_back( instance );
		// Publication: the snapshot is complete before any reader can take
		// it. The lock's release (unlock) and acquire (lock in Snapshot) are
		// the happens-before edge RFC 0006 asks for.
#ifdef RENDER_SCENE_SEEDED_UNSYNCHRONIZED_PUBLICATION
		// Seeded defect for render.scene.publication.tsan.sensitivity only:
		// publication without the edge.
		m_Snapshot = std::move( snapshot );
#else
		{
			std::lock_guard<std::mutex> lock( m_Publish );
			m_Snapshot = std::move( snapshot );
		}
#endif
		m_Revision.store( revision, std::memory_order_release );
		return revision;
	}

	std::shared_ptr<const SceneSnapshot> Snapshot() const override
	{
		std::lock_guard<std::mutex> lock( m_Publish );
		return m_Snapshot;
	}

	std::uint64_t Revision() const override { return m_Revision.load( std::memory_order_acquire ); }

private:
	std::map<std::uint64_t, MeshInstance> m_Instances;
	std::uint64_t m_NextId = 0;
	std::atomic<std::uint64_t> m_Revision{ 0 };
	mutable std::mutex m_Publish;
	std::shared_ptr<const SceneSnapshot> m_Snapshot;
};

} // namespace

std::unique_ptr<IRenderScene> CreateRenderScene()
{
	return std::make_unique<RenderScene>();
}

SceneView MakeView( const ViewDesc &desc )
{
	SceneView view;
	view.desc = desc;
	view.viewProjection = math::Multiply( desc.projection, desc.view );
	view.frustum = math::ExtractFrustum( view.viewProjection );
	return view;
}

namespace
{

// Frustum and view-mask test of instances [begin, end), appending the kept
// indices in order.
std::uint32_t CullRange( const SceneSnapshot &snapshot, const SceneView &view, std::size_t begin,
    std::size_t end, std::vector<std::uint32_t> &candidates )
{
	std::uint32_t culled = 0;
	for ( std::size_t i = begin; i < end; ++i )
	{
		const MeshInstance &instance = snapshot.instances[i];
		const bool inView = view.desc.viewBit >= 32 ||
		                    ( instance.desc.viewMask & ( 1u << view.desc.viewBit ) ) != 0;
		if ( inView && math::Intersects( view.frustum, instance.worldBounds ) )
			candidates.push_back( static_cast<std::uint32_t>( i ) );
		else
			++culled;
	}
	return culled;
}

// The provider pass, the items and their order, from the frustum candidates.
DrawList Finish( const SceneSnapshot &snapshot, const SceneView &view,
    IVisibilityProvider *provider, std::vector<std::uint32_t> candidates, std::uint32_t culled )
{
	DrawList list;
	list.revision = snapshot.revision;
	list.frustumCulled = culled;
	if ( provider )
	{
		const std::vector<std::uint32_t> before = candidates;
		provider->Filter( snapshot, view, candidates );
		// A provider may only remove candidates.
		std::erase_if( candidates,
		    [&]( std::uint32_t index )
		    {
			    return !std::binary_search( before.begin(), before.end(), index );
		    } );
		std::sort( candidates.begin(), candidates.end() );
		candidates.erase( std::unique( candidates.begin(), candidates.end() ), candidates.end() );
		list.providerCulled = static_cast<std::uint32_t>( before.size() - candidates.size() );
	}
	for ( std::uint32_t index : candidates )
	{
		const MeshInstance &instance = snapshot.instances[index];
		const math::float3 center =
		    math::TransformPoint( view.desc.view, instance.worldBounds.Center() );
		list.items.push_back( { index, instance.desc.material, instance.desc.mesh, -center.z } );
	}
	std::stable_sort( list.items.begin(), list.items.end(),
	    []( const DrawItem &a, const DrawItem &b )
	    {
		    if ( a.material != b.material )
			    return a.material < b.material;
		    if ( a.mesh != b.mesh )
			    return a.mesh < b.mesh;
		    return a.depth < b.depth;
	    } );
	return list;
}

} // namespace

DrawList BuildDrawList(
    const SceneSnapshot &snapshot, const SceneView &view, IVisibilityProvider *provider )
{
	std::vector<std::uint32_t> candidates;
	candidates.reserve( snapshot.instances.size() );
	const std::uint32_t culled =
	    CullRange( snapshot, view, 0, snapshot.instances.size(), candidates );
	return Finish( snapshot, view, provider, std::move( candidates ), culled );
}

foundation::Expected<DrawList, CullStatus> BuildDrawListPooled( const SceneSnapshot &snapshot,
    const SceneView &view, jobsystem::IGraphExecutor &jobs, IVisibilityProvider *provider,
    std::uint32_t chunk )
{
	chunk = std::max( chunk, 1u );
	const std::size_t count = snapshot.instances.size();
	const std::size_t chunks = ( count + chunk - 1 ) / chunk;
	std::vector<std::vector<std::uint32_t>> kept( chunks );
	std::vector<std::uint32_t> culled( chunks, 0 );
	jobsystem::JobGraphBuilder builder;
	for ( std::size_t c = 0; c < chunks; ++c )
	{
		jobsystem::JobDesc job;
		job.name = "render-scene-cull";
		job.function = [&, c]( jobsystem::JobRunContext & )
		{
			const std::size_t begin = c * chunk;
			culled[c] =
			    CullRange( snapshot, view, begin, std::min( count, begin + chunk ), kept[c] );
		};
		builder.AddJob( job );
	}
	if ( chunks > 0 )
	{
		auto sealed = builder.Seal();
		if ( !sealed || !jobs.Execute( sealed.Value(), jobsystem::RunOptions() ).AllSucceeded() )
			return foundation::MakeUnexpected( CullStatus::kJobs );
	}
	// Chunks merge in index order, so the candidates equal the serial pass's.
	std::vector<std::uint32_t> candidates;
	candidates.reserve( count );
	std::uint32_t culledTotal = 0;
	for ( std::size_t c = 0; c < chunks; ++c )
	{
		candidates.insert( candidates.end(), kept[c].begin(), kept[c].end() );
		culledTotal += culled[c];
	}
	return Finish( snapshot, view, provider, std::move( candidates ), culledTotal );
}

} // namespace render::scene
