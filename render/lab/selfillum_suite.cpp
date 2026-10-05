//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's self-illumination suite (RFC 0016 surface model,
//			emission term): VertexLitGeneric's $selfillum with its mask,
//			tint and $selfillumfresnel drawn by render.pass.world through the
//			shared PBR mesh point, the program the game's static models,
//			posed models and captured dynamic meshes use.
//
//			The fixture is one quad under an orthographic view whose left
//			half has base alpha 1 (the emitting region) and right half base
//			alpha 0. A distant eye at angle theta from the quad's normal
//			gives every pixel the facing cos(theta), so each image has one
//			analytic emission: tint x albedo x ( min + ( max - min ) c ),
//			c = cos(theta)^exp, over a dark scene; under a light, the
//			region's remainder is ( 1 - w ) times the same surface drawn
//			without self-illumination, w = saturate( b + ( 1 - b ) c ),
//			b = min / max. The neutral value [1 1 x] is bitwise the term
//			off. The emitting half lights nothing (the translation publishes
//			no area light).
//
//			Seeded: fresnel-ignored, brightness-ignored.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"
#include "render/pass/world/world_pass.h"

#include "spv/selfillum_defects_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
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

constexpr std::uint32_t kSize = 64;
constexpr float kPi = 3.14159265358979323846f;
// The base texture's handle: two texels, left alpha 1 (emitting), right 0.
constexpr int kBaseHandle = 1;
// The $selfillummask fixture's handle: two texels, left rgb 0, right rgb 1,
// the inverse of the base alpha, so the mask texture alone decides the region.
constexpr int kMaskHandle = 2;
// Sample points inside the quad (pixels 16 to 47): the emitting and the
// unmasked half.
constexpr std::uint32_t kEmitX = 22, kPlainX = 41, kRow = 32;

class FixtureTextures final : public IWorldTextures
{
public:
	TextureId base;
	TextureId mask;
	TextureId Import( int handle, bool ) override
	{
		if ( handle == kBaseHandle )
			return base;
		if ( handle == kMaskHandle )
			return mask;
		return TextureId{};
	}
	SamplerDesc Sampler( int ) override
	{
		SamplerDesc sampler;
		sampler.address = AddressMode::kClampToEdge;
		sampler.minFilter = sampler.magFilter = Filter::kNearest;
		return sampler;
	}
};

// The quad at z 0.5 facing +z, uv across it (u with x), with the vertex
// normal `normal` (zero for a mesh that carries none).
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

WorldMaterial Material( const Variables &variables, int maskHandle = 0 )
{
	WorldMaterial material;
	material.name = "selfillum-fixture";
	material.shader = "VertexLitGeneric";
	material.mesh = true;
	material.variables = { { "$basetexture", "selfillum/two_texels" } };
	material.variables.insert( material.variables.end(), variables.begin(), variables.end() );
	material.textures = { { "$basetexture", kBaseHandle } };
	if ( maskHandle != 0 )
		material.textures.push_back( { "$selfillummask", maskHandle } );
	return material;
}

WorldData World( const Variables &variables, int maskHandle = 0 )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	world.materials.push_back( Material( variables, maskHandle ) );
	WorldData::StaticMesh mesh;
	mesh.AddLevel( WorldData::StaticMeshLod::MakeLevel( QuadVertices(), { 0, 1, 2, 0, 2, 3 } ),
	    { { 0, 0, 0, 6 } } );
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

// The analytic emission of a fully masked texel: Source's
// $selfillumfresnelminmaxexp at facing cos(theta), tint and albedo 1.
float Emission( float min, float max, float exponent, float cosine )
{
	const float shaped = exponent > 0.0f ? std::pow( cosine, exponent ) : 1.0f;
	const float bias = max != 0.0f ? min / max : 0.0f;
	return max * std::clamp( bias + ( 1.0f - bias ) * shaped, 0.0f, 1.0f );
}

// The fraction of the region the term covers (max not zero).
float Weight( float min, float max, float exponent, float cosine )
{
	return Emission( min, max, exponent, cosine ) / max;
}

bool Near( float actual, float expected, float relative = 4.0e-3f, float absolute = 2.0e-3f )
{
	return std::isfinite( actual ) &&
	       std::fabs( actual - expected ) <= absolute + relative * std::fabs( expected );
}

std::string Detail( const char *label, const float *pixel, float expected )
{
	return std::string( label ) + " rgb " + std::to_string( pixel[0] ) + " " +
	       std::to_string( pixel[1] ) + " " + std::to_string( pixel[2] ) + ", expected " +
	       std::to_string( expected );
}

bool Gray( const float *pixel, float expected )
{
	return Near( pixel[0], expected ) && Near( pixel[1], expected ) && Near( pixel[2], expected );
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	// The translation (render.material), headless: the cohort's settings
	// are each interpreted or refused by name.
	// Why the shared mesh point refuses the material, or empty when it
	// claims it.
	auto refusal = []( const Variables &variables ) -> std::string
	{
		std::vector<material::VmtPair> pairs;
		pairs.push_back( { "$basetexture", "paint/bridge_paint_single" } );
		for ( const auto &[key, value] : variables )
			pairs.push_back( { key, value } );
		auto mapped = material::MapVariables( "VertexLitGeneric", std::move( pairs ), {} );
		if ( !mapped )
			return "the material does not map";
		auto claim = material::ClaimForMesh( mapped.Value(), true );
		return claim ? std::string() : claim.Error();
	};
	auto refusedNaming = [&]( const Variables &variables, const char *named )
	{
		const std::string why = refusal( variables );
		return !why.empty() && why.find( named ) != std::string::npos;
	};
	const Variables fresnel = { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	    { "$selfillumfresnelminmaxexp", "[0.2 2 2]" } };
	results.That( refusal( fresnel ).empty(), "selfillum.claim.fresnel-region-is-claimed",
	    refusal( fresnel ) );
	Variables tinted = fresnel;
	tinted.push_back( { "$selfillumtint", "[1 .5 .25]" } );
	tinted.push_back( { "$nocull", "1" } );
	results.That( refusal( tinted ).empty(), "selfillum.claim.tint-and-two-sided-are-claimed",
	    refusal( tinted ) );
	results.That( refusal( { { "$selfillumfresnel", "1" } } ).empty(),
	    "selfillum.claim.fresnel-without-selfillum-is-inert" );
	Variables masked = fresnel;
	masked.push_back( { "$selfillummask", "paint/mask" } );
	results.That( refusedNaming( masked, "$selfillummask" ),
	    "selfillum.claim.refuses-mask-texture-with-fresnel-by-name" );
	results.That( refusal( { { "$selfillum", "1" }, { "$selfillummask", "paint/mask" } } ).empty(),
	    "selfillum.claim.mask-texture-without-fresnel-is-claimed",
	    refusal( { { "$selfillum", "1" }, { "$selfillummask", "paint/mask" } } ) );
	Variables detailed = fresnel;
	detailed.push_back( { "$detail", "paint/detail" } );
	results.That( refusedNaming( detailed, "$detail" ),
	    "selfillum.claim.refuses-detail-with-fresnel-by-name" );
	Variables warped = fresnel;
	warped.push_back( { "$phong", "1" } );
	warped.push_back( { "$lightwarptexture", "paint/warp" } );
	results.That( refusedNaming( warped, "$lightwarptexture" ),
	    "selfillum.claim.refuses-light-warp-with-fresnel-by-name" );
	Variables normalAlpha = fresnel;
	normalAlpha.push_back( { "$normalmapalphaenvmapmask", "1" } );
	results.That( refusedNaming( normalAlpha, "$normalmapalphaenvmapmask" ),
	    "selfillum.claim.refuses-normal-alpha-mask-with-fresnel-by-name" );
	results.That( refusedNaming( { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	                                 { "$selfillumfresnelminmaxexp", "[0 -1 1]" } },
	                  "$selfillumfresnelminmaxexp" ),
	    "selfillum.claim.refuses-negative-controls-by-name" );
	results.That(
	    refusedNaming( { { "$selfillum", "1" }, { "$selfillumtypo", "1" } }, "$selfillumtypo" ),
	    "selfillum.claim.unknown-setting-stays-a-named-gap" );

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
	TextureDesc baseDesc;
	baseDesc.format = Format::kRGBA8Srgb;
	baseDesc.width = 2;
	baseDesc.height = 1;
	baseDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	const std::array<std::byte, 8> baseTexels = { std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 0 } };
	auto base = textures.Stage( "selfillum/two_texels", baseDesc, baseTexels );
	if ( !base )
		return "the two-texel base fixture could not be staged";
	fixture.base = base.Value().texture;
	// The mask texture, the inverse of the base alpha: left rgb 0, right rgb 1.
	TextureDesc maskDesc;
	maskDesc.format = Format::kRGBA8Unorm;
	maskDesc.width = 2;
	maskDesc.height = 1;
	maskDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	const std::array<std::byte, 8> maskTexels = { std::byte{ 0 }, std::byte{ 0 }, std::byte{ 0 },
	    std::byte{ 0 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 } };
	auto mask = textures.Stage( "selfillum/mask", maskDesc, maskTexels );
	if ( !mask )
		return "the two-texel self-illumination mask fixture could not be staged";
	fixture.mask = mask.Value().texture;

	std::vector<std::unique_ptr<WorldPass>> passes;
	std::uint64_t frame = 0;
	// One frame of the fixture: the material's world, a distant eye at
	// `degrees` from the normal (in the x-z plane), with or without a light;
	// `dynamic` draws the quad as a captured dynamic mesh (the game's legacy
	// stream capture) instead of a posed model; `noNormal` gives that mesh no
	// vertex normal.
	auto render = [&]( const Variables &variables, float degrees, bool lit, CanvasImage &image,
	                  bool dynamic = false, bool noNormal = false,
	                  int maskHandle = 0 ) -> std::optional<std::string>
	{
		auto pass = std::make_unique<WorldPass>();
		pass->SetSurfaceFragmentModule( module );
		pass->SetWorld( World( variables, maskHandle ) );
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = ++frame;
		if ( dynamic )
		{
			WorldView::DynamicDraw draw;
			draw.material = Material( variables, maskHandle );
			for ( const material::SurfaceModelVertex &vertex : QuadVertices() )
			{
				WorldVertex out;
				std::copy_n( vertex.position, 3, out.position );
				std::copy_n( vertex.uv, 2, out.uv );
				if ( noNormal )
					out.normal[2] = 0.0f;
				draw.vertices.push_back( out );
			}
			draw.indices = { 0, 1, 2, 0, 2, 3 };
			view.dynamicDraws.push_back( std::move( draw ) );
		}
		else
		{
			WorldView::PosedModel pose;
			pose.vertices = QuadVertices();
			view.posedModels.push_back( std::move( pose ) );
		}
		if ( lit )
		{
			auto lights = std::make_shared<StageViewLights>();
			material::SurfaceAreaLight area;
			area.center[2] = 0.8f;
			area.center[3] = 1.0f;
			area.halfU[0] = 0.5f;
			area.halfU[3] = 10.0f;
			area.halfV[1] = -0.5f;
			area.radiance[0] = area.radiance[1] = area.radiance[2] = 3.0f;
			area.radiance[3] = -1.0f;
			lights->areas.push_back( area );
			view.lights = std::move( lights );
		}
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
		if ( ( dynamic ? stats.dynamicDrawsDrawn : stats.posedDrawsDrawn ) != 1 )
			return std::string( "the fixture quad was not drawn by the core" );
		passes.push_back( std::move( pass ) );
		return std::nullopt;
	};

	const float cos60 = 0.5f;
	const float cos80 = std::cos( 80.0f * kPi / 180.0f );
	CanvasImage facing, sixty, eighty, neutral, plainSelfIllum, notSelfLit, inert, aboveMax,
	    zeroMax, litPlain, litFresnel, dynamicSixty, noNormal;
	if ( auto why = render( fresnel, 0.0f, false, facing ) )
		return why;
	if ( auto why = render( fresnel, 60.0f, false, sixty ) )
		return why;
	if ( auto why = render( fresnel, 80.0f, false, eighty ) )
		return why;
	if ( const char *directory = std::getenv( "RENDER_LAB_IMAGES" ) )
	{
		std::filesystem::create_directories( directory );
		for ( const auto &[name, image] : { std::pair{ "selfillum-0", &facing },
		          std::pair{ "selfillum-60", &sixty }, std::pair{ "selfillum-80", &eighty } } )
		{
			if ( !WritePfm( std::filesystem::path( directory ) / ( std::string( name ) + ".pfm" ),
			         image->width, image->height, image->rgba ) )
				return "could not write a self-illumination image";
		}
	}
	// A dark scene: the unmasked half shows no light, so the emitting half
	// is its emission alone, and it lit nothing beside it.
	results.That(
	    Gray( facing.At( kPlainX, kRow ), 0.0f ) && Gray( eighty.At( kPlainX, kRow ), 0.0f ),
	    "selfillum.emitter-lights-nothing-else",
	    Detail( "unmasked half", facing.At( kPlainX, kRow ), 0.0f ) );
	results.That( Gray( facing.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, 1.0f ) ),
	    "selfillum.fresnel.facing-emits-the-authored-max",
	    Detail( "0 degrees", facing.At( kEmitX, kRow ), 2.0f ) );
	results.That( Gray( sixty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos60 ) ),
	    "selfillum.fresnel.sixty-degrees-follows-min-max-exp",
	    Detail( "60 degrees", sixty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos60 ) ) );
	results.That( Gray( eighty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos80 ) ),
	    "selfillum.fresnel.grazing-falls-toward-the-authored-min",
	    Detail( "80 degrees", eighty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos80 ) ) );

	// The neutral value: [1 1 x] covers the region fully at radiance 1, which
	// is bitwise $selfillum without the fresnel term.
	if ( auto why = render( { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	                            { "$selfillumfresnelminmaxexp", "[1 1 3]" } },
	         60.0f, true, neutral ) )
		return why;
	if ( auto why = render( { { "$selfillum", "1" } }, 60.0f, true, plainSelfIllum ) )
		return why;
	results.That( neutral.rgba == plainSelfIllum.rgba, "selfillum.neutral.is-the-term-absent",
	    "[1 1 3] against $selfillum alone, lit, bitwise" );
	// $selfillumfresnel without $selfillum is inert, bitwise.
	if ( auto why = render(
	         { { "$selfillumfresnel", "1" }, { "$selfillumfresnelminmaxexp", "[0.2 2 2]" } }, 60.0f,
	         true, inert ) )
		return why;
	if ( auto why = render( {}, 60.0f, true, notSelfLit ) )
		return why;
	results.That( inert.rgba == notSelfLit.rgba, "selfillum.neutral.fresnel-alone-is-inert" );
	// A min above the max saturates the coverage: radiance max at any angle.
	if ( auto why = render( { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	                            { "$selfillumfresnelminmaxexp", "[3 1 1]" } },
	         80.0f, false, aboveMax ) )
		return why;
	results.That( Gray( aboveMax.At( kEmitX, kRow ), Emission( 3.0f, 1.0f, 1.0f, cos80 ) ),
	    "selfillum.fresnel.min-above-max-saturates-coverage",
	    Detail( "[3 1 1] at 80", aboveMax.At( kEmitX, kRow ), 1.0f ) );
	// A zero max emits nothing.
	if ( auto why = render( { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	                            { "$selfillumfresnelminmaxexp", "[0.5 0 1]" } },
	         0.0f, false, zeroMax ) )
		return why;
	results.That( Gray( zeroMax.At( kEmitX, kRow ), 0.0f ),
	    "selfillum.fresnel.zero-max-emits-nothing",
	    Detail( "[0.5 0 1]", zeroMax.At( kEmitX, kRow ), 0.0f ) );

	// Under a light the region keeps ( 1 - w ) of the surface's own lit
	// color: the same quad drawn without self-illumination.
	if ( auto why = render( {}, 80.0f, true, litPlain ) )
		return why;
	if ( auto why = render( fresnel, 80.0f, true, litFresnel ) )
		return why;
	{
		const float w = Weight( 0.2f, 2.0f, 2.0f, cos80 );
		const float e = Emission( 0.2f, 2.0f, 2.0f, cos80 );
		const float *plain = litPlain.At( kEmitX, kRow );
		const float *mixed = litFresnel.At( kEmitX, kRow );
		bool ok = plain[0] > 0.02f;
		for ( int c = 0; c < 3; ++c )
			ok = ok && Near( mixed[c], ( 1.0f - w ) * plain[c] + e );
		results.That( ok, "selfillum.lit.remainder-keeps-uncovered-lighting",
		    Detail( "lit at 80", mixed, ( 1.0f - w ) * plain[0] + e ) + " (plain " +
		        std::to_string( plain[0] ) + ")" );
		const float *plainRight = litPlain.At( kPlainX, kRow );
		const float *mixedRight = litFresnel.At( kPlainX, kRow );
		results.That( mixedRight[0] == plainRight[0] && mixedRight[1] == plainRight[1] &&
		                  mixedRight[2] == plainRight[2],
		    "selfillum.lit.unmasked-half-is-the-plain-surface" );
	}

	// The game's captured legacy meshes (CoreWorld::QueueMesh) reach the
	// same point as dynamic draws; a mesh without vertex normals (the
	// projected wall's quads) faces the eye.
	if ( auto why = render( fresnel, 60.0f, false, dynamicSixty, true ) )
		return why;
	results.That( Gray( dynamicSixty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos60 ) ),
	    "selfillum.dynamic.captured-mesh-follows-the-same-term",
	    Detail(
	        "dynamic 60", dynamicSixty.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, cos60 ) ) );
	if ( auto why = render( fresnel, 60.0f, false, noNormal, true, true ) )
		return why;
	results.That( Gray( noNormal.At( kEmitX, kRow ), Emission( 0.2f, 2.0f, 2.0f, 1.0f ) ),
	    "selfillum.dynamic.normal-less-mesh-faces-the-eye",
	    Detail( "no normal", noNormal.At( kEmitX, kRow ), 2.0f ) );

	// The tint colors the region in linear light (Source's gamma 2.2 rule).
	CanvasImage tintedImage;
	if ( auto why = render( { { "$selfillum", "1" }, { "$selfillumfresnel", "1" },
	                            { "$selfillumfresnelminmaxexp", "[0.2 2 2]" },
	                            { "$selfillumtint", "[1 .5 .25]" } },
	         0.0f, false, tintedImage ) )
		return why;
	{
		const float *pixel = tintedImage.At( kEmitX, kRow );
		const float g = std::pow( 128.0f / 255.0f, 2.2f ), b = std::pow( 64.0f / 255.0f, 2.2f );
		results.That(
		    Near( pixel[0], 2.0f ) && Near( pixel[1], 2.0f * g ) && Near( pixel[2], 2.0f * b ),
		    "selfillum.tint.is-linear-and-scales-the-region",
		    Detail( "tint [1 .5 .25]", pixel, 2.0f ) );
	}

	// The $selfillummask texture, not base alpha, selects the region. The
	// fixture's mask is the inverse of the base alpha, so the emitting half is
	// the one base alpha would leave dark; a shader that dropped the mask
	// would light the other half.
	CanvasImage maskedImage;
	if ( auto why = render( { { "$selfillum", "1" }, { "$selfillummask", "selfillum/mask" },
	                            { "$selfillumtint", "[1 .5 .25]" } },
	         0.0f, false, maskedImage, false, false, kMaskHandle ) )
		return why;
	{
		const float g = std::pow( 128.0f / 255.0f, 2.2f ), b = std::pow( 64.0f / 255.0f, 2.2f );
		results.That( Gray( maskedImage.At( kEmitX, kRow ), 0.0f ),
		    "selfillum.mask.base-lit-half-stays-dark-under-the-mask",
		    Detail( "masked texel 0", maskedImage.At( kEmitX, kRow ), 0.0f ) );
		const float *pixel = maskedImage.At( kPlainX, kRow );
		results.That( Near( pixel[0], 1.0f ) && Near( pixel[1], g ) && Near( pixel[2], b ),
		    "selfillum.mask.texture-controls-the-emitting-region",
		    Detail( "masked texel 1", pixel, 1.0f ) );
	}

	for ( auto &pass : passes )
		pass->ReleaseDevice( *device );
	(void)device->WaitIdle();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "fresnel-ignored", spirv::kSurfaceSelfIllumFresnelIgnored, "selfillum.fresnel" },
    { "brightness-ignored", spirv::kSurfaceSelfIllumBrightnessIgnored, "selfillum.fresnel" },
    { "mask-ignored", spirv::kSurfaceSelfIllumMaskIgnored, "selfillum.mask" },
};

} // namespace

int RunSelfIllumSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "selfillum", kSeeded, RunChecks );
}

} // namespace render::lab
