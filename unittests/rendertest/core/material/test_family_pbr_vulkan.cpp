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

#include "family_devices.h"
#include "family_pixel_cases.h"
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
		// $envmap is the view's reflection probes (6049332f6); a named cube is
		// refused by the program resolver, not the family.
		{
			VmtImportContext context;
			context.resolve = []( std::string_view ) -> std::optional<std::string>
			{
				return std::string();
			};
			auto imported =
			    ImportVmt( ( std::string( base ) + "\"$envmap\" \"c\" }" ).c_str(), context );
			bool claimed = false;
			if ( imported )
			{
				ParameterBlock block( *pbr );
				if ( ApplyValues( imported.Value(), block ) )
				{
					for ( const MaterialValue &value : imported.Value().values )
						if ( value.kind == ValueKind::kTexture )
							(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
					claimed = ClaimPbr( block, false ).claimed;
				}
			}
			checks.That( claimed, "claim.takes-an-environment-map-as-the-probes" );
		}
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

	CaseDevices devices( checks );
	for ( const CaseDevices::Entry &entry : devices.entries )
	{
		std::printf( "INFO device %s\n", entry.name.c_str() );
		device::IRenderDevice2 *const device = entry.device.get();
		int drawnCases = 0;
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
			continue;

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
			draw.groups.push_back( SurfaceFrameGroup( family.Value()->FrameLayout(),
			    std::as_bytes( std::span( &frame, 1 ) ), &splitSum, &unused, &unused, &unused,
			    &unused ) );
			draw.groups.push_back( { device::BindGroupRole::kDraw, family.Value()->DrawLayout(),
			    std::as_bytes( std::span( &lighting, 1 ) ), { &unused, &unused, &unused, &unused }, 1 } );
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
			devices.Compare( checks, entry, name, drawn );
			JudgeCase( checks, testCase, drawn );
			// The world vertex (R91: the mesh handoff's dynamic draws on a map
			// without a stage): the case's quads placed in world space, the same
			// model lighting in the draw group, attenuated per vertex as the model
			// vertex attenuates it: the pixels match within one level. With the
			// lights removed they must change wherever a light reached them.
			{
				SurfaceVariant worldVariant = claim.Variant();
				worldVariant.layout = SurfaceVertexLayout::kWorld;
				auto worldPipeline = family.Value()->Program().Pipeline( worldVariant );
				std::vector<SurfaceWorldVertex> worldVertices;
				const float *m = modelCase.modelMatrix;
				for ( const SurfaceModelVertex &v : vertices )
				{
					SurfaceWorldVertex w;
					const float p[3] = { v.position[0], v.position[1], v.position[2] };
					for ( int r = 0; r < 3; ++r )
					{
						w.position[r] = m[r * 4] * p[0] + m[r * 4 + 1] * p[1] +
						                m[r * 4 + 2] * p[2] + m[r * 4 + 3];
						w.normal[r] = m[r * 4] * v.normal[0] + m[r * 4 + 1] * v.normal[1] +
						              m[r * 4 + 2] * v.normal[2];
						w.tangentS[r] = m[r * 4] * v.tangent[0] + m[r * 4 + 1] * v.tangent[1] +
						                m[r * 4 + 2] * v.tangent[2];
					}
					for ( int r = 0; r < 3; ++r )
						w.tangentT[r] = ( w.normal[( r + 1 ) % 3] * w.tangentS[( r + 2 ) % 3] -
						                    w.normal[( r + 2 ) % 3] * w.tangentS[( r + 1 ) % 3] ) *
						                v.tangent[3];
					std::copy( v.uv, v.uv + 2, w.uv );
					worldVertices.push_back( w );
				}
				FamilyDrawConstants worldConstants;
				const std::array<float, 16> clip = CaseToClip();
				std::copy( clip.begin(), clip.end(), worldConstants.toClip );
				for ( int d = 0; d < 4; ++d )
					worldConstants.world[d * 5] = 1.0f;
				if ( checks.That( worldPipeline.HasValue(), "world-vertex.pipeline." + name ) )
				{
					CaseDraw worldDraw = draw;
					worldDraw.pipeline = worldPipeline.Value();
					worldDraw.vertices = std::as_bytes( std::span( worldVertices ) );
					worldDraw.vertexCount = std::uint32_t( worldVertices.size() );
					worldDraw.drawConstants = std::as_bytes( std::span( &worldConstants, 1 ) );
					const Drawn world = DrawCase( *device, worldDraw );
					int worst = 0;
					std::size_t over = 0;
					for ( std::size_t i = 0; world.ok && i < world.rgba.size(); ++i )
					{
						if ( i % 4 == 3 )
							continue;
						const int d = std::abs( int( world.rgba[i] ) - int( drawn.rgba[i] ) );
						worst = std::max( worst, d );
						over += d > 3;
					}
					std::printf( "INFO world-vertex %s: within %d levels, %zu channels over 3\n",
					    name.c_str(), worst, over );
					checks.That(
					    world.ok && worst <= 1, "world-vertex.matches-the-model-vertex." + name );
					if ( lighting.eye[3] > 0.0f )
					{
						ModelLighting dark = lighting;
						dark.eye[3] = 0.0f;
						CaseDraw darkDraw = worldDraw;
						darkDraw.groups[2].constants = std::as_bytes( std::span( &dark, 1 ) );
						const Drawn unlit = DrawCase( *device, darkDraw );
						checks.That( unlit.ok && unlit.rgba != world.rgba,
						    "world-vertex.the-model-lights-reach-it." + name );
					}
				}
			}
			// Compare against the raw uniform-driven pipeline above, with exactly
			// the same bindings and geometry. Empty view lighting is specialized
			// away; model/ambient lighting remains in the draw group.
			SurfaceVariant specialized = claim.Variant();
			specialized.materialFeatures = SurfaceMaterialFeatures( constants );
			specialized.viewFeatures = 0;
			auto specializedPipeline = family.Value()->Program().Pipeline( specialized );
			if ( checks.That( specializedPipeline.HasValue(), "specialized.pipeline." + name ) )
			{
				draw.pipeline = specializedPipeline.Value();
				const Drawn optimized = DrawCase( *device, draw );
				checks.That( optimized.ok && optimized.rgba == drawn.rgba,
				    "specialized.identical-pixels." + name );
			}
			draw.pipeline = pipeline.Value();
			if ( name == "pbr_ambient" )
			{
				// These predicates must remain present when authored. Restore
				// constants between cases; cutoff/exponent are still uniforms.
				const SurfaceConstants original = constants;
				CaseTexture authoredBase = *base;
				for ( std::size_t texel = 3; texel < authoredBase.texels.size(); texel += 4 )
					authoredBase.texels[texel] = 160;
				draw.groups.back().textures[0] = &authoredBase;
				frame.sunColor[0] = frame.sunColor[1] = frame.sunColor[2] = 0.2f;
				frame.sunDirection[2] = 1.0f;
				specialized.viewFeatures = kSurfaceViewSun;
				for ( int feature = 0; feature < 5; ++feature )
				{
					constants = original;
					if ( feature == 0 )
					{
						constants.flags[1] = 1.0f;
						constants.flags[2] = 0.75f;
					}
					if ( feature == 1 )
						constants.flags[3] = 1.0f;
					if ( feature == 2 )
						constants.meshModes[0] = 1.0f;
					if ( feature == 3 )
						constants.meshProbeColor[2] = 1.0f;
					if ( feature == 4 )
						constants.meshModes[3] = 1.0f;
					draw.pipeline = pipeline.Value();
					const Drawn uniform = DrawCase( *device, draw );
					specialized.materialFeatures = SurfaceMaterialFeatures( constants );
					auto selected = family.Value()->Program().Pipeline( specialized );
					if ( !selected )
					{
						checks.That( false, "specialized.authored-pipeline" );
						continue;
					}
					draw.pipeline = selected.Value();
					const Drawn optimized = DrawCase( *device, draw );
					checks.That( uniform.ok && optimized.ok && uniform.rgba == optimized.rgba,
					    "specialized.authored-pixels." + std::to_string( feature ) );
					SurfaceVariant requested = claim.Variant();
					requested.viewFeatures = kSurfaceViewSun;
					auto request =
					    family.Value()->Program().Request( requested, constants, {}, {} );
					requested.materialFeatures = feature == 2 || feature == 3
					                                 ? kSurfaceDynamicMaterialFeatures
					                                 : SurfaceMaterialFeatures( constants );
					auto expected = family.Value()->Program().Pipeline( requested );
					checks.That(
					    request && expected && request.Value().pipeline == expected.Value(),
					    "specialized.request-policy." + std::to_string( feature ) );
					if ( request )
					{
						draw.pipeline = request.Value().pipeline;
						const Drawn requestedImage = DrawCase( *device, draw );
						checks.That( requestedImage.ok && requestedImage.rgba == uniform.rgba,
						    "specialized.request-pixels." + std::to_string( feature ) );
					}
					SurfaceVariant omitted = specialized;
					omitted.materialFeatures &= ~( 1u << feature );
					auto missing = family.Value()->Program().Pipeline( omitted );
					if ( missing )
					{
						draw.pipeline = missing.Value();
						const Drawn wrong = DrawCase( *device, draw );
						checks.That( wrong.ok && wrong.rgba != uniform.rgba,
						    "specialized.omitted-authored-feature-detected." +
						        std::to_string( feature ) );
					}
					else
						checks.That( false, "specialized.omitted-authored-pipeline" );
				}
				draw.groups.back().textures[0] = &*base;
				constants = original;
				draw.pipeline = pipeline.Value();
				frame.sunColor[0] = frame.sunColor[1] = frame.sunColor[2] = 0.2f;
				frame.sunDirection[2] = 1.0f;
				const Drawn sunUniform = DrawCase( *device, draw );
				auto sun =
				    family.Value()->Program().ViewPipeline( pipeline.Value(), {}, kSurfaceViewSun );
				auto missingSun = family.Value()->Program().ViewPipeline( pipeline.Value(), {}, 0 );
				if ( sun && missingSun )
				{
					draw.pipeline = sun.Value();
					const Drawn present = DrawCase( *device, draw );
					draw.pipeline = missingSun.Value();
					const Drawn absent = DrawCase( *device, draw );
					checks.That( sunUniform.ok && present.ok && present.rgba == sunUniform.rgba,
					    "specialized.sun-reappears-with-identical-pixels" );
					checks.That( absent.ok && absent.rgba != sunUniform.rgba,
					    "specialized.missing-required-sun-is-detected" );
				}
				else
					checks.That( false, "specialized.sun-pipelines" );
				frame.sunColor[0] = frame.sunColor[1] = frame.sunColor[2] = 0.0f;
				draw.pipeline = pipeline.Value();
			}
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
	}
	return devices.Finish( checks );
}
