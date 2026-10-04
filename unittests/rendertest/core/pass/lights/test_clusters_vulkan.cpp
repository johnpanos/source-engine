//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lights.clusters.gpu (RFC 0016 K7, render.lights.v1):
//          GPU assignment, independent geometric coverage checks, capacity
//          accounting, direct consumer buffers and seeded defective kernels.
//          No CPU light-assignment implementation or timing path is retained.
//
//=============================================================================//

#include "spv/cluster_defects_spv.h"
#include "cluster_oracle.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/pass/lights/cluster_pass.h"
#include "render/graph/pass_timers.h"
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
    const lights::ClusterGrid &grid, std::span<const RuntimeLight> sceneLights,
    std::span<const area_light::AreaLight> areas = {}, bool direct = false )
{
	DeviceLists result;
	auto data = std::make_shared<const lights::ClusterDispatchData>(
	    lights::PrepareClusterDispatch( grid, sceneLights, areas ) );
	BufferDesc froxelDesc;
	froxelDesc.size = std::max<std::uint64_t>( data->FroxelBytes(), 8 );
	froxelDesc.usages = { ResourceUsage::kCopyDestination };
	froxelDesc.memory = MemoryKind::kReadback;
	BufferDesc indexDesc = froxelDesc;
	indexDesc.size = data->IndexBytes();
	auto froxelReadback = device.CreateBuffer( froxelDesc );
	auto indexReadback = device.CreateBuffer( indexDesc );
	if ( froxelReadback && indexReadback && direct )
	{
		auto encoded = device.BeginEncoder( QueueKind::kGraphics );
		if ( encoded )
		{
			auto assigned = kernel.RecordView( encoded.Value(), *data );
			if ( assigned )
			{
				auto &encoder = encoded.Value();
				encoder.TransitionBuffer( assigned.Value().froxels, ResourceUsage::kStorageRead,
				    ResourceUsage::kCopySource );
				encoder.TransitionBuffer( assigned.Value().indices, ResourceUsage::kStorageRead,
				    ResourceUsage::kCopySource );
				encoder.TransitionBuffer( froxelReadback.Value(), ResourceUsage::kUndefined,
				    ResourceUsage::kCopyDestination );
				encoder.TransitionBuffer( indexReadback.Value(), ResourceUsage::kUndefined,
				    ResourceUsage::kCopyDestination );
				encoder.CopyBuffer( assigned.Value().froxels, froxelReadback.Value(),
				    { 0, 0, data->FroxelBytes() } );
				encoder.CopyBuffer(
				    assigned.Value().indices, indexReadback.Value(), { 0, 0, data->IndexBytes() } );
				auto token = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
				result.ok = token && Wait( device, token.Value() );
				if ( token )
					kernel.Collect( token.Value() );
			}
		}
	}
	else if ( froxelReadback && indexReadback )
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

struct Comparison
{
	std::uint64_t scenes = 0;
	std::uint64_t deviceFailures = 0;
	std::uint64_t shapeErrors = 0;
	std::uint64_t countErrors = 0; // repeated GPU dispatches disagree
	std::uint64_t pairs = 0;
	std::uint64_t mismatches = 0;
	std::string first;

	void Note( const std::string &what )
	{
		if ( first.empty() )
			first = what;
	}
};

// Two GPU dispatches on identical inputs must agree. Mutant kernels use the
// production GPU kernel as the baseline.
void CompareLists( const lights::ClusterGrid &grid, const lights::ClusterLists &baseline,
    const lights::ClusterStats &stats, const DeviceLists &device, Comparison &out )
{
	const std::uint64_t id = out.scenes++;
	if ( !device.ok )
	{
		++out.deviceFailures;
		out.Note( "device run failed (scene " + std::to_string( id ) + ")" );
		return;
	}
	if ( device.ranges.size() != baseline.froxels.size() ||
	     !RangesWellFormed( device, grid.limits.maxLightIndices ) )
	{
		++out.shapeErrors;
		out.Note( "ranges (scene " + std::to_string( id ) + ")" );
		return;
	}
	if ( device.header.requested != stats.assignments )
	{
		++out.countErrors;
		out.Note( "requested count (scene " + std::to_string( id ) + ")" );
	}
	out.pairs += stats.assignments;
	for ( std::uint32_t f = 0; f < baseline.froxels.size(); ++f )
	{
		const auto a = ListOf( baseline, f );
		const auto b = ListOf( device, f );
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
		out.mismatches += onlyOne.size();
		if ( !onlyOne.empty() )
			out.Note( "different GPU lists (scene " + std::to_string( id ) + ")" );
	}
}

// The device's lists compacted in froxel order for geometric checks.
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

using Built = BuildOutput;

Built GpuBaseline( IRenderDevice2 &device, lights::ClusterKernel &kernel, const Scene &scene )
{
	Built built;
	auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
	if ( !grid )
		return built;
	built.grid = std::move( grid ).Value();
	const DeviceLists gpu = AssignOnDevice( device, kernel, built.grid, scene.lights );
	if ( !gpu.ok || !RangesWellFormed( gpu, scene.limits.maxLightIndices ) )
		return built;
	built = Compacted( built.grid, gpu );
	built.stats.froxelsOverflowed = gpu.header.froxelsOverflowed;
	built.stats.assignmentsDropped = gpu.header.assignmentsDropped;
	return built;
}

// The scenes of G1: MakeScene's, with room for every possible pair,
// plus an empty and a full scene.
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
		const auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
		scene.limits.maxLightIndices = std::max<std::uint32_t>(
		    grid.Value().FroxelCount() * std::uint32_t( scene.lights.size() ) + 64, 64 );
		scenes.push_back( std::move( scene ) );
	}
	return scenes;
}

struct LimitResult
{
	std::uint64_t scenes = 0;
	std::uint64_t skipped = 0; // scenes without relevant lights
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
	Scene fullScene = base;
	fullScene.limits.maxLightsPerFroxel = fullScene.limits.maxLights;
	const Built full = GpuBaseline( device, kernel, fullScene );
	Scene scene = fullScene;
	scene.limits.maxLightsPerFroxel = 1 + random() % 8;
	auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
	const DeviceLists lists =
	    grid ? AssignOnDevice( device, kernel, grid.Value(), scene.lights ) : DeviceLists{};
	++out.scenes;
	if ( !full.ok || !lists.ok || lists.ranges.size() != full.lists.froxels.size() )
	{
		++out.listErrors;
		if ( out.first.empty() )
			out.first = "run failed";
		return;
	}
	std::uint32_t dropped = 0, overflowed = 0;
	for ( std::uint32_t f = 0; f < lists.ranges.size(); ++f )
	{
		const auto a = ListOf( full.lists, f );
		const auto b = ListOf( lists, f );
		const std::size_t kept = std::min<std::size_t>( a.size(), scene.limits.maxLightsPerFroxel );
		dropped += std::uint32_t( a.size() - kept );
		overflowed += kept < a.size();
		if ( b.size() != kept || !std::equal( b.begin(), b.end(), a.begin() ) )
		{
			++out.listErrors;
			if ( out.first.empty() )
				out.first = "per-froxel prefix differs";
			return;
		}
	}
	out.overflowed += overflowed;
	if ( lists.header.requested != full.stats.assignments - dropped ||
	     lists.header.assignmentsDropped != dropped ||
	     lists.header.froxelsOverflowed != overflowed )
	{
		++out.counterErrors;
		if ( out.first.empty() )
			out.first = "limit counters differ";
	}
}

// G4: index capacity. Totals are defined; which froxels lose room is not.
void CheckIndexCapacity( IRenderDevice2 &device, lights::ClusterKernel &kernel, const Scene &base,
    std::mt19937 &random, LimitResult &out )
{
	Scene rawScene = base;
	rawScene.limits.maxLightsPerFroxel = rawScene.limits.maxLights;
	const Built raw = GpuBaseline( device, kernel, rawScene );
	Scene unlimited = rawScene;
	unlimited.limits.maxLightsPerFroxel = 1 + random() % 16;
	const Built full = GpuBaseline( device, kernel, unlimited );
	Scene scene = unlimited;
	scene.limits.maxLightIndices =
	    std::max<std::uint32_t>( 1, full.stats.assignments / ( 2 + random() % 4 ) );
	auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
	const DeviceLists lists =
	    grid ? AssignOnDevice( device, kernel, grid.Value(), scene.lights ) : DeviceLists{};
	++out.scenes;
	if ( !raw.ok || !full.ok || !lists.ok || lists.ranges.size() != full.lists.froxels.size() )
	{
		++out.listErrors;
		if ( out.first.empty() )
			out.first = "run failed";
		return;
	}
	std::uint32_t overflowed = 0;
	for ( std::uint32_t f = 0; f < lists.ranges.size(); ++f )
	{
		const auto a = ListOf( full.lists, f );
		const auto b = ListOf( lists, f );
		overflowed += b.size() < ListOf( raw.lists, f ).size();
		if ( b.size() > a.size() || !std::equal( b.begin(), b.end(), a.begin() ) )
		{
			++out.listErrors;
			if ( out.first.empty() )
				out.first = "a list is not a prefix of its per-froxel list";
			return;
		}
	}
	out.overflowed += overflowed;
	const std::uint32_t kept = std::min( full.stats.assignments, scene.limits.maxLightIndices );
	if ( !RangesWellFormed( lists, scene.limits.maxLightIndices ) ||
	     lists.header.requested != full.stats.assignments ||
	     lists.header.assignmentsDropped != raw.stats.assignments - kept ||
	     lists.header.froxelsOverflowed != overflowed )
	{
		++out.counterErrors;
		if ( out.first.empty() )
			out.first = "capacity counters differ";
	}
}

// Repeated GPU dispatches and independent geometry checks. Seeded defects
// compare with the production GPU kernel on the same immutable inputs.
Comparison CompareScenes( IRenderDevice2 &device, lights::ClusterKernel &kernel,
    const std::vector<Scene> &scenes, std::size_t count, std::mt19937 *reference, Tally *tally,
    lights::ClusterKernel *baselineKernel = nullptr )
{
	Comparison comparison;
	for ( std::size_t s = 0; s < count && s < scenes.size(); ++s )
	{
		const Built baseline =
		    GpuBaseline( device, baselineKernel ? *baselineKernel : kernel, scenes[s] );
		if ( !baseline.ok )
		{
			++comparison.deviceFailures;
			comparison.Note( "baseline GPU dispatch failed" );
			continue;
		}
		const DeviceLists lists = AssignOnDevice( device, kernel, baseline.grid, scenes[s].lights );
		CompareLists( baseline.grid, baseline.lists, baseline.stats, lists, comparison );
		if ( tally && reference && s < std::size_t( kReferenceScenes ) && lists.ok )
			CheckScene( scenes[s], Compacted( baseline.grid, lists ), *reference, *tally );
	}
	return comparison;
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::uint32_t seed = Seed();
	std::printf( "INFO seed %u\n", seed );
	std::mt19937 nativeRandom( seed );
	auto nativeView = MakeScene( nativeRandom ).desc;
	nativeView.widthPixels = 7680;
	nativeView.heightPixels = 4320;
	auto nativeGrid = lights::CreateClusterGrid( nativeView, lights::DesktopClusterLimits() );
	checks.That( nativeGrid && nativeGrid.Value().tilesX == 120 &&
	                 nativeGrid.Value().tilesY == 68 && nativeGrid.Value().slices == 24,
	    "native-8k.preserves-tile-and-depth-quality" );
	nativeView.widthPixels = nativeView.heightPixels = 16384;
	auto excessiveGrid = lights::CreateClusterGrid( nativeView, lights::DesktopClusterLimits() );
	checks.That( !excessiveGrid && excessiveGrid.Error() == lights::ClusterError::kTooManyFroxels,
	    "native-8k.capacity-remains-bounded" );
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
		const Comparison g1 =
		    CompareScenes( *device, *kernel.Value(), scenes, scenes.size(), &reference, &tally );
		std::uint64_t froxels = 0;
		std::uint64_t sceneLights = 0;
		for ( const Scene &scene : scenes )
		{
			sceneLights += scene.lights.size();
			auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
			froxels += grid ? grid.Value().FroxelCount() : 0;
		}
		std::printf( "INFO G1 %llu scenes, %llu froxels, %llu lights, %llu GPU assignments; "
		             "%llu mismatches%s%s\n",
		    static_cast<unsigned long long>( g1.scenes ),
		    static_cast<unsigned long long>( froxels ),
		    static_cast<unsigned long long>( sceneLights ),
		    static_cast<unsigned long long>( g1.pairs ),
		    static_cast<unsigned long long>( g1.mismatches ), g1.first.empty() ? "" : ": ",
		    g1.first.c_str() );
		checks.Equal( g1.deviceFailures, std::uint64_t( 0 ), "G1.every-dispatch-runs" );
		checks.Equal( g1.shapeErrors, std::uint64_t( 0 ), "G1.ranges-disjoint-and-covering" );
		checks.Equal( g1.countErrors, std::uint64_t( 0 ), "G1.repeated-request-count-matches" );
		checks.That( g1.scenes == std::uint64_t( kScenes ) && g1.pairs > 1000000,
		    "G1.the-scenes-exercise-the-kernel" );
		checks.Equal( g1.mismatches, std::uint64_t( 0 ), "G1.repeated-gpu-lists-match" );
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
			CheckPerFroxelLimit( *device, *kernel.Value(), scenes[s], random, g3 );
		std::printf( "INFO G3 %llu scenes, %llu froxels "
		             "overflowed%s%s\n",
		    static_cast<unsigned long long>( g3.scenes ),
		    static_cast<unsigned long long>( g3.overflowed ), g3.first.empty() ? "" : ": ",
		    g3.first.c_str() );
		checks.That( g3.scenes >= std::uint64_t( kLimitScenes / 2 ) && g3.overflowed > 0,
		    "G3.the-limit-is-exercised" );
		checks.Equal( g3.listErrors, std::uint64_t( 0 ), "G3.the-same-prefixes" );
		checks.Equal( g3.counterErrors, std::uint64_t( 0 ), "G3.the-same-counters" );

		LimitResult g4;
		for ( int s = kLimitScenes; s < kLimitScenes + kCapacityScenes; ++s )
		{
			if ( scenes[s].lights.empty() )
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
		checks.Equal( g4.listErrors, std::uint64_t( 0 ), "G4.lists-are-gpu-prefixes" );
		checks.Equal( g4.counterErrors, std::uint64_t( 0 ), "G4.totals-equal-discarded-pairs" );
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
				const Comparison seeded = CompareScenes( *device, *broken.Value(), scenes,
				    kDefectScenes, nullptr, nullptr, kernel.Value().get() );
				std::printf( "INFO seeded %s: %llu mismatches, %llu scenes with "
				             "another count%s%s\n",
				    defect.name, static_cast<unsigned long long>( seeded.mismatches ),
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

		// The actual inline consumer path, all 32 BVH children, equal Morton keys,
		// and both words of the area mask. Compare every output, not just counters.
		Scene dense = scenes[1];
		dense.limits.maxLights = 1024;
		dense.limits.maxLightsPerFroxel = 1024;
		dense.limits.maxLightIndices = 1u << 22;
		while ( dense.lights.size() < 1024 )
			dense.lights.push_back( dense.lights[dense.lights.size() % 256] );
		const Built denseReference = GpuBaseline( *device, *kernel.Value(), dense );
		std::vector<area_light::AreaLight> areas( 64 );
		for ( std::size_t i = 0; i < areas.size(); ++i )
		{
			auto &area = areas[i];
			area.rect.center[0] = float( i ) * 13.0f - 300.0f;
			area.rect.center[2] = -100.0f;
			area.rect.halfU[0] = 20.0f;
			area.rect.halfV[1] = 30.0f;
			area.reach = 200.0f + float( i ) * 10.0f;
		}
		const DeviceLists gpu = AssignOnDevice(
		    *device, *kernel.Value(), denseReference.grid, dense.lights, areas, true );
		checks.That( gpu.ok, "G6.inline-consumer-dispatch" );
		bool identical = gpu.ok;
		if ( gpu.ok )
			for ( std::uint32_t f = 0; f < denseReference.grid.FroxelCount(); ++f )
			{
				const auto a = ListOf( denseReference.lists, f ), b = ListOf( gpu, f );
				identical = identical && std::equal( a.begin(), a.end(), b.begin(), b.end() );
			}
		checks.That( identical, "G6.all-1024-light-lists-match" );
		const DeviceLists graphGpu =
		    AssignOnDevice( *device, *kernel.Value(), denseReference.grid, dense.lights, areas );
		bool masks = gpu.ok && graphGpu.ok;
		if ( masks )
		{
			masks = gpu.header.reserved == denseReference.grid.limits.maxLightIndices + 1;
			for ( std::size_t f = 0; f < gpu.ranges.size(); ++f )
				for ( std::size_t word = 0; word < 2; ++word )
					masks =
					    masks &&
					    graphGpu.indices[denseReference.grid.limits.maxLightIndices + 2 * f +
					                     word] ==
					        gpu.indices[denseReference.grid.limits.maxLightIndices + 2 * f + word];
		}
		checks.That( masks, "G6.graph-and-direct-area-masks-match" );

		// Diagnostic full-cost assignment measurements. These are NOT complete-frame
		// performance acceptance: submit/wait is included and no surfaces are drawn.
		if ( std::getenv( "CONFORMANCE_CLUSTER_BENCH" ) )
		{
			bool measured = true;
			for ( const unsigned count : { 0u, 43u, 256u, 1024u } )
			{
				Scene scene = dense;
				scene.lights.resize( count );
				scene.desc.widthPixels = 1920;
				scene.desc.heightPixels = 1080;
				auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
				if ( !grid )
				{
					measured = false;
					continue;
				}
				std::vector<double> hostTimes, gpuTimes;
				graph::GpuPassTimers timers( *device );
				for ( unsigned repeat = 0; repeat < 9; ++repeat )
				{
					auto start = std::chrono::steady_clock::now();
					auto data = lights::PrepareSurfaceClusterDispatch( grid.Value(), scene.lights );
					auto encoded = device->BeginEncoder( QueueKind::kGraphics );
					if ( !encoded )
					{
						measured = false;
						break;
					}
					timers.BeginFrame( repeat + 1, {} );
					timers.Attach( encoded.Value() );
					encoded.Value().BeginLabel( "assignment including uploads" );
					auto output = kernel.Value()->RecordView( encoded.Value(), data );
					encoded.Value().EndLabel();
					timers.Detach( encoded.Value() );
					auto token =
					    device->Submit( QueueKind::kGraphics, { &encoded.Value(), 1 }, {} );
					if ( !output || !token || !Wait( *device, token.Value() ) )
					{
						measured = false;
						break;
					}
					const double hostMs = std::chrono::duration<double, std::milli>(
					    std::chrono::steady_clock::now() - start )
					                          .count();
					timers.EndFrame( token.Value() );
					timers.Collect();
					auto report = timers.Take();
					kernel.Value()->Collect( token.Value() );
					if ( repeat == 0 )
						continue;
					hostTimes.push_back( hostMs );
					for ( const auto &pass : report.passes )
						if ( pass.name == "assignment including uploads" )
							gpuTimes.push_back( pass.milliseconds );
				}
				const auto median = []( std::vector<double> v )
				{
					std::sort( v.begin(), v.end() );
					return v.empty() ? -1.0 : v[v.size() / 2];
				};
				std::printf( "BENCH %u lights %u froxels: GPU upload+build+assign %.3f "
				             "ms; prepare+submit+wait %.3f ms (%zu samples)\n",
				    count, grid.Value().FroxelCount(), median( gpuTimes ), median( hostTimes ),
				    hostTimes.size() );
				measured = measured && hostTimes.size() == 8;
			}
			checks.That( measured, "G7.assignment-measurements-complete" );
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
