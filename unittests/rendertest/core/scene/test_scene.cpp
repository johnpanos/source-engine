//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 (RFC 0016):
//
//			C1 the scene contract, run over the scene and over deliberately
//			   broken ones: a commit applies all of its change set or none of
//			   it, a held snapshot never changes, and a snapshot holds exactly
//			   the instances and bounds a reference model holds across block
//			   boundaries (adds in and out of id order, updates, removals);
//			C2 cost scaling: a commit of k changes to n instances shares every
//			   block it did not touch with the previous snapshot, so it never
//			   copies the whole table. The suite rejects a scene whose
//			   snapshots alias one mutable table and one that copies the
//			   table per commit;
//			C3 on seeded random scenes, culling never drops an instance that a
//			   corner test proves visible, and drops every instance that lies
//			   wholly outside one clip plane;
//			C4 a visibility provider can only remove candidates; the suite
//			   rejects one that adds or repeats;
//			C5 two scenes are independent, and a held snapshot outlives its
//			   scene: it shares its blocks with the table, so it stays valid
//			   and unchanged after the scene that published it is destroyed;
//			C6 pooled culling equals serial culling on 1,000 seeded scenes;
//			C7 a view's explicit frustum replaces the extracted one.
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"
#include "render/scene/draw_list.h"
#include "render/scene/scene.h"
#include "testing/checks.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
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

// Removes odd candidates, then breaks the contract: a candidate that was
// never there, and one listed twice.
class DropOdd final : public IVisibilityProvider
{
public:
	explicit DropOdd( bool misbehave = true ) : m_Misbehave( misbehave ) {}
	void Filter(
	    const SceneSnapshot &, const SceneView &, std::vector<std::uint32_t> &candidates ) override
	{
		std::erase_if( candidates,
		    []( std::uint32_t index )
		    {
			    return index % 2 == 1;
		    } );
		if ( m_Misbehave )
			candidates.push_back( 100000 );
		if ( m_Misbehave && !candidates.empty() )
			candidates.push_back( candidates.front() );
	}

private:
	bool m_Misbehave;
};

// Whether a provider only removed: every candidate it leaves was one, once.
bool OnlyRemoves(
    IVisibilityProvider &provider, const SceneSnapshot &snapshot, const SceneView &view )
{
	std::vector<std::uint32_t> kept;
	for ( std::uint32_t i = 0; i < snapshot.instances.size(); ++i )
		kept.push_back( i );
	provider.Filter( snapshot, view, kept );
	std::sort( kept.begin(), kept.end() );
	return std::adjacent_find( kept.begin(), kept.end() ) == kept.end() &&
	       ( kept.empty() || kept.back() < snapshot.instances.size() );
}

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

// Wraps a scene and republishes its snapshots through `publish`: the broken
// scenes below differ only in what they publish.
class WrappedScene final : public IRenderScene
{
public:
	using Publish = std::function<std::shared_ptr<const SceneSnapshot>(
	    const std::shared_ptr<const SceneSnapshot> &, std::shared_ptr<SceneSnapshot> & )>;
	explicit WrappedScene( Publish publish ) : m_Inner( CreateRenderScene() ), m_Publish( publish )
	{
	}
	InstanceId Reserve() override { return m_Inner->Reserve(); }
	foundation::Expected<std::uint64_t, SceneError> Commit( const ChangeSet &changes ) override
	{
		auto result = m_Inner->Commit( changes );
		if ( result )
			m_Latest = m_Publish( m_Inner->Snapshot(), m_Live );
		return result;
	}
	std::shared_ptr<const SceneSnapshot> Snapshot() const override
	{
		return m_Latest ? m_Latest : m_Inner->Snapshot();
	}
	std::uint64_t Revision() const override { return m_Inner->Revision(); }

private:
	std::unique_ptr<IRenderScene> m_Inner;
	Publish m_Publish;
	std::shared_ptr<SceneSnapshot> m_Live;
	std::shared_ptr<const SceneSnapshot> m_Latest;
};

using SceneFactory = std::function<std::unique_ptr<IRenderScene>()>;

// The scene-service contract: C1 commits and held snapshots, C2 cost scaling.
void SceneContract( testing::Checks &c, const SceneFactory &make )
{
	const auto live = [&]( const SceneSnapshot &snapshot )
	{
		std::size_t count = 0;
		for ( std::size_t i = 0; i < snapshot.instances.size(); ++i )
			count += snapshot.instances[i].id.IsValid();
		return count;
	};
	auto scene = make();
	std::vector<InstanceId> ids;
	ChangeSet first;
	for ( int i = 0; i < 1500; ++i ) // several blocks
	{
		ids.push_back( scene->Reserve() );
		first.Add( ids.back(), Box( { float( i ), 0.0f, 0.0f }, 0.5f ) );
	}
	c.That( scene->Commit( first ).HasValue() && live( *scene->Snapshot() ) == 1500,
	    "C1.the-first-commit-applies" );
	// An update, a removal in the middle and an add, in one commit.
	const std::shared_ptr<const SceneSnapshot> held = scene->Snapshot();
	ChangeSet edit;
	edit.UpdateTransform( ids[0], math::Translation( { -5.0f, 0.0f, 0.0f } ) );
	edit.Remove( ids[700] );
	edit.Add( scene->Reserve(), Box( { 9000.0f, 0.0f, 0.0f }, 0.5f ) );
	c.That( scene->Commit( edit ).HasValue(), "C1.an-edit-commit-applies" );
	const auto now = scene->Snapshot();
	c.That( live( *now ) == 1500 && now->revision == held->revision + 1 &&
	            now->instances[0].worldBounds.Center().x == -5.0f,
	    "C1.updates-removals-and-adds-reach-the-snapshot" );
	c.That( live( *held ) == 1500 && held->instances[0].worldBounds.Center().x == 0.0f,
	    "C1.a-held-snapshot-never-changes" );
	ChangeSet bad;
	bad.UpdateTransform( ids[1], math::Translation( { 7.0f, 7.0f, 7.0f } ) );
	bad.Remove( ids[700] ); // already removed
	const auto rejected = scene->Commit( bad );
	c.That( !rejected && rejected.Error().change == 1 && scene->Snapshot() == now,
	    "C1.a-failed-commit-names-its-change-and-applies-nothing" );

	// C2: a commit shares every block it did not touch.
	auto big = make();
	ChangeSet fill;
	for ( int i = 0; i < 20000; ++i )
		fill.Add( big->Reserve(), Box( { float( i ), 0.0f, 0.0f }, 0.5f ) );
	(void)big->Commit( fill );
	const std::shared_ptr<const SceneSnapshot> a = big->Snapshot();
	ChangeSet one;
	one.UpdateTransform( a->instances[10000].id, math::Translation( { 1.0f, 2.0f, 3.0f } ) );
	one.Remove( a->instances[3000].id );
	(void)big->Commit( one );
	const std::shared_ptr<const SceneSnapshot> b = big->Snapshot();
	// A block is shared when both snapshots hold its instances at one address.
	std::size_t shared = 0;
	const std::size_t blocks = a->instances.size() / InstanceTable::kBlock;
	for ( std::size_t i = 0; i < blocks; ++i )
		shared +=
		    &a->instances[i * InstanceTable::kBlock] == &b->instances[i * InstanceTable::kBlock];
	c.That( blocks > 50 && shared + 2 >= blocks, "C2.a-commit-copies-only-the-blocks-it-touches" );
}

} // namespace

int main()
{
	testing::Checks checks;
	SceneContract( checks,
	    []()
	    {
		    return CreateRenderScene();
	    } );
	{
		// Broken scenes the contract must reject: snapshots that alias one
		// mutable table, and snapshots that copy the whole table per commit.
		std::FILE *sink = std::tmpfile();
		testing::Checks aliased( sink ? sink : stdout ), copying( sink ? sink : stdout );
		SceneContract( aliased,
		    []()
		    {
			    return std::make_unique<WrappedScene>(
			        []( const std::shared_ptr<const SceneSnapshot> &inner,
			            std::shared_ptr<SceneSnapshot> &live )
			        {
				        if ( !live )
					        live = std::make_shared<SceneSnapshot>();
				        *live = *inner;
				        return std::shared_ptr<const SceneSnapshot>( live );
			        } );
		    } );
		SceneContract( copying,
		    []()
		    {
			    return std::make_unique<WrappedScene>(
			        []( const std::shared_ptr<const SceneSnapshot> &inner,
			            std::shared_ptr<SceneSnapshot> & )
			        {
				        auto copy = std::make_shared<SceneSnapshot>();
				        copy->revision = inner->revision;
				        copy->instances.resize( inner->instances.size() );
				        for ( std::size_t i = 0; i < inner->instances.size(); ++i )
					        copy->instances[i] = inner->instances[i];
				        return std::shared_ptr<const SceneSnapshot>( copy );
			        } );
		    } );
		if ( sink )
			std::fclose( sink );
		checks.That( aliased.Failures() > 0, "C2.the-suite-rejects-aliased-snapshots" );
		checks.That( copying.Failures() > 0, "C2.the-suite-rejects-a-whole-table-copy" );
	}

	// C5: two scenes are independent, and a held snapshot outlives its scene.
	// A snapshot shares its blocks with the table, so destroying the scene
	// must leave the held snapshot valid and unchanged, and the other scene
	// must neither see nor be seen by it (RFC 0016 K5, "Two scenes").
	{
		auto first = CreateRenderScene(), second = CreateRenderScene();
		ChangeSet one, two;
		for ( int i = 0; i < 600; ++i ) // several blocks
			one.Add( first->Reserve(), Box( { float( i ), 0.0f, 0.0f }, 0.5f ) );
		for ( int i = 0; i < 3; ++i )
			two.Add( second->Reserve(), Box( { 0.0f, float( i ), 0.0f }, 0.5f, 9 ) );
		(void)first->Commit( one );
		const std::shared_ptr<const SceneSnapshot> held = first->Snapshot();
		ChangeSet later;
		later.Remove( held->instances[10].id );
		later.Add( first->Reserve(), Box( { 100.0f, 0.0f, 0.0f }, 0.5f ) );
		(void)first->Commit( later );
		(void)second->Commit( two );
		const std::shared_ptr<const SceneSnapshot> other = second->Snapshot();
		first.reset(); // the held snapshot must outlive the scene
		std::size_t live = 0;
		for ( std::size_t i = 0; i < held->instances.size(); ++i )
			live += held->instances[i].id.IsValid();
		checks.That( held->revision == 1 && live == 600 && held->instances[10].id.IsValid() &&
		                 held->instances[10].worldBounds.Center().x == 10.0f,
		    "C5.a-held-snapshot-outlives-its-scene" );
		// The other scene saw none of the first scene's 600 instances, and
		// still commits and publishes after the first scene is gone.
		ChangeSet more;
		more.Add( second->Reserve(), Box( { 0.0f, 3.0f, 0.0f }, 0.5f, 9 ) );
		bool independent = second->Commit( more ).HasValue() &&
		                   second->Snapshot()->instances.size() == 4 &&
		                   other->instances.size() == 3;
		for ( std::size_t i = 0; independent && i < other->instances.size(); ++i )
			independent = other->instances[i].desc.material == 9;
		checks.That( independent, "C5.scenes-are-independent" );
	}

	// C3: random scenes against the corner oracle.
	std::mt19937 random( 1234 );
	std::uniform_real_distribution<float> coordinate( -60.0f, 60.0f );
	std::uniform_real_distribution<float> size( 0.1f, 6.0f );
	int missed = 0, kept = 0, decided = 0;
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
		for ( std::size_t i = 0; i < 200; ++i )
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

	DropOdd provider( false ), bad;
	const DrawList filtered = BuildDrawList( *sorted->Snapshot(), frontView, &provider );
	bool onlyEven = filtered.items.size() == 3 && filtered.providerCulled == 3;
	for ( const DrawItem &item : filtered.items )
		onlyEven &= item.instance % 2 == 0;
	const auto snapshot = sorted->Snapshot();
	checks.That( onlyEven && OnlyRemoves( provider, *snapshot, frontView ),
	    "C4.a-provider-removes-but-cannot-add" );
	checks.That( !OnlyRemoves( bad, *snapshot, frontView ) &&
	                 BuildDrawList( *snapshot, frontView, &bad ).items.size() == 3,
	    "C4.the-suite-rejects-a-provider-that-adds-or-repeats" );

	// C6: serial equals pooled.
	jobsystem::ParallelExecutor jobs( 4 );
	const std::uint32_t chunks[] = { 1, 7, 64, 256 };
	int equal = 0, failed = 0;
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
		const auto pooledSnapshot = pooledScene->Snapshot();
		IVisibilityProvider *filter = trial % 2 ? &provider : nullptr;
		const DrawList serial = BuildDrawList( *pooledSnapshot, view, filter );
		auto pooled = BuildDrawListPooled( *pooledSnapshot, view, jobs, filter, chunks[trial % 4] );
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

	// C7: the matrices see all six boxes; a frustum whose one real plane is
	// x >= 1 keeps none of them, and x >= -1 keeps all.
	ViewDesc explicitDesc = front;
	math::Frustum planes;
	for ( math::Plane &plane : planes.planes )
		plane = { { 0.0f, 0.0f, 0.0f }, 1.0f };         // always inside
	planes.planes[0] = { { 1.0f, 0.0f, 0.0f }, -1.0f }; // x >= 1
	explicitDesc.frustum = planes;
	const DrawList none = BuildDrawList( *sorted->Snapshot(), MakeView( explicitDesc ) );
	planes.planes[0] = { { 1.0f, 0.0f, 0.0f }, 1.0f }; // x >= -1
	explicitDesc.frustum = planes;
	const DrawList all = BuildDrawList( *sorted->Snapshot(), MakeView( explicitDesc ) );
	checks.That( none.items.empty() && none.frustumCulled == 6 && all.items.size() == 6,
	    "C7.an-explicit-frustum-replaces-the-extracted-one" );
	return checks.Report();
}
