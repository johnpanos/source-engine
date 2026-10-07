//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's P2:CE PBR compatibility suite (RFC 0016 surface
//			model): P2:CE (Strata) `PBR` materials drawn by the core's pbr
//			point. The headless half checks the importer's interpretation of
//			P2:CE's own keys (vmt_mapping.cpp's kPbrCompatKeys) through the
//			family's claim: $mraoscale is the MRAO multiplier, $envmaptint
//			the probe tint and $srgbtint the base tint (Source gamma),
//			$envmaplightscale Portal 2's light-scaled reflection,
//			$basetexturetransform the texture coordinate's transform; the
//			second-layer and parallax settings are inert and enabling them
//			is refused by name.
//
//			The pixel half draws one lit quad as a posed model through
//			render.pass.world. Its oracle is relational: an MRAO texel m
//			under $mraoscale s draws the image of the texel m * s under
//			[1 1 1] (the texel values are chosen so m * s is exact in 8
//			bits), and the scale is visible (the unscaled image differs).
//			[1 1 1] is the texture unchanged, bitwise, so PBRMetalRough is
//			unchanged.
//
//			Seeded: mrao-scale-ignored.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/material/pbr_family.h"
#include "render/material/vmt_mapping.h"
#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"
#include "render/pass/world/world_pass.h"

#include "spv/pbr_compat_defects_spv.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
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
constexpr std::uint32_t kX = 32, kY = 32;
constexpr int kBaseHandle = 1;
// MRAO texels: R metalness, G roughness, B AO.
constexpr int kMraoHandle = 2;        // ( 200, 150, 255 )
constexpr int kMraoScaledHandle = 3;  // ( 100, 90, 255 ): kMraoHandle under [0.5 0.6 1]

class FixtureTextures final : public IWorldTextures
{
public:
	TextureId base, mrao, mraoScaled;
	TextureId Import( int handle, bool ) override
	{
		if ( handle == kBaseHandle )
			return base;
		if ( handle == kMraoHandle )
			return mrao;
		if ( handle == kMraoScaledHandle )
			return mraoScaled;
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

WorldMaterial Material( const Variables &variables, int mraoHandle )
{
	WorldMaterial material;
	material.name = "pbr-compat-fixture";
	material.shader = "PBR";
	material.mesh = true;
	material.variables = { { "$basetexture", "pbrcompat/base" },
	    { "$mraotexture", mraoHandle == kMraoHandle ? "pbrcompat/mrao" : "pbrcompat/mrao_scaled" } };
	material.variables.insert( material.variables.end(), variables.begin(), variables.end() );
	material.textures = { { "$basetexture", kBaseHandle }, { "$mraotexture", mraoHandle } };
	return material;
}

WorldData World( const Variables &variables, int mraoHandle )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	world.materials.push_back( Material( variables, mraoHandle ) );
	WorldData::StaticMesh mesh;
	mesh.AddLevel( WorldData::StaticMeshLod::MakeLevel( QuadVertices(), { 0, 1, 2, 0, 2, 3 } ),
	    { { 0, 0, 0, 6 } } );
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

// Source's GammaToLinear for a material color (mathlib color_conversion.cpp),
// written here independently of render.material's copy: the 256-entry
// pow(2.2) table indexed by RoundFloatToInt, one from 0.95.
float SourceGamma( float gamma )
{
	if ( gamma >= 0.95f )
		return gamma > 1.0f ? gamma : 1.0f;
	return std::pow( float( std::lrint( gamma * 255.0f ) ) / 255.0f, 2.2f );
}

bool Near( float actual, float expected, float relative = 2.0e-3f, float absolute = 1.0e-4f )
{
	return std::isfinite( actual ) &&
	       std::fabs( actual - expected ) <= absolute + relative * std::fabs( expected );
}

bool SamePixel( const float *a, const float *b )
{
	return Near( a[0], b[0] ) && Near( a[1], b[1] ) && Near( a[2], b[2] );
}

std::string Rgb( const char *label, const float *pixel )
{
	return std::string( label ) + " " + std::to_string( pixel[0] ) + " " +
	       std::to_string( pixel[1] ) + " " + std::to_string( pixel[2] );
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	// The importer and the family, headless: P2:CE's keys mapped onto the
	// claim's constants, or refused by name.
	material::FamilyRegistry registry;
	for ( const material::FamilyDesc &desc :
	    material::FamiliesFromMapping( material::BuiltinVmtMapping() ) )
		(void)registry.Register( desc );
	const material::FamilySchema *pbr = registry.Find( "pbr" );
	if ( !pbr )
		return "the pbr family is not registered";
	struct Claimed
	{
		material::PbrClaim claim;
		std::string why; // the importer's refusal, if any
	};
	auto claimOf = [&]( const Variables &variables ) -> Claimed
	{
		std::vector<material::VmtPair> pairs = { { "$basetexture", "pbrcompat/base" },
		    { "$mraotexture", "pbrcompat/mrao" } };
		for ( const auto &[key, value] : variables )
			pairs.push_back( { key, value } );
		Claimed out;
		auto mapped = material::MapVariables( "PBR", std::move( pairs ), {} );
		if ( !mapped )
		{
			out.why = "the material does not map";
			return out;
		}
		if ( mapped.Value().family != "pbr" )
		{
			out.why = "PBR maps to family " + mapped.Value().family;
			return out;
		}
		material::ParameterBlock block( *pbr );
		if ( !material::ApplyValues( mapped.Value(), block ) )
		{
			out.why = "the values do not apply";
			return out;
		}
		for ( const material::MaterialValue &value : mapped.Value().values )
		{
			if ( value.kind == material::ValueKind::kTexture )
				(void)block.SetTexture( value.parameter, TextureId( 1 ) );
		}
		out.claim = material::ClaimPbr( block );
		return out;
	};
	// The mesh claim through the resolver's own rules (env_cubemap needs the
	// stage's reflection probes; a named cube is refused).
	auto meshRefusal = [&]( const Variables &variables, bool probes ) -> std::string
	{
		std::vector<material::VmtPair> pairs = { { "$basetexture", "pbrcompat/base" },
		    { "$mraotexture", "pbrcompat/mrao" } };
		for ( const auto &[key, value] : variables )
			pairs.push_back( { key, value } );
		auto mapped = material::MapVariables( "PBR", std::move( pairs ), {} );
		if ( !mapped )
			return "the material does not map";
		auto claim = material::ClaimForMesh( mapped.Value(), probes );
		return claim ? std::string() : claim.Error();
	};

	// P2:CE's default model material (materials/dev/l2_default_model_pbr.vmt)
	// and the layer-2 defaults its world materials add (l2_default_pbr.vmt).
	const Variables defaults = { { "$mraoscale", "[0.9 1 1]" }, { "$envmaptint", "0.5" },
	    { "$envmaplightscale", "0.5" }, { "$model", "1" }, { "$parallaxscale", "2" },
	    { "$blendtintbymraoalpha", "1" }, { "$envmap", "env_cubemap" },
	    { "$basetexturetransform", "center 0 0 scale 2 2 rotate 0 translate 0 0" },
	    { "$basetexturetransform2", "center 0 0 scale 2 2 rotate 0 translate 0 0" },
	    { "$mraoscale2", "[0.9 1 1]" }, { "$envmaptint2", "0.5" } };
	results.That( meshRefusal( defaults, true ).empty(), "pbr-compat.claim.p2ce-defaults-claimed",
	    meshRefusal( defaults, true ) );
	{
		const std::string why = meshRefusal( defaults, false );
		results.That( why.find( "reflection probes" ) != std::string::npos,
		    "pbr-compat.claim.env-cubemap-needs-the-stage-probes", why );
	}
	{
		const std::string why = meshRefusal( { { "$envmap", "cubemaps/named" } }, true );
		results.That( why.find( "named cube" ) != std::string::npos,
		    "pbr-compat.claim.refuses-a-named-cube-by-name", why );
	}
	const Claimed mapped = claimOf( { { "$mraoscale", "[0.9 0.8 0.7]" }, { "$envmaptint", "0.5" },
	    { "$srgbtint", "[1 0.5 0.25]" }, { "$envmaplightscale", "0.75" },
	    { "$basetexturetransform", "center 0 0 scale 2 4 rotate 0 translate 0 0" } } );
	{
		const material::SurfaceConstants &c = mapped.claim.constants;
		results.That( mapped.claim.claimed, "pbr-compat.claim.mapped-keys-claimed",
		    mapped.why + mapped.claim.reason );
		results.That( Near( c.mraoScale[0], 0.9f ) && Near( c.mraoScale[1], 0.8f ) &&
		                  Near( c.mraoScale[2], 0.7f ),
		    "pbr-compat.mraoscale.is-the-mrao-multiplier" );
		const float half = SourceGamma( 0.5f );
		results.That( Near( c.envSaturation[0], half ) && Near( c.envSaturation[2], half ),
		    "pbr-compat.envmaptint.is-the-probe-tint-in-linear-light" );
		results.That( Near( c.tint[0], 1.0f ) && Near( c.tint[1], half ) &&
		                  Near( c.tint[2], SourceGamma( 0.25f ) ),
		    "pbr-compat.srgbtint.is-the-base-tint-in-linear-light" );
		results.That( Near( c.envLightScale[0], 0.0f ) && Near( c.envLightScale[1], 1.0f ) &&
		                  Near( c.envLightScale[2], 0.75f ),
		    "pbr-compat.envmaplightscale.is-portal-2s-light-scaled-reflection" );
		results.That( Near( c.baseTransform[0], 2.0f ) && Near( c.baseTransform[5], 4.0f ),
		    "pbr-compat.basetexturetransform.scales-the-coordinate" );
	}
	{
		const Claimed neutral = claimOf( {} );
		const material::SurfaceConstants &c = neutral.claim.constants;
		results.That( neutral.claim.claimed && c.mraoScale[0] == 1.0f &&
		                  c.mraoScale[1] == 1.0f && c.mraoScale[2] == 1.0f &&
		                  c.envSaturation[0] == 1.0f && c.tint[0] == 1.0f &&
		                  c.envLightScale[2] == 0.0f && c.baseTransform[0] == 1.0f &&
		                  c.baseTransform[5] == 1.0f,
		    "pbr-compat.neutral.unset-keys-are-the-identity" );
	}
	auto refusedNaming = [&]( const Variables &variables, const char *named )
	{
		const Claimed out = claimOf( variables );
		const std::string why = out.why.empty() ? out.claim.reason : out.why;
		return !out.claim.claimed && why.find( named ) != std::string::npos;
	};
	results.That( refusedNaming( { { "$parallax", "1" } }, "$parallax" ),
	    "pbr-compat.refuses.parallax-by-name" );
	results.That( refusedNaming( { { "$mraoscale", "[-1 1 1]" } }, "$mraoscale" ),
	    "pbr-compat.refuses.negative-mraoscale-by-name" );
	results.That( refusedNaming( { { "$envmaplightscale", "2" } }, "$envmaplightscale" ),
	    "pbr-compat.refuses.envmaplightscale-out-of-range-by-name" );
	{
		const std::string why = meshRefusal( { { "$detail", "pbrcompat/detail" } }, true );
		results.That( why.find( "$detail" ) != std::string::npos,
		    "pbr-compat.refuses.unread-key-stays-a-named-gap", why );
	}
	{
		const std::string why =
		    meshRefusal( { { "$blendtintbymraoalpha", "1" }, { "$color2", "[1 0 0]" } }, true );
		results.That( why.find( "$color2" ) != std::string::npos,
		    "pbr-compat.refuses.blend-tint-with-color2-by-name", why );
	}

	// Pixels: the scale is the multiplier the shader applies.
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
	auto stage = [&]( const char *name, Format format, std::array<std::byte, 4> texel,
	                 TextureId &out ) -> std::optional<std::string>
	{
		TextureDesc desc;
		desc.format = format;
		desc.width = desc.height = 1;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		auto staged = textures.Stage( name, desc, texel );
		if ( !staged )
			return std::string( "could not stage " ) + name;
		out = staged.Value().texture;
		return std::nullopt;
	};
	if ( auto why = stage( "pbrcompat/base", Format::kRGBA8Srgb,
	         { std::byte{ 200 }, std::byte{ 160 }, std::byte{ 120 }, std::byte{ 255 } },
	         fixture.base ) )
		return why;
	if ( auto why = stage( "pbrcompat/mrao", Format::kRGBA8Unorm,
	         { std::byte{ 200 }, std::byte{ 150 }, std::byte{ 255 }, std::byte{ 255 } },
	         fixture.mrao ) )
		return why;
	if ( auto why = stage( "pbrcompat/mrao_scaled", Format::kRGBA8Unorm,
	         { std::byte{ 100 }, std::byte{ 90 }, std::byte{ 255 }, std::byte{ 255 } },
	         fixture.mraoScaled ) )
		return why;

	std::vector<std::unique_ptr<WorldPass>> passes;
	std::uint64_t frame = 0;
	auto render = [&]( const Variables &variables, int mraoHandle,
	                  CanvasImage &image ) -> std::optional<std::string>
	{
		auto pass = std::make_unique<WorldPass>();
		pass->SetSurfaceFragmentModule( module );
		pass->SetWorld( World( variables, mraoHandle ) );
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = ++frame;
		WorldView::PosedModel pose;
		pose.vertices = QuadVertices();
		view.posedModels.push_back( std::move( pose ) );
		// A small rectangular area light in front of the quad, off its axis,
		// so diffuse and specular both depend on metalness and roughness.
		auto lights = std::make_shared<StageViewLights>();
		material::SurfaceAreaLight area;
		area.center[0] = 0.3f;
		area.center[2] = 0.9f;
		area.center[3] = 1.0f;
		area.halfU[0] = 0.2f;
		area.halfU[3] = 10.0f;
		area.halfV[1] = -0.2f;
		area.radiance[0] = area.radiance[1] = area.radiance[2] = 4.0f;
		area.radiance[3] = -1.0f;
		lights->areas.push_back( area );
		view.lights = std::move( lights );
		const std::uint32_t tag = pass->QueueView( std::move( view ) );
		if ( !tag )
			return "the fixture view queued nothing: " + pass->Stats().lastRefusal;
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
			target.eye[0] = -0.4f;
			target.eye[2] = 3.0f;
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
			return std::string( "the fixture quad was not drawn by the core" );
		passes.push_back( std::move( pass ) );
		return std::nullopt;
	};

	CanvasImage scaled, prescaled, unscaled, identity;
	if ( auto why = render( { { "$mraoscale", "[0.5 0.6 1]" } }, kMraoHandle, scaled ) )
		return why;
	if ( auto why = render( {}, kMraoScaledHandle, prescaled ) )
		return why;
	if ( auto why = render( {}, kMraoHandle, unscaled ) )
		return why;
	if ( auto why = render( { { "$mraoscale", "[1 1 1]" } }, kMraoHandle, identity ) )
		return why;
	const float *a = scaled.At( kX, kY ), *b = prescaled.At( kX, kY ), *u = unscaled.At( kX, kY );
	results.That( a[0] > 0.0f || a[1] > 0.0f || a[2] > 0.0f, "pbr-compat.pixels.quad-is-lit",
	    Rgb( "scaled", a ) );
	results.That( SamePixel( a, b ), "pbr-compat.mraoscale.draws-the-scaled-texel",
	    Rgb( "texel x [0.5 0.6 1]", a ) + " / " + Rgb( "texel x 0.5, 0.6", b ) );
	results.That( !SamePixel( a, u ), "pbr-compat.mraoscale.scale-is-visible",
	    Rgb( "scaled", a ) + " / " + Rgb( "unscaled", u ) );
	results.That( identity.rgba == unscaled.rgba, "pbr-compat.mraoscale.identity-is-bitwise-unset",
	    "[1 1 1] against no $mraoscale, bitwise" );

	for ( auto &pass : passes )
		pass->ReleaseDevice( *device );
	(void)device->WaitIdle();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "mrao-scale-ignored", spirv::kSurfacePbrMraoScaleIgnored, "pbr-compat.mraoscale" },
};

} // namespace

int RunPbrCompatSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "pbr-compat", kSeeded, RunChecks );
}

} // namespace render::lab
