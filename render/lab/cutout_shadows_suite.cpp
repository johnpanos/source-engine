//========= Copyright Valve Corporation, All rights reserved. ============//
// Native shadow coverage against texture texels and an independent foliage oracle.
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "tree_sway_reference.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"
#include "render/pass/shadows/shadow_passes.h"
#include "spv/cutout_shadow_defects_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::lab
{
namespace
{
using namespace device;
namespace shadows = pass::shadows;
constexpr unsigned kSize = 64;
constexpr unsigned kTexture = 8;
constexpr unsigned char kAlpha[] = { 0, 96, 160, 255 };
struct Owned
{
	IRenderDevice2 &device;
	std::vector<ResourceId> ids = {};
	CompletionToken token = {};
	~Owned()
	{
		(void)device.WaitIdle();
		for ( ResourceId id : ids )
			(void)device.Release( id, token );
	}
};
struct Vertex
{
	math::float3 position;
	float uv[2];
};
const Vertex kQuad[] = { { { -.8f, -.8f, .4f }, { 0, 1 } }, { { .8f, -.8f, .4f }, { 1, 1 } },
    { { .8f, .8f, .4f }, { 1, 0 } }, { { -.8f, .8f, .4f }, { 0, 0 } } };
const std::uint32_t kIndices[] = { 0, 1, 2, 0, 2, 3 };

std::optional<std::string> Check( IRenderDevice2 &device, std::span<const std::uint32_t> module,
    bool model, int transform, int sway, bool alphaTest, Results &results, double reference = .5,
    bool multisample = false, bool rooted = false )
{
	Owned owned{ device };
	resources::TextureCache textures( device );
	material::GroupResidency groups( device, textures );
	resources::MeshCache meshes( device );
	TextureDesc image;
	image.format = Format::kRGBA8Unorm;
	image.width = image.height = kTexture;
	image.usages = { ResourceUsage::kSampled, ResourceUsage::kCopyDestination };
	std::vector<unsigned char> texels( kTexture * kTexture * 4, 255 );
	for ( unsigned y = 0; y < kTexture; ++y )
		for ( unsigned x = 0; x < kTexture; ++x )
			texels[( y * kTexture + x ) * 4 + 3] = kAlpha[( x + 3 * y ) % 4];
	if ( !textures.Stage( "materials/coverage", image, std::as_bytes( std::span( texels ) ) ) )
		return "coverage image refused";
	auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
	    Format::kD32Float, multisample ? 4 : 1,
	    model ? material::VertexLayout::kModel : material::VertexLayout::kSurface, { {}, module } );
	if ( !resolver )
		return resolver.Error();
	resolver.Value()->SetWorldPbr( true );
	std::vector<material::VmtPair> variables = { { "$basetexture", "coverage" },
	    { "$alphatest", alphaTest ? "1" : "0" },
	    { "$alphatestreference", std::to_string( reference ) } };
	if ( !model )
		variables.push_back( { "$vertexcolor", "1" } );
	if ( transform )
		variables.push_back( { "$basetexturetransform",
		    transform == 1 ? "[ 1 0 0 .125 0 .75 0 .125 0 0 1 0 0 0 0 1 ]"
		                   : "[ 0 -1 0 1.125 1 0 0 -.25 0 0 1 0 0 0 0 1 ]" } );
	if ( sway )
	{
		variables.insert( variables.end(),
		    { { "$treesway", std::to_string( sway ) },
		        { "$treeswayheight", sway == 1 ? "1" : "-1" }, { "$treeswayradius", ".6" },
		        { "$treeswaystrength", ".07" }, { "$treeswayscrumblestrength", ".04" } } );
	}
	auto desc = material::MapVariables( model       ? "VertexLitGeneric"
	                                    : transform ? "UnlitGeneric"
	                                                : "LightmappedGeneric",
	    std::move( variables ), {} );
	if ( !desc )
		return "coverage VMT refused";
	auto resolved = model ? resolver.Value()->ResolveMesh( desc.Value() )
	                      : resolver.Value()->Resolve( desc.Value() );
	if ( !resolved )
		return resolved.Error();
	auto pipeline = resolver.Value()->Program().ShadowPipeline( resolved.Value().request.pipeline );
	if ( !pipeline )
		return "coverage depth pipeline refused: " + resolver.Value()->Program().PipelineFailure();
	const auto &request = resolved.Value().request;
	material::SurfaceConstants constants;
	std::memcpy( &constants, request.material.constants.data(), sizeof( constants ) );
	material::FrameTerms terms;
	terms.foliageAvailable = true;
	for ( auto &sample : terms.foliage )
	{
		sample[0] = .7f;
		sample[1] = -.4f;
		sample[2] = 1.3f;
	}
	// The previous frame differs: a depth caster must use the current wind/time.
	terms.foliage[1][0] = -.3f;
	terms.foliage[1][1] = .8f;
	terms.foliage[1][2] = .6f;
	auto frame = resolver.Value()->FrameGroup( resolved.Value(), terms );
	auto draw = resolver.Value()->Program().DrawGroup( "" );
	if ( !frame || !request.neutralView )
		return "coverage frame refused";
	material::GroupRequest materialGroup = request.material;
	for ( auto &texture : materialGroup.textures )
		texture.sampler.minFilter = texture.sampler.magFilter = texture.sampler.mipFilter =
		    Filter::kNearest;
	if ( !groups.Set( 1, materialGroup ) || !groups.Set( 2, *frame ) ||
	     !groups.Set( 3, *request.neutralView ) || !groups.Set( 4, draw ) )
		return "coverage group refused";
	for ( const auto &input : materialGroup.textures )
		if ( !input.name.empty() && !textures.Find( input.name ) )
			return "coverage fixture is missing texture: " + input.name;
	std::vector<material::SurfaceModelVertex> modelVertices;
	std::vector<material::SurfaceWorldVertex> worldVertices;
	for ( const Vertex &source : kQuad )
	{
		if ( model )
		{
			material::SurfaceModelVertex vertex{};
			vertex.position[0] = source.position.x;
			vertex.position[1] = source.position.y;
			vertex.position[2] = sway == 2 ? -.4f : source.position.z;
			vertex.normal[2] = vertex.tangent[0] = vertex.tangent[3] = 1;
			std::copy_n( source.uv, 2, vertex.uv );
			modelVertices.push_back( vertex );
		}
		else
		{
			material::SurfaceWorldVertex vertex{};
			vertex.position[0] = source.position.x;
			vertex.position[1] = source.position.y;
			vertex.position[2] = source.position.z;
			std::copy_n( source.uv, 2, vertex.uv );
			std::fill_n( vertex.color, 4, 255 );
			vertex.color[3] = 128;
			worldVertices.push_back( vertex );
		}
	}
	resources::MeshData data;
	data.vertices = model ? std::as_bytes( std::span( modelVertices ) )
	                      : std::as_bytes( std::span( worldVertices ) );
	data.vertexStride = request.vertexStride;
	data.indices = std::as_bytes( std::span( kIndices ) );
	data.indexFormat = IndexFormat::kUint32;
	auto mesh = meshes.Stage( "coverage", data );
	if ( !mesh )
		return "coverage mesh refused";
	auto commands = device.BeginEncoder( QueueKind::kGraphics );
	if ( !commands )
		return "coverage upload encoder refused";
	textures.RecordUploads( commands.Value() );
	groups.RecordUploads( commands.Value() );
	meshes.RecordUploads( commands.Value() );
	auto submitted = device.Submit( QueueKind::kGraphics, { &commands.Value(), 1 }, {} );
	if ( !submitted || !device.WaitIdle() )
		return "coverage upload failed";
	owned.token = submitted.Value();
	for ( std::uint64_t id : { 1u, 2u, 3u, 4u } )
		if ( !groups.Group( id ) )
			return "coverage group not resident: " + std::to_string( id );
	shadows::ShadowMaterial coverage;
	coverage.program.pipeline = pipeline.Value();
	coverage.program.material = *groups.Group( 1 );
	coverage.program.vertexStride = request.vertexStride;
	coverage.program.drawConstantBytes = request.drawConstantBytes;
	coverage.program.frameLayout = request.frameLayout;
	coverage.program.viewLayout = request.viewLayout;
	coverage.program.drawLayout = request.drawLayout;
	coverage.program.neutralView = *groups.Group( 3 );
	coverage.program.hasNeutralView = true;
	coverage.frame = *groups.Group( 2 );
	coverage.draw = { request.drawLayout, *groups.Group( 4 ) };
	shadows::ShadowCaster caster;
	caster.mesh = mesh.Value();
	caster.material = coverage;
	if ( rooted )
	{
		caster.world.rows[0] = { 0, -1, 0, .1f };
		caster.world.rows[1] = { 1, 0, 0, -.2f };
		caster.world.rows[2].w = .02f;
	}
	math::float4x4 view;
	if ( sway == 2 )
		view.rows[2].z = -1;
	TextureDesc atlas;
	atlas.format = Format::kD32Float;
	atlas.width = atlas.height = kSize;
	atlas.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kCopySource };
	auto target = device.CreateTexture( atlas );
	BufferDesc buffer;
	buffer.size = kSize * kSize * sizeof( float );
	buffer.memory = MemoryKind::kReadback;
	buffer.usages = { ResourceUsage::kCopyDestination };
	auto readback = device.CreateBuffer( buffer );
	if ( target )
		owned.ids.emplace_back( target.Value() );
	if ( readback )
		owned.ids.emplace_back( readback.Value() );
	if ( !target || !readback )
		return "coverage target refused";
	auto depth = shadows::ShadowDepthRenderer::Create( device );
	if ( !depth )
		return "coverage renderer refused";
	graph::GraphBuilder builder;
	const auto atlasRef = builder.ImportTexture( "coverage-atlas", target.Value(), atlas,
	    ResourceUsage::kUndefined, ResourceUsage::kCopySource );
	shadows::ShadowDepthView depthView{ view, { 0, 0, kSize }, { &caster, 1 } };
	auto stats = depth.Value()->AddPasses( builder, { atlasRef, kSize, 0 }, { &depthView, 1 } );
	if ( !stats )
		return "coverage pass refused";
	const auto bufferRef = builder.ImportBuffer( "coverage-readback", readback.Value(), buffer,
	    ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	builder.AddPass( "coverage-readback", graph::PassKind::kCopy )
	    .Read( atlasRef, ResourceUsage::kCopySource )
	    .Write( bufferRef, ResourceUsage::kCopyDestination )
	    .Execute(
	        [atlasRef, bufferRef]( graph::RecordContext &context )
	        {
		        context.Encoder().CopyTextureToBuffer( context.Texture( atlasRef ),
		            context.Buffer( bufferRef ), { 0, 0, 0, kSize, kSize } );
	        } );
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return "coverage graph refused";
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), device );
	if ( !executed || !device.WaitIdle() )
		return "coverage graph failed";
	owned.token = executed.Value().token;
	groups.Retire( owned.token );
	textures.Retire( owned.token );
	meshes.Retire( owned.token );
	depth.Value()->Collect( owned.token );
	std::vector<float> pixels( kSize * kSize );
	if ( !device.ReadBuffer( readback.Value(), 0, std::as_writable_bytes( std::span( pixels ) ) ) )
		return "coverage depth readback failed";
	std::array<std::array<double, 3>, 4> positions;
	for ( int i = 0; i < 4; ++i )
	{
		tree_sway_oracle::Case c;
		c.positionTime[0] = kQuad[i].position.x;
		c.positionTime[1] = kQuad[i].position.y;
		c.positionTime[2] = sway == 2 ? -.4f : .4f;
		c.positionTime[3] = terms.foliage[0][2];
		c.windMode[0] = terms.foliage[0][0];
		c.windMode[1] = terms.foliage[0][1];
		c.windMode[2] = float( sway );
		std::copy_n( constants.treeGeometry, 4, c.geometry );
		std::copy_n( constants.treeMotion, 4, c.motion );
		std::copy_n( constants.treeCurves, 4, c.curves );
		std::copy_n( constants.treeWind, 4, c.windControls );
		for ( unsigned row = 0; row < 4; ++row )
			std::copy_n( &caster.world.rows[row].x, 4, c.objectToWorld + row * 4 );
		positions[i] = tree_sway_oracle::Reference( c );
		if ( rooted )
		{
			const auto local = positions[i];
			positions[i] = { -local[1] + .1f, local[0] - .2f, local[2] + .02f };
		}
		if ( sway == 2 )
			positions[i][2] = -positions[i][2];
	}
	unsigned judged = 0, holes = 0, solid = 0, wrong = 0;
	for ( unsigned y = 0; y < kSize; ++y )
		for ( unsigned x = 0; x < kSize; ++x )
		{
			const double px = 2 * ( x + .5 ) / kSize - 1;
			const double py = 1 - 2 * ( y + .5 ) / kSize;
			for ( int triangle = 0; triangle < 2; ++triangle )
			{
				const int a = kIndices[triangle * 3], b = kIndices[triangle * 3 + 1],
				          d = kIndices[triangle * 3 + 2];
				const auto &p = positions[a], &q = positions[b], &r = positions[d];
				const double determinant =
				    ( q[1] - r[1] ) * ( p[0] - r[0] ) + ( r[0] - q[0] ) * ( p[1] - r[1] );
				const double wa =
				    ( ( q[1] - r[1] ) * ( px - r[0] ) + ( r[0] - q[0] ) * ( py - r[1] ) ) /
				    determinant;
				const double wb =
				    ( ( r[1] - p[1] ) * ( px - r[0] ) + ( p[0] - r[0] ) * ( py - r[1] ) ) /
				    determinant;
				const double wd = 1 - wa - wb;
				if ( std::min( { wa, wb, wd } ) < .04 )
					continue;
				const double u = wa * kQuad[a].uv[0] + wb * kQuad[b].uv[0] + wd * kQuad[d].uv[0];
				const double v = wa * kQuad[a].uv[1] + wb * kQuad[b].uv[1] + wd * kQuad[d].uv[1];
				const double s = transform == 2 ? 1.125 - v : u + ( transform == 1 ? .125 : 0 );
				const double t = transform == 2 ? u - .25 : transform == 1 ? .75 * v + .125 : v;
				if ( std::abs( s * kTexture - std::round( s * kTexture ) ) < .04 ||
				     std::abs( t * kTexture - std::round( t * kTexture ) ) < .04 )
					continue;
				const auto tx = unsigned( std::floor( ( s - std::floor( s ) ) * kTexture ) );
				const auto ty = unsigned( std::floor( ( t - std::floor( t ) ) * kTexture ) );
				const double alpha =
				    kAlpha[( tx + 3 * ty ) % 4] / 255.0 * ( model || transform ? 1 : 128.0 / 255 );
				const bool covered = !alphaTest || alpha >= std::floor( reference * 255 ) / 255;
				const double expected = covered ? wa * p[2] + wb * q[2] + wd * r[2] : 1;
				++judged;
				covered ? ++solid : ++holes;
				wrong += std::abs( pixels[y * kSize + x] - expected ) > 2e-5;
				break;
			}
		}
	const std::string name = "cutout-shadow." + std::string( model ? "model" : "world" ) + "." +
	                         std::to_string( transform ) + "." + std::to_string( sway ) + "." +
	                         std::to_string( alphaTest ) + "." + std::to_string( reference ) +
	                         ( multisample ? ".4x-color" : ".1x-color" ) +
	                         ( rooted ? ".rotated-translated-root" : "" );
	results.That(
	    judged > 500 && solid > 100 && ( !alphaTest || holes > 100 ), name + ".nonzero-regions" );
	results.That( wrong == 0, name + ".independent-coverage-depth",
	    std::to_string( wrong ) + " of " + std::to_string( judged ) );
	results.That( stats.Value().draws == 1 && stats.Value().views == 1 &&
	                  depth.Value()->RecordFailures() == 0,
	    name + ".recorded-complete" );
	shadows::ShadowCaster broken = caster;
	broken.material->program.material.group = {};
	graph::GraphBuilder refused;
	const auto refusedAtlas = refused.ImportTexture(
	    "refused", target.Value(), atlas, ResourceUsage::kCopySource, ResourceUsage::kCopySource );
	shadows::ShadowDepthView refusedView{ view, { 0, 0, kSize }, { &broken, 1 } };
	auto failure =
	    depth.Value()->AddPasses( refused, { refusedAtlas, kSize, 0 }, { &refusedView, 1 } );
	results.That( !failure && failure.Error() == shadows::ShadowPassStatus::kInvalidCaster &&
	                  refused.Passes().empty(),
	    name + ".missing-coverage-refused-before-mutation" );
	if ( model && !transform && !sway && alphaTest && reference == .5 )
	{
		const char *defects[] = { "frame", "view", "draw", "layout", "stride", "texture" };
		for ( unsigned defect = 0; defect < std::size( defects ); ++defect )
		{
			broken = caster;
			switch ( defect )
			{
			case 0:
				broken.material->frame.group = {};
				break;
			case 1:
				broken.material->program.neutralView.group = {};
				break;
			case 2:
				broken.material->draw.resident.group = {};
				break;
			case 3:
				broken.material->draw.layout = {};
				break;
			case 4:
				++broken.material->program.vertexStride;
				break;
			case 5:
				broken.material->program.material.textures[0].texture = {};
				break;
			}
			graph::GraphBuilder rejected;
			const auto ref = rejected.ImportTexture( "rejected", target.Value(), atlas,
			    ResourceUsage::kCopySource, ResourceUsage::kCopySource );
			shadows::ShadowDepthView invalid{ view, { 0, 0, kSize }, { &broken, 1 } };
			auto verdict = depth.Value()->AddPasses( rejected, { ref, kSize, 0 }, { &invalid, 1 } );
			results.That( !verdict &&
			                  verdict.Error() == shadows::ShadowPassStatus::kInvalidCaster &&
			                  rejected.Passes().empty(),
			    "cutout-shadow.required-" + std::string( defects[defect] ) + "-refused" );
		}
		for ( const char *flag : { "$translucent", "$additive", "$alpha" } )
		{
			auto blended = material::MapVariables( "VertexLitGeneric",
			    { { "$basetexture", "coverage" },
			        { flag, std::strcmp( flag, "$alpha" ) == 0 ? ".25" : "1" } },
			    {} );
			if ( !blended )
				return "blended fixture did not map";
			auto point = resolver.Value()->ResolveMesh( blended.Value() );
			if ( !point )
				return "blended fixture did not resolve: " + point.Error();
			auto refusedPoint =
			    resolver.Value()->Program().ShadowPipeline( point.Value().request.pipeline );
			results.That(
			    !refusedPoint && refusedPoint.Error() == material::SurfaceStatus::kInvalidRequest,
			    "cutout-shadow.blended-" + std::string( flag ) + "-refused" );
		}
	}
	return std::nullopt;
}
std::optional<std::string> Run( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	for ( bool model : { false, true } )
		for ( int transform : { 0, 1, 2 } )
		{
			if ( model && transform )
				continue;
			if ( auto why = Check( *device, module, model, transform, 0, true, results ) )
				return why;
		}
	for ( int sway : { 1, 2 } )
		if ( auto why = Check( *device, module, true, 0, sway, true, results ) )
			return why;
	for ( int sway : { 1, 2 } )
		if ( auto why = Check( *device, module, true, 0, sway, true, results, .5, false, true ) )
			return why;
	if ( auto why = Check( *device, module, true, 0, 0, false, results ) )
		return why;
	if ( auto why = Check( *device, module, true, 0, 0, true, results, .65, true ) )
		return why;
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
const Seeded kSeeded[] = { { "alpha-ignored", spirv::kShadowAlphaIgnored, "cutout-shadow." } };
}
int RunCutoutShadowsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "cutout-shadows", kSeeded, Run );
}
}
