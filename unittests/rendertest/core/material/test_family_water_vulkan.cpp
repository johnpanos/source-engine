//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.family.water (RFC 0016, the `water` family: Portal 2's
//			water_ps2x as the surface program's water point) on
//			render.device.vulkan.
//
//			The claim: Portal 2's goo VMTs (reflective, env map and plain
//			reflective water) are claimed with their constants packed as the
//			CS:GO water.cpp packs them; refraction, water seen from below,
//			the cheap path, bumped-lightmap water and flow debug views are
//			refused by name.
//
//			The pixels: each case draws a full-target quad of the water point
//			on the world vertex into a 256x256 sRGB target, with 1x1 inputs
//			(so every texture read is exact) and a 1x2 reflection target
//			(top and bottom texels differ, so the reflected view's vertical
//			flip and its viewport mapping are judged). Each sampled pixel must
//			be within kTolerance levels of an independent C++ transcription of
//			water_ps2x's arithmetic for that pixel (its world position, eye
//			direction and fresnel term). Cases: the sludge layer over lit fog
//			with the view's reflection (Portal 2's a2 goo), reflective water
//			without sludge, the forced env map (a3 goo), a forced fresnel, and
//			$waterblendfactor blending over the target.
//
//			Seeded defect (sensitivity row): RENDER_MATERIAL_WATER_SEEDED_
//			GAMMA_FOG_COLOR (water_family.cpp) packs $fogcolor undecoded.
//
//=============================================================================//

#include "family_devices.h"
#include "family_pixel_cases.h"
#include "render/material/draw_program.h"
#include "render/material/surface_program.h"
#include "render/material/vmt_import.h"
#include "render/material/water_family.h"
#include "testing/checks.h"

#include <algorithm>
#include <array>
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

// Portal 2's goo (materials/nature/toxicslime_a2_laser_over_goo.vmt, its
// GPU>=1 block taken) and old Aperture goo (toxicslime_a3_jump_intro.vmt with
// the cube map vbsp patches in), and reflective water without sludge
// (water_a1_intro3.vmt without $pseudotranslucent).
const char *const kGooA2 = R"(Water
{
$forceexpensive 1
$normalmap "liquids/water_river_normal_sharp"
$flowmap "liquids/sp_a2_laser_over_goo_hflowmap"
$flow_normaluvscale 160
$flow_worlduvscale 0.5
$flow_timeintervalinseconds 0.6
$flow_uvscrolldistance 0.1
$flow_bumpstrength 1.0
$flow_noise_texture "liquids/water_noise"
$flow_noise_scale 0.001
$flow_debug 0
$basetexture "liquids/toxicslime_color"
$color_flow_uvscale 400
$color_flow_timeintervalinseconds 6
$color_flow_uvscrolldistance 0.105
$color_flow_lerpexp 1.2
$color_flow_displacebynormalstrength 0.009
"GPU>=1"
{
$reflecttexture _rt_WaterReflection
$reflectamount "0.1"
$reflecttint "{ 190 205 205 }"
$reflectskyboxonly 0
$reflectonlymarkedentities 1
}
$abovewater 1
$bottommaterial "nature/toxicslime002a_beneath"
$fogenable 1
$fogstart 0
$fogend 100
$lightmapwaterfog 1
$fogcolor "{ 58 35 20 }"
$flashlighttint 1
"%compilewater" 1
$surfaceprop slime
})";

const char *const kGooA3 = R"(Water
{
$forceexpensive 1
$normalmap "liquids/water_river_normal_sharp"
$flowmap "liquids/toxic_rep_flowmapr"
$flow_normaluvscale 160.0
$flow_worlduvscale 0.6
$flow_timeintervalinseconds 0.45
$flow_uvscrolldistance 0.1
$flow_bumpstrength 1.0
$flow_noise_texture "liquids/water_noise"
$flow_noise_scale 0.001
$basetexture "liquids/toxicslime_color"
$color_flow_uvscale 200
$color_flow_timeintervalinseconds 4
$color_flow_uvscrolldistance 0.16
$color_flow_lerpexp 0.98
$color_flow_displacebynormalstrength 0.02
$forceenvmap 1
$envmap "maps/sp_a3_jump_intro/c-256_1728_128"
$reflecttint "{ 190 205 205 }"
$abovewater 1
$fogenable 1
$fogstart 0
$fogend 100
$lightmapwaterfog 1
$fogcolor "{ 58 35 20 }"
$surfaceprop slime
})";

const char *const kWaterA1 = R"(Water
{
$forceexpensive 1
$normalmap "liquids/water_river_normal_sharp"
$flowmap "liquids/c3m2_hflowmap_a"
$flow_normaluvscale 155.0
$flow_worlduvscale 0.5
$flow_timeintervalinseconds 0.2
$flow_uvscrolldistance 0.075
$flow_bumpstrength 0.4
$flow_timescale 0.35
$flow_noise_texture "liquids/water_noise"
$flow_noise_scale 0.0005
$reflecttexture _rt_WaterReflection
$reflectamount "0.4"
$reflecttint "{ 190 190 190 }"
$abovewater 1
$fogenable 1
$fogstart 0
$fogend 60
$lightmapwaterfog 1
$fogcolor "{ 90 70 20 }"
})";

struct Claimed
{
	std::optional<ParameterBlock> block;
	WaterClaim claim;
};

// A VMT imported, applied to a `water` block with a stand-in for each texture
// it names, and claimed.
Claimed ClaimVmt( const FamilySchema &water, const std::string &vmt )
{
	Claimed out;
	VmtImportContext context;
	auto imported = ImportVmt( vmt, context );
	if ( !imported || imported.Value().family != "water" )
	{
		out.claim.reason = "the VMT did not import as water";
		return out;
	}
	out.block.emplace( water );
	if ( !ApplyValues( imported.Value(), *out.block ) )
	{
		out.claim.reason = "its values did not apply";
		return out;
	}
	for ( const MaterialValue &value : imported.Value().values )
	{
		if ( value.kind == ValueKind::kTexture )
			(void)out.block->SetTexture( value.parameter, device::TextureId( 1 ) );
	}
	out.claim = ClaimWater( *out.block );
	return out;
}

// A VMT with one line added before its closing brace.
std::string With( const char *vmt, const std::string &line )
{
	std::string text( vmt );
	text.insert( text.rfind( '}' ), line + "\n" );
	return text;
}

// A VMT with one of its lines replaced (the importer keeps a key's first
// definition, so a set key is changed in place).
std::string Replaced( const char *vmt, const std::string &from, const std::string &to )
{
	std::string text( vmt );
	text.replace( text.find( from ), from.size(), to );
	return text;
}

double SrgbToLinear( double c )
{
	return c <= 0.04045 ? c / 12.92 : std::pow( ( c + 0.055 ) / 1.055, 2.4 );
}

int LinearToSrgbByte( double c )
{
	c = std::clamp( c, 0.0, 1.0 );
	const double s = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow( c, 1.0 / 2.4 ) - 0.055;
	return int( std::lround( s * 255.0 ) );
}

struct Vec3
{
	double x = 0, y = 0, z = 0;
};

Vec3 Mix( const Vec3 &a, const Vec3 &b, double t )
{
	return { a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t, a.z + ( b.z - a.z ) * t };
}

double Saturate( double v )
{
	return std::clamp( v, 0.0, 1.0 );
}

double Fract( double v )
{
	return v - std::floor( v );
}

// The inputs of a case: 1x1 texels (bytes) and the frame's terms.
struct Inputs
{
	std::uint8_t normal[4] = { 170, 100, 230, 200 };      // data
	std::uint8_t flow[4] = { 200, 90, 0, 180 };           // data: rg the flow vector, a the mask
	std::uint8_t noise[4] = { 90, 140, 60, 255 };         // data: g offsets the interval
	std::uint8_t base[4] = { 150, 120, 70, 110 };         // sRGB: the sludge, a its cover
	std::uint8_t reflectTop[4] = { 40, 160, 200, 255 };   // sRGB
	std::uint8_t reflectBottom[4] = { 220, 90, 30, 255 }; // sRGB
	std::uint8_t cube[4] = { 30, 60, 90, 255 };           // data (water reads it unconverted)
	std::uint8_t lightmap[4] = { 90, 110, 130, 255 };     // sRGB
	float time = 12.3f;
	float lightmapScale = 4.5947938f;
	float outputScale = 1.5f;
	float tintScale = 4.0f;
	float envmapScale = 16.0f;
	float eye[3] = { 0.1f, -2.0f, 3.0f };
	float right[2] = { 0.8f, 0.6f };
	// The VMT's $fogcolor and $reflecttint (gamma bytes): the reference
	// decodes them itself (the sRGB curve; Source's pow 2.2 table).
	int fogColor[3] = { 58, 35, 20 };
	int reflectTint[3] = { 190, 205, 205 };
};

// water_ps2x (Portal 2's, the CS:GO source) for the pixel (px, py) of a
// full-target quad whose world positions are its clip coordinates at z 0.5,
// in linear light, before the target's encoding.
Vec3 ReferencePixel( const Inputs &in, const SurfaceConstants &c, int px, int py )
{
	const Vec3 world = { ( px + 0.5 ) / 128.0 - 1.0, 1.0 - ( py + 0.5 ) / 128.0, 0.5 };
	Vec3 toEye = { in.eye[0] - world.x, in.eye[1] - world.y, in.eye[2] - world.z };
	const double eyeLength = std::sqrt( toEye.x * toEye.x + toEye.y * toEye.y + toEye.z * toEye.z );
	toEye = { toEye.x / eyeLength, toEye.y / eyeLength, toEye.z / eyeLength };
	const auto data = []( const std::uint8_t *t, int i )
	{
		return t[i] / 255.0;
	};
	const bool flow = c.waterFlowTime[3] != 0.0f;
	const bool sludge = c.waterMode[1] != 0.0f;
	Vec3 n;
	double na = 1.0;
	double flowRgb[3] = {};
	double flowA = 0.0;
	if ( flow )
	{
		const double noise = data( in.noise, 1 );
		const double fx = data( in.flow, 0 ) * 2.0 - 1.0, fy = data( in.flow, 1 ) * 2.0 - 1.0;
		// Both layers read the same texel: their lerp is that texel.
		double x = data( in.normal, 0 ) * 2.0 - 1.0, y = data( in.normal, 1 ) * 2.0 - 1.0;
		const double strength = ( fx * fx + fy * fy + 0.1 ) * c.waterFlow[2];
		x *= strength;
		y *= strength;
		n = { x, y, std::sqrt( Saturate( 1.0 - x * x - y * y ) ) };
		if ( sludge )
		{
			const double t = in.time / ( c.waterColorFlow[1] * 2.0 ) + noise;
			const double w1 =
			    std::pow( std::fabs( 2.0 * Fract( t + 0.5 ) - 1.0 ), c.waterColorFlow[3] );
			const double w2 = std::pow( std::fabs( 2.0 * Fract( t ) - 1.0 ), c.waterColorFlow[3] );
			const double mask = data( in.flow, 3 );
			for ( int k = 0; k < 3; ++k )
				flowRgb[k] = SrgbToLinear( in.base[k] / 255.0 ) * ( w1 + w2 ) * mask;
			flowA = data( in.base, 3 ) * ( w1 + w2 ) * mask;
		}
	}
	else
	{
		n = { data( in.normal, 0 ) * 2.0 - 1.0, data( in.normal, 1 ) * 2.0 - 1.0,
		    data( in.normal, 2 ) * 2.0 - 1.0 };
		na = data( in.normal, 3 );
	}

	Vec3 reflection;
	if ( c.waterMode[0] != 0.0f )
	{
		// The view position (u, down), flipped vertically, offset along the
		// camera's right and forward in the water plane.
		const double u = ( px + 0.5 ) / 256.0;
		const double down = ( py + 0.5 ) / 256.0;
		const double forward[2] = { -in.right[1], in.right[0] };
		const double offsetV = ( forward[0] * n.x + forward[1] * n.y ) * na * c.waterFog[3];
		(void)u;
		const double v = 1.0 - down + offsetV;
		const std::uint8_t *texel = v < 0.5 ? in.reflectTop : in.reflectBottom;
		reflection = { SrgbToLinear( texel[0] / 255.0 ), SrgbToLinear( texel[1] / 255.0 ),
		    SrgbToLinear( texel[2] / 255.0 ) };
	}
	else
	{
		reflection = { in.envmapScale * data( in.cube, 0 ), in.envmapScale * data( in.cube, 1 ),
		    in.envmapScale * data( in.cube, 2 ) };
	}
	const auto tint = [&]( int k )
	{
		return std::pow( in.reflectTint[k] / 255.0, 2.2 );
	};
	reflection = { reflection.x * tint( 0 ) * in.tintScale, reflection.y * tint( 1 ) * in.tintScale,
	    reflection.z * tint( 2 ) * in.tintScale };

	const double nDotV = Saturate( n.x * toEye.x + n.y * toEye.y + n.z * toEye.z );
	const double fresnel =
	    c.waterMode[3] != -1.0f ? c.waterMode[3] : 0.2 + 0.8 * std::pow( 1.0 - nDotV, 5.0 );
	Vec3 light = { 1.0, 1.0, 1.0 };
	Vec3 fog = { SrgbToLinear( in.fogColor[0] / 255.0 ), SrgbToLinear( in.fogColor[1] / 255.0 ),
	    SrgbToLinear( in.fogColor[2] / 255.0 ) };
	if ( c.waterMode[2] != 0.0f )
	{
		const double scale = double( in.lightmapScale ) * in.outputScale;
		light = { SrgbToLinear( in.lightmap[0] / 255.0 ) * scale,
		    SrgbToLinear( in.lightmap[1] / 255.0 ) * scale,
		    SrgbToLinear( in.lightmap[2] / 255.0 ) * scale };
		fog = { fog.x * light.x, fog.y * light.y, fog.z * light.z };
	}
	if ( sludge )
	{
		const Vec3 lit = { flowRgb[0] * light.x, flowRgb[1] * light.y, flowRgb[2] * light.z };
		const Vec3 underWater = Mix( fog, lit, Saturate( flowA * 2.0 ) );
		const double edge = Saturate( ( flowA - 0.5 ) / 0.2 );
		const double aboveWater = edge * edge * ( 3.0 - 2.0 * edge );
		return Mix( underWater, reflection, Saturate( fresnel * ( 1.0 - aboveWater ) ) );
	}
	return Mix( fog, reflection, fresnel );
}

CaseTexture Texel( const std::uint8_t ( &bytes )[4], device::Format format, bool cube = false )
{
	CaseTexture texture;
	texture.width = texture.height = 1;
	texture.cube = cube;
	texture.format = format;
	for ( int face = 0; face < ( cube ? 6 : 1 ); ++face )
		texture.texels.insert( texture.texels.end(), bytes, bytes + 4 );
	return texture;
}

} // namespace

int main()
{
	testing::Checks checks;
	const FamilyRegistry registry = BuiltinFamilies();
	const FamilySchema *water = registry.Find( "water" );
	if ( !checks.That( water != nullptr, "setup.water-family-registers" ) )
		return checks.Report();

	// The claim on Portal 2's VMTs, and its constants as water.cpp packs them.
	const Claimed a2 = ClaimVmt( *water, kGooA2 );
	That( checks, a2.claim.claimed, "claim.portal2-a2-goo", a2.claim.reason );
	checks.That( a2.claim.reflectTarget && a2.claim.sludge && a2.claim.flow,
	    "claim.a2-goo-reflects-the-view-with-sludge" );
	checks.That( a2.claim.blend == device::BlendMode::kOpaque, "claim.a2-goo-is-opaque" );
	checks.That( std::fabs( a2.claim.constants.waterFlow[0] - 2.0f ) < 1e-6f &&
	                 std::fabs( a2.claim.constants.waterFlow[1] - 1.0f / 160.0f ) < 1e-9f &&
	                 std::fabs( a2.claim.constants.waterColorFlow[0] - 1.0f / 400.0f ) < 1e-9f &&
	                 std::fabs( a2.claim.constants.waterFlow[3] - 0.009f ) < 1e-9f,
	    "claim.flow-scales-are-reciprocals" );
	checks.That(
	    std::fabs( a2.claim.constants.waterFog[0] - float( SrgbToLinear( 58.0 / 255.0 ) ) ) < 1e-5f,
	    "claim.fog-color-takes-the-srgb-curve" );
	checks.That( std::fabs( a2.claim.constants.waterFog[3] - 0.1f ) < 1e-6f &&
	                 a2.claim.constants.waterMode[3] == -1.0f,
	    "claim.reflect-amount-and-no-forced-fresnel" );
	const Claimed a3 = ClaimVmt( *water, kGooA3 );
	That( checks, a3.claim.claimed && !a3.claim.reflectTarget,
	    "claim.portal2-a3-goo-uses-its-env-map", a3.claim.reason );
	const Claimed a1 = ClaimVmt( *water, kWaterA1 );
	That( checks, a1.claim.claimed && !a1.claim.sludge, "claim.reflective-water-without-sludge",
	    a1.claim.reason );
	auto refused = [&]( const std::string &vmt, const char *named, const char *check )
	{
		const Claimed c = ClaimVmt( *water, vmt );
		That( checks, !c.claim.claimed && c.claim.reason.find( named ) != std::string::npos, check,
		    c.claim.claimed ? std::string( "claimed" ) : c.claim.reason );
	};
	{
		// water_ps2x's REFRACT: the refraction target is claimed with its
		// tint (Source's GammaToLinear) and amount; below water too.
		const Claimed r = ClaimVmt( *water,
		    With( kGooA2, "$refracttexture _rt_WaterRefraction\n$refractamount 0.5\n"
		                  "$refracttint \"{ 151 135 34 }\"" ) );
		That( checks,
		    r.claim.claimed && r.claim.refractTarget &&
		        r.claim.constants.waterRefractMode[0] == 1.0f &&
		        r.claim.constants.waterRefractMode[1] == 1.0f &&
		        std::fabs( r.claim.constants.waterRefract[3] - 0.5f ) < 1e-6f &&
		        r.claim.constants.waterRefract[0] > r.claim.constants.waterRefract[2],
		    "claim.refraction-above-water", r.claim.claimed ? std::string() : r.claim.reason );
		const std::string refracting = With( kGooA2, "$refracttexture _rt_WaterRefraction" );
		const Claimed below = ClaimVmt(
		    *water, Replaced( refracting.c_str(), "$abovewater 1", "$abovewater 0" ) );
		That( checks,
		    below.claim.claimed && below.claim.constants.waterRefractMode[0] == 1.0f &&
		        below.claim.constants.waterRefractMode[1] == 0.0f,
		    "claim.refraction-below-water",
		    below.claim.claimed ? std::string() : below.claim.reason );
	}
	refused( Replaced( kGooA2, "$abovewater 1", "$abovewater 0" ), "below",
	    "claim.refuses-water-seen-from-below-without-refraction" );
	refused( With( kGooA2, "$forcecheap 1" ), "forcecheap", "claim.refuses-forced-cheap-water" );
	refused( Replaced( kGooA2, "$flow_debug 0", "$flow_debug 1" ), "flow_debug",
	    "claim.refuses-flow-debug-views" );
	refused( With( kGooA2, "$bumptransform \"center .5 .5 scale 2 2 rotate 0 translate 0 0\"" ),
	    "bumptransform", "claim.refuses-a-bump-transform-by-name" );
	{
		std::string noReflect = kGooA3;
		noReflect.replace( noReflect.find( "$forceenvmap 1" ), 14, "$forceenvmap 0" );
		refused( noReflect, "cheap", "claim.refuses-the-cheap-path" );
	}
	{
		std::string noFlow = kGooA2;
		const std::size_t line = noFlow.find( "$flowmap" );
		noFlow.erase( line, noFlow.find( '\n', line ) - line );
		refused( noFlow, "no $flowmap", "claim.refuses-bumped-lightmap-water" );
	}

	CaseDevices devices( checks );
	for ( const CaseDevices::Entry &entry : devices.entries )
	{
		std::printf( "INFO device %s\n", entry.name.c_str() );
		device::IRenderDevice2 *const device = entry.device.get();
		int drawnCases = 0;
		auto program =
		    SurfaceProgram::Create( *device, device::Format::kRGBA8Srgb, device::Format::kUnknown );
		if ( !checks.That( program.HasValue(), "program.creates" ) )
			continue;

		struct Case
		{
			const char *name;
			WaterClaim claim;
			Inputs inputs;
		};
		std::vector<Case> cases;
		cases.push_back( { "a2-goo", a2.claim, {} } );
		{
			Case reflective = { "reflective-water", a1.claim, {} };
			const int fog[3] = { 90, 70, 20 };
			std::copy( fog, fog + 3, reflective.inputs.fogColor );
			std::fill( reflective.inputs.reflectTint, reflective.inputs.reflectTint + 3, 190 );
			cases.push_back( reflective );
		}
		cases.push_back( { "a3-goo-env-map", a3.claim, {} } );
		{
			Case forced = { "forced-fresnel", a2.claim, {} };
			forced.claim.constants.waterMode[3] = 0.5f;
			cases.push_back( forced );
		}
		{
			Case bright = { "a2-goo-covering-sludge", a2.claim, {} };
			bright.inputs.base[3] = 250; // the sludge floats above the water
			bright.inputs.flow[3] = 255;
			cases.push_back( bright );
		}
		{
			const Claimed blended = ClaimVmt( *water, With( kWaterA1, "$waterblendfactor 0.75" ) );
			That( checks, blended.claim.claimed && blended.claim.blend == device::BlendMode::kAlpha,
			    "claim.water-blend-factor-blends", blended.claim.reason );
			Case blend = { "blend-factor", blended.claim, {} };
			const int fog[3] = { 90, 70, 20 };
			std::copy( fog, fog + 3, blend.inputs.fogColor );
			std::fill( blend.inputs.reflectTint, blend.inputs.reflectTint + 3, 190 );
			cases.push_back( blend );
		}

		for ( const Case &testCase : cases )
		{
			const std::string name = testCase.name;
			const Inputs &in = testCase.inputs;
			auto pipeline = program.Value()->Pipeline( testCase.claim.Variant() );
			if ( !checks.That( pipeline.HasValue(), "pipeline." + name ) )
				continue;
			const CaseTexture base = Texel( in.base, device::Format::kRGBA8Srgb );
			const CaseTexture cube = Texel( in.cube, device::Format::kRGBA8Unorm, true );
			const CaseTexture flowmap = Texel( in.flow, device::Format::kRGBA8Unorm );
			const CaseTexture normal = Texel( in.normal, device::Format::kRGBA8Unorm );
			const CaseTexture noise = Texel( in.noise, device::Format::kRGBA8Unorm );
			const CaseTexture white = NeutralCaseTexture( device::Format::kRGBA8Unorm );
			const CaseTexture whiteSrgb = NeutralCaseTexture( device::Format::kRGBA8Srgb );
			const CaseTexture page = Texel( in.lightmap, device::Format::kRGBA8Srgb );
			CaseTexture reflection;
			reflection.width = 1;
			reflection.height = 2;
			reflection.point = reflection.clamp = true;
			reflection.format = device::Format::kRGBA8Srgb;
			reflection.texels.assign( in.reflectTop, in.reflectTop + 4 );
			reflection.texels.insert(
			    reflection.texels.end(), in.reflectBottom, in.reflectBottom + 4 );

			SurfaceConstants constants = testCase.claim.constants;
			constants.state[0] = testCase.claim.blend == device::BlendMode::kOpaque ? 1.0f : 0.0f;
			SurfaceFrame frame;
			frame.light[0] = in.lightmapScale;
			frame.light[1] = in.outputScale;
			frame.eye[0] = in.eye[0];
			frame.eye[1] = in.eye[1];
			frame.eye[2] = in.eye[2];
			frame.eye[3] = in.envmapScale;
			frame.water[0] = in.time;
			frame.water[1] = in.tintScale;
			frame.water[2] = in.right[0];
			frame.water[3] = in.right[1];
			frame.viewport[2] = frame.viewport[3] = 1.0f / float( kSize );
			const ModelLighting lighting;

			// A full-target quad at z 0.5 (clip space, D3D9 conventions).
			std::vector<SurfaceWorldVertex> quad;
			const float corners[6][2] = {
			    { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, -1 }, { 1, 1 }, { -1, 1 } };
			for ( const auto &corner : corners )
			{
				SurfaceWorldVertex vertex;
				vertex.position[0] = corner[0];
				vertex.position[1] = corner[1];
				vertex.position[2] = 0.5f;
				vertex.uv[0] = corner[0];
				vertex.uv[1] = corner[1];
				vertex.lightmapUv[0] = vertex.lightmapUv[1] = 0.5f;
				quad.push_back( vertex );
			}
			CaseGroup view = NeutralViewGroup( program.Value()->ViewLayout() );
			view.textures[3] = &reflection; // the planar reflection (binding 12)
			CaseDraw draw;
			draw.pipeline = pipeline.Value();
			draw.groups.push_back( { device::BindGroupRole::kMaterial,
			    program.Value()->MaterialLayout(), std::as_bytes( std::span( &constants, 1 ) ),
			    { &base, &cube, &flowmap, &normal, &white, &noise, &whiteSrgb } } );
			draw.groups.push_back( { device::BindGroupRole::kDraw, program.Value()->DrawLayout(),
			    std::as_bytes( std::span( &lighting, 1 ) ), { &page, &white, &white, &white }, 1 } );
			draw.groups.push_back( view );
			draw.groups.push_back( SurfaceFrameGroup( program.Value()->FrameLayout(),
			    std::as_bytes( std::span( &frame, 1 ) ), &white, &white, &white, &white, &white ) );
			draw.vertices = std::as_bytes( std::span( quad ) );
			draw.vertexCount = std::uint32_t( quad.size() );
			// The water point draws world vertices, so its push block is the whole
			// { toClip, world }; DrawCase's flat fallback alone is refused as an
			// incomplete block. The quad is already in clip space and nothing sways.
			FamilyDrawConstants drawConstants;
			const std::array<float, 16> toClip = CaseToClip();
			std::copy( toClip.begin(), toClip.end(), drawConstants.toClip );
			for ( int i = 0; i < 4; ++i )
				drawConstants.world[i * 4 + i] = 1.0f;
			draw.drawConstants = std::as_bytes( std::span( &drawConstants, 1 ) );
			const Drawn drawn = DrawCase( *device, draw );
			if ( !checks.That( drawn.ok, "draw." + name ) )
				continue;
			++drawnCases;
			devices.Compare( checks, entry, name, drawn );
			// Pixels in the top and bottom halves (the reflection's two
			// texels) at several columns (the fresnel term varies).
			const int samples[][2] = {
			    { 32, 40 }, { 128, 60 }, { 220, 90 }, { 40, 170 }, { 128, 200 }, { 230, 240 } };
			int worst = 0;
			for ( const auto &sample : samples )
			{
				const int px = sample[0], py = sample[1];
				Vec3 expected = ReferencePixel( in, constants, px, py );
				if ( testCase.claim.blend == device::BlendMode::kAlpha )
				{
					// Blended over the clear color (black) in linear light.
					const double alpha = constants.waterReflect[3];
					expected = { expected.x * alpha, expected.y * alpha, expected.z * alpha };
				}
				const double channels[3] = { expected.x, expected.y, expected.z };
				for ( int k = 0; k < 3; ++k )
				{
					const int got = drawn.rgba[( std::size_t( py ) * kSize + px ) * 4 + k];
					worst = std::max( worst, std::abs( got - LinearToSrgbByte( channels[k] ) ) );
				}
			}
			if ( !checks.That( worst <= kTolerance, "pixels." + name + ".match-water_ps2x" ) )
				std::printf( "  %s: worst channel off by %d levels (tolerance %d)\n", name.c_str(),
				    worst, kTolerance );
			// The two halves read different reflection texels, so they differ
			// (the flip is judged above; this guards a constant image).
			if ( name == "a2-goo" || name == "reflective-water" )
			{
				const std::size_t top = ( std::size_t( 40 ) * kSize + 32 ) * 4;
				const std::size_t bottom = ( std::size_t( 170 ) * kSize + 40 ) * 4;
				checks.That( drawn.rgba[top] != drawn.rgba[bottom],
				    "pixels." + name + ".reflection-follows-the-view" );
			}
		}
		checks.That( drawnCases == int( cases.size() ), "cases.every-case-drew" );
	}
	return devices.Finish( checks );
}
