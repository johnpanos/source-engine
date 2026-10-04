//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent double-precision oracle for Source foliage vertex
// animation, including the published intro4 map's negative-height vines.
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "spv/tree_sway_check_spv.h"

#include "render/material/program_resolver.h"
#include "render/material/vertexlit_family.h"
#include "render/material/vmt_import.h"
#include "render/pass/world/world_pass.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <limits>
#include <vector>

namespace render::lab
{
namespace
{
struct Case
{
	float positionTime[4] = {};
	float windMode[4] = {};
	float geometry[4] = { 1000.0f, 0.1f, 300.0f, 0.2f };
	float motion[4] = { 1.0f, 10.0f, 12.0f, 10.0f };
	float curves[4] = { 2.0f, 1.0f, 1.5f, 5.0f };
	float windControls[4] = { 3.0f, 6.0f, 0.0f, 0.0f };
	float objectToWorld[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};
static_assert( sizeof( Case ) == 160 );

std::array<double, 3> Reference( const Case &c )
{
	std::array<double, 3> out = { c.positionTime[0], c.positionTime[1], c.positionTime[2] };
	const int mode = int( c.windMode[2] );
	if ( mode == 0 )
		return out;
	const double wx = c.windControls[2] ? 0.5 : c.windMode[0];
	const double wy = c.windControls[2] ? 0.5 : c.windMode[1];
	const double intensity = std::hypot( wx, wy );
	double wind[3];
	for ( int axis = 0; axis < 3; ++axis )
		wind[axis] = c.objectToWorld[axis] * wx + c.objectToWorld[4 + axis] * wy;
	const double x = out[0], y = out[1], z = out[2], time = c.positionTime[3];
	const double heightScale = std::clamp( ( z - double( c.geometry[0] ) * c.geometry[1] ) /
	                                           ( ( 1.0 - c.geometry[1] ) * c.geometry[0] ),
	    0.0, 1.0 );
	const double radiusStart = double( c.geometry[2] ) * c.geometry[3];
	const double radiusScale = std::clamp( std::hypot( x - radiusStart, y - radiusStart ) /
	                                           ( ( 1.0 - c.geometry[3] ) * c.geometry[2] ),
	    0.0, 1.0 );
	const bool active = mode == 2 ? z <= double( c.geometry[0] ) * c.geometry[1]
	                              : z >= double( c.geometry[0] ) * c.geometry[1];
	const double orthogonal =
	    1.0 - std::clamp( std::abs( wind[0] * x + wind[1] * y ) /
	                          ( std::max( std::hypot( wind[0], wind[1], wind[2] ), 0.0001 ) *
	                              std::max( std::hypot( x, y ), 0.0001 ) ),
	              0.0, 1.0 );
	const double trunk = c.motion[1] * std::pow( heightScale, c.curves[2] );
	const double branch = mode == 2 ? 0.0 : c.motion[1] * orthogonal * radiusScale * active;
	const double phase =
	    ( double( c.objectToWorld[3] ) + c.objectToWorld[7] + c.objectToWorld[11] ) * 19.0;
	const double slowTime = ( time + phase ) * c.motion[0];
	const double t = std::clamp(
	    ( intensity - c.windControls[0] ) / ( c.windControls[1] - c.windControls[0] ), 0.0, 1.0 );
	const double blend = t * t * ( 3.0 - 2.0 * t );
	const double sway0 =
	    std::lerp( std::sin( slowTime ), std::sin( slowTime * c.curves[0] ), blend );
	const double sway1 =
	    std::lerp( std::sin( slowTime * 2.31 ), std::sin( slowTime * 2.14 * c.curves[0] ), blend );
	const double scramble = c.motion[3] * std::pow( radiusScale, c.curves[1] ) * active;
	const double length = std::max( std::hypot( x, y, z ), 0.0001 );
	const double position[] = { x, y, z };
	for ( int axis = 0; axis < 3; ++axis )
	{
		out[axis] += wind[axis] * ( trunk * ( sway0 + 0.1 ) + branch * ( sway1 + 0.4 ) );
		const double scale = mode == 2 && axis < 2 ? 0.5 : 1.0;
		out[axis] += intensity * scramble * scale *
		             std::sin( c.curves[3] * time +
		                       position[( axis + 1 ) % 3] / length * c.motion[2] + phase );
	}
	return out;
}

class EmptyTextures final : public pass::world::IWorldTextures
{
public:
	device::TextureId Import( int, bool ) override { return {}; }
	device::SamplerDesc Sampler( int ) override { return {}; }
};

void CheckClaims( Results &results )
{
	using namespace material;
	FamilyRegistry registry;
	for ( const auto &desc : FamiliesFromMapping( BuiltinVmtMapping() ) )
		(void)registry.Register( desc );
	const auto *family = registry.Find( "vertexlit" );
	results.That( family != nullptr, "tree-sway.claim.family" );
	if ( !family )
		return;
	ParameterBlock defaults( *family );
	(void)defaults.SetInt( "treesway", 2 );
	const auto initial = ClaimVertexLitMesh( defaults );
	results.That( initial.claimed && initial.Variant().treeSwayMode == 2 &&
	                  initial.constants.treeGeometry[0] == 1000 &&
	                  initial.constants.treeMotion[3] == 10 &&
	                  initial.constants.treeCurves[2] == 1.5f && initial.constants.treeWind[1] == 6,
	    "tree-sway.claim.source-defaults" );
	(void)defaults.SetFloat( "treeswayheight", -300 );
	results.That( ClaimVertexLitMesh( defaults ).claimed, "tree-sway.claim.negative-height" );
	(void)defaults.SetInt( "lowqualityflashlightshadows", 1 );
	const auto hinted = ClaimVertexLitMesh( defaults );
	(void)defaults.SetInt( "lowqualityflashlightshadows", 0 );
	const auto full = ClaimVertexLitMesh( defaults );
	results.That(
	    hinted.claimed && full.claimed &&
	        std::memcmp( &hinted.constants, &full.constants, sizeof( full.constants ) ) == 0,
	    "tree-sway.claim.flashlight-cost-hint-keeps-full-native-filter" );
	for ( int mode : { -1, 3 } )
	{
		ParameterBlock block( *family );
		(void)block.SetInt( "treesway", mode );
		const auto claim = ClaimVertexLitMesh( block );
		results.That( !claim.claimed && claim.reason.find( "$treesway" ) != std::string::npos,
		    "tree-sway.claim.invalid-mode." + std::to_string( mode ) );
	}
	const std::pair<const char *, float> invalid[] = { { "treeswayheight", 0 },
	    { "treeswayradius", 0 }, { "treeswaystartheight", 1 }, { "treeswaystartradius", -1 },
	    { "treeswayspeed", -1 }, { "treeswaystrength", -1 }, { "treeswayscrumblefrequency", -1 },
	    { "treeswayscrumblestrength", -1 }, { "treeswayspeedhighwindmultiplier", -1 },
	    { "treeswayscrumblefalloffexp", 0 }, { "treeswayfalloffexp", 0 },
	    { "treeswayscrumblespeed", -1 }, { "treeswayspeedlerpstart", -1 },
	    { "treeswayspeedlerpend", 3 }, { "treeswayheight", std::numeric_limits<float>::infinity() },
	    { "treeswaystrength", std::numeric_limits<float>::quiet_NaN() } };
	for ( const auto &[key, value] : invalid )
	{
		ParameterBlock block( *family );
		(void)block.SetInt( "treesway", 1 );
		(void)block.SetFloat( key, value );
		const auto claim = ClaimVertexLitMesh( block );
		results.That(
		    !claim.claimed && claim.reason.find( std::string( "$" ) + key ) != std::string::npos,
		    std::string( "tree-sway.claim.invalid." ) + key );
	}
	ResolvedProgram program;
	program.foliage = true;
	FrameTerms frame;
	results.That( FrameInputError( program, frame ).has_value(), "tree-sway.frame.missing" );
	frame.foliageAvailable = true;
	results.That( !FrameInputError( program, frame ), "tree-sway.frame.calm-valid" );
	for ( int sample = 0; sample < 2; ++sample )
		for ( int axis = 0; axis < 4; ++axis )
		{
			frame.foliage[sample][axis] = std::numeric_limits<float>::quiet_NaN();
			results.That( FrameInputError( program, frame ).has_value(),
			    "tree-sway.frame.nonfinite." + std::to_string( sample * 4 + axis ) );
			frame.foliage[sample][axis] = 0;
		}
}

// Canvas::Render waits for the submission. Even a failed fixture drains before
// this local owner retires the motion targets and readback buffer.
struct MotionReadback
{
	device::IRenderDevice2 &device;
	device::TextureId motion, depth;
	device::BufferId readback;
	explicit MotionReadback( device::IRenderDevice2 &owner ) : device( owner ) {}
	~MotionReadback()
	{
		(void)device.WaitIdle();
		for ( auto id : { motion, depth } )
			if ( id.IsValid() )
				(void)device.Release( id, {} );
		if ( readback.IsValid() )
			(void)device.Release( readback, {} );
	}
	bool Create()
	{
		device::TextureDesc desc;
		desc.width = desc.height = 64;
		desc.format = device::Format::kRG16Float;
		desc.usages = {
		    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
		auto image = device.CreateTexture( desc );
		if ( image )
			motion = image.Value();
		desc.format = device::Format::kR32Float;
		auto surface = device.CreateTexture( desc );
		if ( surface )
			depth = surface.Value();
		device::BufferDesc buffer;
		buffer.size = 64 * 64 * 4;
		buffer.memory = device::MemoryKind::kReadback;
		buffer.usages = { device::ResourceUsage::kCopyDestination };
		auto created = device.CreateBuffer( buffer );
		if ( created )
			readback = created.Value();
		return image && surface && created;
	}
};

void CheckMotion( const Case &current, const pass::world::WorldData::StaticMesh &mesh,
    const CanvasImage &image, std::span<const std::uint16_t> pixels, Results &results,
    const std::string &prefix, bool history )
{
	std::array<std::array<double, 3>, 4> now, before;
	Case c = current;
	for ( int vertex = 0; vertex < 4; ++vertex )
	{
		std::copy_n( mesh.vertices[vertex].position, 3, c.positionTime );
		now[vertex] = Reference( c );
		Case previous = c;
		previous.positionTime[3] = .2f;
		previous.windMode[0] = -.3f;
		previous.windMode[1] = .6f;
		before[vertex] = Reference( previous );
	}
	std::size_t checked = 0;
	double worst = 0, signal = 0;
	bool matches = true;
	for ( unsigned y = 0; y < 64; ++y )
		for ( unsigned x = 0; x < 64; ++x )
		{
			if ( image.At( x, y )[0] < .5f )
				continue;
			// Source's half-pixel shift puts pixel centers on integer coordinates.
			const double px = double( x ) / 32 - 1, py = 1 - double( y ) / 32;
			for ( int triangle = 0; triangle < 2; ++triangle )
			{
				const int indexes[] = { 0, triangle + 1, triangle + 2 };
				const auto &a = now[indexes[0]], &b = now[indexes[1]], &d = now[indexes[2]];
				const double denominator =
				    ( b[1] - d[1] ) * ( a[0] - d[0] ) + ( d[0] - b[0] ) * ( a[1] - d[1] );
				const double u =
				    ( ( b[1] - d[1] ) * ( px - d[0] ) + ( d[0] - b[0] ) * ( py - d[1] ) ) /
				    denominator;
				const double v =
				    ( ( d[1] - a[1] ) * ( px - d[0] ) + ( a[0] - d[0] ) * ( py - d[1] ) ) /
				    denominator;
				const double weights[] = { u, v, 1 - u - v };
				if ( *std::min_element( weights, weights + 3 ) < .01 )
					continue;
				++checked;
				for ( int axis = 0; axis < 2; ++axis )
				{
					double expected = 0;
					for ( int i = 0; i < 3; ++i )
						expected +=
						    weights[i] * ( before[indexes[i]][axis] - now[indexes[i]][axis] );
					expected *= axis == 0 ? 32 : -32;
					signal = std::max( signal, std::abs( expected ) );
					if ( !history )
						expected = 65504;
					const float value = HalfToFloat( pixels[( y * 64 + x ) * 2 + axis] );
					const double error = std::abs( value - expected );
					worst = std::max( worst, error );
					matches = matches && std::isfinite( value ) && error < .003;
				}
				break;
			}
		}
	results.That(
	    checked > 800 && signal > .1, prefix + ".motion-oracle-has-coverage-and-animation" );
	results.That( matches,
	    prefix + ( history ? ".previous-deformation-motion" : ".reset-motion-invalid" ),
	    "worst pixel error " + std::to_string( worst ) );
}

std::optional<std::string> CheckImages( device::IRenderDevice2 &device, Results &results )
{
	using namespace pass::world;
	resources::TextureCache textures( device );
	material::GroupResidency groups( device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( device, 64, 64, canvas ) )
		return why;
	EmptyTextures imports;
	for ( int mode : { 1, 2 } )
	{
		Case c;
		c.geometry[0] = mode == 1 ? 1.0f : -1.0f;
		c.geometry[2] = 1;
		c.motion[0] = .2f;
		c.motion[1] = .15f;
		c.motion[2] = 2;
		c.motion[3] = .03f;
		c.positionTime[2] = mode == 1 ? .5f : -.5f;
		c.positionTime[3] = .7f;
		c.windMode[0] = .4f;
		c.windMode[1] = .3f;
		c.windMode[2] = float( mode );
		c.objectToWorld[11] = mode == 1 ? 0.0f : 1.0f;
		WorldData world;
		auto stage = std::make_shared<WorldStage>();
		stage->lightmap.width = stage->lightmap.height = 1;
		stage->lightmap.flat.resize( 8 );
		world.stage = std::move( stage );
		WorldMaterial foliage;
		foliage.name = "tree-sway-image";
		foliage.shader = "VertexLitGeneric";
		foliage.mesh = true;
		foliage.variables = { { "$selfillum", "1" }, { "$treesway", std::to_string( mode ) },
		    { "$treeswayheight", std::to_string( c.geometry[0] ) }, { "$treeswayradius", "1" },
		    { "$treeswayspeed", ".2" }, { "$treeswaystrength", ".15" },
		    { "$treeswayscrumblefrequency", "2" }, { "$treeswayscrumblestrength", ".03" } };
		world.materials.push_back( foliage );
		WorldData::StaticMesh mesh;
		for ( const auto &xy : { std::pair{ -.5f, -.5f }, std::pair{ .5f, -.5f },
		          std::pair{ .5f, .5f }, std::pair{ -.5f, .5f } } )
		{
			material::SurfaceModelVertex vertex;
			vertex.position[0] = xy.first;
			vertex.position[1] = xy.second;
			vertex.position[2] = c.positionTime[2];
			vertex.normal[2] = 1;
			vertex.tangent[0] = vertex.tangent[3] = 1;
			mesh.vertices.push_back( vertex );
		}
		mesh.indices = { 0, 1, 2, 0, 2, 3 };
		mesh.surfaces.push_back( { 0, 0, 0, 6 } );
		world.staticMeshes.push_back( mesh );
		WorldData::StaticInstance instance;
		std::copy_n( c.objectToWorld, 16, instance.world );
		world.staticInstances.push_back( instance );
		WorldData rest = world;
		rest.materials[0].variables[1].second = "0";
		WorldData reference = rest;
		reference.materials[0].variables[1].second = "0";
		for ( auto &vertex : reference.staticMeshes[0].vertices )
		{
			std::copy_n( vertex.position, 3, c.positionTime );
			const auto deformed = Reference( c );
			for ( int axis = 0; axis < 3; ++axis )
				vertex.position[axis] = float( deformed[axis] );
		}
		WorldPass animated, expected, stationary;
		animated.SetWorld( std::move( world ) );
		expected.SetWorld( std::move( reference ) );
		stationary.SetWorld( std::move( rest ) );
		MotionReadback temporal{ device };
		if ( !temporal.Create() )
			return "foliage motion fixture targets refused";
		bool recordMotion = false, history = false;
		auto render = [&]( WorldPass &pass, bool prepass, bool available,
		                  CanvasImage &image ) -> std::optional<std::string>
		{
			WorldView view;
			for ( int i = 0; i < 4; ++i )
			{
				view.toClip[i * 5] = 1;
				view.motionToClip[i * 5] = view.previousToClip[i * 5] = 1;
			}
			view.previousViewValid = history;
			view.staticInstances = { 0 };
			view.viewport = { 0, 0, 64, 64, 0, 1 };
			const auto tag = pass.QueueView( std::move( view ) );
			if ( !tag )
				return "foliage fixture queued no model";
			return canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, &image,
			    [&]( device::CommandEncoder &encoder, device::TextureId color,
			        device::TextureId depth ) -> std::optional<std::string>
			    {
				    WorldTarget target;
				    target.device = &device;
				    target.color = color;
				    target.depth = depth;
				    target.colorFormat = kCanvasColor;
				    target.depthFormat = kCanvasDepth;
				    target.width = target.height = 64;
				    target.textures = &imports;
				    target.depthPrepass = prepass;
				    target.foliageAvailable = available;
				    target.foliage[0][0] = c.windMode[0];
				    target.foliage[0][1] = c.windMode[1];
				    target.foliage[0][2] = c.positionTime[3];
				    target.foliage[1][0] = -.3f;
				    target.foliage[1][1] = .6f;
				    target.foliage[1][2] = .2f;
				    if ( recordMotion )
				    {
					    using namespace device;
					    target.motion = temporal.motion;
					    target.motionDepth = temporal.depth;
					    for ( auto id : { temporal.motion, temporal.depth } )
						    encoder.TransitionTexture(
						        id, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
					    const ColorAttachment attachments[] = {
					        { temporal.motion, LoadOp::kClear, StoreOp::kStore,
					            { 65504, 65504, 0, 0 }, {} },
					        { temporal.depth, LoadOp::kClear, StoreOp::kStore, { 1, 0, 0, 0 },
					            {} } };
					    RenderingDesc clear;
					    clear.colors = attachments;
					    clear.width = clear.height = 64;
					    encoder.BeginRendering( clear );
					    encoder.EndRendering();
				    }
				    pass.Record( tag, encoder, target );
				    if ( recordMotion )
				    {
					    using namespace device;
					    encoder.TransitionTexture( temporal.motion, ResourceUsage::kColorAttachment,
					        ResourceUsage::kCopySource );
					    encoder.TransitionBuffer( temporal.readback, ResourceUsage::kUndefined,
					        ResourceUsage::kCopyDestination );
					    encoder.CopyTextureToBuffer(
					        temporal.motion, temporal.readback, { 0, 0, 0, 64, 64 } );
				    }
				    return std::nullopt;
			    } );
		};
		CanvasImage actual, oracle, prepassed, missing, undeformed;
		if ( auto why = render( animated, false, true, actual ) )
			return why;
		if ( auto why = render( expected, false, false, oracle ) )
			return why;
		if ( auto why = render( animated, true, true, prepassed ) )
			return why;
		if ( auto why = render( stationary, false, false, undeformed ) )
			return why;
		const std::string prefix = "tree-sway.model-stage." + std::to_string( mode );
		results.That( actual.rgba == oracle.rgba && animated.Stats().viewsFailed == 0,
		    prefix + ".independently-deformed-mesh-exact", animated.Stats().lastFailure );
		results.That( actual.rgba == prepassed.rgba, prefix + ".depth-prepass-exact" );
		results.That( actual.rgba != undeformed.rgba, prefix + ".no-deformation-control-rejected" );
		results.That( std::count_if( actual.rgba.begin(), actual.rgba.end(),
		                  []( float value )
		                  {
			                  return value > .5f;
		                  } ) > 5000,
		    prefix + ".visible-coverage" );
		recordMotion = true;
		for ( bool valid : { true, false } )
		{
			history = valid;
			CanvasImage temporalImage;
			if ( auto why = render( animated, false, true, temporalImage ) )
				return why;
			results.That( actual.rgba == temporalImage.rgba && animated.Stats().viewsFailed == 0,
			    prefix + ".temporal-preserves-visible-image", animated.Stats().lastFailure );
			std::vector<std::uint16_t> pixels( 64 * 64 * 2 );
			if ( !device.ReadBuffer(
			         temporal.readback, 0, std::as_writable_bytes( std::span( pixels ) ) ) )
				return "foliage motion readback refused";
			CheckMotion( c, mesh, temporalImage, pixels, results, prefix, valid );
		}
		recordMotion = false;
		if ( auto why = render( animated, false, false, missing ) )
			return why;
		results.That( animated.Stats().viewsFailed == 1 &&
		                  animated.Stats().lastFailure.find( "$treesway" ) != std::string::npos,
		    prefix + ".missing-wind-refused-by-name", animated.Stats().lastFailure );
		animated.ReleaseDevice( device );
		expected.ReleaseDevice( device );
		stationary.ReleaseDevice( device );
	}
	return std::nullopt;
}

std::optional<std::string> Run( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	CheckClaims( results );
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		CheckKernel kernel( *device );
		if ( auto why = kernel.Create(
		         module.empty() ? std::span<const std::uint32_t>( spirv::kTreeSwayCheck ) : module,
		         0, 0, "render_lab.tree-sway" ) )
			return why;
		std::vector<Case> cases;
		// Every distinct authored set in intro4: leaves/cluster vines, fern
		// bushes, suspended vines, thick vines and thick static vines.
		const float geometry[][4] = { { -300, .1f, 30, .125f }, { 60, .1f, .5f, .25f },
		    { -128, .1f, .5f, .25f }, { -300, .1f, 10, .5f }, { 300, .1f, 10, .5f } };
		const float motion[][4] = { { .0125f, 1, 25, .015f }, { .0015f, .0015f, 10, .0075f },
		    { 0, 0, 12, .02f }, { 0, 0, 12, .0125f }, { 0, 0, 10, .009f } };
		const float curves[][4] = { { 25, 2, 1.4f, 5 }, { 10, 7, 5, 2 }, { 12, 4, 3, 4 },
		    { 12, 4, 3, 3 }, { 10, 4, 3, 2 } };
		for ( int authored = 0; authored < 5; ++authored )
			for ( int mode = 0; mode <= 2; ++mode )
				for ( int sample = 0; sample < 96; ++sample )
				{
					Case c;
					std::copy_n( geometry[authored], 4, c.geometry );
					std::copy_n( motion[authored], 4, c.motion );
					std::copy_n( curves[authored], 4, c.curves );
					c.windControls[0] = 2;
					c.windControls[1] = authored >= 3 ? 12 : 6;
					c.positionTime[0] = float( sample % 8 ) * 6;
					c.positionTime[1] = float( sample % 7 ) * -5;
					c.positionTime[2] = sample == 0 ? 0 : ( sample / 8 - 2 ) * c.geometry[0] / 8;
					c.positionTime[3] = float( sample ) / 7;
					c.windMode[0] = float( sample % 5 ) * 4;
					c.windMode[1] = float( sample % 3 ) * -3;
					c.windMode[2] = float( mode );
					c.windControls[2] = sample % 4 == 0 ? 1.0f : 0.0f;
					c.objectToWorld[3] = 17;
					c.objectToWorld[7] = -24;
					c.objectToWorld[11] = 3;
					if ( sample % 2 )
					{
						c.objectToWorld[0] = c.objectToWorld[5] = 0;
						c.objectToWorld[1] = -1;
						c.objectToWorld[4] = 1;
					}
					cases.push_back( c );
				}
		resources::TextureCache textures( *device );
		std::vector<std::byte> bytes;
		if ( auto why = kernel.Run( textures, {}, unsigned( cases.size() ),
		         std::as_bytes( std::span( cases ) ), cases.size() * 16, bytes ) )
			return why;
		std::vector<float> gpu( bytes.size() / sizeof( float ) );
		std::memcpy( gpu.data(), bytes.data(), bytes.size() );
		for ( int cohort = 0; cohort < 15; ++cohort )
		{
			bool matches = true, finite = true;
			double worst = 0;
			for ( int sample = 0; sample < 96; ++sample )
			{
				const size_t i = cohort * 96 + sample;
				const auto expected = Reference( cases[i] );
				for ( int axis = 0; axis < 3; ++axis )
				{
					const double error = std::abs( gpu[i * 4 + axis] - expected[axis] );
					worst = std::max( worst, error );
					finite = finite && std::isfinite( gpu[i * 4 + axis] );
					matches = matches && error <= .002 + std::abs( expected[axis] ) * 2e-5;
				}
			}
			results.That( finite, "tree-sway.finite." + std::to_string( cohort ) );
			results.That( matches, "tree-sway.oracle." + std::to_string( cohort ),
			    "worst source-unit error " + std::to_string( worst ) );
		}
	}
	if ( auto why = CheckImages( *device, results ) )
		return why;
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
}

int RunTreeSwaySuite( int argc, char **argv )
{
	const Seeded seeded[] = {
	    { "static-ignored", spirv::kTreeSwayStaticIgnored, "tree-sway.oracle." },
	    { "wind-unrotated", spirv::kTreeSwayWindUnrotated, "tree-sway.oracle." },
	    { "hanging-ignored", spirv::kTreeSwayHangingIgnored, "tree-sway.oracle." },
	    { "root-ignored", spirv::kTreeSwayRootIgnored, "tree-sway.oracle." } };
	return RunSeededSuite( argc, argv, "tree-sway", seeded, Run );
}
} // namespace render::lab
