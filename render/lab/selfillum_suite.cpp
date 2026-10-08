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

#include "render/material/model_lighting.h"
#include "render/map_media/projector_cookies.h"
#include "render/projected_light.h"
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
// The eyes fixture's iris: two texels, left opaque black, right transparent.
constexpr int kIrisHandle = 3;
// The EyeRefract fixture: a uniform iris (sRGB 200 150 100, alpha 0: no
// cornea noise), a flat cornea (rg 0.5, no parallax offset or highlight) and
// an ambient occlusion texture black on the left texel, white on the right.
constexpr int kEyeIrisHandle = 4;
constexpr int kCorneaHandle = 5;
constexpr int kOcclusionHandle = 6;
constexpr unsigned kEyeIris[] = { 200, 150, 100 };
constexpr float kOcclusionColor[] = { 0.5f, 0.25f, 0.75f };
// Sample points inside the quad (pixels 16 to 47): the emitting and the
// unmasked half.
constexpr std::uint32_t kEmitX = 22, kPlainX = 41, kRow = 32;

class FixtureTextures final : public IWorldTextures
{
public:
	TextureId base;
	TextureId mask;
	TextureId iris;
	TextureId eyeIris, cornea, occlusion;
	TextureId Import( int handle, bool ) override
	{
		if ( handle == kEyeIrisHandle )
			return eyeIris;
		if ( handle == kCorneaHandle )
			return cornea;
		if ( handle == kOcclusionHandle )
			return occlusion;
		if ( handle == kBaseHandle )
			return base;
		if ( handle == kMaskHandle )
			return mask;
		if ( handle == kIrisHandle )
			return iris;
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

WorldMaterial Material(
    const Variables &variables, int maskHandle = 0, const char *shader = "VertexLitGeneric" )
{
	WorldMaterial material;
	material.name = "selfillum-fixture";
	material.shader = shader;
	material.mesh = true;
	const bool eyeRefract = std::string_view( shader ) == "EyeRefract";
	if ( !eyeRefract && std::string_view( shader ) != "Bik" )
	{
		material.variables = { { "$basetexture", "selfillum/two_texels" } };
		material.textures = { { "$basetexture", kBaseHandle } };
	}
	material.variables.insert( material.variables.end(), variables.begin(), variables.end() );
	// The detail fixtures reuse the base texture: rgb 1 on both texels.
	for ( const auto &[key, value] : variables )
	{
		if ( key == "$detail" )
			material.textures.push_back( { "$detail", kBaseHandle } );
	}
	if ( maskHandle != 0 )
		material.textures.push_back( { "$selfillummask", maskHandle } );
	// Eyes: the iris fixture, and the base's white texel as the glint.
	for ( const auto &[key, value] : variables )
	{
		if ( key == "$iris" )
			material.textures.push_back( { "$iris", kIrisHandle } );
		if ( key == "$glint" )
			material.textures.push_back( { "$glint", kBaseHandle } );
		if ( key == "$envmapmask" )
			material.textures.push_back( { "$envmapmask", kMaskHandle } );
		if ( eyeRefract && key == "$iris" )
			material.textures.back() = { "$iris", kEyeIrisHandle };
		if ( key == "$corneatexture" )
			material.textures.push_back( { "$corneatexture", kCorneaHandle } );
		if ( key == "$ambientoccltexture" )
			material.textures.push_back( { "$ambientoccltexture", kOcclusionHandle } );
		// Bik: Y and Cb the cornea's uniform 128, Cr the occlusion's 0 | 1.
		if ( key == "$ytexture" || key == "$cbtexture" )
			material.textures.push_back( { key, kCorneaHandle } );
		if ( key == "$crtexture" )
			material.textures.push_back( { key, kOcclusionHandle } );
	}
	return material;
}

WorldData World(
    const Variables &variables, int maskHandle = 0, const char *shader = "VertexLitGeneric" )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	world.materials.push_back( Material( variables, maskHandle, shader ) );
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
	const std::array<std::byte, 8> irisTexels = { std::byte{ 0 }, std::byte{ 0 }, std::byte{ 0 },
	    std::byte{ 255 }, std::byte{ 0 }, std::byte{ 0 }, std::byte{ 0 }, std::byte{ 0 } };
	auto iris = textures.Stage( "selfillum/iris", baseDesc, irisTexels );
	if ( !iris )
		return "the two-texel iris fixture could not be staged";
	fixture.iris = iris.Value().texture;
	{
		TextureDesc eyeDesc;
		eyeDesc.format = Format::kRGBA8Srgb;
		eyeDesc.width = 1;
		eyeDesc.height = 1;
		eyeDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		const std::array<std::byte, 4> eyeIris = { std::byte( kEyeIris[0] ),
		    std::byte( kEyeIris[1] ), std::byte( kEyeIris[2] ), std::byte{ 0 } };
		auto staged = textures.Stage( "selfillum/eye-iris", eyeDesc, eyeIris );
		if ( !staged )
			return "the eye iris fixture could not be staged";
		fixture.eyeIris = staged.Value().texture;
		eyeDesc.format = Format::kRGBA8Unorm;
		const std::array<std::byte, 4> cornea = {
		    std::byte{ 128 }, std::byte{ 128 }, std::byte{ 0 }, std::byte{ 0 } };
		auto corneaStaged = textures.Stage( "selfillum/cornea", eyeDesc, cornea );
		if ( !corneaStaged )
			return "the cornea fixture could not be staged";
		fixture.cornea = corneaStaged.Value().texture;
		eyeDesc.format = Format::kRGBA8Srgb;
		eyeDesc.width = 2;
		const std::array<std::byte, 8> occlusion = { std::byte{ 0 }, std::byte{ 0 }, std::byte{ 0 },
		    std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 },
		    std::byte{ 255 } };
		auto occlusionStaged = textures.Stage( "selfillum/eye-occlusion", eyeDesc, occlusion );
		if ( !occlusionStaged )
			return "the eye occlusion fixture could not be staged";
		fixture.occlusion = occlusionStaged.Value().texture;
	}

	// A white two-layer cookie array for the projected-light fixtures,
	// uploaded in the first frame that draws a projector.
	map_media::CookieArray cookies;
	{
		map_media::CookieImages white;
		white.names = { "white", "white" };
		white.bytes.assign( 8, std::byte{ 255 } );
		if ( auto why = cookies.Create( *device, white ) )
			return "the cookie array fixture: " + *why;
	}
	const TextureId cookieArray = cookies.Texture();
	const TextureDesc cookieArrayDesc = cookies.Desc();
	bool cookiesUploaded = false;

	std::vector<std::unique_ptr<WorldPass>> passes;
	std::uint64_t frame = 0;
	// One frame of the fixture: the material's world, a distant eye at
	// `degrees` from the normal (in the x-z plane), with or without a light;
	// `dynamic` draws the quad as a captured dynamic mesh (the game's legacy
	// stream capture) instead of a posed model; `noNormal` gives that mesh no
	// vertex normal.
	auto render = [&]( const Variables &variables, float degrees, bool lit, CanvasImage &image,
	                  bool dynamic = false, bool noNormal = false, int maskHandle = 0,
	                  const char *shader = "VertexLitGeneric",
	                  const material::ModelLighting *lighting = nullptr,
	                  const projected_light::LightGpu *projector = nullptr ) -> std::optional<std::string>
	{
		auto pass = std::make_unique<WorldPass>();
		pass->SetSurfaceFragmentModule( module );
		pass->SetWorld( World( variables, maskHandle, shader ) );
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = ++frame;
		if ( dynamic )
		{
			WorldView::DynamicDraw draw;
			draw.material = Material( variables, maskHandle, shader );
			if ( lighting )
				draw.lighting = *lighting;
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
		if ( projector )
		{
			auto lights = std::make_shared<StageViewLights>();
			lights->projectors.push_back( *projector );
			lights->view.counts[0] = 1.0f; // the view's projected lights
			lights->cookies = cookieArray;
			lights->cookiesDesc = cookieArrayDesc;
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
			if ( projector && !cookiesUploaded )
			{
				cookies.RecordUpload( encoder );
				cookiesUploaded = true;
			}
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

	// Detail modes 5 and 6 (TextureCombinePostLighting, the bridge paint's
	// glow): the detail is added after lighting, so a dark scene shows the
	// term alone on both halves, and a lit one adds it to the plain surface.
	// The detail texel is rgb 1: mode 5 adds the factor; mode 6 at factor f
	// below 0.5 adds saturate( 4f d - 2f ).
	results.That( refusal( { { "$detail", "paint/detail" }, { "$detailblendmode", "5" } } ).empty(),
	    "selfillum.detail.post-lighting-modes-are-claimed",
	    refusal( { { "$detail", "paint/detail" }, { "$detailblendmode", "5" } } ) );
	CanvasImage additive, threshold, litAdditive;
	const Variables mode5 = { { "$detail", "selfillum/two_texels" }, { "$detailblendmode", "5" },
	    { "$detailblendfactor", "0.3" } };
	const Variables mode6 = { { "$detail", "selfillum/two_texels" }, { "$detailblendmode", "6" },
	    { "$detailblendfactor", "0.25" } };
	if ( auto why = render( mode5, 0.0f, false, additive ) )
		return why;
	if ( auto why = render( mode6, 0.0f, false, threshold ) )
		return why;
	if ( auto why = render( mode5, 80.0f, true, litAdditive ) )
		return why;
	results.That( Gray( additive.At( kEmitX, kRow ), 0.3f ) &&
	                  Gray( additive.At( kPlainX, kRow ), 0.3f ),
	    "selfillum.detail.mode-5-adds-factor-times-detail-unlit",
	    Detail( "mode 5", additive.At( kEmitX, kRow ), 0.3f ) );
	results.That( Gray( threshold.At( kEmitX, kRow ), 0.5f ),
	    "selfillum.detail.mode-6-remaps-the-threshold-band",
	    Detail( "mode 6", threshold.At( kEmitX, kRow ), 0.5f ) );
	{
		const float *plain = litPlain.At( kEmitX, kRow );
		const float *added = litAdditive.At( kEmitX, kRow );
		bool ok = plain[0] > 0.02f;
		for ( int c = 0; c < 3; ++c )
			ok = ok && Near( added[c], plain[c] + 0.3f );
		results.That( ok, "selfillum.detail.mode-5-adds-after-lighting",
		    Detail( "lit mode 5", added, plain[0] + 0.3f ) );
	}

	// Teeth (teeth_vs20) on the same point: the lit surface times
	// $illumfactor x saturate( N . $forward ). Against the plain surface
	// under the same light: half with the factor 0.5 along the normal, black
	// with $forward across it, and black unset (the shader system's zeros).
	{
		CanvasImage halfLit, across, unset;
		if ( auto why = render( { { "$illumfactor", "0.5" }, { "$forward", "[0 0 1]" } }, 80.0f,
		         true, halfLit, false, false, 0, "Teeth" ) )
			return why;
		if ( auto why = render( { { "$illumfactor", "1" }, { "$forward", "[1 0 0]" } }, 80.0f,
		         true, across, false, false, 0, "Teeth" ) )
			return why;
		if ( auto why = render( {}, 80.0f, true, unset, false, false, 0, "Teeth" ) )
			return why;
		const float *plain = litPlain.At( kEmitX, kRow );
		const float *half = halfLit.At( kEmitX, kRow );
		bool halved = plain[0] > 0.02f;
		for ( int c = 0; c < 3; ++c )
			halved = halved && Near( half[c], 0.5f * plain[c] );
		results.That( halved, "selfillum.teeth.factor-scales-the-lit-surface",
		    Detail( "teeth 0.5", half, 0.5f * plain[0] ) );
		results.That( Gray( across.At( kEmitX, kRow ), 0.0f ) &&
		                  Gray( unset.At( kEmitX, kRow ), 0.0f ),
		    "selfillum.teeth.across-and-unset-are-black",
		    Detail( "teeth across", across.At( kEmitX, kRow ), 0.0f ) );
	}

	// Eyes (eyes_ps2x) on the same point, as captured meshes under Source's
	// model lighting: an ambient cube lit only from +z, so the eyeball's
	// normal decides the light. With the eye's origin far behind the quad its
	// normal is the quad's: a transparent iris is the plain surface, an
	// opaque black iris (projected onto the left half) covers it; with the
	// origin in front the normal turns away and the surface goes dark. The
	// glint (the base's white texel) adds one after lighting, damped to
	// nothing under a black ambient cube.
	{
		material::ModelLighting front;
		front.cube[4][0] = front.cube[4][1] = front.cube[4][2] = 1.0f; // +z
		material::ModelLighting dark;
		const Variables behind = { { "$iris", "selfillum/iris" }, { "$eyeorigin", "[0 0 -1e6]" },
		    { "$irisu", "[1 0 0 0.5]" }, { "$irisv", "[0 0 0 0.5]" } };
		Variables clear = behind;
		clear[2] = { "$irisu", "[0 0 0 0.75]" }; // every pixel reads the transparent texel
		Variables inFront = clear;
		inFront[1] = { "$eyeorigin", "[0 0 1e6]" };
		Variables glinting = clear;
		glinting.push_back( { "$glint", "selfillum/two_texels" } );
		glinting.push_back( { "$glintu", "[0 0 0 0.25]" } );
		glinting.push_back( { "$glintv", "[0 0 0 0.5]" } );
		CanvasImage plainEye, clearEye, coveredEye, turnedEye, glintEye, darkGlint, darkClear;
		for ( auto [variables, image, shader, light] :
		    { std::tuple{ Variables{}, &plainEye, "VertexLitGeneric", &front },
		        std::tuple{ clear, &clearEye, "Eyes", &front },
		        std::tuple{ behind, &coveredEye, "Eyes", &front },
		        std::tuple{ inFront, &turnedEye, "Eyes", &front },
		        std::tuple{ glinting, &glintEye, "Eyes", &front },
		        std::tuple{ glinting, &darkGlint, "Eyes", &dark },
		        std::tuple{ clear, &darkClear, "Eyes", &dark } } )
		{
			if ( auto why =
			         render( variables, 0.0f, false, *image, true, false, 0, shader, light ) )
				return why;
		}
		const float *plain = plainEye.At( kPlainX, kRow );
		auto same = []( const float *a, const float *b )
		{ return Near( a[0], b[0] ) && Near( a[1], b[1] ) && Near( a[2], b[2] ); };
		results.That( plain[0] > 0.05f && same( clearEye.At( kPlainX, kRow ), plain ) &&
		                  same( clearEye.At( kEmitX, kRow ), plainEye.At( kEmitX, kRow ) ),
		    "selfillum.eyes.iris.transparent-iris-is-the-sclera",
		    Detail( "clear iris", clearEye.At( kPlainX, kRow ), plain[0] ) );
		results.That( coveredEye.At( kEmitX, kRow )[0] < 0.25f * plainEye.At( kEmitX, kRow )[0] &&
		                  same( coveredEye.At( kPlainX, kRow ), plain ),
		    "selfillum.eyes.iris.opaque-iris-covers-the-sclera",
		    Detail( "covered half", coveredEye.At( kEmitX, kRow ), 0.0f ) );
		results.That( turnedEye.At( kPlainX, kRow )[0] < 0.25f * plain[0],
		    "selfillum.eyes.normal.from-the-eye-origin",
		    Detail( "origin in front", turnedEye.At( kPlainX, kRow ), 0.0f ) );
		const float *glint = glintEye.At( kPlainX, kRow );
		const float *clearPixel = clearEye.At( kPlainX, kRow );
		results.That( Near( glint[0] - clearPixel[0], 1.0f ) && Near( glint[1] - clearPixel[1], 1.0f ),
		    "selfillum.eyes.glint.adds-after-lighting",
		    Detail( "glint", glint, clearPixel[0] + 1.0f ) );
		results.That( same( darkGlint.At( kPlainX, kRow ), darkClear.At( kPlainX, kRow ) ),
		    "selfillum.eyes.glint-damped-in-the-dark",
		    Detail( "dark glint", darkGlint.At( kPlainX, kRow ),
		        darkClear.At( kPlainX, kRow )[0] ) );
	}

	// EyeRefract (eye_refract_ps2x) under an ambient cube lit only from +z,
	// no lights: the eyeball's normal (origin far behind) and the bent normal
	// face +z, so the vertex light is 1; the flat cornea adds no highlight or
	// parallax. The lit iris is the iris; where the occlusion texture is black
	// $ambientocclcolor tints it.
	{
		material::ModelLighting front;
		front.cube[4][0] = front.cube[4][1] = front.cube[4][2] = 1.0f; // +z
		const Variables eye = { { "$iris", "selfillum/eye-iris" },
		    { "$corneatexture", "selfillum/cornea" },
		    { "$ambientoccltexture", "selfillum/eye-occlusion" }, { "$eyeorigin", "[0 0 -1e6]" },
		    { "$irisu", "[1 0 0 0.5]" }, { "$irisv", "[0 -1 0 0.5]" },
		    { "$ambientocclcolor", "[0.5 0.25 0.75]" }, { "$halflambert", "1" } };
		CanvasImage eyeImage;
		if ( auto why = render( eye, 0.0f, false, eyeImage, true, false, 0, "EyeRefract", &front ) )
			return why;
		const float *open = eyeImage.At( kPlainX, kRow );
		const float *occluded = eyeImage.At( kEmitX, kRow );
		bool lit = true, tinted = true;
		std::string detail;
		for ( int c = 0; c < 3; ++c )
		{
			const float linear = kEyeIris[c] / 255.0f <= 0.04045f
			                         ? kEyeIris[c] / 255.0f / 12.92f
			                         : std::pow( ( kEyeIris[c] / 255.0f + 0.055f ) / 1.055f, 2.4f );
			lit = lit && Near( open[c], linear, 0.01f );
			tinted = tinted && Near( occluded[c], linear * kOcclusionColor[c], 0.01f );
			detail += std::to_string( open[c] ) + "/" + std::to_string( linear ) + " " +
			          std::to_string( occluded[c] ) + " ";
		}
		results.That( lit, "selfillum.eye-refract.lit-iris-is-the-iris", detail );
		results.That( tinted, "selfillum.eye-refract.ambient-occlusion-tints-the-light", detail );
		std::vector<material::VmtPair> intro;
		for ( const auto &[key, value] : eye )
			intro.push_back( { key, value } );
		intro.push_back( { "$intro", "1" } );
		auto mapped = material::MapVariables( "EyeRefract", std::move( intro ), {} );
		const auto introClaim = mapped
		                            ? material::ClaimForMesh( mapped.Value(), true )
		                            : foundation::Expected<device::BlendMode, std::string>(
		                                  foundation::MakeUnexpected( std::string( "no map" ) ) );
		const std::string refused = introClaim ? std::string() : introClaim.Error();
		results.That( refused.find( "$intro" ) != std::string::npos,
		    "selfillum.eye-refract.refuse-intro", refused );
	}

	// Bik (bik_ps2x): Y and Cb 128/255, Cr 0 on the left texel and 1 on the
	// right. Each pixel is bik_ps2x's RGB, read as sRGB, clamped.
	{
		const Variables video = { { "$ytexture", "selfillum/cornea" },
		    { "$crtexture", "selfillum/eye-occlusion" }, { "$cbtexture", "selfillum/cornea" } };
		CanvasImage videoImage;
		if ( auto why = render( video, 0.0f, false, videoImage, true, false, 0, "Bik" ) )
			return why;
		const auto expected = []( float cr, int c )
		{
			const float y = 128.0f / 255.0f, cb = 128.0f / 255.0f;
			const float rows[3][4] = { { 1.164123535f, 1.595794678f, 0.0f, -0.87065506f },
			    { 1.164123535f, -0.813476563f, -0.391448975f, 0.529705048f },
			    { 1.164123535f, 0.0f, 2.017822266f, -1.081668854f } };
			const float v = std::clamp(
			    rows[c][0] * y + rows[c][1] * cr + rows[c][2] * cb + rows[c][3], 0.0f, 1.0f );
			return v <= 0.04045f ? v / 12.92f : std::pow( ( v + 0.055f ) / 1.055f, 2.4f );
		};
		bool converted = true;
		std::string detail;
		for ( int c = 0; c < 3; ++c )
		{
			const float left = videoImage.At( kEmitX, kRow )[c];
			const float right = videoImage.At( kPlainX, kRow )[c];
			converted = converted && Near( left, expected( 0.0f, c ), 0.01f ) &&
			            Near( right, expected( 1.0f, c ), 0.01f );
			detail += std::to_string( left ) + "/" + std::to_string( expected( 0.0f, c ) ) + " " +
			          std::to_string( right ) + "/" + std::to_string( expected( 1.0f, c ) ) + " ";
		}
		results.That( converted, "selfillum.video.planes-convert-to-rgb", detail );
		std::vector<material::VmtPair> partial;
		partial.push_back( { "$ytexture", "selfillum/cornea" } );
		auto mapped = material::MapVariables( "Bik", std::move( partial ), {} );
		const auto partialClaim = mapped
		                              ? material::ClaimForMesh( mapped.Value(), true )
		                              : foundation::Expected<device::BlendMode, std::string>(
		                                    foundation::MakeUnexpected( std::string( "no map" ) ) );
		const std::string refused = partialClaim ? std::string() : partialClaim.Error();
		results.That( refused.find( "$crtexture" ) != std::string::npos,
		    "selfillum.video.refuse-missing-planes", refused );
	}

	// $selfillum_envmapmask_alpha: the envmap mask's alpha x 8 replaces the
	// surface with the albedo (the mask fixture: alpha 0 on the left, 1 on
	// the right). Dark: the left half black, the right 8 x the white albedo.
	// With $selfillum the pair has no shader combo and is refused.
	{
		CanvasImage inMask;
		if ( auto why = render( { { "$envmapmask", "selfillum/mask" },
		                            { "$selfillum_envmapmask_alpha", "1" } },
		         0.0f, false, inMask ) )
			return why;
		results.That( Gray( inMask.At( kEmitX, kRow ), 0.0f ) &&
		                  Gray( inMask.At( kPlainX, kRow ), 8.0f ),
		    "selfillum.envmapmask-alpha.weight-is-eight-times-the-alpha",
		    Detail( "mask alpha 1", inMask.At( kPlainX, kRow ), 8.0f ) );
		results.That(
		    refusedNaming( { { "$selfillum", "1" }, { "$envmapmask", "paint/mask" },
		                       { "$selfillum_envmapmask_alpha", "1" } },
		        "$selfillum_envmapmask_alpha" ),
		    "selfillum.envmapmask-alpha.refuses-with-selfillum-by-name" );
	}

	// $flashlightnolambert: the projected lights' diffuse takes no N.L. A
	// projector behind the quad lights nothing of plain VertexLitGeneric but
	// lights the no-Lambert surface; in front, head on (N.L 1), both agree.
	{
		auto projectorAt = []( float z, float direction )
		{
			projected_light::Light light;
			light.origin[2] = z;
			const float forward[3] = { 0, 0, direction }, right[3] = { 1, 0, 0 },
			            up[3] = { 0, direction, 0 };
			std::copy_n( forward, 3, light.forward );
			std::copy_n( right, 3, light.right );
			std::copy_n( up, 3, light.up );
			light.horizontalFovDegrees = light.verticalFovDegrees = 120.0f;
			light.nearZ = 1.0f;
			light.farZ = 100.0f;
			light.color[0] = light.color[1] = light.color[2] = 1.0f;
			light.atten[0] = 1.0f;
			light.atten[1] = 0.0f;
			return projected_light::PackLightGpu( light, 0 );
		};
		const auto behind = projectorAt( -5.0f, 1.0f ), front = projectorAt( 6.0f, -1.0f );
		const Variables noLambert = { { "$flashlightnolambert", "1" } };
		CanvasImage plainBack, wrapBack, plainFront, wrapFront;
		for ( auto [variables, image, light] :
		    { std::tuple{ Variables{}, &plainBack, &behind },
		        std::tuple{ noLambert, &wrapBack, &behind },
		        std::tuple{ Variables{}, &plainFront, &front },
		        std::tuple{ noLambert, &wrapFront, &front } } )
		{
			if ( auto why = render( variables, 0.0f, false, *image, false, false, 0,
			         "VertexLitGeneric", nullptr, light ) )
				return why;
		}
		const float *back = wrapBack.At( kEmitX, kRow );
		results.That( Gray( plainBack.At( kEmitX, kRow ), 0.0f ) && back[0] > 0.05f,
		    "selfillum.nolambert.back-faces-take-the-projector",
		    Detail( "no-Lambert from behind", back, 0.0f ) );
		const float *a = plainFront.At( kEmitX, kRow );
		const float *b = wrapFront.At( kEmitX, kRow );
		results.That( a[0] > 0.05f && Near( a[0], b[0] ) && Near( a[1], b[1] ) && Near( a[2], b[2] ),
		    "selfillum.nolambert.head-on-matches-lambert",
		    Detail( "head on", b, a[0] ) );
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
    { "teeth-ignored", spirv::kSurfaceTeethIgnored, "selfillum.teeth" },
    { "eyes-iris-ignored", spirv::kSurfaceEyesIrisIgnored, "selfillum.eyes.iris" },
    { "eyes-glint-ignored", spirv::kSurfaceEyesGlintIgnored, "selfillum.eyes.glint" },
    { "eye-refract-ao-ignored", spirv::kEyeRefractAoIgnored,
        "selfillum.eye-refract.ambient-occlusion" },
    { "envmapmask-alpha-ignored", spirv::kSurfaceSelfIllumEnvmapMaskAlphaIgnored,
        "selfillum.envmapmask-alpha.weight" },
    { "nolambert-ignored", spirv::kSurfaceFlashlightNoLambertIgnored,
        "selfillum.nolambert.back-faces" },
};

} // namespace

int RunSelfIllumSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "selfillum", kSeeded, RunChecks );
}

} // namespace render::lab
