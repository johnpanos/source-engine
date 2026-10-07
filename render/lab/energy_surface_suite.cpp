//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's energy-surface suite (RFC 0016 K12, R91): SolidEnergy
//			(Portal 2's fizzlers, light bridges, tractor beams) drawn by
//			render.pass.world through the surface program's energy point.
//
//			The fixture is one quad under a view whose clip w is 100 (so
//			solidenergy_ps20b's camera fade, which reads clip z, is one) over
//			a black clear, with 1x1 textures, so each image has one analytic
//			value: the base and detail layers' combine, the additive
//			( 1 + alpha ) scale, the fresnel opacity at the eye's angle, a
//			cheap flow field's two weighted samples and its radiance, the
//			power-up reveal (render.energy-field.v1's CPU oracle), an inactive
//			field and $outputintensity.
//
//			Seeded: additive-alpha-ignored, detail2-ignored, fresnel-ignored,
//			reveal-ignored.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/energy_field.h"
#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"
#include "render/pass/world/world_pass.h"

#include "spv/energy_surface_defects_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;

constexpr std::uint32_t kSize = 32;
constexpr std::uint32_t kX = 16, kY = 16;
constexpr float kPi = 3.14159265358979323846f;

// The fixture's 1x1 textures by handle.
enum Handle : int
{
	kBase = 1,     // sRGB 128 with alpha 128
	kDetail1 = 2,  // sRGB 128
	kDetail2 = 3,  // sRGB 64
	kFlowMap = 4,  // data, the neutral flow (128, 128)
	kNoise = 5,    // data, green 64
	kBounds = 6,   // data, red 0, green 1 (the edge), blue 1
	kBoundsIn = 7, // data, red 0, green 0 (inside the field), blue 1
};

// The sRGB curve and an 8-bit unorm value.
float Srgb( int byte )
{
	const float c = float( byte ) / 255.0f;
	return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
}
float Unorm( int byte )
{
	return float( byte ) / 255.0f;
}

class FixtureTextures final : public IWorldTextures
{
public:
	std::map<int, TextureId> byHandle;
	TextureId Import( int handle, bool ) override
	{
		const auto found = byHandle.find( handle );
		return found == byHandle.end() ? TextureId{} : found->second;
	}
	SamplerDesc Sampler( int ) override
	{
		SamplerDesc sampler;
		sampler.address = AddressMode::kClampToEdge;
		sampler.minFilter = sampler.magFilter = Filter::kNearest;
		return sampler;
	}
};

std::vector<material::SurfaceModelVertex> QuadVertices()
{
	std::vector<material::SurfaceModelVertex> vertices;
	for ( const auto &xy : { std::pair{ -0.5f, -0.5f }, std::pair{ 0.5f, -0.5f },
	          std::pair{ 0.5f, 0.5f }, std::pair{ -0.5f, 0.5f } } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = xy.first;
		vertex.position[1] = xy.second;
		vertex.position[2] = 0.5f;
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		vertex.uv[0] = xy.first + 0.5f;
		vertex.uv[1] = 0.5f - xy.second;
		vertices.push_back( vertex );
	}
	return vertices;
}

using Variables = std::vector<std::pair<std::string, std::string>>;

// The texture parameters the fixture binds, by the handle each names.
const std::pair<const char *, int> kTextureKeys[] = { { "$basetexture", kBase },
    { "$detail1", kDetail1 }, { "$detail2", kDetail2 }, { "$flowmap", kFlowMap },
    { "$flow_noise_texture", kNoise } };

WorldMaterial Material( const Variables &variables, int bounds )
{
	WorldMaterial material;
	material.name = "energy-fixture";
	material.shader = "SolidEnergy";
	material.mesh = true;
	material.variables = { { "$basetexture", "energy/base" }, { "$translucent", "1" },
	    { "$additive", "1" } };
	material.variables.insert( material.variables.end(), variables.begin(), variables.end() );
	for ( const auto &[key, handle] : kTextureKeys )
		for ( const auto &[name, value] : material.variables )
			if ( name == key )
			{
				material.textures.push_back( { key, handle } );
				break;
			}
	for ( const auto &[name, value] : material.variables )
		if ( name == "$flowbounds" )
			material.textures.push_back( { "$flowbounds", bounds } );
	return material;
}

WorldData World( const Variables &variables, int bounds )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	world.materials.push_back( Material( variables, bounds ) );
	WorldData::StaticMesh mesh;
	mesh.AddLevel( WorldData::StaticMeshLod::MakeLevel( QuadVertices(), { 0, 1, 2, 0, 2, 3 } ),
	    { { 0, 0, 0, 6 } } );
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

bool Near( float actual, float expected, float relative = 6.0e-3f, float absolute = 3.0e-3f )
{
	return std::isfinite( actual ) &&
	       std::fabs( actual - expected ) <= absolute + relative * std::fabs( expected );
}

std::string Detail( const char *label, const float *pixel, const float ( &expected )[3] )
{
	return std::string( label ) + " rgb " + std::to_string( pixel[0] ) + " " +
	       std::to_string( pixel[1] ) + " " + std::to_string( pixel[2] ) + ", expected " +
	       std::to_string( expected[0] ) + " " + std::to_string( expected[1] ) + " " +
	       std::to_string( expected[2] );
}

bool Rgb( const float *pixel, const float ( &expected )[3] )
{
	return Near( pixel[0], expected[0] ) && Near( pixel[1], expected[1] ) &&
	       Near( pixel[2], expected[2] );
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	// The translation (render.material), headless.
	auto refusal = []( const Variables &variables ) -> std::string
	{
		std::vector<material::VmtPair> pairs;
		pairs.push_back( { "$basetexture", "effects/fizzler_ripples" } );
		for ( const auto &[key, value] : variables )
			pairs.push_back( { key, value } );
		auto mapped = material::MapVariables( "SolidEnergy", std::move( pairs ), {} );
		if ( !mapped )
			return "the material does not map";
		auto claim = material::ClaimForDrawing( mapped.Value(), false );
		return claim ? std::string() : claim.Error();
	};
	const Variables beam = { { "$detail1", "effects/tractor_beam_core2" },
	    { "$detail2", "effects/tractor_beam_core1" }, { "$detail1blendmode", "1" },
	    { "$tangenttopacityranges", "[1 -1 28 0.3]" }, { "$translucent", "1" },
	    { "$additive", "1" }, { "$vertexcolor", "1" }, { "$nocull", "1" } };
	results.That( refusal( beam ).empty(), "energy.claim.tractor-beam-is-claimed", refusal( beam ) );
	const Variables field = { { "$flowmap", "effects/fizzler_flow" },
	    { "$flowbounds", "effects/fizzler_bounds" },
	    { "$flow_noise_texture", "effects/fizzler_noise" }, { "$flow_vortex1", "1" },
	    { "$flow_vortex_pos1", "[-160 64 64]" }, { "$powerup", "0.4" } };
	results.That( refusal( field ).empty(), "energy.claim.flow-field-is-claimed", refusal( field ) );
	const std::string noNoise = refusal(
	    { { "$flowmap", "effects/fizzler_flow" }, { "$flowbounds", "effects/fizzler_bounds" } } );
	results.That( noNoise.find( "$flow_noise_texture" ) != std::string::npos,
	    "energy.claim.refuses-flow-without-noise-by-name", noNoise );
	const std::string unknown = refusal( { { "$energytypo", "1" } } );
	results.That( unknown.find( "$energytypo" ) != std::string::npos,
	    "energy.claim.unknown-setting-stays-a-named-gap", unknown );

	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( std::optional<std::string> why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	FixtureTextures fixture;
	auto stage = [&]( int handle, const char *name, Format format,
	                 std::array<int, 4> rgba ) -> std::optional<std::string>
	{
		TextureDesc desc;
		desc.format = format;
		desc.width = desc.height = 1;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		std::array<std::byte, 4> texel;
		for ( int c = 0; c < 4; ++c )
			texel[c] = std::byte( rgba[c] );
		auto staged = textures.Stage( name, desc, texel );
		if ( !staged )
			return std::string( "the fixture texture " ) + name + " could not be staged";
		fixture.byHandle[handle] = staged.Value().texture;
		return std::nullopt;
	};
	for ( auto why : { stage( kBase, "energy/base", Format::kRGBA8Srgb, { 128, 128, 128, 128 } ),
	          stage( kDetail1, "energy/detail1", Format::kRGBA8Srgb, { 128, 128, 128, 255 } ),
	          stage( kDetail2, "energy/detail2", Format::kRGBA8Srgb, { 64, 64, 64, 255 } ),
	          stage( kFlowMap, "energy/flow", Format::kRGBA8Unorm, { 128, 128, 0, 255 } ),
	          stage( kNoise, "energy/noise", Format::kRGBA8Unorm, { 0, 64, 0, 255 } ),
	          stage( kBounds, "energy/bounds", Format::kRGBA8Unorm, { 0, 255, 255, 255 } ),
	          stage( kBoundsIn, "energy/bounds_in", Format::kRGBA8Unorm, { 0, 0, 255, 255 } ) } )
		if ( why )
			return why;

	std::vector<std::unique_ptr<WorldPass>> passes;
	std::uint64_t frame = 0;
	// One frame: the quad as a posed model, the eye far away at `degrees`
	// from the normal in the x-z plane.
	auto render = [&]( const Variables &variables, float degrees, CanvasImage &image,
	                  int bounds = kBounds ) -> std::optional<std::string>
	{
		auto pass = std::make_unique<WorldPass>();
		pass->SetSurfaceFragmentModule( module );
		pass->SetWorld( World( variables, bounds ) );
		WorldView view;
		// Clip w and z scaled by 100: the same image, and clip z 50, past
		// ComputeCameraFade's ramp (clip z / 40).
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 100.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = ++frame;
		WorldView::PosedModel pose;
		pose.vertices = QuadVertices();
		view.posedModels.push_back( std::move( pose ) );
		const std::uint32_t tag = pass->QueueView( std::move( view ) );
		if ( !tag )
			return "the fixture view queued nothing: " + pass->Stats().lastRefusal;
		const float radians = degrees * kPi / 180.0f;
		const std::uint64_t thisFrame = frame;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.colorCopySource = true;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &fixture;
			target.frame = thisFrame;
			target.eye[0] = 1.0e4f * std::sin( radians );
			target.eye[2] = 0.5f + 1.0e4f * std::cos( radians );
			pass->Record( tag, encoder, target );
			return std::nullopt;
		};
		const ClearColor black{ 0, 0, 0, 1 };
		if ( std::optional<std::string> why =
		         canvas->Render( textures, groups, {}, black, &image, post ) )
			return why;
		const WorldStats stats = pass->Stats();
		if ( stats.viewsFailed != 0 )
			return "the fixture view failed: " + stats.lastFailure;
		if ( stats.posedDrawsDrawn != 1 )
			return "the fixture quad was not drawn by the core: " + stats.lastRefusal;
		passes.push_back( std::move( pass ) );
		return std::nullopt;
	};
	auto gray = []( float v ) -> std::array<float, 3> { return { v, v, v }; };
	auto expect = [&]( const CanvasImage &image, std::array<float, 3> value, const char *name,
	                  const char *label )
	{
		const float expected[3] = { value[0], value[1], value[2] };
		results.That( Rgb( image.At( kX, kY ), expected ), name,
		    Detail( label, image.At( kX, kY ), expected ) );
	};

	const float base = Srgb( 128 ), baseAlpha = Unorm( 128 );
	const float d1 = Srgb( 128 ), d2 = Srgb( 64 );

	// Base alone, additive: alpha is the base's, and the color is scaled by
	// ( 1 + alpha ) with alpha 1 written (src-alpha, one).
	CanvasImage additive;
	if ( auto why = render( {}, 0.0f, additive ) )
		return why;
	expect( additive, gray( base * ( 1.0f + baseAlpha ) ), "energy.additive.base-times-one-plus-alpha",
	    "base" );
	// $outputintensity scales the output.
	CanvasImage intensity;
	if ( auto why = render( { { "$outputintensity", "2" } }, 0.0f, intensity ) )
		return why;
	expect( intensity, gray( 2.0f * base * ( 1.0f + baseAlpha ) ),
	    "energy.additive.output-intensity-scales", "$outputintensity 2" );

	// Detail 1 (mode 0, mod2x) and detail 2 (mode 0, added times detail 1).
	CanvasImage layered;
	if ( auto why = render( { { "$detail1", "energy/detail1" }, { "$detail2", "energy/detail2" } },
	         0.0f, layered ) )
		return why;
	expect( layered, gray( ( base * 2.0f * d1 + d2 * d1 ) * ( 1.0f + baseAlpha ) ),
	    "energy.detail.mod2x-then-added-second-layer", "details mode 0" );
	// Detail 1 mode 1: lerp( base * d1, base, base alpha ), and the base
	// alpha no longer sets the opacity.
	CanvasImage lerped;
	if ( auto why = render(
	         { { "$detail1", "energy/detail1" }, { "$detail1blendmode", "1" } }, 0.0f, lerped ) )
		return why;
	{
		const float mixed = base * d1 + ( base - base * d1 ) * baseAlpha;
		expect( lerped, gray( mixed * 2.0f ), "energy.detail1.mode-1-lerps-by-base-alpha",
		    "detail 1 mode 1" );
	}

	// The fresnel opacity: alpha = mix( x, y, |N.V|^z ), not the base's.
	CanvasImage facing, sixty;
	const Variables fresnel = { { "$fresnelopacityranges", "[0.2 1 1 1]" } };
	if ( auto why = render( fresnel, 0.0f, facing ) )
		return why;
	if ( auto why = render( fresnel, 60.0f, sixty ) )
		return why;
	expect( facing, gray( base * 2.0f ), "energy.fresnel.facing-is-opaque", "fresnel at 0" );
	expect( sixty, gray( base * ( 1.0f + 0.2f + 0.8f * 0.5f ) ),
	    "energy.fresnel.sixty-degrees-follows-ranges", "fresnel at 60" );

	// A cheap flow field at time 0: two samples weighted by the noise's
	// interval phase, then render.energy-field.v1's reveal and radiance.
	const Variables flow = { { "$flowmap", "energy/flow" }, { "$flowbounds", "energy/bounds" },
	    { "$flow_noise_texture", "energy/noise" }, { "$flow_cheap", "1" },
	    { "$flow_color", "[0.2 0.1 0.05]" }, { "$flow_timeintervalinseconds", "0.4" } };
	auto flowRadiance = [&]( float powerUp, float edge ) -> std::array<float, 3>
	{
		const float noise = Unorm( 64 );
		const float t = noise; // time 0
		const float w1 = std::pow( std::fabs( 2.0f * energy_field::Fract( t + 0.5f ) - 1.0f ), 2.0f );
		const float w2 = std::pow( std::fabs( 2.0f * energy_field::Fract( t ) - 1.0f ), 2.0f );
		float sample[4] = { base * ( w1 + w2 ), base * ( w1 + w2 ), base * ( w1 + w2 ),
		    baseAlpha * ( w1 + w2 ) };
		energy_field::Reveal( sample, noise, edge, powerUp );
		const float a = sample[3];
		// Additive with alpha 1 (a flow field's alpha is its opacity terms').
		return { a * 0.2f * 2.0f, a * 0.1f * 2.0f, a * 0.05f * 2.0f };
	};
	CanvasImage flowing;
	if ( auto why = render( flow, 0.0f, flowing ) )
		return why;
	expect( flowing, flowRadiance( 1.0f, 1.0f ), "energy.flow.cheap-field-radiance", "flow" );
	// Power-up halfway inside the field (edge 0): the reveal has not reached
	// this texel, so it is dark; fully powered it shows its own alpha.
	Variables powering = flow;
	powering.push_back( { "$powerup", "0.5" } );
	CanvasImage revealing, powered;
	if ( auto why = render( powering, 0.0f, revealing, kBoundsIn ) )
		return why;
	if ( auto why = render( flow, 0.0f, powered, kBoundsIn ) )
		return why;
	expect( revealing, flowRadiance( 0.5f, 0.0f ), "energy.powerup.unrevealed-texel-is-dark",
	    "power-up 0.5 inside" );
	expect( powered, flowRadiance( 1.0f, 0.0f ), "energy.powerup.powered-texel-shows-the-field",
	    "power-up 1 inside" );
	// No intensity: the field is inactive and draws black.
	Variables off = flow;
	off.push_back( { "$flow_color_intensity", "0" } );
	CanvasImage inactive;
	if ( auto why = render( off, 0.0f, inactive ) )
		return why;
	expect( inactive, gray( 0.0f ), "energy.flow.inactive-field-is-black", "intensity 0" );

	for ( auto &pass : passes )
		pass->ReleaseDevice( *device );
	(void)device->WaitIdle();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "additive-alpha-ignored", spirv::kSurfaceEnergyAdditiveAlphaIgnored, "energy.additive" },
    { "detail2-ignored", spirv::kSurfaceEnergyDetail2Ignored, "energy.detail." },
    { "fresnel-ignored", spirv::kSurfaceEnergyFresnelIgnored, "energy.fresnel" },
    { "reveal-ignored", spirv::kSurfaceEnergyRevealIgnored, "energy.powerup" },
};

} // namespace

int RunEnergySurfaceSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "energy-surface", kSeeded, RunChecks );
}

} // namespace render::lab
