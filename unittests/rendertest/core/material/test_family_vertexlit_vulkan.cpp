//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.vertexlit (RFC 0016 K4 "Families match ports", the
//			`vertexlit` family) on render.device.vulkan.
//
//			Each case of quality/fixtures/legacy-shaders/families/
//			vertexlit.vdf (a VertexLitGeneric material, its textures, an
//			ambient cube and up to four lights, and a clip-space quad with
//			per-vertex normals) is imported with render.material's VMT
//			importer into a `vertexlit` parameter block, claimed by the family
//			(ClaimVertexLit) and drawn with the family's pipeline, the
//			material group and a draw group holding the case's lighting
//			(PackSourceModelLighting, as LightingGroup packs it), into a
//			256x256 sRGB target, as the material pixel harness draws it. The
//			draw constants are the case's to-clip and the identity
//			object-to-world, as the harness's identity transforms. The sampled
//			pixels must match the legacy port's within kTolerance levels per
//			channel (quality/fixtures/render-families/vertexlit-port-v1.vdf,
//			recorded by tools/render/family_port_pixels.py from a
//			legacy_shader_conformance.py run judged against the retail D3D9
//			bytecode). With the Khronos validation layer installed, the run
//			must report no message. family_pixel_cases.h holds the shared
//			harness.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_VERTEXLIT_
//			SEEDED_IGNORE_HALF_LAMBERT (vertexlit_family.cpp) drops
//			$halflambert, so the family lights with plain Lambert.
//
//=============================================================================//

#include "family_pixel_cases.h"
#include "render/device/vulkan/provider.h"
#include "render/material/vertexlit_family.h"
#include "render/material/vmt_import.h"
#include "testing/checks.h"

#include <algorithm>
#include <array>
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

const char *const kCaseFile = "quality/fixtures/legacy-shaders/families/vertexlit.vdf";
const char *const kFixture = "quality/fixtures/render-families/vertexlit-port-v1.vdf";

ModelLightType LightType( const std::string &type )
{
	if ( type == "spot" )
		return ModelLightType::kSpot;
	if ( type == "directional" )
		return ModelLightType::kDirectional;
	return ModelLightType::kPoint;
}

// The case's lighting as the legacy frontend hands it over: the harness's
// eye at the origin, its cube and its lights.
VertexLitLighting CaseLighting( const FamilyCase &testCase )
{
	std::vector<ModelLightDesc> lights;
	for ( const ModelLight &light : testCase.lights )
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
	const float eye[3] = {};
	return PackSourceModelLighting( eye, testCase.cube, lights );
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::optional<CaseSet> set = LoadCases( checks, kCaseFile, kFixture );
	if ( !set )
		return checks.Report();
	const FamilyRegistry registry = BuiltinFamilies();
	const FamilySchema *vertexlit = registry.Find( "vertexlit" );
	if ( !checks.That( vertexlit != nullptr, "setup.vertexlit-family-registers" ) )
		return checks.Report();

	auto claimOf = [&]( const char *vmt ) -> std::optional<VertexLitClaim>
	{
		VmtImportContext context;
		auto imported = ImportVmt( vmt, context );
		if ( !imported )
			return std::nullopt;
		ParameterBlock block( *vertexlit );
		if ( !ApplyValues( imported.Value(), block ) )
			return std::nullopt;
		for ( const MaterialValue &value : imported.Value().values )
		{
			if ( value.kind == ValueKind::kTexture && !value.text.empty() )
				(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
		}
		return ClaimVertexLit( block );
	};

	// The family refuses what it does not draw, naming it.
	{
		auto refused = [&]( const std::string &key, const char *value, const char *named )
		{
			const std::string vmt =
			    "\"VertexLitGeneric\" { \"$basetexture\" \"a\" \"" + key + "\" \"" + value + "\" }";
			const std::optional<VertexLitClaim> claim = claimOf( vmt.c_str() );
			return claim && !claim->claimed && claim->reason.find( named ) != std::string::npos;
		};
		checks.That( refused( "$bumpmap", "b", "bumpmap" ), "claim.refuses-a-bump-map-by-name" );
		checks.That( refused( "$envmap", "e", "envmap" ), "claim.refuses-an-env-map-by-name" );
		checks.That( refused( "$detail", "d", "detail" ), "claim.refuses-detail-by-name" );
		checks.That(
		    refused( "$selfillum", "1", "selfillum" ), "claim.refuses-self-illumination-by-name" );
		checks.That( refused( "$phong", "1", "phong" ), "claim.refuses-phong-by-name" );
		checks.That( refused( "$rimlight", "1", "rimlight" ), "claim.refuses-a-rim-light-by-name" );
		checks.That( refused( "$lightwarptexture", "w", "lightwarptexture" ),
		    "claim.refuses-a-light-warp-by-name" );
		checks.That( refused( "$additive", "1", "additive" ), "claim.refuses-additive-by-name" );
		checks.That( refused( "$basetexturetransform",
		                 "center .5 .5 scale 2 2 rotate 0 translate 0 0", "basetexturetransform" ),
		    "claim.refuses-a-base-texture-transform-by-name" );
	}
	// Blending and the alpha write follow the port's blend requirements.
	{
		auto blendOf = [&]( const char *vmt, device::BlendMode blend, bool alphaWrite )
		{
			const std::optional<VertexLitClaim> claim = claimOf( vmt );
			return claim && claim->claimed && claim->blend == blend &&
			       claim->alphaWrite == alphaWrite;
		};
		checks.That( blendOf( "\"VertexLitGeneric\" { \"$basetexture\" \"a\" }",
		                 device::BlendMode::kOpaque, true ),
		    "claim.opaque-writes-alpha" );
		checks.That( blendOf( "\"VertexLitGeneric\" { \"$basetexture\" \"a\" \"$alpha\" \"0.5\" }",
		                 device::BlendMode::kAlpha, false ),
		    "claim.alpha-modulation-blends" );
		checks.That(
		    blendOf( "\"VertexLitGeneric\" { \"$basetexture\" \"a\" \"$vertexalpha\" \"1\" }",
		        device::BlendMode::kAlpha, false ),
		    "claim.vertex-alpha-blends" );
		checks.That(
		    blendOf( "\"VertexLitGeneric\" { \"$basetexture\" \"a\" \"$alphatest\" \"1\" }",
		        device::BlendMode::kOpaque, false ),
		    "claim.alpha-test-leaves-alpha" );
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
		auto family = VertexLitFamily::Create(
		    *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( family.HasValue(), "family.creates" ) )
			return checks.Report();

		// The requests MaterialPrograms and DrawGroups take.
		{
			const std::optional<VertexLitClaim> claim =
			    claimOf( "\"VertexLitGeneric\" { \"$basetexture\" \"a\" }" );
			auto request = family.Value()->Request( claim.value_or( VertexLitClaim() ), "a" );
			checks.That(
			    claim && claim->claimed && request.HasValue() &&
			        request.Value().pipeline.IsValid() &&
			        request.Value().drawLayout == family.Value()->DrawLayout() &&
			        request.Value().material.layout == family.Value()->MaterialLayout() &&
			        request.Value().vertexStride == sizeof( VertexLitVertex ) &&
			        request.Value().drawConstantBytes == sizeof( FamilyDrawConstants ) &&
			        request.Value().material.constants.size() == sizeof( VertexLitConstants ) &&
			        request.Value().material.textures.size() == 1,
			    "request.names-the-draw-layout-and-the-material-group" );
			const VertexLitLighting lighting;
			const GroupRequest group = family.Value()->LightingGroup( lighting );
			checks.That( group.layout == family.Value()->DrawLayout() &&
			                 group.constants.size() == sizeof( VertexLitLighting ) &&
			                 group.textures.empty(),
			    "request.lighting-is-the-draw-group" );
		}

		for ( const FamilyCase &testCase : set->cases )
		{
			const std::string &name = testCase.name;
			const ImportedCase imported = ImportCase( checks, testCase, *vertexlit );
			if ( !imported.block || testCase.triangles.empty() )
				continue;
			const CaseTexture *texture = nullptr;
			for ( const MaterialValue &value : imported.material.values )
			{
				if ( value.parameter == "basetexture" )
					texture = FindTexture( *set, value.text );
			}
			if ( !checks.That( texture != nullptr, "import." + name + ".base-texture-resolves" ) )
				continue;
			const VertexLitClaim claim = ClaimVertexLit( *imported.block );
			if ( !That( checks, claim.claimed, "claim." + name, claim.reason ) )
				continue;
			auto pipeline = family.Value()->Pipeline( claim );
			if ( !checks.That( pipeline.HasValue(), "pipeline." + name ) )
				continue;

			std::vector<VertexLitVertex> quad;
			for ( const CaseVertex &corner : testCase.triangles )
			{
				VertexLitVertex vertex;
				std::copy( corner.position, corner.position + 3, vertex.position );
				std::copy( corner.normal, corner.normal + 3, vertex.normal );
				std::copy( corner.uv0, corner.uv0 + 2, vertex.uv );
				quad.push_back( vertex );
			}
			const GroupRequest lighting = family.Value()->LightingGroup( CaseLighting( testCase ) );
			VertexLitDrawConstants drawConstants;
			const std::array<float, 16> toClip = CaseToClip();
			std::copy( toClip.begin(), toClip.end(), drawConstants.toClip );
			for ( int i = 0; i < 4; ++i )
				drawConstants.world[i * 5] = 1.0f;
			CaseDraw draw;
			draw.pipeline = pipeline.Value();
			draw.groups.push_back(
			    { device::BindGroupRole::kMaterial, family.Value()->MaterialLayout(),
			        std::as_bytes( std::span( &claim.constants, 1 ) ), { texture } } );
			draw.groups.push_back(
			    { device::BindGroupRole::kDraw, lighting.layout, lighting.constants, {} } );
			draw.vertices = std::as_bytes( std::span( quad ) );
			draw.vertexCount = std::uint32_t( quad.size() );
			draw.drawConstants = std::as_bytes( std::span( &drawConstants, 1 ) );
			std::copy( testCase.clear, testCase.clear + 4, draw.clear );
			const Drawn drawn = DrawCase( *device, draw );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			JudgeCase( checks, testCase, drawn );
		}
		checks.That( drawnCases == 10, "cases.every-case-drew" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
