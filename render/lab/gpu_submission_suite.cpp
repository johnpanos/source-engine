//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GPU-driven submission through the core world pass (RFC 0016
//          S3/S4), independent of the engine: the same world view drawn
//          per surface and by GPU culling, compaction and one indirect draw
//          per (material, lightmap page) bucket must give the same image.
//
//          The fixture is 400 overlapping quads in six colours at seeded
//          positions and depths, a quarter of them outside the view (the GPU
//          culler removes those), listed with the materials interleaved (the
//          pass groups them by material, as for the per-surface path).
//          Checks: the two images are identical, pixel for pixel; the GPU
//          path drew the view with one indirect draw per material bucket and
//          no fallback; the image is not trivial; validation is silent;
//          asking for occlusion without a stage prepass changes nothing.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/pass/world/world_pass.h"

#include <atomic>
#include <random>
#include <set>

namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::world;
constexpr unsigned kSize = 96;
constexpr unsigned kQuads = 400;

class NoTextures final : public IWorldTextures
{
public:
	TextureId Import( int, bool ) override { return {}; }
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldData Fixture()
{
	WorldData world;
	for ( const char *color : { "[1 0 0]", "[0 1 0]", "[0 0 1]", "[1 1 0]", "[0 1 1]", "[1 0 1]" } )
	{
		WorldMaterial material;
		material.name = color;
		material.shader = "UnlitGeneric";
		material.variables.emplace_back( "$color", color );
		world.materials.push_back( std::move( material ) );
	}
	std::mt19937 random( 1234 );
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	for ( unsigned q = 0; q < kQuads; ++q )
	{
		// A quarter of the quads lie wholly outside clip x or y in [-1, 1].
		const bool outside = q % 4 == 3;
		const float x = outside ? 1.2f + unit( random ) : -1.0f + 1.8f * unit( random );
		const float y = -1.0f + 1.8f * unit( random );
		const float w = 0.05f + 0.2f * unit( random );
		const float z = 0.05f + 0.9f * unit( random );
		const unsigned base = world.vertices.size(), first = world.indices.size();
		for ( auto xy : { std::pair{ x, y }, std::pair{ x + w, y }, std::pair{ x + w, y + w },
		          std::pair{ x, y + w } } )
		{
			WorldVertex v{};
			v.position[0] = xy.first;
			v.position[1] = xy.second;
			v.position[2] = z;
			v.normal[2] = v.tangentS[0] = 1.0f;
			for ( auto &c : v.color )
				c = 255;
			world.vertices.push_back( v );
		}
		world.indices.insert(
		    world.indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 } );
		world.surfaces.push_back( { unsigned( random() % 6 ), 0, first, 6 } );
	}
	return world;
}

// The view's list: runs of a few surfaces of one material, materials
// interleaved (the pass orders them itself).
std::vector<std::uint32_t> ViewList( const WorldData &world )
{
	std::vector<std::vector<std::uint32_t>> byMaterial( world.materials.size() );
	for ( std::uint32_t i = 0; i < world.surfaces.size(); ++i )
		byMaterial[world.surfaces[i].material].push_back( i );
	std::vector<std::uint32_t> list;
	for ( bool more = true; more; )
	{
		more = false;
		for ( auto &surfaces : byMaterial )
		{
			for ( int k = 0; k < 7 && !surfaces.empty(); ++k )
			{
				list.push_back( surfaces.back() );
				surfaces.pop_back();
			}
			more = more || !surfaces.empty();
		}
	}
	return list;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	if ( !device->Facts().capabilities.Has( Capability::kDrawIndirectCount ) )
		return "the lab device does not claim kDrawIndirectCount";
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	TextureDesc depthDesc;
	depthDesc.width = depthDesc.height = kSize;
	depthDesc.format = Format::kD32Float;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	auto depth = device->CreateTexture( depthDesc );
	if ( !depth )
		return "depth attachment unavailable";
	NoTextures imports;
	WorldPass pass;
	const WorldData fixture = Fixture();
	const std::vector<std::uint32_t> list = ViewList( fixture );
	pass.SetWorld( fixture );
	// The pass draws its list grouped by material (one lightmap page here), so
	// a bucket is a material the list names.
	std::set<std::uint32_t> listed;
	for ( std::uint32_t index : list )
		listed.insert( fixture.surfaces[index].material );
	const auto buckets = static_cast<std::uint32_t>( listed.size() );
	std::uint64_t frame = 0;
	bool initialized = false;
	auto render = [&]( bool gpu, CanvasImage &image,
	                  bool occlusion = false ) -> std::optional<std::string>
	{
		WorldView view;
		view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1;
		view.viewport = { 0, 0, kSize, kSize, 0, 1 };
		view.surfaces = list;
		const unsigned tag = pass.QueueView( std::move( view ) );
		if ( tag == 0 )
			return "the world view was not queued";
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId ) -> std::optional<std::string>
		{
			encoder.TransitionTexture( depth.Value(),
			    initialized ? ResourceUsage::kDepthWrite : ResourceUsage::kUndefined,
			    ResourceUsage::kDepthWrite );
			initialized = true;
			RenderingDesc clear;
			clear.width = clear.height = kSize;
			clear.depth = DepthAttachment{ depth.Value(), LoadOp::kClear, StoreOp::kStore, 1 };
			encoder.BeginRendering( clear );
			encoder.EndRendering();
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth.Value();
			target.depthFormat = depthDesc.format;
			target.width = target.height = kSize;
			target.textures = &imports;
			target.frame = ++frame;
			target.drawState.overrideDepth = true;
			target.drawState.depthTest = true;
			target.drawState.depthWrite = true;
			target.drawState.depthCompare = CompareOp::kLess;
			target.gpuSubmission = gpu;
			target.gpuOcclusion = occlusion;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, &image, post );
	};
	CanvasImage perSurface, gpuDriven;
	if ( auto why = render( false, perSurface ) )
		return why;
	const WorldStats before = pass.Stats();
	if ( auto why = render( true, gpuDriven ) )
		return why;
	const WorldStats after = pass.Stats();
	results.That( perSurface.rgba == gpuDriven.rgba,
	    "gpu-submission.indirect-image-equals-per-surface-image" );
	results.That( after.gpuViews == before.gpuViews + 1 && after.gpuFallbacks == 0,
	    "gpu-submission.the-view-was-drawn-gpu-driven" );
	results.That( after.gpuIndirectDraws - before.gpuIndirectDraws == buckets,
	    "gpu-submission.one-indirect-draw-per-bucket",
	    std::to_string( after.gpuIndirectDraws - before.gpuIndirectDraws ) + " of " +
	        std::to_string( buckets ) );
	std::set<std::tuple<int, int, int>> colours;
	for ( std::uint32_t y = 0; y < kSize; ++y )
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const float *p = gpuDriven.At( x, y );
			colours.insert( { int( p[0] > 0.5f ), int( p[1] > 0.5f ), int( p[2] > 0.5f ) } );
		}
	// Occlusion needs a world stage's screen prepass; without one the view
	// is frustum-culled only, with the same image.
	CanvasImage noStage;
	if ( auto why = render( true, noStage, true ) )
		return why;
	results.That( noStage.rgba == gpuDriven.rgba && pass.Stats().gpuOcclusionViews == 0 &&
	                  pass.Stats().gpuViews == after.gpuViews + 1,
	    "gpu-submission.occlusion-without-a-stage-prepass-is-frustum-culling" );
	results.That( colours.size() >= 7, "gpu-submission.the-image-shows-every-colour-and-clear",
	    std::to_string( colours.size() ) );
	results.That( buckets == 6, "gpu-submission.every-material-is-a-bucket" );
	results.That(
	    pass.Stats().viewsFailed == 0, "gpu-submission.no-lost-views", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	(void)device->Release( depth.Value(), {} );
	messages = counter.load();
	results.That( messages == 0, "gpu-submission.validation-silent" );
	return std::nullopt;
}
} // namespace

int RunGpuSubmissionSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "gpu-submission", std::span<const Seeded>(), RunChecks );
}
} // namespace render::lab
