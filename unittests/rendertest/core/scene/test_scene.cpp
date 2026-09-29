//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 (RFC 0016, first slice):
//
//			C1 a commit applies all of its change set or none of it;
//			C2 a snapshot never changes after publication;
//			C3 on seeded random scenes, culling never drops an instance that a
//			   corner test proves visible, and drops every instance that lies
//			   wholly outside one clip plane (the corner oracle works in clip
//			   space, independently of the plane extraction);
//			C4 a visibility provider can only remove candidates;
//			C5 two scenes are independent;
//			C6 pooled culling (BuildDrawListPooled on a 4-worker jobs.graph
//			   ParallelExecutor) gives the serial draw list exactly (items,
//			   depths and counts) on 1,000 seeded scenes, views, chunk sizes
//			   and with and without a provider.
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"
#include "render/scene/draw_list.h"
#include "render/scene/scene.h"
#include "testing/checks.h"

#include <cmath>
#include <cstdio>
#include <random>

namespace
{

using namespace render;
using namespace render::scene;

MeshInstanceDesc Box( math::float3 center, float half, std::uint64_t material = 1 )
{
	MeshInstanceDesc desc;
	desc.world = math::Translation( center );
	desc.localBounds = { { -half, -half, -half }, { half, half, half } };
	desc.material = material;
	desc.mesh = 1;
	return desc;
}

class DropOdd final : public IVisibilityProvider
{
public:
	void Filter(
	    const SceneSnapshot &, const SceneView &, std::vector<std::uint32_t> &candidates ) override
	{
		std::erase_if( candidates,
		    []( std::uint32_t index )
		    {
			    return index % 2 == 1;
		    } );
		candidates.push_back( 100000 ); // not a candidate: must be ignored
	}
};

// 0: some corner inside the clip volume; 1: all corners outside one plane;
// 2: undecided by corners.
int CornerVerdict( const math::float4x4 &viewProjection, const math::Aabb &box )
{
	bool anyInside = false;
	int outside[6] = {};
	for ( int corner = 0; corner < 8; ++corner )
	{
		const math::float4 p = math::Transform( viewProjection,
		    { ( corner & 1 ) ? box.max.x : box.min.x, ( corner & 2 ) ? box.max.y : box.min.y,
		        ( corner & 4 ) ? box.max.z : box.min.z, 1.0f } );
		const bool in[6] = {
		    p.x >= -p.w, p.x <= p.w, p.y >= -p.w, p.y <= p.w, p.z >= 0.0f, p.z <= p.w };
		bool all = true;
		for ( int plane = 0; plane < 6; ++plane )
		{
			all &= in[plane];
			outside[plane] += in[plane] ? 0 : 1;
		}
		anyInside |= all;
	}
	if ( anyInside )
		return 0;
	for ( int plane = 0; plane < 6; ++plane )
	{
		if ( outside[plane] == 8 )
			return 1;
	}
	return 2;
}

} // namespace

int main()
{
	testing::Checks checks;
	auto scene = CreateRenderScene();
	const InstanceId a = scene->Reserve();
	const InstanceId b = scene->Reserve();
	ChangeSet first;
	first.Add( a, Box( { 0, 0, -10 }, 1 ) );
	first.Add( b, Box( { 0, 0, -20 }, 1 ) );
	checks.That(
	    scene->Commit( first ).HasValue() && scene->Revision() == 1, "C1.a-commit-applies" );
	const std::shared_ptr<const SceneSnapshot> before = scene->Snapshot();

	ChangeSet bad;
	bad.Remove( a );
	bad.UpdateTransform( InstanceId{ 999 }, math::float4x4() );
	auto rejected = scene->Commit( bad );
	checks.That( !rejected && rejected.Error().status == SceneStatus::kInvalidId &&
	                 rejected.Error().change == 1,
	    "C1.an-invalid-change-fails-naming-it" );
	checks.That( scene->Revision() == 1 && scene->Snapshot()->instances.size() == 2,
	    "C1.a-failed-commit-applies-nothing" );
	ChangeSet twice;
	twice.Add( a, Box( {}, 1 ) );
	checks.That( !scene->Commit( twice ), "C1.adding-a-live-id-fails" );

	ChangeSet move;
	move.UpdateTransform( a, math::Translation( { 5, 0, -10 } ) );
	move.Remove( b );
	checks.That( scene->Commit( move ).HasValue(), "C2.a-second-commit-applies" );
	checks.That( before->revision == 1 && before->instances.size() == 2 &&
	                 before->instances[0].worldBounds.Center() == math::float3{ 0, 0, -10 },
	    "C2.an-earlier-snapshot-is-unchanged" );
	checks.That(
	    scene->Snapshot()->instances.size() == 1 &&
	        scene->Snapshot()->instances[0].worldBounds.Center() == math::float3{ 5, 0, -10 },
	    "C2.the-new-snapshot-has-the-changes" );

	// C3: random scenes against the corner oracle.
	std::mt19937 random( 1234 );
	std::uniform_real_distribution<float> coordinate( -60.0f, 60.0f );
	std::uniform_real_distribution<float> size( 0.1f, 6.0f );
	int missed = 0;
	int kept = 0;
	int decided = 0;
	for ( int trial = 0; trial < 50; ++trial )
	{
		auto random_scene = CreateRenderScene();
		ChangeSet changes;
		for ( int i = 0; i < 200; ++i )
			changes.Add( random_scene->Reserve(),
			    Box( { coordinate( random ), coordinate( random ), coordinate( random ) },
			        size( random ) ) );
		(void)random_scene->Commit( changes );
		ViewDesc desc;
		desc.view =
		    math::LookAt( { coordinate( random ), coordinate( random ), coordinate( random ) },
		        { 0, 0, 0 }, { 0, 0, 1 } );
		desc.projection = math::Perspective( 1.2f, 16.0f / 9.0f, 1.0f, 80.0f );
		const SceneView view = MakeView( desc );
		const auto snapshot = random_scene->Snapshot();
		const DrawList list = BuildDrawList( *snapshot, view );
		std::vector<bool> drawn( snapshot->instances.size(), false );
		for ( const DrawItem &item : list.items )
			drawn[item.instance] = true;
		for ( std::size_t i = 0; i < snapshot->instances.size(); ++i )
		{
			const int verdict =
			    CornerVerdict( view.viewProjection, snapshot->instances[i].worldBounds );
			decided += verdict != 2;
			missed += verdict == 0 && !drawn[i];
			kept += verdict == 1 && drawn[i];
		}
	}
	checks.Equal( missed, 0, "C3.no-provably-visible-instance-is-culled" );
	checks.Equal( kept, 0, "C3.every-instance-outside-one-plane-is-culled" );
	checks.That( decided > 5000, "C3.the-oracle-decides-most-instances" );

	auto sorted = CreateRenderScene();
	ChangeSet many;
	for ( int i = 0; i < 6; ++i )
		many.Add( sorted->Reserve(), Box( { 0, 0, -5.0f - i }, 0.5f, 6 - i % 3 ) );
	(void)sorted->Commit( many );
	ViewDesc front;
	front.projection = math::Perspective( 1.2f, 1.0f, 1.0f, 100.0f );
	const SceneView frontView = MakeView( front );
	const DrawList list = BuildDrawList( *sorted->Snapshot(), frontView );
	bool ordered = list.items.size() == 6;
	for ( std::size_t i = 1; ordered && i < list.items.size(); ++i )
	{
		const DrawItem &p = list.items[i - 1];
		const DrawItem &q = list.items[i];
		ordered = p.material < q.material || ( p.material == q.material && p.depth <= q.depth );
	}
	checks.That( ordered, "C3.draws-sort-by-material-then-front-to-back" );

	DropOdd provider;
	const DrawList filtered = BuildDrawList( *sorted->Snapshot(), frontView, &provider );
	bool onlyEven = filtered.items.size() == 3 && filtered.providerCulled == 3;
	for ( const DrawItem &item : filtered.items )
		onlyEven &= item.instance % 2 == 0;
	checks.That( onlyEven, "C4.a-provider-removes-but-cannot-add" );

	auto other = CreateRenderScene();
	checks.That( other->Revision() == 0 && other->Snapshot()->instances.empty() &&
	                 scene->Snapshot()->instances.size() == 1,
	    "C5.scenes-are-independent" );
	scene.reset();
	checks.That( before->instances.size() == 2, "C5.a-held-snapshot-outlives-its-scene" );

	// C6: serial equals pooled.
	jobsystem::ParallelExecutor jobs( 4 );
	const std::uint32_t chunks[] = { 1, 7, 64, 256 };
	int equal = 0;
	int failed = 0;
	std::uint64_t items = 0;
	for ( int trial = 0; trial < 1000; ++trial )
	{
		auto pooledScene = CreateRenderScene();
		ChangeSet changes;
		const int count = static_cast<int>( random() % 600 );
		for ( int i = 0; i < count; ++i )
		{
			MeshInstanceDesc desc =
			    Box( { coordinate( random ), coordinate( random ), coordinate( random ) },
			        size( random ), random() % 5 );
			desc.mesh = random() % 3;
			desc.viewMask = random() % 4 ? ~0u : 1u;
			changes.Add( pooledScene->Reserve(), desc );
		}
		(void)pooledScene->Commit( changes );
		ViewDesc desc;
		desc.view =
		    math::LookAt( { coordinate( random ), coordinate( random ), coordinate( random ) },
		        { coordinate( random ), coordinate( random ), coordinate( random ) }, { 0, 0, 1 } );
		desc.projection =
		    math::Perspective( 0.6f + 1.2f * size( random ) / 6.0f, 1.5f, 1.0f, 120.0f );
		desc.viewBit = random() % 2;
		const SceneView view = MakeView( desc );
		const auto snapshot = pooledScene->Snapshot();
		IVisibilityProvider *filter = trial % 2 ? &provider : nullptr;
		const DrawList serial = BuildDrawList( *snapshot, view, filter );
		auto pooled = BuildDrawListPooled( *snapshot, view, jobs, filter, chunks[trial % 4] );
		if ( !pooled )
		{
			++failed;
			continue;
		}
		const DrawList &p = pooled.Value();
		bool same = p.revision == serial.revision && p.frustumCulled == serial.frustumCulled &&
		            p.providerCulled == serial.providerCulled &&
		            p.items.size() == serial.items.size();
		for ( std::size_t i = 0; same && i < p.items.size(); ++i )
			same = p.items[i].instance == serial.items[i].instance &&
			       p.items[i].material == serial.items[i].material &&
			       p.items[i].mesh == serial.items[i].mesh &&
			       p.items[i].depth == serial.items[i].depth;
		equal += same;
		items += serial.items.size();
	}
	std::printf( "INFO scene: serial and pooled culling compared on 1000 scenes, %llu draws\n",
	    static_cast<unsigned long long>( items ) );
	checks.Equal( failed, 0, "C6.pooled-culling-runs" );
	checks.Equal( equal, 1000, "C6.pooled-culling-equals-serial" );
	checks.That( items > 10000, "C6.the-scenes-draw-something" );

	// C7: a view's explicit frustum replaces the one its matrices imply. The
	// matrices see all six boxes (x within +-0.5); a frustum whose one real
	// plane is x >= 1 keeps none of them, and x >= -1 keeps all.
	{
		ViewDesc explicitDesc = front;
		math::Frustum planes;
		for ( math::Plane &plane : planes.planes )
			plane = { { 0.0f, 0.0f, 0.0f }, 1.0f };         // always inside
		planes.planes[0] = { { 1.0f, 0.0f, 0.0f }, -1.0f }; // x >= 1
		explicitDesc.frustum = planes;
		const SceneView explicitView = MakeView( explicitDesc );
		const DrawList none = BuildDrawList( *sorted->Snapshot(), explicitView );
		planes.planes[0] = { { 1.0f, 0.0f, 0.0f }, 1.0f }; // x >= -1
		explicitDesc.frustum = planes;
		const DrawList all = BuildDrawList( *sorted->Snapshot(), MakeView( explicitDesc ) );
		checks.That(
		    none.items.empty() && none.frustumCulled == 6 && all.items.size() == 6 &&
		        explicitView.viewProjection.rows[0].x == frontView.viewProjection.rows[0].x,
		    "C7.an-explicit-frustum-replaces-the-extracted-one" );
	}
	return checks.Report();
}
