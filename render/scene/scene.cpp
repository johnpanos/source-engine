//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 (RFC 0016): the scene, snapshots and draw lists.
//
//			The instance table is persistent (public/render/scene/snapshot.h):
//			a commit copies the chunks its changes touch and shares the rest
//			with the previous snapshot, so its cost follows the change set,
//			not the instance count. Validation reads the table in place and
//			keeps only the ids the change set itself touches.
//
//=============================================================================//

#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "render/scene/draw_list.h"
#include "render/scene/scene.h"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <mutex>
#include <numeric>
#include <tuple>
#include <unordered_map>

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
		// Validate against the scene as the changes would leave it: the ids
		// the change set touches overlay the slot map.
		std::unordered_map<std::uint64_t, bool> live;
		const auto &list = changes.Changes();
		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			const std::uint64_t id = list[i].id.value;
			const bool add = list[i].op == ChangeSet::Op::kAdd;
			const bool invalid = !list[i].id.IsValid() || id > m_NextId;
			const bool present =
			    live.count( id ) ? live[id] : !invalid && m_Slot.size() > id && m_Slot[id];
			if ( invalid || present == add )
				return foundation::MakeUnexpected(
				    SceneError{ invalid ? SceneStatus::kInvalidId
				                : add   ? SceneStatus::kDuplicateInstance
				                        : SceneStatus::kUnknownInstance,
				        std::uint32_t( i ) } );
			live[id] = list[i].op != ChangeSet::Op::kRemove;
		}
		m_Slot.resize( m_NextId + 1, 0 );
		for ( const ChangeSet::Change &change : list )
		{
			std::uint32_t &slot = m_Slot[change.id.value];
			if ( change.op == ChangeSet::Op::kAdd )
			{
				slot = m_Free.empty() ? std::uint32_t( m_Table.size() ) : m_Free.back();
				if ( !m_Free.empty() )
					m_Free.pop_back();
				else
					m_Table.resize( m_Table.size() + 1 );
				m_Table[slot] = { change.id, change.desc,
				    math::TransformBounds( change.desc.world, change.desc.localBounds ) };
				++slot; // stored +1: 0 is no slot
				continue;
			}
			MeshInstance &instance = m_Table[slot - 1];
			if ( change.op == ChangeSet::Op::kUpdateTransform )
			{
				instance.desc.world = change.desc.world;
				instance.worldBounds =
				    math::TransformBounds( instance.desc.world, instance.desc.localBounds );
				continue;
			}
			m_Free.push_back( slot - 1 );
			instance = MeshInstance{};
			slot = 0;
		}
		const std::uint64_t revision = m_Revision.load( std::memory_order_relaxed ) + 1;
		auto snapshot = std::make_shared<SceneSnapshot>();
		snapshot->revision = revision;
		snapshot->instances = m_Table; // block pointers: the unchanged blocks are shared
		// Publication: the snapshot is complete before any reader can take
		// it. The lock's release (unlock) and acquire (lock in Snapshot) are
		// the happens-before edge RFC 0006 asks for.
		{
			// Seeded defect for render.scene.publication.tsan.sensitivity only:
			// publication without the edge.
#ifndef RENDER_SCENE_SEEDED_UNSYNCHRONIZED_PUBLICATION
			std::lock_guard<std::mutex> lock( m_Publish );
#endif
			m_Snapshot = std::move( snapshot );
		}
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
	InstanceTable m_Table;             // the owner's working table; snapshots share its blocks
	std::vector<std::uint32_t> m_Slot; // id -> slot + 1; 0 for none
	std::vector<std::uint32_t> m_Free;
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
	view.frustum = desc.frustum ? *desc.frustum : math::ExtractFrustum( view.viewProjection );
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
		const bool kept = ( view.desc.viewBit >= 32 ||
		                      ( instance.desc.viewMask & ( 1u << view.desc.viewBit ) ) != 0 ) &&
		                  math::Intersects( view.frustum, instance.worldBounds );
		if ( kept )
			candidates.push_back( static_cast<std::uint32_t>( i ) );
		culled += !kept;
	}
	return culled;
}

// The provider pass, the items and their order, from the frustum candidates.
DrawList Finish( const SceneSnapshot &snapshot, const SceneView &view,
    IVisibilityProvider *provider, std::vector<std::uint32_t> candidates, std::uint32_t culled )
{
	DrawList list{ snapshot.revision, {}, culled, 0 };
	if ( provider )
	{
		const std::vector<std::uint32_t> before = candidates;
		provider->Filter( snapshot, view, candidates );
		// A provider may only remove candidates: what it leaves is the part
		// of the original set it still names (each once).
		std::sort( candidates.begin(), candidates.end() );
		std::vector<std::uint32_t> kept;
		std::set_intersection( before.begin(), before.end(), candidates.begin(), candidates.end(),
		    std::back_inserter( kept ) );
		candidates = std::move( kept );
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
		    return std::tie( a.material, a.mesh, a.depth ) <
		           std::tie( b.material, b.mesh, b.depth );
	    } );
	return list;
}

} // namespace

DrawList BuildDrawList(
    const SceneSnapshot &snapshot, const SceneView &view, IVisibilityProvider *provider )
{
	std::vector<std::uint32_t> candidates;
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
	for ( const auto &part : kept )
		candidates.insert( candidates.end(), part.begin(), part.end() );
	return Finish( snapshot, view, provider, std::move( candidates ),
	    std::accumulate( culled.begin(), culled.end(), 0u ) );
}

} // namespace render::scene
