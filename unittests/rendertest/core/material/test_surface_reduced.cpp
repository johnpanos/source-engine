//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.surface-reduced (RFC 0026 decision 8, P3): the
//			reduced 3DS material model's rules, judged on the host.
//
//			R1 each refused term and point is refused, by its own reason;
//			R2 each dropped term is named, and fog always is;
//			R3 the vertex program follows the layout and static light;
//			R4 the fragment programs, evaluated by this file's own model of
//			   the PICA200 combiners (sources, operands, combine functions,
//			   scale, the combiner buffer), give the model's colour:
//			   lightmapped, unlit, vertex-lit model, static light, self-
//			   illumination and the modulating decal, over sample texels;
//			R5 the alpha test reference from $alphatest;
//			R6 every reduced program writes, reads back and validates as a
//			   PFP1 artifact against the reduced layouts' bindings;
//			R7 seeded defects in the programs are caught by R4's oracle.
//
//=============================================================================//

#include "render/device/pica_codes.h"
#include "render/device/pica_format.h"
#include "render/material/surface_program.h"
#include "render/material/surface_reduced.h"
#include "testing/checks.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>

namespace
{

using namespace render::material;
namespace pf = render::device::pica_format;

using Rgba = std::array<float, 4>;

static_assert( offsetof( SurfaceConstants, tint ) == kReducedTintOffset );
static_assert( offsetof( SurfaceConstants, selfIllumTint ) == kReducedSelfIllumTintOffset );

// The inputs one fragment sees.
struct Fragment
{
	Rgba tex0{ 1, 1, 1, 1 };
	Rgba tex1{ 1, 1, 1, 1 };
	Rgba primary{ 1, 1, 1, 1 };
	Rgba tint{ 1, 1, 1, 1 };
	Rgba selfIllumTint{ 1, 1, 1, 1 };
};

float Clamp01( float x )
{
	return std::fmin( std::fmax( x, 0.0f ), 1.0f );
}

// The PICA200 combiners as GPUREG_TEXENVn describes them (3dbrew), in
// floats: each stage's three sources through their operands, combined,
// scaled and clamped; the buffer a stage's PreviousBuffer reads is the
// output of the last earlier stage (of the first four) whose update bit is
// set, else the buffer colour.
Rgba Evaluate( const pf::FragmentProgram &program, const Fragment &in )
{
	Rgba previous{ 0, 0, 0, 0 };
	Rgba buffer{ 0, 0, 0, 0 };
	for ( std::size_t i = 0; i < program.stages.size(); ++i )
	{
		const pf::CombinerStage &stage = program.stages[i];
		Rgba constant{ 0, 0, 0, 0 };
		if ( stage.constantKind == pf::ConstantKind::kUniformBuffer )
		{
			if ( stage.constantOffset == kReducedTintOffset )
				constant = in.tint;
			else if ( stage.constantOffset == kReducedSelfIllumTintOffset )
				constant = in.selfIllumTint;
		}
		auto source = [&]( std::uint8_t code ) -> Rgba
		{
			switch ( code )
			{
			case pf::source::kPrimaryColor:
				return in.primary;
			case pf::source::kTexture0:
				return in.tex0;
			case pf::source::kTexture1:
				return in.tex1;
			case pf::source::kPreviousBuffer:
				return buffer;
			case pf::source::kConstant:
				return constant;
			case pf::source::kPrevious:
				return previous;
			}
			return { 0, 0, 0, 0 };
		};
		auto combine = [&]( std::uint8_t function, const std::array<float, 3> &a ) -> float
		{
			switch ( function )
			{
			case pf::combine::kReplace:
				return a[0];
			case pf::combine::kModulate:
				return a[0] * a[1];
			case pf::combine::kAdd:
				return a[0] + a[1];
			case pf::combine::kInterpolate:
				return a[0] * a[2] + a[1] * ( 1.0f - a[2] );
			}
			return 0.0f;
		};
		auto scale = []( std::uint8_t code )
		{
			return code == pf::scale::k4 ? 4.0f : code == pf::scale::k2 ? 2.0f : 1.0f;
		};
		Rgba out{};
		for ( int c = 0; c < 3; ++c )
		{
			std::array<float, 3> a{};
			for ( int s = 0; s < 3; ++s )
			{
				const Rgba v = source( stage.rgbSources[s] );
				// Only the operands the reduced model uses: colour or alpha.
				a[s] = stage.rgbOperands[s] == pf::operand::kRgbAlpha ? v[3] : v[c];
			}
			out[c] = Clamp01( combine( stage.rgbCombine, a ) * scale( stage.rgbScale ) );
		}
		{
			std::array<float, 3> a{};
			for ( int s = 0; s < 3; ++s )
				a[s] = source( stage.alphaSources[s] )[3];
			out[3] = Clamp01( combine( stage.alphaCombine, a ) * scale( stage.alphaScale ) );
		}
		// The buffer the next stage reads.
		if ( i < 4 )
		{
			const Rgba kept = buffer;
			buffer = kept;
			if ( program.bufferWrite & ( 1u << i ) )
				for ( int c = 0; c < 3; ++c )
					buffer[c] = out[c];
			if ( program.bufferWrite & ( 1u << ( i + 4 ) ) )
				buffer[3] = out[3];
		}
		previous = out;
	}
	return previous;
}

Rgba Mul( Rgba a, const Rgba &b, float rgbScale = 1.0f )
{
	for ( int c = 0; c < 4; ++c )
		a[c] = Clamp01( a[c] * b[c] * ( c < 3 ? rgbScale : 1.0f ) );
	return a;
}

bool Near( const Rgba &a, const Rgba &b )
{
	for ( int c = 0; c < 4; ++c )
		if ( std::fabs( a[c] - b[c] ) > 1e-4f )
			return false;
	return true;
}

std::string Show( const Rgba &v )
{
	return std::to_string( v[0] ) + " " + std::to_string( v[1] ) + " " + std::to_string( v[2] ) +
	       " " + std::to_string( v[3] );
}

// Sample texels, chosen so every factor differs and nothing saturates
// except where the 2x light is meant to.
const Fragment kSamples[] = {
    { { 0.8f, 0.5f, 0.25f, 0.6f }, { 0.3f, 0.4f, 0.2f, 1.0f }, { 0.45f, 0.25f, 0.4f, 0.7f },
        { 0.9f, 0.7f, 0.5f, 0.8f }, { 0.5f, 1.0f, 0.75f, 1.0f } },
    { { 0.2f, 0.9f, 0.6f, 0.1f }, { 0.7f, 0.1f, 0.45f, 1.0f }, { 0.2f, 0.35f, 0.15f, 0.5f },
        { 1.0f, 0.5f, 0.8f, 1.0f }, { 1.0f, 0.25f, 0.5f, 1.0f } },
    { { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.9f, 0.9f, 0.9f, 1.0f }, { 0.6f, 0.6f, 0.6f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } } };

enum class Light
{
	kLightmap,
	kVertexDoubled,
	kVertexUnlit
};

Rgba Expected( const Fragment &in, Light light, bool selfIllum )
{
	const Rgba surface = Mul( in.tex0, in.tint );
	Rgba lit = surface;
	if ( light == Light::kLightmap )
		lit = Mul( surface, { in.tex1[0], in.tex1[1], in.tex1[2], 1.0f }, 2.0f );
	else
		lit = Mul( surface, { in.primary[0], in.primary[1], in.primary[2], 1.0f },
		    light == Light::kVertexDoubled ? 2.0f : 1.0f );
	if ( selfIllum )
	{
		const Rgba glow = Mul( in.tex0, in.selfIllumTint );
		const float a = in.tex0[3];
		for ( int c = 0; c < 3; ++c )
			lit[c] = glow[c] * a + lit[c] * ( 1.0f - a );
	}
	return lit;
}

// Whether a program gives the model's colour on every sample.
bool Matches( const pf::FragmentProgram &program, Light light, bool selfIllum, std::string *why )
{
	for ( const Fragment &in : kSamples )
	{
		const Rgba got = Evaluate( program, in );
		const Rgba want = Expected( in, light, selfIllum );
		if ( !Near( got, want ) )
		{
			if ( why )
				*why = "got " + Show( got ) + " want " + Show( want );
			return false;
		}
	}
	return true;
}

struct PointCase
{
	std::string_view name;
	SurfaceVariant variant;
	Light light;
	bool selfIllum;
	ReducedVertex vertex;
	std::size_t textures;
};

SurfaceVariant Variant( SurfaceVertexLayout layout, std::uint32_t terms, bool staticLight = false )
{
	SurfaceVariant v;
	v.layout = layout;
	v.terms = terms;
	v.staticVertexLight = staticLight;
	return v;
}

std::vector<PointCase> Points()
{
	return { { "lightmapped world", Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap ),
	             Light::kLightmap, false, ReducedVertex::kFlat, 2 },
	    { "bumped lightmapped world (flat page)",
	        Variant( SurfaceVertexLayout::kWorld,
	            kSurfaceBakedLightmap | kSurfaceBump | kSurfaceDiffuseBump ),
	        Light::kLightmap, false, ReducedVertex::kFlat, 2 },
	    { "unlit flat", Variant( SurfaceVertexLayout::kFlat, kSurfaceUnlit ), Light::kVertexUnlit,
	        false, ReducedVertex::kFlat, 1 },
	    { "vertex-lit model", Variant( SurfaceVertexLayout::kModel, kSurfaceVertexLit ),
	        Light::kVertexDoubled, false, ReducedVertex::kModel, 1 },
	    { "static prop", Variant( SurfaceVertexLayout::kWorld, kSurfaceVertexLit, true ),
	        Light::kVertexDoubled, false, ReducedVertex::kStaticLight, 1 },
	    { "model handed over as world geometry (pbr, no baked lightmap)",
	        Variant( SurfaceVertexLayout::kWorld, kSurfacePbr ), Light::kVertexDoubled, false,
	        ReducedVertex::kWorldLit, 1 },
	    { "pbr world surface with its baked lightmap",
	        Variant( SurfaceVertexLayout::kWorld, kSurfacePbr | kSurfaceBakedLightmap ),
	        Light::kLightmap, false, ReducedVertex::kFlat, 2 },
	    { "self-illuminated model",
	        Variant( SurfaceVertexLayout::kModel, kSurfaceVertexLit | kSurfaceSelfIllum ),
	        Light::kVertexDoubled, true, ReducedVertex::kModel, 1 },
	    { "self-illuminated world",
	        Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap | kSurfaceSelfIllum ),
	        Light::kLightmap, true, ReducedVertex::kFlat, 2 },
	    { "unlit self-illum (unlit wins)",
	        Variant( SurfaceVertexLayout::kFlat, kSurfaceUnlit | kSurfaceSelfIllum ),
	        Light::kVertexUnlit, false, ReducedVertex::kFlat, 1 },
	    { "masked self-illum (dropped)",
	        Variant( SurfaceVertexLayout::kModel,
	            kSurfaceVertexLit | kSurfaceSelfIllum | kSurfaceSelfIllumMask ),
	        Light::kVertexDoubled, false, ReducedVertex::kModel, 1 } };
}

bool Names( const ReducedPoint &point, std::string_view needle )
{
	for ( std::string_view name : point.dropped )
		if ( name.find( needle ) != std::string_view::npos )
			return true;
	return false;
}

void RuleCases( testing::Checks &checks )
{
	// R1: refused terms and points.
	const std::pair<std::uint32_t, std::string_view> refusedTerms[] = { { kSurfaceWater, "water" },
	    { kSurfaceTransmission, "transmission" }, { kSurfaceDepthNormal, "depth-normal" },
	    { kSurfaceRsm, "reflective shadow" }, { kSurfaceSsrTargets, "screen-space" },
	    { kSurfaceDepthOnly, "depth-only" } };
	for ( const auto &[term, reason] : refusedTerms )
	{
		const ReducedPoint p =
		    ReduceSurface( Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap | term ) );
		checks.That( p.refusal.find( reason ) != std::string::npos,
		    "R1 term " + std::string( reason ) + " refused by name (" + p.refusal + ")" );
	}
	struct Flag
	{
		std::string_view reason;
		void ( *set )( SurfaceVariant & );
	};
	const Flag flags[] = { { "shadow depth",
	                           []( SurfaceVariant &v )
	                           {
		                           v.shadowDepth = true;
	                           } },
	    { "temporal",
	        []( SurfaceVariant &v )
	        {
		        v.temporal = true;
	        } },
	    { "energy",
	        []( SurfaceVariant &v )
	        {
		        v.energy = true;
	        } },
	    { "cable",
	        []( SurfaceVariant &v )
	        {
		        v.cable = true;
	        } },
	    { "portal",
	        []( SurfaceVariant &v )
	        {
		        v.portalMask = true;
	        } },
	    { "instanced",
	        []( SurfaceVariant &v )
	        {
		        v.instanced = true;
	        } } };
	for ( const Flag &flag : flags )
	{
		SurfaceVariant v = Variant( SurfaceVertexLayout::kModel, kSurfaceVertexLit );
		flag.set( v );
		const ReducedPoint p = ReduceSurface( v );
		checks.That( p.refusal.find( flag.reason ) != std::string::npos,
		    "R1 point " + std::string( flag.reason ) + " refused by name (" + p.refusal + ")" );
	}

	// R2: dropped terms named, each alone, and the point still drawn.
	const std::pair<std::uint32_t, std::string_view> droppedTerms[] = {
	    { kSurfaceDetail, "detail" }, { kSurfaceBump, "bump" }, { kSurfaceSsbump, "self-shadowed" },
	    { kSurfaceDiffuseBump, "bumped diffuse" }, { kSurfaceHalfLambert, "half-Lambert" },
	    { kSurfaceEnvmap, "environment map" }, { kSurfaceEnvmapMask, "environment map mask" },
	    { kSurfaceBaseAlphaEnvmapMask, "base-alpha" },
	    { kSurfaceNormalMapAlphaEnvmapMask, "normal-alpha" },
	    { kSurfaceEmissionTexture, "emission texture" },
	    { kSurfaceSelfIllumMask, "self-illumination mask" }, { kSurfaceClustered, "clustered" },
	    { kSurfaceRuntimeDirect, "runtime direct" }, { kSurfaceProbeVolume, "probe volume" },
	    { kSurfaceReflectionProbes, "reflection probes" },
	    { kSurfaceAmbientOcclusion, "ambient occlusion" }, { kSurfaceProbeBounce, "probe bounce" },
	    { kSurfaceMeshDirect, "mesh direct" },
	    { kSurfaceDirectionalLightmap, "directional lightmap" },
	    { kSurfaceMraoTexture, "metalness" }, { kSurfacePhongExponentTexture, "Phong exponent" },
	    { kSurfacePbr, "physically based" } };
	for ( const auto &[term, name] : droppedTerms )
	{
		const ReducedPoint p =
		    ReduceSurface( Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap | term ) );
		checks.That( p.refusal.empty() && Names( p, name ),
		    "R2 " + std::string( name ) + " dropped by name, point drawn" );
	}
	{
		const ReducedPoint p =
		    ReduceSurface( Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap ) );
		checks.That( p.dropped.size() == 1 && Names( p, "fog" ),
		    "R2 a plain lightmapped point drops fog alone" );
		SurfaceVariant sway = Variant( SurfaceVertexLayout::kModel, kSurfaceVertexLit );
		sway.treeSwayMode = 1;
		const ReducedPoint swaying = ReduceSurface( sway );
		checks.That( swaying.refusal.empty() && Names( swaying, "tree sway" ),
		    "R2 tree sway is dropped by name, the foliage still drawn" );
	}

	// R5: the alpha test reference.
	SurfaceConstants constants;
	checks.That( ReducedAlphaTest( constants ) == -1, "R5 no $alphatest: no test" );
	constants.flags[1] = 1.0f;
	constants.flags[2] = 0.5f;
	checks.That( ReducedAlphaTest( constants ) == 128, "R5 $alphatestreference 0.5 is 128" );
	constants.flags[2] = 2.0f;
	checks.That( ReducedAlphaTest( constants ) == 255, "R5 references clamp to 255" );
	constants.flags[2] = std::nanf( "" );
	checks.That( ReducedAlphaTest( constants ) == 0, "R5 a NaN reference is 0" );
	SurfaceVariant tested = Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap );
	tested.alphaTestReference = 128;
	const ReducedPoint p = ReduceSurface( tested );
	checks.That(
	    p.fragment.alphaTest == pf::test::kGreaterEqual && p.fragment.alphaReference == 128,
	    "R5 the variant's reference becomes the GPU's greater-or-equal test" );
	checks.That( ReduceSurface( Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap ) )
	                     .fragment.alphaTest == pf::kNone,
	    "R5 without a reference the GPU does not test" );
}

void ProgramCases( testing::Checks &checks )
{
	for ( const PointCase &c : Points() )
	{
		const ReducedPoint p = ReduceSurface( c.variant );
		const std::string name( c.name );
		checks.That( p.refusal.empty(), "R4 " + name + " is drawn" );
		checks.That( p.vertex == c.vertex, "R3 " + name + " vertex program" );
		checks.That( p.fragment.textures.size() == c.textures, "R4 " + name + " texture units" );
		std::string why;
		checks.That(
		    Matches( p.fragment, c.light, c.selfIllum, &why ), "R4 " + name + " colour " + why );
		checks.That( p.fragment.stages.size() <= 6, "R4 " + name + " fits six combiners" );

		// R6: the artifact round trip.
		const std::vector<std::byte> bytes = pf::WriteFragmentProgram( p.fragment );
		pf::FragmentProgram read;
		const pf::ArtifactProblem problem = pf::ReadFragmentProgram( bytes, 0, read );
		checks.That(
		    !problem, "R6 " + name + " PFP1 validates (" + std::string( problem.rule ) + ")" );
		checks.That(
		    pf::WriteFragmentProgram( read ) == bytes, "R6 " + name + " reads back exactly" );
		bool material = false, texture = false;
		for ( const auto &b : pf::BindingsOf( read ) )
		{
			material =
			    material || b.group == std::uint32_t( render::device::BindGroupRole::kMaterial );
			texture = texture || b.kind == render::device::BindingKind::kSampledTexture;
		}
		checks.That(
		    material && texture, "R6 " + name + " reflects the material buffer and a texture" );
	}

	// The modulating decal: the base texel unlit.
	SurfaceVariant decal = Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap );
	decal.decalModulate = true;
	const ReducedPoint d = ReduceSurface( decal );
	bool decalOk = true;
	for ( const Fragment &in : kSamples )
		decalOk = decalOk && Near( Evaluate( d.fragment, in ), in.tex0 );
	checks.That( d.refusal.empty() && decalOk, "R4 modulating decal draws the base texel" );
}

// R7: defects the oracle must catch, each seeded into a copy of a program.
void SensitivityCases( testing::Checks &checks )
{
	const ReducedPoint world = ReduceSurface(
	    Variant( SurfaceVertexLayout::kWorld, kSurfaceBakedLightmap | kSurfaceSelfIllum ) );
	const ReducedPoint model =
	    ReduceSurface( Variant( SurfaceVertexLayout::kModel, kSurfaceVertexLit ) );
	struct Seed
	{
		std::string_view name;
		const ReducedPoint *base;
		Light light;
		bool selfIllum;
		void ( *mutate )( pf::FragmentProgram & );
	};
	const Seed seeds[] = { { "light not doubled", &model, Light::kVertexDoubled, false,
	                           []( pf::FragmentProgram &f )
	                           {
		                           f.stages[1].rgbScale = pf::scale::k1;
	                           } },
	    { "tint not applied", &model, Light::kVertexDoubled, false,
	        []( pf::FragmentProgram &f )
	        {
		        f.stages[0].rgbCombine = pf::combine::kReplace;
	        } },
	    { "light alpha taken", &model, Light::kVertexDoubled, false,
	        []( pf::FragmentProgram &f )
	        {
		        f.stages[1].alphaSources = f.stages[1].rgbSources;
		        f.stages[1].alphaCombine = pf::combine::kModulate;
	        } },
	    { "self-illumination buffer not written", &world, Light::kLightmap, true,
	        []( pf::FragmentProgram &f )
	        {
		        f.bufferWrite = 0;
	        } },
	    { "self-illumination mix inverted", &world, Light::kLightmap, true,
	        []( pf::FragmentProgram &f )
	        {
		        std::swap( f.stages[3].rgbSources[0], f.stages[3].rgbSources[1] );
	        } },
	    { "self-illumination tint offset", &world, Light::kLightmap, true,
	        []( pf::FragmentProgram &f )
	        {
		        f.stages[0].constantOffset = kReducedTintOffset;
	        } },
	    { "lightmap unit swapped for the base", &world, Light::kLightmap, true,
	        []( pf::FragmentProgram &f )
	        {
		        f.stages[2].rgbSources[1] = pf::source::kTexture0;
	        } },
	    { "mix weighted by colour, not alpha", &world, Light::kLightmap, true,
	        []( pf::FragmentProgram &f )
	        {
		        f.stages[3].rgbOperands[2] = pf::operand::kRgbColor;
	        } } };
	for ( const Seed &seed : seeds )
	{
		pf::FragmentProgram program = seed.base->fragment;
		checks.That( Matches( program, seed.light, seed.selfIllum, nullptr ),
		    "R7 " + std::string( seed.name ) + ": the unmutated program passes" );
		seed.mutate( program );
		checks.That( !Matches( program, seed.light, seed.selfIllum, nullptr ),
		    "R7 " + std::string( seed.name ) + " is caught" );
	}
}

} // namespace

int main()
{
	testing::Checks checks;
	RuleCases( checks );
	ProgramCases( checks );
	SensitivityCases( checks );
	return checks.Report();
}
