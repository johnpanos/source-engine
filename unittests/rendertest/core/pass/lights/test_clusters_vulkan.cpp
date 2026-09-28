//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lights.clusters.gpu (RFC 0016 K7, render.lights.v1) on
//			render.device.vulkan: cluster_assign.comp as a render.graph
//			compute pass (upload, assign and readback passes through the
//			serial executor) against the serial path, AssignLights.
//
//			G1 over 300 seeded scenes (cluster_oracle.h's MakeScene: cameras
//			   anywhere within 16,000 units, 0 to 256 points and spots, plus
//			   an empty and a full scene), each froxel's list equals the
//			   serial path's list, light-set indices in the same ascending
//			   order. A pair listed by one side only is a mismatch. It is
//			   excused only when it lies on the boundary: the serial path
//			   lists the froxel for the light grown by 1e-4 (radius and
//			   half-angle) and not for it shrunk by 1e-4. Every other
//			   mismatch fails, and boundary mismatches stay under a recorded
//			   ceiling. The ranges are disjoint and cover exactly the indices
//			   written, and the requested count equals the serial
//			   assignments;
//			G2 zero false negatives: the device's lists, compacted to froxel
//			   order, pass the independent reference (CheckScene) on the
//			   first 100 scenes;
//			G3 the per-froxel limit (1 to 8 lights) on 60 scenes: the same
//			   prefixes, and froxelsOverflowed and assignmentsDropped equal
//			   the serial path's;
//			G4 index capacity (a half to a fifth of the assignments) on 40
//			   scenes: each list is a prefix of its per-froxel-limited serial
//			   list, the lists fill the capacity exactly, and the requested
//			   and dropped totals equal the serial path's;
//			G5 the seeded defective kernels (cluster_defects_spv.h) are each
//			   rejected: slice boundaries off by one and the spot cone
//			   ignored (G1 mismatches off the boundary), and capacity losses
//			   not counted (G3 counters);
//			validation: with the Khronos validation layer (synchronization
//			   validation included) the run reports no message; without the
//			   layer the clause prints SKIP and certifies nothing.
//
//			RENDER_VK_ADAPTER=<n> picks the physical device; CONFORMANCE_SEED
//			changes the scenes. A missing Vulkan device fails the run.
//
//=============================================================================//

#include "cluster_defects_spv.h"
#include "cluster_oracle.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/pass/lights/cluster_pass.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <string>
#include <thread>

namespace
{

using namespace cluster_oracle;
using namespace render::device;
namespace lights = render::pass::lights;
namespace graph = render::graph;

constexpr int kScenes = 300;
constexpr int kReferenceScenes = 100;
constexpr int kLimitScenes = 60;
constexpr int kCapacityScenes = 40;
constexpr int kDefectScenes = 40;
// A boundary mismatch is a pair within kBoundary (relative) of the serial
// path's decision. Measured at introduction (2026-09-28): see the progress
// record; the ceiling fails a kernel that disagrees broadly.
constexpr float kBoundary = 1.0e-4f;
constexpr double kBoundaryCeiling = 1.0e-5; // per assignment

std::uint32_t Seed()
{
	const char *text = std::getenv( "CONFORMANCE_SEED" );
	return text ? std::uint32_t( std::strtoul( text, nullptr, 10 ) ) : 20260928u;
}

bool Wait( IRenderDevice2 &device, CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct DeviceLists
{
	bool ok = false;
	std::vector<lights::FroxelRange> ranges;
	lights::ClusterIndexHeader header;
	std::vector<std::uint32_t> indices; // the whole index list (capacity entries)
};

// Runs the upload, assign and readback passes for one view.
DeviceLists AssignOnDevice( IRenderDevice2 &device, lights::ClusterKernel &kernel,
    const lights::ClusterGrid &grid, std::span<const RuntimeLight> sceneLights )
{
	DeviceLists result;
	auto data = std::make_shared<const lights::ClusterDispatchData>(
	    lights::PrepareClusterDispatch( grid, sceneLights ) );
	BufferDesc froxelDesc;
	froxelDesc.size = std::max<std::uint64_t>( data->FroxelBytes(), 8 );
	froxelDesc.usages = { ResourceUsage::kCopyDestination };
	froxelDesc.memory = MemoryKind::kReadback;
	BufferDesc indexDesc = froxelDesc;
	indexDesc.size = data->IndexBytes();
	auto froxelReadback = device.CreateBuffer( froxelDesc );
	auto indexReadback = device.CreateBuffer( indexDesc );
	if ( froxelReadback && indexReadback )
	{
		graph::GraphBuilder builder;
		const lights::ClusterPassResources resources =
		    lights::CreateClusterResources( builder, *data );
		lights::AddClusterUploadPass( builder, data, resources );
		lights::AddClusterAssignPass( builder, kernel, *data, resources );
		const graph::ResourceRef froxelCopy =
		    builder.ImportBuffer( "froxel-readback", froxelReadback.Value(), froxelDesc,
		        ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		const graph::ResourceRef indexCopy =
		    builder.ImportBuffer( "index-readback", indexReadback.Value(), indexDesc,
		        ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		const std::uint64_t froxelBytes = data->FroxelBytes();
		const std::uint64_t indexBytes = data->IndexBytes();
		builder.AddPass( "cluster-readback", graph::PassKind::kCopy )
		    .Read( resources.froxels, ResourceUsage::kCopySource )
		    .Read( resources.indices, ResourceUsage::kCopySource )
		    .Write( froxelCopy, ResourceUsage::kCopyDestination )
		    .Write( indexCopy, ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [=]( graph::RecordContext &context )
		        {
			        if ( froxelBytes )
				        context.Encoder().CopyBuffer( context.Buffer( resources.froxels ),
				            context.Buffer( froxelCopy ), { 0, 0, froxelBytes } );
			        context.Encoder().CopyBuffer( context.Buffer( resources.indices ),
			            context.Buffer( indexCopy ), { 0, 0, indexBytes } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			result.ok = executed.HasValue() && Wait( device, executed.Value().token );
			if ( executed )
				kernel.Collect( executed.Value().token );
		}
	}
	if ( result.ok )
	{
		result.ranges.resize( data->froxelCount );
		std::vector<std::uint32_t> words( data->IndexBytes() / 4 );
		result.ok =
		    device
		        .ReadBuffer( froxelReadback.Value(), 0,
		            std::as_writable_bytes( std::span<lights::FroxelRange>( result.ranges ) ) )
		        .HasValue() &&
		    device
		        .ReadBuffer(
		            indexReadback.Value(), 0, std::as_writable_bytes( std::span( words ) ) )
		        .HasValue();
		result.header.requested = words[0];
		result.header.froxelsOverflowed = words[1];
		result.header.assignmentsDropped = words[2];
		result.header.reserved = words[3];
		result.indices.assign( words.begin() + 4, words.end() );
	}
	if ( froxelReadback )
		(void)device.Release( froxelReadback.Value(), CompletionToken() );
	if ( indexReadback )
		(void)device.Release( indexReadback.Value(), CompletionToken() );
	(void)device.Poll();
	return result;
}

std::span<const std::uint32_t> ListOf( const lights::ClusterLists &lists, std::uint32_t froxel )
{
	const lights::FroxelRange range = lists.froxels[froxel];
	return std::span( lists.lightIndices ).subspan( range.offset, range.count );
}

std::span<const std::uint32_t> ListOf( const DeviceLists &lists, std::uint32_t froxel )
{
	const lights::FroxelRange range = lists.ranges[froxel];
	return std::span( lists.indices ).subspan( range.offset, range.count );
}

// The device's ranges: inside the kept part of the list, disjoint, and
// covering it exactly. An empty range's offset is unspecified (a froxel that
// found no room keeps the offset it was handed past the capacity).
bool RangesWellFormed( const DeviceLists &lists, std::uint32_t capacity )
{
	const std::uint64_t kept = std::min( lists.header.requested, capacity );
	std::vector<std::uint8_t> used( kept, 0 );
	std::uint64_t total = 0;
	for ( const lights::FroxelRange &range : lists.ranges )
	{
		if ( range.count == 0 )
			continue;
		if ( std::uint64_t( range.offset ) + range.count > kept )
			return false;
		for ( std::uint32_t i = 0; i < range.count; ++i )
		{
			if ( used[range.offset + i]++ )
				return false;
		}
		total += range.count;
	}
	return total == kept;
}

// Whether the serial path lists `froxel` for light `index` grown or shrunk by
// kBoundary: the pair is on the decision's boundary when the grown light is
// listed and the shrunk one is not.
bool OnBoundary( const lights::ClusterGrid &grid, const RuntimeLight &light, std::uint32_t froxel )
{
	const auto listed = [&]( float scale )
	{
		RuntimeLight changed = light;
		changed.radius *= scale;
		if ( light.shape == LightShape::Spot )
		{
			const double half = std::acos( std::clamp( double( light.outerCos ), -1.0, 1.0 ) );
			changed.outerCos = float( std::cos( std::min( half * scale, kPi ) ) );
		}
		lights::ClusterLists lists;
		auto stats = lights::AssignLights( grid, std::span( &changed, 1 ), lists );
		return stats && lists.froxels[froxel].count != 0;
	};
	return listed( 1.0f + kBoundary ) && !listed( 1.0f - kBoundary );
}

struct Comparison
{
	std::uint64_t scenes = 0;
	std::uint64_t deviceFailures = 0;
	std::uint64_t shapeErrors = 0;
	std::uint64_t countErrors = 0; // requested count differs from the serial assignments
	std::uint64_t pairs = 0;       // serial assignments compared
	std::uint64_t boundary = 0;
	std::uint64_t mismatches = 0; // off the boundary
	std::string first;

	void Note( const std::string &what )
	{
		if ( first.empty() )
			first = what;
	}
};

// G1 for one scene: the device's lists equal the serial path's, up to
// boundary pairs. Returns the number of boundary pairs (they make the
// scene unusable for exact prefix checks).
std::uint64_t CompareLists( const Scene &scene, const lights::ClusterGrid &grid,
    const lights::ClusterLists &serial, const lights::ClusterStats &stats,
    const DeviceLists &device, Comparison &out )
{
	const std::uint64_t id = out.scenes++;
	if ( !device.ok )
	{
		++out.deviceFailures;
		out.Note( "device run failed (scene " + std::to_string( id ) + ")" );
		return 0;
	}
	if ( device.ranges.size() != serial.froxels.size() ||
	     !RangesWellFormed( device, grid.limits.maxLightIndices ) )
	{
		++out.shapeErrors;
		out.Note( "ranges (scene " + std::to_string( id ) + ")" );
		return 0;
	}
	if ( device.header.requested != stats.assignments )
	{
		++out.countErrors;
		out.Note( "requested count (scene " + std::to_string( id ) + ")" );
	}
	out.pairs += stats.assignments;
	std::uint64_t boundary = 0;
	for ( std::uint32_t f = 0; f < serial.froxels.size(); ++f )
	{
		const auto a = ListOf( serial, f );
		const auto b = ListOf( device, f );
		if ( std::equal( a.begin(), a.end(), b.begin(), b.end() ) )
			continue;
		if ( !std::is_sorted( b.begin(), b.end() ) ||
		     std::adjacent_find( b.begin(), b.end() ) != b.end() )
		{
			++out.shapeErrors;
			out.Note( "unordered device list (scene " + std::to_string( id ) + ")" );
			continue;
		}
		std::vector<std::uint32_t> onlyOne;
		std::set_symmetric_difference(
		    a.begin(), a.end(), b.begin(), b.end(), std::back_inserter( onlyOne ) );
		for ( const std::uint32_t light : onlyOne )
		{
			if ( light < scene.lights.size() && OnBoundary( grid, scene.lights[light], f ) )
			{
				++boundary;
				continue;
			}
			++out.mismatches;
			char text[160];
			std::snprintf( text, sizeof( text ),
			    "scene %llu froxel %u light %u listed by the %s path only",
			    static_cast<unsigned long long>( id ), f, light,
			    std::binary_search( a.begin(), a.end(), light ) ? "serial" : "device" );
			out.Note( text );
		}
	}
	out.boundary += boundary;
	return boundary;
}

// The device's lists in froxel order, as the serial path lays them out.
BuildOutput Compacted( const lights::ClusterGrid &grid, const DeviceLists &device )
{
	BuildOutput out;
	out.grid = grid;
	out.ok = device.ok;
	for ( std::uint32_t f = 0; f < device.ranges.size(); ++f )
	{
		const auto list = ListOf( device, f );
		out.lists.froxels.push_back(
		    { std::uint32_t( out.lists.lightIndices.size() ), std::uint32_t( list.size() ) } );
		out.lists.lightIndices.insert( out.lists.lightIndices.end(), list.begin(), list.end() );
	}
	out.stats.assignments = std::uint32_t( out.lists.lightIndices.size() );
	return out;
}

struct Built
{
	bool ok = false;
	lights::ClusterGrid grid;
	lights::ClusterLists lists;
	lights::ClusterStats stats;
};

Built Serial( const Scene &scene )
{
	Built built;
	auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
	if ( !grid )
		return built;
	built.grid = std::move( grid ).Value();
	auto stats = lights::AssignLights( built.grid, scene.lights, built.lists );
	if ( !stats )
		return built;
	built.stats = stats.Value();
	built.ok = true;
	return built;
}

// The scenes of G1: MakeScene's, with index capacity sized to the serial
// assignments (so the device's list stays small), plus an empty and a full
// scene.
std::vector<Scene> MakeScenes( std::mt19937 &random, int count )
{
	std::vector<Scene> scenes;
	for ( int s = 0; s < count; ++s )
	{
		Scene scene = MakeScene( random );
		if ( s == 0 )
			scene.lights.clear();
		while ( s == 1 && scene.lights.size() < 256 )
		{
			// Lights of other scenes, as world lights of this one.
			const Scene other = MakeScene( random );
			for ( const RuntimeLight &light : other.lights )
			{
				if ( scene.lights.size() < 256 )
					scene.lights.push_back( light );
			}
		}
		const Built built = Serial( scene );
		scene.limits.maxLightIndices = std::max<std::uint32_t>( built.stats.assignments + 64, 64 );
		scenes.push_back( std::move( scene ) );
	}
	return scenes;
}

struct LimitResult
{
	std::uint64_t scenes = 0;
	std::uint64_t skipped = 0; // scenes with boundary pairs
	std::uint64_t listErrors = 0;
	std::uint64_t counterErrors = 0;
	std::uint64_t overflowed = 0; // froxels that lost a light, over the scenes
	std::string first;
};

// G3: the per-froxel limit keeps the same prefixes and counts the same
// losses.
void CheckPerFroxelLimit( IRenderDevice2 &device, lights::ClusterKernel &kernel, const Scene &base,
    std::mt19937 &random, LimitResult &out )
{
	Scene scene = base;
	scene.limits.maxLightsPerFroxel = 1 + random() % 8;
	const Built serial = Serial( scene );
	const DeviceLists lists = AssignOnDevice( device, kernel, serial.grid, scene.lights );
	++out.scenes;
	if ( !serial.ok || !lists.ok || lists.ranges.size() != serial.lists.froxels.size() )
	{
		++out.listErrors;
		if ( out.first.empty() )
			out.first = "run failed";
		return;
	}
	out.overflowed += serial.stats.froxelsOverflowed;
	if ( lists.header.froxelsOverflowed != serial.stats.froxelsOverflowed ||
	     lists.header.assignmentsDropped != serial.stats.assignmentsDropped ||
	     lists.header.requested != serial.stats.assignments )
	{
		++out.counterErrors;
		if ( out.first.empty() )
			out.first = "counters differ";
	}
	for ( std::uint32_t f = 0; f < lists.ranges.size(); ++f )
	{
		const auto a = ListOf( serial.lists, f );
		const auto b = ListOf( lists, f );
		if ( !std::equal( a.begin(), a.end(), b.begin(), b.end() ) )
		{
			++out.listErrors;
			if ( out.first.empty() )
				out.first = "per-froxel prefix differs";
			return;
		}
	}
}

// G4: index capacity. Totals are defined; which froxels lose room is not.
void CheckIndexCapacity( IRenderDevice2 &device, lights::ClusterKernel &kernel, const Scene &base,
    std::mt19937 &random, LimitResult &out )
{
	Scene unlimited = base;
	unlimited.limits.maxLightsPerFroxel = 1 + random() % 16;
	const Built full = Serial( unlimited );
	Scene scene = unlimited;
	scene.limits.maxLightIndices =
	    std::max<std::uint32_t>( 1, full.stats.assignments / ( 2 + random() % 4 ) );
	const Built serial = Serial( scene );
	const DeviceLists lists = AssignOnDevice( device, kernel, serial.grid, scene.lights );
	++out.scenes;
	if ( !full.ok || !serial.ok || !lists.ok || lists.ranges.size() != serial.lists.froxels.size() )
	{
		++out.listErrors;
		if ( out.first.empty() )
			out.first = "run failed";
		return;
	}
	out.overflowed += serial.stats.froxelsOverflowed;
	if ( !RangesWellFormed( lists, scene.limits.maxLightIndices ) ||
	     lists.header.requested != full.stats.assignments ||
	     lists.header.assignmentsDropped != serial.stats.assignmentsDropped ||
	     std::min( lists.header.requested, scene.limits.maxLightIndices ) !=
	         serial.stats.assignments )
	{
		++out.counterErrors;
		if ( out.first.empty() )
		{
			char text[200];
			std::snprintf( text, sizeof( text ),
			    "capacity totals differ: requested %u (serial %u), dropped %u (serial %u), "
			    "capacity %u, kept %u, ranges %s",
			    lists.header.requested, full.stats.assignments, lists.header.assignmentsDropped,
			    serial.stats.assignmentsDropped, scene.limits.maxLightIndices,
			    serial.stats.assignments,
			    RangesWellFormed( lists, scene.limits.maxLightIndices ) ? "ok" : "malformed" );
			out.first = text;
		}
	}
	for ( std::uint32_t f = 0; f < lists.ranges.size(); ++f )
	{
		const auto a = ListOf( full.lists, f );
		const auto b = ListOf( lists, f );
		if ( b.size() > a.size() || !std::equal( b.begin(), b.end(), a.begin() ) )
		{
			++out.listErrors;
			if ( out.first.empty() )
				out.first = "a list is not a prefix of its per-froxel list";
			return;
		}
	}
}

// Runs G1 for a kernel over `scenes`; the defects reuse it.
Comparison CompareScenes( IRenderDevice2 &device, lights::ClusterKernel &kernel,
    const std::vector<Scene> &scenes, std::size_t count, std::vector<std::uint64_t> *boundary,
    std::mt19937 *reference, Tally *tally )
{
	Comparison comparison;
	for ( std::size_t s = 0; s < count && s < scenes.size(); ++s )
	{
		const Built serial = Serial( scenes[s] );
		if ( !serial.ok )
		{
			++comparison.deviceFailures;
			comparison.Note( "serial build failed" );
			continue;
		}
		const DeviceLists lists = AssignOnDevice( device, kernel, serial.grid, scenes[s].lights );
		const std::uint64_t pairs =
		    CompareLists( scenes[s], serial.grid, serial.lists, serial.stats, lists, comparison );
		if ( boundary )
			boundary->push_back( pairs );
		if ( tally && reference && s < std::size_t( kReferenceScenes ) && lists.ok )
			CheckScene( scenes[s], Compacted( serial.grid, lists ), *reference, *tally );
	}
	return comparison;
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::uint32_t seed = Seed();
	std::printf( "INFO seed %u\n", seed );
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
		const std::string_view name = device->Facts().adapterName;
		std::printf(
		    "INFO clusters adapter: %.*s\n", static_cast<int>( name.size() ), name.data() );
		auto kernel = lights::ClusterKernel::Create( *device );
		if ( !checks.That( kernel.HasValue(), "kernel.is-created" ) )
			return checks.Report();

		std::mt19937 random( seed );
		const std::vector<Scene> scenes = MakeScenes( random, kScenes );
		std::mt19937 reference( seed + 1 );
		Tally tally;
		std::vector<std::uint64_t> boundary;
		const Comparison g1 = CompareScenes(
		    *device, *kernel.Value(), scenes, scenes.size(), &boundary, &reference, &tally );
		std::uint64_t froxels = 0;
		std::uint64_t sceneLights = 0;
		for ( const Scene &scene : scenes )
		{
			sceneLights += scene.lights.size();
			auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
			froxels += grid ? grid.Value().FroxelCount() : 0;
		}
		std::printf( "INFO G1 %llu scenes, %llu froxels, %llu lights, %llu serial assignments; "
		             "%llu boundary pairs, %llu mismatches%s%s\n",
		    static_cast<unsigned long long>( g1.scenes ),
		    static_cast<unsigned long long>( froxels ),
		    static_cast<unsigned long long>( sceneLights ),
		    static_cast<unsigned long long>( g1.pairs ),
		    static_cast<unsigned long long>( g1.boundary ),
		    static_cast<unsigned long long>( g1.mismatches ), g1.first.empty() ? "" : ": ",
		    g1.first.c_str() );
		checks.Equal( g1.deviceFailures, std::uint64_t( 0 ), "G1.every-dispatch-runs" );
		checks.Equal( g1.shapeErrors, std::uint64_t( 0 ), "G1.ranges-disjoint-and-covering" );
		checks.Equal(
		    g1.countErrors, std::uint64_t( 0 ), "G1.requested-equals-serial-assignments" );
		checks.That( g1.scenes == std::uint64_t( kScenes ) && g1.pairs > 1000000,
		    "G1.the-scenes-exercise-the-kernel" );
		checks.Equal( g1.mismatches, std::uint64_t( 0 ), "G1.lists-equal-the-serial-path" );
		checks.That( double( g1.boundary ) <= kBoundaryCeiling * double( g1.pairs ),
		    "G1.boundary-pairs-under-their-ceiling" );
		checks.Equal( scenes[0].lights.size(), std::size_t( 0 ), "G1.an-empty-scene-is-included" );
		checks.Equal( scenes[1].lights.size(), std::size_t( 256 ), "G1.a-full-scene-is-included" );

		std::printf( "INFO G2 %llu scenes against the reference: %llu reached pairs, %llu false "
		             "negatives, false positives points %.4f spots %.4f%s%s\n",
		    static_cast<unsigned long long>( tally.scenes ),
		    static_cast<unsigned long long>( tally.reached ),
		    static_cast<unsigned long long>( tally.falseNegatives ), tally.PointFalsePositiveRate(),
		    tally.SpotFalsePositiveRate(), tally.first.empty() ? "" : ": ", tally.first.c_str() );
		checks.That( tally.scenes == std::uint64_t( kReferenceScenes ) && tally.reached > 0 &&
		                 tally.buildFailures == 0 && tally.shapeErrors == 0,
		    "G2.the-device-lists-are-well-formed" );
		checks.Equal( tally.falseNegatives, std::uint64_t( 0 ), "G2.zero-false-negatives" );
		checks.That( tally.lookupOutside == 0 && tally.lookupMisses == 0,
		    "G2.shading-lookups-find-their-lights" );

		LimitResult g3;
		for ( int s = 0; s < kLimitScenes; ++s )
		{
			if ( boundary[s] != 0 )
			{
				++g3.skipped;
				continue;
			}
			CheckPerFroxelLimit( *device, *kernel.Value(), scenes[s], random, g3 );
		}
		std::printf( "INFO G3 %llu scenes (%llu skipped for boundary pairs), %llu froxels "
		             "overflowed%s%s\n",
		    static_cast<unsigned long long>( g3.scenes ),
		    static_cast<unsigned long long>( g3.skipped ),
		    static_cast<unsigned long long>( g3.overflowed ), g3.first.empty() ? "" : ": ",
		    g3.first.c_str() );
		checks.That( g3.scenes >= std::uint64_t( kLimitScenes / 2 ) && g3.overflowed > 0,
		    "G3.the-limit-is-exercised" );
		checks.Equal( g3.listErrors, std::uint64_t( 0 ), "G3.the-same-prefixes" );
		checks.Equal( g3.counterErrors, std::uint64_t( 0 ), "G3.the-same-counters" );

		LimitResult g4;
		for ( int s = kLimitScenes; s < kLimitScenes + kCapacityScenes; ++s )
		{
			if ( boundary[s] != 0 || scenes[s].lights.empty() )
			{
				++g4.skipped;
				continue;
			}
			CheckIndexCapacity( *device, *kernel.Value(), scenes[s], random, g4 );
		}
		std::printf( "INFO G4 %llu scenes (%llu skipped), %llu froxels overflowed%s%s\n",
		    static_cast<unsigned long long>( g4.scenes ),
		    static_cast<unsigned long long>( g4.skipped ),
		    static_cast<unsigned long long>( g4.overflowed ), g4.first.empty() ? "" : ": ",
		    g4.first.c_str() );
		checks.That( g4.scenes >= std::uint64_t( kCapacityScenes / 2 ) && g4.overflowed > 0,
		    "G4.the-capacity-is-exercised" );
		checks.Equal( g4.listErrors, std::uint64_t( 0 ), "G4.lists-are-serial-prefixes" );
		checks.Equal( g4.counterErrors, std::uint64_t( 0 ), "G4.totals-equal-the-serial-path" );
		checks.Equal( kernel.Value()->RecordFailures(), 0u, "G1.records-without-failure" );

		struct Defect
		{
			const char *name;
			std::span<const std::uint32_t> code;
			bool counters; // detected by the G3 counters rather than G1
		};
		const Defect defects[] = {
		    { "slice-off-by-one", rendertest::lights::spirv::kClusterSliceOffByOne, false },
		    { "cone-ignored", rendertest::lights::spirv::kClusterConeIgnored, false },
		    { "uncounted-overflow", rendertest::lights::spirv::kClusterUncountedOverflow, true },
		};
		for ( const Defect &defect : defects )
		{
			auto broken = lights::ClusterKernel::Create( *device, defect.code );
			bool detected = false;
			if ( broken && !defect.counters )
			{
				const Comparison seeded = CompareScenes(
				    *device, *broken.Value(), scenes, kDefectScenes, nullptr, nullptr, nullptr );
				std::printf(
				    "INFO seeded %s: %llu mismatches, %llu boundary pairs, %llu scenes with "
				    "another count%s%s\n",
				    defect.name, static_cast<unsigned long long>( seeded.mismatches ),
				    static_cast<unsigned long long>( seeded.boundary ),
				    static_cast<unsigned long long>( seeded.countErrors ),
				    seeded.first.empty() ? "" : ": ", seeded.first.c_str() );
				detected = seeded.deviceFailures == 0 && seeded.mismatches > 0;
			}
			else if ( broken )
			{
				std::mt19937 limits( seed + 2 );
				LimitResult seeded;
				for ( int s = 0; s < kDefectScenes; ++s )
					CheckPerFroxelLimit( *device, *broken.Value(), scenes[s], limits, seeded );
				std::printf( "INFO seeded %s: %llu counter errors over %llu scenes\n", defect.name,
				    static_cast<unsigned long long>( seeded.counterErrors ),
				    static_cast<unsigned long long>( seeded.scenes ) );
				detected = seeded.counterErrors > 0;
			}
			checks.That( detected, std::string( "G5.detects-" ) + defect.name );
		}
		kernel.Value().reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
