//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.pbr (RFC 0016 K4 "Families match ports", the `pbr`
//			family) on render.device.vulkan.
//
//			quality/fixtures/render-families/pbr-port-v1.vdf is a passing
//			native `pbr-model` material pixel run (judged per pixel against an
//			independent BRDF model by material_pixel_pbr_model.py), recorded by
//			tools/render/family_port_pixels.py record-model: the harness's
//			textures, PBRMetalRough materials, quads and per case the
//			placement, ambient cube and Source model lights, with the model
//			port's pixels wherever the oracle judges one. Each case's
//			material is imported with the VMT importer into a `pbr` block,
//			claimed by ClaimPbr and drawn with the family: the frame group
//			holds the split-sum table, the view group the model lighting
//			(PackSourceModelLighting), the material group the constants and
//			textures, and the draw constants object-to-clip (the case's
//			placement, then the D3D9 half-pixel shift) and object-to-world. The pixels must match
//			the port's within kTolerance levels per channel. pbr_skinned is
//			drawn with its placement as a rigid transform: skinning is
//			render.pass.skinning's (K6). With the Khronos validation layer
//			installed, the run must report no message. family_pixel_cases.h
//			holds the shared harness.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_PBR_SEEDED_
//			IGNORE_NORMAL_MAP (pbr_family.cpp) claims every material without
//			its normal map.
//
//=============================================================================//

#include "family_pixel_cases.h"
#include "render/device/vulkan/provider.h"
#include "render/material/pbr_family.h"
#include "render/material/vmt_import.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace
{

using namespace render;
using namespace render::material;
using namespace rendertest::families;
namespace vulkan = render::device::vulkan;

const char *const kFixture = "quality/fixtures/render-families/pbr-port-v1.vdf";

ModelLightType LightType( const std::string &type )
{
	if ( type == "spot" )
		return ModelLightType::kSpot;
	return type == "directional" ? ModelLightType::kDirectional : ModelLightType::kPoint;
}

// A texture's copy in the format the family samples it with.
CaseTexture As( const CaseTexture &texture, device::Format format )
{
	CaseTexture copy = texture;
	copy.format = format;
	return copy;
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::optional<ModelCaseSet> set = LoadModelCases( checks, kFixture );
	if ( !set )
		return checks.Report();
	const FamilyRegistry registry = BuiltinFamilies();
	const FamilySchema *pbr = registry.Find( "pbr" );
	if ( !checks.That( pbr != nullptr, "setup.pbr-family-registers" ) )
		return checks.Report();

	// The family refuses what it does not draw, naming it.
	{
		auto refused = [&]( const char *vmt, const char *named, bool sceneColor = false )
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return false;
			ParameterBlock block( *pbr );
			if ( !ApplyValues( imported.Value(), block ) )
				return false;
			for ( const MaterialValue &value : imported.Value().values )
			{
				if ( value.kind == ValueKind::kTexture )
					(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
			}
			const PbrClaim claim = ClaimPbr( block, sceneColor );
			return !claim.claimed && claim.reason.find( named ) != std::string::npos;
		};
		const char *const base =
		    "\"PBRMetalRough\" { \"$basetexture\" \"a\" \"$mraotexture\" \"b\" "
		    "\"$fallbackmaterial\" \"f\" ";
		checks.That( refused( ( std::string( base ) + "\"$envmap\" \"c\" }" ).c_str(), "envmap" ),
		    "claim.refuses-an-environment-map-by-name" );
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported = ImportVmt(
			    ( std::string( base ) + "\"$alphatest\" \"1\" \"$alphatestreference\" \"0.5\" }" )
			        .c_str(),
			    context );
			ParameterBlock block( *pbr );
			bool claimed = imported && ApplyValues( imported.Value(), block );
			if ( claimed )
			{
				(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
				(void)block.SetTexture( "mraotexture", device::TextureId( 1 ) );
				const PbrClaim claim = ClaimPbr( block );
				claimed = claim.claimed && claim.alphaTest && claim.constants.flags[1] == 1.0f &&
				          claim.constants.flags[2] == 127.0f / 255.0f &&
				          !claim.Variant().alphaWrite;
			}
			checks.That( claimed, "claim.alpha-test-with-byte-reference" );
		}
		{
			ParameterBlock block( *pbr );
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			(void)block.SetTexture( "mraotexture", device::TextureId( 1 ) );
			(void)block.SetInt( "translucent", 1 );
			const PbrClaim claim = ClaimPbr( block );
			checks.That( claim.claimed && claim.translucent &&
			                 claim.Variant().blend == device::BlendMode::kAlpha &&
			                 !claim.Variant().alphaWrite,
			    "claim.translucent-preserves-destination-alpha" );
		}
		checks.That(
		    refused( ( std::string( base ) + "\"$clearcoat\" \"0.5\" }" ).c_str(), "clearcoat" ),
		    "claim.refuses-clear-coat-by-name" );
		checks.That( refused( ( std::string( base ) + "\"$transmission\" \"1\" }" ).c_str(),
		                 "transmission" ),
		    "claim.refuses-glass-without-scene-color" );
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported = ImportVmt( ( std::string( base ) +
			                             "\"$transmission\" \"0.75\" \"$ior\" \"1.25\" }" )
			                                .c_str(),
			    context );
			ParameterBlock block( *pbr );
			bool claimed = imported && ApplyValues( imported.Value(), block );
			if ( claimed )
			{
				(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
				(void)block.SetTexture( "mraotexture", device::TextureId( 1 ) );
				const PbrClaim glass = ClaimPbr( block, true );
				claimed = glass.claimed && glass.transmission &&
				          ( glass.Variant().terms & kSurfaceTransmission ) != 0 &&
				          glass.constants.transmission[0] == 0.75f &&
				          glass.constants.transmission[1] == 1.25f;
			}
			checks.That( claimed, "claim.thin-glass-uses-the-shared-pbr-point" );
		}
		checks.That( refused( ( std::string( base ) +
		                       "\"$transmission\" \"1\" \"$thickness\" \"4\" }" )
		                          .c_str(),
		                 "thickness", true ),
		    "claim.thick-glass-requires-depth-aware-refraction" );
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported = ImportVmt(
			    ( std::string( base ) + "\"$emissiontexture\" \"e\" "
			                            "\"$emissiononesided\" \"1\" "
			                            "\"$emissioncameraonly\" \"1\" \"$emissioncone\" \"1\" "
			                            "\"$emissionconeinner\" \"0.95\" "
			                            "\"$emissionconeouter\" \"0.85\" "
			                            "\"$emissionconeexponent\" \"2\" }" )
			        .c_str(),
			    context );
			ParameterBlock block( *pbr );
			bool claimed = imported && ApplyValues( imported.Value(), block );
			if ( claimed )
			{
				(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
				(void)block.SetTexture( "mraotexture", device::TextureId( 1 ) );
				(void)block.SetTexture( "emissiontexture", device::TextureId( 1 ) );
				const PbrClaim cone = ClaimPbr( block );
				claimed = cone.claimed && cone.emission && cone.constants.emission[1] == 1.0f &&
				          cone.constants.emission[2] == 1.0f &&
				          cone.constants.emissionCone[0] == 0.95f &&
				          cone.constants.emissionCone[1] == 0.85f &&
				          cone.constants.emissionCone[2] == 2.0f &&
				          cone.constants.emissionCone[3] == 1.0f;
			}
			checks.That( claimed, "claim.one-sided-emitter-cone" );
		}
		checks.That( refused( ( std::string( base ) + "\"$emissioncone\" \"1\" }" ).c_str(),
		                 "emissioncone" ),
		    "claim.refuses-cone-without-emission" );
		checks.That( refused( ( std::string( base ) + "\"$emissioncameraonly\" \"1\" }" ).c_str(),
		                 "emissioncameraonly" ),
		    "claim.refuses-camera-only-without-emission" );
		// The importer already requires $mraotexture; a block without one bound
		// (a texture that failed to load) is refused too.
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported = ImportVmt( ( std::string( base ) + "}" ).c_str(), context );
			ParameterBlock block( *pbr );
			bool refused = false;
			if ( imported && ApplyValues( imported.Value(), block ) )
			{
				(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
				const PbrClaim claim = ClaimPbr( block );
				refused = !claim.claimed && claim.reason.find( "mraotexture" ) != std::string::npos;
			}
			checks.That( refused, "claim.refuses-a-material-without-mrao-bound" );
		}
	}

	// PackSourceModelLighting sorts spot, point, directional, stably.
	{
		ModelLightDesc lights[3];
		lights[0].type = ModelLightType::kDirectional;
		lights[0].color[0] = 1.0f;
		lights[1].type = ModelLightType::kPoint;
		lights[1].color[0] = 2.0f;
		lights[2].type = ModelLightType::kSpot;
		lights[2].color[0] = 3.0f;
		lights[2].theta = 0.5f;
		lights[2].phi = 1.0f;
		const float eye[3] = {};
		const float cube[6][3] = {};
		const ModelLighting packed = PackSourceModelLighting( eye, cube, lights );
		checks.That( packed.eye[3] == 3.0f && packed.lights[0].color[0] == 3.0f &&
		                 packed.lights[1].color[0] == 2.0f && packed.lights[2].color[0] == 1.0f,
		    "lighting.lights-sort-spot-point-directional" );
		checks.That( packed.lights[2].color[3] == 1.0f && packed.lights[0].direction[3] == 1.0f &&
		                 packed.lights[0].spot[1] > packed.lights[0].spot[2],
		    "lighting.directional-and-spot-flags-and-cone" );
	}

	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	int drawnCases = 0;
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		// Portal views keep stencil on the imported D32S8 target. Building the
		// same PBR point with that raster state must remain a valid pipeline.
		{
			auto portal = SurfaceProgram::Create(
			    *device, device::Format::kRGBA8Srgb, device::Format::kD32FloatS8 );
			bool portalState = portal.HasValue();
			if ( portalState )
			{
				SurfaceVariant variant;
				variant.layout = SurfaceVertexLayout::kWorld;
				variant.terms = kSurfacePbr | kSurfaceMraoTexture | kSurfaceBakedLightmap |
				                kSurfaceClustered | kSurfaceRuntimeDirect;
				auto base = portal.Value()->Pipeline( variant );
				portalState = base.HasValue();
				if ( portalState )
				{
					SurfaceDrawState state;
					state.stencil.enabled = true;
					state.stencil.compare = device::CompareOp::kEqual;
					state.stencil.pass = device::StencilOp::kReplace;
					state.stencil.reference = 1;
					portalState = portal.Value()->StatePipeline( base.Value(), state ).HasValue();
				}
			}
			checks.That( portalState, "pipeline.pbr-world-with-portal-stencil" );
		}
		{
			auto program = SurfaceProgram::Create(
			    *device, device::Format::kRGBA8Srgb, device::Format::kD32Float );
			bool named = false;
			if ( program )
			{
				SurfaceVariant variant;
				variant.layout = SurfaceVertexLayout::kWorld;
				variant.terms = kSurfacePbr;
				auto base = program.Value()->Pipeline( variant );
				if ( base )
				{
					SurfaceDrawState state;
					state.stencil.enabled = true;
					auto refused = program.Value()->StatePipeline( base.Value(), state );
					named =
					    !refused &&
					    program.Value()->PipelineFailure().find( "invalid description" ) !=
					        std::string::npos &&
					    program.Value()->PipelineFailure().find( "native 0" ) != std::string::npos;
				}
			}
			checks.That( named, "pipeline.refused-stencil-keeps-the-device-failure" );
		}
		auto family =
		    PbrFamily::Create( *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( family.HasValue(), "family.creates" ) )
			return checks.Report();

		const PbrSplitSumTable table = SplitSumTable();
		CaseTexture splitSum;
		splitSum.width = table.width;
		splitSum.height = table.height;
		splitSum.clamp = true;
		splitSum.format = table.format;
		splitSum.texels.resize( table.texels.size() * sizeof( float ) );
		std::memcpy( splitSum.texels.data(), table.texels.data(), splitSum.texels.size() );
		CaseTexture unused;
		unused.width = unused.height = 1;
		unused.format = device::Format::kRGBA8Unorm;
		unused.texels = { 0, 0, 0, 255 };
		CaseTexture unusedCube = unused;
		unusedCube.cube = true;
		unusedCube.texels.resize( 6 * 4 );
		for ( std::size_t texel = 3; texel < unusedCube.texels.size(); texel += 4 )
			unusedCube.texels[texel] = 255;

		for ( const ModelCase &modelCase : set->cases )
		{
			const FamilyCase &testCase = modelCase.common;
			const std::string &name = testCase.name;
			const ImportedCase imported = ImportCase( checks, testCase, *pbr );
			if ( !imported.block )
				continue;
			const PbrClaim claim = ClaimPbr( *imported.block );
			if ( !That( checks, claim.claimed, "claim." + name, claim.reason ) )
				continue;
			auto pipeline = family.Value()->Pipeline( claim );
			if ( !checks.That( pipeline.HasValue(), "pipeline." + name ) )
				continue;
			// The textures, in the formats the family samples them with.
			std::optional<CaseTexture> base, mrao, normal, emission;
			for ( const MaterialValue &value : imported.material.values )
			{
				const CaseTexture *texture = FindTexture( set->textures, value.text );
				if ( !texture )
					continue;
				if ( value.parameter == "basetexture" )
					base = As( *texture, device::Format::kRGBA8Srgb );
				else if ( value.parameter == "mraotexture" )
					mrao = As( *texture, device::Format::kRGBA8Unorm );
				else if ( value.parameter == "bumpmap" )
					normal = As( *texture, device::Format::kRGBA8Unorm );
				else if ( value.parameter == "emissiontexture" )
					emission = As( *texture, device::Format::kRGBA8Srgb );
			}
			if ( !checks.That( base && mrao, "import." + name + ".textures-resolve" ) )
				continue;

			std::vector<ModelLightDesc> lights;
			for ( const ModelLight &light : modelCase.lights )
			{
				ModelLightDesc desc;
				desc.type = LightType( light.type );
				std::copy( light.color, light.color + 3, desc.color );
				std::copy( light.position, light.position + 3, desc.position );
				std::copy( light.direction, light.direction + 3, desc.direction );
				std::copy( light.attenuation, light.attenuation + 3, desc.attenuation );
				desc.theta = light.theta;
				desc.phi = light.phi;
				desc.falloff = light.falloff;
				lights.push_back( desc );
			}
			const ModelLighting lighting =
			    PackSourceModelLighting( set->eye, modelCase.cube, lights );

			std::vector<SurfaceModelVertex> vertices;
			const float uvs[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
			for ( const ModelQuad &quad : set->quads )
			{
				for ( int corner : { 0, 1, 2, 0, 2, 3 } )
				{
					SurfaceModelVertex vertex;
					vertex.position[0] = quad.corners[corner][0];
					vertex.position[1] = quad.corners[corner][1];
					vertex.position[2] = set->quadZ;
					std::copy( quad.normal, quad.normal + 3, vertex.normal );
					std::copy( quad.tangent, quad.tangent + 4, vertex.tangent );
					std::copy( uvs[corner], uvs[corner] + 2, vertex.uv );
					vertices.push_back( vertex );
				}
			}
			// Object-to-clip: the harness's clip space with the D3D9 half-pixel
			// shift, after the case's placement.
			FamilyDrawConstants drawConstants;
			std::copy( modelCase.modelMatrix, modelCase.modelMatrix + 12, drawConstants.world );
			drawConstants.world[15] = 1.0f;
			const std::array<float, 16> shift = CaseToClip();
			for ( int row = 0; row < 4; ++row )
			{
				for ( int column = 0; column < 4; ++column )
				{
					float sum = 0.0f;
					for ( int k = 0; k < 4; ++k )
						sum += shift[row * 4 + k] * drawConstants.world[k * 4 + column];
					drawConstants.toClip[row * 4 + column] = sum;
				}
			}

			CaseDraw draw;
			draw.pipeline = pipeline.Value();
			// The surface program's groups: the frame's terms (the eye is the
			// view's) and the split-sum table; the material group's seven
			// textures (base, env map, mask, normal map, detail, MRAO,
			// emission); the draw's lightmap page (unused) and its lighting.
			SurfaceFrame frame;
			std::copy( set->eye, set->eye + 3, frame.eye );
			draw.groups.push_back( NeutralViewGroup( family.Value()->ViewLayout() ) );
			draw.groups.push_back( { device::BindGroupRole::kFrame, family.Value()->FrameLayout(),
			    std::as_bytes( std::span( &frame, 1 ) ),
			    { &splitSum, &unused, &unused, &unused, &unused, &unused } } );
			draw.groups.push_back( { device::BindGroupRole::kDraw, family.Value()->DrawLayout(),
			    std::as_bytes( std::span( &lighting, 1 ) ), { &unused, &unused, &unused }, 1 } );
			SurfaceConstants constants = claim.constants;
			constants.state[0] = 1.0f; // opaque, as Request packs it
			draw.groups.push_back( { device::BindGroupRole::kMaterial,
			    family.Value()->MaterialLayout(), std::as_bytes( std::span( &constants, 1 ) ),
			    { &*base, &unusedCube, &unused, normal ? &*normal : &unused, &unused, &*mrao,
			        emission ? &*emission : &unused } } );
			draw.vertices = std::as_bytes( std::span( vertices ) );
			draw.vertexCount = std::uint32_t( vertices.size() );
			draw.drawConstants = std::as_bytes( std::span( &drawConstants, 1 ) );
			std::copy( testCase.clear, testCase.clear + 4, draw.clear );
			const Drawn drawn = DrawCase( *device, draw );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			JudgeCase( checks, testCase, drawn );
			if ( name == "pbr_ambient" )
			{
				// Keep the lighting and camera fixed while only the base
				// texture's alpha crosses the material's cutoff.
				CaseTexture masked = *base;
				for ( std::size_t texel = 3; texel < masked.texels.size(); texel += 4 )
					masked.texels[texel] = 160;
				PbrClaim cutout = claim;
				cutout.alphaTest = true;
				auto cutoutPipeline = family.Value()->Pipeline( cutout );
				if ( !checks.That( cutoutPipeline.HasValue(), "cutout.pipeline" ) )
					continue;
				draw.pipeline = cutoutPipeline.Value();
				draw.groups.back().textures[0] = &masked;
				constants.flags[1] = 1.0f;
				const auto clearPixel = [&]( std::size_t pixel, const Drawn &frame )
				{
					for ( int channel = 0; channel < 3; ++channel )
						if ( frame.rgba[pixel + channel] != testCase.clear[channel] )
							return false;
					return true;
				};
				std::size_t sample = drawn.rgba.size();
				for ( std::size_t pixel = 0; pixel + 3 < drawn.rgba.size(); pixel += 4 )
				{
					if ( !clearPixel( pixel, drawn ) )
					{
						sample = pixel;
						break;
					}
				}
				if ( !checks.That( sample < drawn.rgba.size(), "cutout.sample-is-covered" ) )
					continue;
				constants.flags[2] = 178.0f / 255.0f; // Source's default 0.7 byte reference
				const Drawn discarded = DrawCase( *device, draw );
				checks.That( discarded.ok && clearPixel( sample, discarded ),
				    "cutout.alpha-below-reference-discards" );
				constants.flags[2] = 127.0f / 255.0f;
				const Drawn kept = DrawCase( *device, draw );
				bool sameRgb = kept.ok;
				for ( int channel = 0; channel < 3 && sameRgb; ++channel )
					sameRgb = kept.rgba[sample + channel] == drawn.rgba[sample + channel];
				checks.That( sameRgb, "cutout.alpha-above-reference-keeps-lit-pixel" );
			}
		}
		checks.That( drawnCases == 8, "cases.every-case-drew" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
