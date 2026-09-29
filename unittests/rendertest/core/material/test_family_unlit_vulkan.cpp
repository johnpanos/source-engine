//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.unlit (RFC 0016 K4 "Families match ports", the
//			`unlit` family) on render.device.vulkan.
//
//			Each case of quality/fixtures/legacy-shaders/families/unlit.vdf
//			(an UnlitGeneric material, its textures and a clip-space quad) is
//			imported with render.material's VMT importer into an `unlit`
//			parameter block, claimed by the family (ClaimUnlit) and drawn with
//			the family's pipeline into a 256x256 sRGB target, as the material
//			pixel harness draws it. The sampled pixels must match the legacy
//			port's within kTolerance levels per channel: the port's pixels are
//			the versioned fixture quality/fixtures/render-families/
//			unlit-port-v1.vdf, recorded from a legacy_shader_conformance.py run
//			that judged the port against the retail D3D9 bytecode
//			(tools/render/family_port_pixels.py). The fixture must name the
//			case file with its current sha256, so a changed case needs a new
//			recording. With the Khronos validation layer installed, the run
//			must report no message.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_UNLIT_SEEDED_
//			IGNORE_VERTEX_COLOR (unlit_family.cpp) packs $vertexcolor as off.
//
//=============================================================================//

#include "family_pixel_cases.h"
#include "render/device/vulkan/provider.h"
#include "render/material/unlit_family.h"
#include "render/material/vmt_import.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
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

const char *const kCaseFile = "quality/fixtures/legacy-shaders/families/unlit.vdf";
const char *const kFixture = "quality/fixtures/render-families/unlit-port-v1.vdf";

} // namespace

int main()
{
	testing::Checks checks;
	const std::optional<CaseSet> set = LoadCases( checks, kCaseFile, kFixture );
	if ( !set )
		return checks.Report();
	const FamilyRegistry registry = BuiltinFamilies();
	const FamilySchema *unlit = registry.Find( "unlit" );
	if ( !checks.That( unlit != nullptr, "setup.unlit-family-registers" ) )
		return checks.Report();

	// The family refuses what it does not draw, naming it.
	{
		auto refused = [&]( const char *vmt, const char *named )
		{
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return false;
			ParameterBlock block( *unlit );
			if ( !ApplyValues( imported.Value(), block ) )
				return false;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			if ( const std::optional<std::size_t> index = unlit->IndexOf( named );
			    index && unlit->layout[*index].type == ParameterType::kTexture )
				(void)block.SetTexture( named, device::TextureId( 2 ) );
			const UnlitClaim claim = ClaimUnlit( block );
			return !claim.claimed && claim.reason.find( named ) != std::string::npos;
		};
		checks.That(
		    refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$detail\" \"b\" }", "detail" ),
		    "claim.refuses-a-detail-texture-by-name" );
		checks.That(
		    refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$envmapcontrast\" \"0.5\" }",
		        "envmapcontrast" ),
		    "claim.refuses-an-env-map-parameter-by-name" );
		checks.That( refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$translucent\" \"1\" "
		                      "\"$additive\" \"1\" }",
		                 "additive" ),
		    "claim.refuses-translucent-additive" );
		checks.That( refused( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$basetexturetransform\" "
		                      "\"center .5 .5 scale 2 2 rotate 0 translate 0 0\" }",
		                 "basetexturetransform" ),
		    "claim.refuses-a-texture-transform-by-name" );
	}

	// The alpha test's reference as the legacy shaders set it: 0.7 when the
	// material gives none, and held as a byte.
	{
		auto reference = [&]( const char *vmt ) -> float
		{
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return -1.0f;
			ParameterBlock block( *unlit );
			if ( !ApplyValues( imported.Value(), block ) )
				return -1.0f;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			const UnlitClaim claim = ClaimUnlit( block );
			return claim.claimed ? claim.constants.flags[2] : -1.0f;
		};
		checks.That(
		    reference( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$alphatest\" \"1\" }" ) ==
		        178.0f / 255.0f,
		    "claim.alpha-test-without-a-reference-uses-0.7" );
		checks.That( reference( "\"UnlitGeneric\" { \"$basetexture\" \"a\" \"$alphatest\" \"1\" "
		                        "\"$alphatestreference\" \"0.5\" }" ) == 127.0f / 255.0f,
		    "claim.alpha-test-reference-is-a-byte" );
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
		auto family =
		    UnlitFamily::Create( *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( family.HasValue(), "family.creates" ) )
			return checks.Report();
		// The surface program's unread inputs: env map, mask, bump, detail,
		// MRAO, emission, the split-sum table and the lightmap page.
		const CaseTexture neutral = NeutralCaseTexture( device::Format::kRGBA8Unorm );
		const CaseTexture neutralSrgb = NeutralCaseTexture( device::Format::kRGBA8Srgb );
		const CaseTexture neutralCube = NeutralCaseTexture( device::Format::kRGBA8Srgb, true );
		const SurfaceFrame frame;
		const ModelLighting lighting;

		for ( const FamilyCase &testCase : set->cases )
		{
			const std::string &name = testCase.name;
			const ImportedCase imported = ImportCase( checks, testCase, *unlit );
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
			const UnlitClaim claim = ClaimUnlit( *imported.block );
			if ( !That( checks, claim.claimed, "claim." + name, claim.reason ) )
				continue;
			auto pipeline = family.Value()->Pipeline( claim );
			if ( !checks.That( pipeline.HasValue(), "pipeline." + name ) )
				continue;

			std::vector<SurfaceFlatVertex> quad;
			for ( const CaseVertex &corner : testCase.triangles )
			{
				SurfaceFlatVertex vertex;
				std::copy( corner.position, corner.position + 3, vertex.position );
				std::copy( corner.uv0, corner.uv0 + 2, vertex.uv );
				std::copy( corner.color, corner.color + 4, vertex.color );
				quad.push_back( vertex );
			}
			CaseDraw draw;
			draw.pipeline = pipeline.Value();
			// The constants as Request packs them.
			SurfaceConstants constants = claim.constants;
			constants.state[0] =
			    claim.blend == device::BlendMode::kOpaque && claim.alphaWrite ? 1.0f : 0.0f;
			draw.groups.push_back( { device::BindGroupRole::kMaterial,
			    family.Value()->MaterialLayout(), std::as_bytes( std::span( &constants, 1 ) ),
			    { texture, &neutralCube, &neutral, &neutral, &neutral, &neutral, &neutralSrgb } } );
			draw.groups.push_back( { device::BindGroupRole::kDraw, family.Value()->DrawLayout(),
			    std::as_bytes( std::span( &lighting, 1 ) ), { &neutralSrgb }, true } );
			draw.groups.push_back( NeutralViewGroup( family.Value()->ViewLayout() ) );
			draw.groups.push_back( { device::BindGroupRole::kFrame, family.Value()->FrameLayout(),
			    std::as_bytes( std::span( &frame, 1 ) ), { &neutral, &neutral } } );
			draw.vertices = std::as_bytes( std::span( quad ) );
			draw.vertexCount = std::uint32_t( quad.size() );
			std::copy( testCase.clear, testCase.clear + 4, draw.clear );
			const Drawn drawn = DrawCase( *device, draw );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			JudgeCase( checks, testCase, drawn );
		}
		checks.That( drawnCases == 6, "cases.every-case-drew" );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
