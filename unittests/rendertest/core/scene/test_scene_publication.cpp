//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.publication (RFC 0016 K5): snapshot publication under
//			concurrency, for the ThreadSanitizer lane.
//
//			The owner commits 2,000 change sets, each adding one instance
//			whose bounds encode its revision, while three render-side readers
//			take snapshots and build draw lists from them. Every snapshot a
//			reader sees must be complete (revision n holds exactly the n
//			instances of revisions 1..n, each with its own bounds), readers
//			see revisions in order, and Revision() never runs ahead of the
//			latest snapshot a reader can take.
//
//			The sensitivity row builds scene.cpp with
//			RENDER_SCENE_SEEDED_UNSYNCHRONIZED_PUBLICATION (the snapshot is
//			published without the release/acquire edge) and must fail under
//			ThreadSanitizer.
//
//=============================================================================//

#include "render/scene/draw_list.h"
#include "render/scene/scene.h"
#include "testing/checks.h"

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

namespace
{

using namespace render;
using namespace render::scene;

constexpr int kCommits = 2000;
constexpr int kReaders = 3;

struct ReaderResult
{
	std::uint64_t snapshots = 0;
	std::uint64_t incomplete = 0;
	std::uint64_t backwards = 0;
	std::uint64_t ahead = 0;
};

// Instance i (1-based, added by revision i) sits at x = 3 i with half size 1.
bool Complete( const SceneSnapshot &snapshot )
{
	if ( snapshot.instances.size() != snapshot.revision )
		return false;
	for ( std::size_t i = 0; i < snapshot.instances.size(); ++i )
	{
		const MeshInstance &instance = snapshot.instances[i];
		if ( instance.id.value != i + 1 ||
		     instance.worldBounds.Center().x != 3.0f * float( i + 1 ) )
			return false;
	}
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;
	auto scene = CreateRenderScene();
	std::atomic<bool> done{ false };
	std::vector<ReaderResult> results( kReaders );
	std::vector<std::thread> readers;
	ViewDesc desc;
	desc.view = math::LookAt( { 0, -50, 0 }, { 0, 0, 0 }, { 0, 0, 1 } );
	desc.projection = math::Perspective( 1.2f, 1.0f, 1.0f, 10000.0f );
	const SceneView view = MakeView( desc );
	for ( int r = 0; r < kReaders; ++r )
	{
		readers.emplace_back(
		    [&, r]
		    {
			    ReaderResult &result = results[r];
			    std::uint64_t last = 0;
			    while ( !done.load( std::memory_order_acquire ) || last < kCommits )
			    {
				    const std::uint64_t revision = scene->Revision();
				    const std::shared_ptr<const SceneSnapshot> snapshot = scene->Snapshot();
				    ++result.snapshots;
				    result.incomplete += !Complete( *snapshot );
				    result.backwards += snapshot->revision < last;
				    result.ahead += revision > snapshot->revision;
				    last = snapshot->revision;
				    const DrawList list = BuildDrawList( *snapshot, view );
				    result.incomplete += list.revision != snapshot->revision;
			    }
		    } );
	}
	int rejected = 0;
	for ( int i = 1; i <= kCommits; ++i )
	{
		MeshInstanceDesc instance;
		instance.world = math::Translation( { 3.0f * float( i ), 0, 0 } );
		instance.localBounds = { { -1, -1, -1 }, { 1, 1, 1 } };
		instance.material = static_cast<std::uint64_t>( i % 7 );
		ChangeSet changes;
		changes.Add( scene->Reserve(), instance );
		rejected += !scene->Commit( changes );
	}
	done.store( true, std::memory_order_release );
	for ( std::thread &reader : readers )
		reader.join();

	ReaderResult total;
	for ( const ReaderResult &result : results )
	{
		total.snapshots += result.snapshots;
		total.incomplete += result.incomplete;
		total.backwards += result.backwards;
		total.ahead += result.ahead;
	}
	std::printf( "INFO scene publication: %llu snapshots read by %d readers over %d commits\n",
	    static_cast<unsigned long long>( total.snapshots ), kReaders, kCommits );
	checks.Equal( rejected, 0, "P1.every-commit-applies" );
	checks.That( total.snapshots >= std::uint64_t( kReaders ), "P1.readers-read" );
	checks.Equal( total.incomplete, std::uint64_t( 0 ), "P2.every-snapshot-is-complete" );
	checks.Equal( total.backwards, std::uint64_t( 0 ), "P3.readers-see-revisions-in-order" );
	checks.Equal(
	    total.ahead, std::uint64_t( 0 ), "P4.the-revision-never-runs-ahead-of-the-snapshot" );
	checks.Equal( scene->Snapshot()->revision, std::uint64_t( kCommits ),
	    "P1.the-last-revision-is-published" );
	return checks.Report();
}
