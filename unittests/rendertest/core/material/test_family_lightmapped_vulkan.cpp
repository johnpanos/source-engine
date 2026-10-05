//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.lightmapped (RFC 0016 K4 "Families match ports", the
//			`lightmapped` family) on render.device.vulkan.
//
//			Each case of quality/fixtures/legacy-shaders/families/
//			lightmapped.vdf (a LightmappedGeneric material, its textures, a
//			lightmap and a clip-space quad) is imported with render.material's
//			VMT importer into a `lightmapped` parameter block, claimed by the
//			family (ClaimLightmapped) and drawn with the family's pipeline,
//			the material group and a draw group holding the case's lightmap,
//			into a 256x256 sRGB target, as the material pixel harness draws
//			it. The sampled pixels must match the legacy port's within
//			kTolerance levels per channel (quality/fixtures/render-families/
//			lightmapped-port-v1.vdf, recorded by
//			tools/render/family_port_pixels.py from a
//			legacy_shader_conformance.py run judged against the retail D3D9
//			bytecode). With the Khronos validation layer installed, the run
//			must report no message. family_pixel_cases.h holds the shared
//			harness.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_LIGHTMAPPED_
//			SEEDED_GAMMA_COLOR (lightmapped_family.cpp) gamma-converts $color,
//			as unlit does and the port does not.
//
//=============================================================================//

#include "family_devices.h"
#include "family_pixel_cases.h"
#include "render/material/draw_program.h"
#include "render/material/lightmapped_family.h"
#include "render/material/vmt_import.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <cmath>
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

const SurfaceFrame kLdrFrame;

const char *const kCaseFile = "quality/fixtures/legacy-shaders/families/lightmapped.vdf";
const char *const kFixture = "quality/fixtures/render-families/lightmapped-port-v1.vdf";

} // namespace

int main()
{
	testing::Checks checks;
	const std::optional<CaseSet> set = LoadCases( checks, kCaseFile, kFixture );
	if ( !set )
		return checks.Report();
	const FamilyRegistry registry = BuiltinFamilies();
	const FamilySchema *lightmapped = registry.Find( "lightmapped" );
	if ( !checks.That( lightmapped != nullptr, "setup.lightmapped-family-registers" ) )
		return checks.Report();

	// The family refuses what it does not draw, naming it.
	{
		auto refused = [&]( const char *vmt, const char *named )
		{
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return false;
			ParameterBlock block( *lightmapped );
			if ( !ApplyValues( imported.Value(), block ) )
				return false;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			if ( const std::optional<std::size_t> index = lightmapped->IndexOf( named );
			    index && lightmapped->layout[*index].type == ParameterType::kTexture )
				(void)block.SetTexture( named, device::TextureId( 2 ) );
			const LightmappedClaim claim = ClaimLightmapped( block );
			return !claim.claimed && claim.reason.find( named ) != std::string::npos;
		};
		checks.That( refused( "\"WorldVertexTransition\" { \"$basetexture\" \"a\" "
		                      "\"$bumpmap\" \"b\" \"$bumpmap2\" \"c\" }",
		                 "bumpmap2" ),
		    "claim.refuses-a-second-bump-map-by-name" );
		checks.That( refused( "\"WorldVertexTransition\" { \"$basetexture\" \"a\" "
		                      "\"$basetexture2\" \"b\" }",
		                 "basetexture2" ),
		    "claim.refuses-a-second-base-texture-by-name" );
		checks.That(
		    refused( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$additive\" \"1\" }",
		        "additive" ),
		    "claim.refuses-additive-by-name" );
		checks.That(
		    refused( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$envmapmasktransform\" "
		             "\"center .5 .5 scale 2 2 rotate 0 translate 0 0\" }",
		        "envmapmasktransform" ),
		    "claim.refuses-an-env-map-mask-transform-by-name" );
		checks.That(
		    refused( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$basetexturetransform\" "
		             "\"center .5 .5 scale 2 2 rotate 0 translate 0 0\" }",
		        "basetexturetransform" ),
		    "claim.refuses-a-texture-transform-by-name" );
	}

	// The env map's knobs as the port's fast and slow paths use them: contrast
	// takes effect only with saturation, and otherwise is 0 or 1.
	{
		auto claimOf = [&]( const char *vmt ) -> std::optional<LightmappedClaim>
		{
			VmtImportContext context;
			auto imported = ImportVmt( vmt, context );
			if ( !imported )
				return std::nullopt;
			ParameterBlock block( *lightmapped );
			if ( !ApplyValues( imported.Value(), block ) )
				return std::nullopt;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			(void)block.SetTexture( "envmap", device::TextureId( 2 ) );
			const LightmappedClaim claim = ClaimLightmapped( block );
			return claim.claimed ? std::optional<LightmappedClaim>( claim ) : std::nullopt;
		};
		auto fast = claimOf( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$envmap\" \"e\" "
		                     "\"$envmapcontrast\" \"0.4\" }" );
		auto slow = claimOf( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$envmap\" \"e\" "
		                     "\"$envmapcontrast\" \"0.4\" \"$envmapsaturation\" \"0.5\" }" );
		auto one = claimOf( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$envmap\" \"e\" "
		                    "\"$envmapcontrast\" \"1\" }" );
		checks.That( fast && fast->constants.envContrast[0] == 0.0f &&
		                 fast->constants.envSaturation[0] == 1.0f,
		    "claim.contrast-without-saturation-takes-the-fast-path" );
		checks.That( slow && slow->constants.envContrast[0] == 0.4f &&
		                 slow->constants.envSaturation[0] == 0.5f,
		    "claim.contrast-with-saturation-takes-effect" );
		checks.That( one && one->constants.envContrast[0] == 1.0f,
		    "claim.contrast-one-is-fastpathenvmapcontrast" );
		checks.That( fast && ( fast->terms & kSurfaceEnvmap ) != 0, "claim.an-env-map-is-a-term" );
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
			ParameterBlock block( *lightmapped );
			if ( !ApplyValues( imported.Value(), block ) )
				return -1.0f;
			(void)block.SetTexture( "basetexture", device::TextureId( 1 ) );
			const LightmappedClaim claim = ClaimLightmapped( block );
			return claim.claimed ? claim.constants.flags[2] : -1.0f;
		};
		checks.That(
		    reference( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$alphatest\" \"1\" }" ) ==
		        178.0f / 255.0f,
		    "claim.alpha-test-without-a-reference-uses-0.7" );
		checks.That(
		    reference( "\"LightmappedGeneric\" { \"$basetexture\" \"a\" \"$alphatest\" \"1\" "
		               "\"$alphatestreference\" \"0.5\" }" ) == 127.0f / 255.0f,
		    "claim.alpha-test-reference-is-a-byte" );
	}

	CaseDevices devices( checks );
	for ( const CaseDevices::Entry &entry : devices.entries )
	{
		std::printf( "INFO device %s\n", entry.name.c_str() );
		device::IRenderDevice2 *const device = entry.device.get();
		int drawnCases = 0;
		bool fogChecked = false;
		bool areaChecked = false;
		auto family = LightmappedFamily::Create(
		    *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( family.HasValue(), "family.creates" ) )
			continue;

		for ( const FamilyCase &testCase : set->cases )
		{
			const std::string &name = testCase.name;
			const ImportedCase imported = ImportCase( checks, testCase, *lightmapped );
			if ( !imported.block || testCase.triangles.empty() )
				continue;
			// The material's textures by binding, in the formats the family
			// reads them (gamma images through sRGB, data as unorm), and the
			// neutral ones for inputs the material leaves out.
			auto textureOf = [&]( std::string_view parameter ) -> const CaseTexture *
			{
				for ( const MaterialValue &value : imported.material.values )
				{
					if ( value.parameter == parameter )
						return FindTexture( *set, value.text );
				}
				return nullptr;
			};
			const CaseTexture *texture = textureOf( "basetexture" );
			if ( !checks.That( texture != nullptr, "import." + name + ".base-texture-resolves" ) ||
			     !checks.That(
			         testCase.lightmap != nullptr, "case." + name + ".lightmap-resolves" ) )
				continue;
			const LightmappedClaim claim = ClaimLightmapped( *imported.block );
			if ( !That( checks, claim.claimed, "claim." + name, claim.reason ) )
				continue;
			const bool surface = ( claim.terms & kSurfaceNormalTerms ) != 0;
			const SurfaceVertexLayout layout =
			    surface ? SurfaceVertexLayout::kWorld : SurfaceVertexLayout::kFlat;
			auto pipeline = family.Value()->Pipeline( claim, layout );
			if ( !checks.That( pipeline.HasValue(), "pipeline." + name ) )
				continue;
			std::vector<CaseTexture> bound;
			bound.reserve( 8 );
			auto bind = [&]( const CaseTexture *source, device::Format format, bool cube )
			{
				CaseTexture copy;
				if ( source )
					copy = *source;
				else
				{
					copy.width = copy.height = 1;
					copy.cube = cube;
					copy.texels.assign( cube ? 24 : 4, 255 );
				}
				copy.format = format;
				bound.push_back( std::move( copy ) );
				return &bound.back();
			};
			const std::vector<const CaseTexture *> materialTextures = {
			    bind( texture, device::Format::kRGBA8Srgb, false ),
			    bind( textureOf( "envmap" ), device::Format::kRGBA8Srgb, true ),
			    bind( textureOf( "envmapmask" ), device::Format::kRGBA8Unorm, false ),
			    bind( textureOf( "bumpmap" ), device::Format::kRGBA8Unorm, false ),
			    bind( textureOf( "detail" ),
			        claim.detailMode == 1 ? device::Format::kRGBA8Srgb
			                              : device::Format::kRGBA8Unorm,
			        false ),
			    bind( nullptr, device::Format::kRGBA8Unorm, false ),  // MRAO
			    bind( nullptr, device::Format::kRGBA8Srgb, false ) }; // emission
			// The frame's split-sum and LTC tables, which no lightmapped point reads.
			const CaseTexture *splitSum = bind( nullptr, device::Format::kRGBA8Unorm, false );
			// The draw's model lighting: neutral on a world surface.
			const ModelLighting lighting;

			std::vector<SurfaceFlatVertex> flat;
			std::vector<SurfaceWorldVertex> surfaceQuad;
			for ( const CaseVertex &corner : testCase.triangles )
			{
				SurfaceWorldVertex vertex;
				std::copy( corner.position, corner.position + 3, vertex.position );
				std::copy( corner.uv0, corner.uv0 + 2, vertex.uv );
				std::copy( corner.uv1, corner.uv1 + 2, vertex.lightmapUv );
				std::copy( corner.color, corner.color + 4, vertex.color );
				std::copy( corner.normal, corner.normal + 3, vertex.normal );
				std::copy( corner.tangentS, corner.tangentS + 3, vertex.tangentS );
				std::copy( corner.tangentT, corner.tangentT + 3, vertex.tangentT );
				vertex.lightmapOffset = corner.uv2[0];
				surfaceQuad.push_back( vertex );
				SurfaceFlatVertex plain;
				std::copy( corner.position, corner.position + 3, plain.position );
				std::copy( corner.uv0, corner.uv0 + 2, plain.uv );
				std::copy( corner.uv1, corner.uv1 + 2, plain.lightmapUv );
				std::copy( corner.color, corner.color + 4, plain.color );
				flat.push_back( plain );
			}
			// The constants as Request packs them.
			SurfaceConstants constants = claim.constants;
			constants.state[0] =
			    claim.blend == device::BlendMode::kOpaque && claim.alphaWrite ? 1.0f : 0.0f;
			CaseDraw draw;
			draw.pipeline = pipeline.Value();
			draw.groups.push_back(
			    { device::BindGroupRole::kMaterial, family.Value()->MaterialLayout(),
			        std::as_bytes( std::span( &constants, 1 ) ), materialTextures } );
			draw.groups.push_back( { device::BindGroupRole::kDraw, family.Value()->DrawLayout(),
			    std::as_bytes( std::span( &lighting, 1 ) ), { testCase.lightmap, splitSum, splitSum }, 1 } );
			// The frame terms at their LDR defaults (the port's cases are LDR).
			draw.groups.push_back( NeutralViewGroup( family.Value()->ViewLayout() ) );
			draw.groups.push_back( { device::BindGroupRole::kFrame, family.Value()->FrameLayout(),
			    std::as_bytes( std::span( &kLdrFrame, 1 ) ),
			    { splitSum, splitSum, splitSum, splitSum, splitSum, splitSum } } );
			draw.vertices = surface ? std::as_bytes( std::span( surfaceQuad ) )
			                        : std::as_bytes( std::span( flat ) );
			draw.vertexCount = std::uint32_t( flat.size() );
			// The draw constants the layout declares: the world point's push
			// block is { toClip, world } (surface_world_vertex.glsl) and the flat
			// point's is the clip matrix alone. A world-surface case that supplies
			// only the matrix is refused for an incomplete block, and a flat one
			// that supplies the world rows exceeds its declared range.
			FamilyDrawConstants drawConstants;
			const std::array<float, 16> toClip = CaseToClip();
			std::copy( toClip.begin(), toClip.end(), drawConstants.toClip );
			if ( surface )
				for ( int i = 0; i < 4; ++i )
					drawConstants.world[i * 4 + i] = 1.0f;
			draw.drawConstants = std::as_bytes( std::span( &drawConstants, 1 ) )
			                         .first( surface ? sizeof( FamilyDrawConstants )
			                                         : sizeof( FamilyDrawConstants::toClip ) );
			std::copy( testCase.clear, testCase.clear + 4, draw.clear );
			const Drawn drawn = DrawCase( *device, draw );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			devices.Compare( checks, entry, name, drawn );
			JudgeCase( checks, testCase, drawn );

			// The frame's emitting surfaces (render.area-light.v1) on the
			// first opaque case with world vertices, lit by a rectangle
			// covering its hemisphere: an unbaked light brightens it by
			// albedo x tint x radiance, so twice the radiance adds twice the
			// light in linear light; a light whose diffuse is in the bake, or
			// one facing away, adds nothing.
			if ( !areaChecked && surface && claim.blend == device::BlendMode::kOpaque &&
			     ( claim.terms & ( kSurfaceUnlit | kSurfaceSelfIllum ) ) == 0 &&
			     !testCase.triangles.empty() )
			{
				areaChecked = true;
				const CaseVertex &corner = testCase.triangles[0];
				auto frameWith = [&]( float radiance, bool baked, bool away )
				{
					SurfaceFrame frame;
					area_light::AreaLight light;
					const float side = away ? -1.0f : 1.0f;
					// Just off the surface, facing it (or away), far wider
					// than the quad: its form factor is 1 over the quad.
					for ( int k = 0; k < 3; ++k )
						light.rect.center[k] = corner.position[k] + 0.05f * corner.normal[k];
					float u[3], v[3];
					const float *t = corner.tangentS;
					const float *n = corner.normal;
					// V = t x n, so U x V = t x ( t x n ) = -n: the front
					// faces the surface (unless away).
					v[0] = t[1] * n[2] - t[2] * n[1];
					v[1] = t[2] * n[0] - t[0] * n[2];
					v[2] = t[0] * n[1] - t[1] * n[0];
					for ( int k = 0; k < 3; ++k )
					{
						u[k] = 1000.0f * t[k];
						light.rect.halfU[k] = u[k];
						light.rect.halfV[k] = 1000.0f * side * v[k];
					}
					for ( float &c : light.radiance )
						c = radiance;
					light.reach = area_light::kMaxReach;
					frame.areaCount[0] = 1.0f;
					frame.areas[0] = PackAreaLight( light, baked, -1 );
					return frame;
				};
				auto drawWith = [&]( const SurfaceFrame &frame )
				{
					CaseDraw lit = draw;
					lit.groups.back().constants = std::as_bytes( std::span( &frame, 1 ) );
					return DrawCase( *device, lit );
				};
				const SurfaceFrame once = frameWith( 0.05f, false, false );
				const SurfaceFrame twice = frameWith( 0.1f, false, false );
				const SurfaceFrame baked = frameWith( 0.1f, true, false );
				const SurfaceFrame away = frameWith( 0.1f, false, true );
				const Drawn withOnce = drawWith( once );
				const Drawn withTwice = drawWith( twice );
				const Drawn withBaked = drawWith( baked );
				const Drawn withAway = drawWith( away );
				if ( checks.That( withOnce.ok && withTwice.ok && withBaked.ok && withAway.ok,
				         "area.draws" ) )
				{
					const auto toLinear = []( float c )
					{
						c /= 255.0f;
						return c <= 0.04045f ? c / 12.92f
						                     : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
					};
					int brighter = 0, pixels = 0, linearOff = 0;
					bool bakedSame = withBaked.rgba == drawn.rgba;
					bool awaySame = withAway.rgba == drawn.rgba;
					for ( std::size_t i = 0; i + 3 < drawn.rgba.size(); i += 4 )
					{
						for ( int c = 0; c < 3; ++c )
						{
							const float plain = toLinear( drawn.rgba[i + c] );
							const float one = toLinear( withOnce.rgba[i + c] ) - plain;
							// Unsaturated, lit channels only.
							if ( withTwice.rgba[i + c] >= 250 || one <= 0.004f )
								continue;
							++pixels;
							brighter += one > 0.0f ? 1 : 0;
							// Judged in output levels: twice the first gain,
							// encoded, within 2 levels of the second draw.
							const float expected = 2.0f * one + plain;
							const float encoded =
							    255.0f *
							    ( expected <= 0.0031308f
							            ? expected * 12.92f
							            : 1.055f * std::pow( expected, 1.0f / 2.4f ) - 0.055f );
							if ( std::fabs( encoded - float( withTwice.rgba[i + c] ) ) > 2.0f )
								++linearOff;
						}
					}
					That( checks, pixels > 0 && brighter == pixels,
					    "area.an-unbaked-light-brightens-the-surface",
					    std::to_string( pixels ) + " channels lit" );
					That( checks, pixels > 0 && linearOff == 0,
					    "area.twice-the-radiance-adds-twice-the-light",
					    std::to_string( linearOff ) + " of " + std::to_string( pixels ) +
					        " channels off" );
					checks.That( bakedSame, "area.a-baked-light-adds-no-diffuse" );
					checks.That( awaySame, "area.a-light-facing-away-adds-nothing" );
				}
			}

			// The view's range fog, on the first opaque case: with 1 / range 0
			// and start / range -0.5 the factor is 0.5 everywhere, squared to
			// 0.25, so each pixel is a quarter of the way to the fog color in
			// linear light (common_ps_fxc.h BlendPixelFog).
			if ( fogChecked || claim.blend != device::BlendMode::kOpaque )
				continue;
			fogChecked = true;
			SurfaceFrame fogged;
			const float fogColor[3] = { 0.8f, 0.2f, 0.05f };
			std::copy( fogColor, fogColor + 3, fogged.fogColor );
			fogged.fogColor[3] = 0.0f;
			fogged.fogParams[0] = -0.5f;
			fogged.fogParams[2] = 1.0f;
			fogged.fogParams[3] = 0.0f;
			CaseDraw fogDraw = draw;
			fogDraw.groups.back().constants = std::as_bytes( std::span( &fogged, 1 ) );
			const Drawn withFog = DrawCase( *device, fogDraw );
			if ( !checks.That( withFog.ok, "fog.draws" ) )
				continue;
			const auto toLinear = []( float c )
			{
				return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
			};
			const auto toSrgb = []( float c )
			{
				return c <= 0.0031308f ? c * 12.92f : 1.055f * std::pow( c, 1.0f / 2.4f ) - 0.055f;
			};
			int worst = 0;
			for ( std::size_t i = 0; i + 3 < drawn.rgba.size(); i += 4 )
			{
				// A pixel neither draw covered keeps the clear color: a quad
				// edge on a pixel center is the rasterizer's choice in GL
				// (llvmpipe leaves the top-left corner pixel uncovered).
				const auto isClear = [&]( const Drawn &frame )
				{
					for ( int c = 0; c < 3; ++c )
					{
						if ( frame.rgba[i + c] != draw.clear[c] )
							return false;
					}
					return true;
				};
				if ( isClear( drawn ) && isClear( withFog ) )
					continue;
				for ( int c = 0; c < 3; ++c )
				{
					const float plain = toLinear( drawn.rgba[i + c] / 255.0f );
					const float expected =
					    toSrgb( plain + ( fogColor[c] - plain ) * 0.25f ) * 255.0f;
					worst = std::max(
					    worst, int( std::lround( std::fabs( expected - withFog.rgba[i + c] ) ) ) );
				}
			}
			That( checks, worst <= 2, "fog.range-fog-mixes-a-quarter-toward-its-color",
			    "worst " + std::to_string( worst ) + " levels" );
		}
		checks.That( drawnCases == int( set->cases.size() ), "cases.every-case-drew" );
		checks.That( fogChecked, "fog.an-opaque-case-was-fogged" );
		checks.That( areaChecked, "area.an-opaque-world-case-was-lit" );
	}
	return devices.Finish( checks );
}
