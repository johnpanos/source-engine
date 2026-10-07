//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica.portable (RFC 0026 P0): the PICA adapter's
//			portable parts, judged without a device.
//
//			P1 a PFP1 combiner program built byte by byte here (not by the
//			   adapter's writer) reads back field for field, and the writer
//			   reproduces those bytes; the same for PVS1 around a stub DVLB;
//			P2 seeded malformed artifacts are each refused, with the rule
//			   that names their defect;
//			P3 reflection: each binding once, of the right kind;
//			P4 blend: for every port BlendMode, the GPU's add equation with
//			   the table's factors, evaluated on sample colours, equals the
//			   mode's formula in port.h's comments;
//			P5 compare and stencil codes: the GPU test function each code
//			   names, evaluated, equals the port operation;
//			P7 specialization replaces a stage constant; vec4 constants pack
//			   clamped and rounded to RGBA8;
//			P8 texel layout: tiles in Morton order (a bijection with known
//			   corners), padded extents and level offsets, sampled levels,
//			   and copies through the stored form for every storable format;
//			P6 cull, formats, facts and creation: no optional capability,
//			   kPica artifacts, refusal of sRGB/BC/float formats, Create
//			   refuses (kUnavailable) until P1 and a required capability is
//			   named.
//
//=============================================================================//

#include "render/device/pica_format.h"
#include "../../../../render/device/pica/device.h"
#include "render/device/pica_codes.h"
#include "../../../../render/device/pica/texel_layout.h"
#include "render/device/pica/provider.h"
#include "testing/checks.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using namespace render::device;
using namespace render::device::pica;
using namespace render::device::pica_format;

// -- P1/P2 ---------------------------------------------------------------

struct Bytes
{
	std::vector<std::byte> v;
	Bytes &u8( std::uint32_t x )
	{
		v.push_back( std::byte( x & 0xFF ) );
		return *this;
	}
	Bytes &u16( std::uint32_t x ) { return u8( x ).u8( x >> 8 ); }
	Bytes &u32( std::uint32_t x ) { return u16( x ).u16( x >> 16 ); }
};

// Texture 0 modulated by the vertex colour, then times a draw-constant tint;
// specialization constant 7 = 1 replaces the tint with a literal.
Bytes HandFragment()
{
	Bytes b;
	b.u32( 0x31504650 ).u16( 1 ).u8( 2 ).u8( 1 ); // PFP1 v1, 2 stages, 1 texture
	b.u8( 6 ).u8( 128 ).u8( 0x01 ).u8( 1 );       // alpha GREATER 128, buffer <- stage 0, 1 spec
	b.u32( 0xFF000000 );                          // buffer colour
	b.u8( 2 ).u8( 0 ).u8( 2 ).u8( 1 );            // texture (2,0), sampler (2,1)
	// stage 0 (byte 20): texture0 * primary, literal constant
	b.u8( 3 ).u8( 0 ).u8( 0 ).u8( 3 ).u8( 0 ).u8( 0 );
	b.u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 );
	b.u8( 1 ).u8( 1 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u32( 0 ).u32( 0 );
	// stage 1 (byte 48): previous * constant (draw constants at 0), 2x colour
	b.u8( 0xF ).u8( 0xE ).u8( 0xE ).u8( 0xF ).u8( 0xE ).u8( 0xE );
	b.u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 );
	b.u8( 1 ).u8( 1 ).u8( 1 ).u8( 0 ).u8( 1 ).u8( 0 ).u8( 0 ).u8( 0 ).u32( 0 ).u32( 0x11223344 );
	// specialization (byte 76): id 7 = 1 -> stage 1 literal 0xFF00FF00
	b.u32( 7 ).u32( 1 ).u8( 1 ).u8( 0 ).u8( 0 ).u8( 0 ).u32( 0xFF00FF00 );
	return b;
}

std::vector<std::byte> StubDvlb()
{
	Bytes b;
	b.u32( 0x424C5644 ).u32( 1 ).u32( 0 ).u32( 0 ); // "DVLB", one DVLE, padding
	return b.v;
}

// Byte 6 draw-constant register, 7 uniform count, 8 vertex index register,
// 16 the uniform table, 24 the DVLB.
Bytes HandVertex()
{
	const auto dvlb = StubDvlb();
	Bytes b;
	b.u32( 0x31535650 ).u16( 1 ).u8( 8 ).u8( 2 ).u8( 0 ).u8( 0 ).u8( 0 ).u8( 0 );
	b.u32( std::uint32_t( dvlb.size() ) );
	b.u8( 0 ).u8( 0 ).u8( 0 ).u8( 4 ); // frame UBO -> c0..c3
	b.u8( 1 ).u8( 0 ).u8( 4 ).u8( 4 ); // view UBO -> c4..c7
	b.v.insert( b.v.end(), dvlb.begin(), dvlb.end() );
	return b;
}

void ArtifactCases( testing::Checks &checks )
{
	const Bytes fragmentBytes = HandFragment();
	FragmentProgram fragment;
	const auto problem = ReadFragmentProgram( fragmentBytes.v, 16, fragment );
	checks.That(
	    !problem, std::string( "P1 hand-built PFP1 reads: " ) + std::string( problem.rule ) );
	checks.Equal( fragment.stages.size(), std::size_t( 2 ), "P1 PFP1 stage count" );
	checks.Equal( fragment.textures.size(), std::size_t( 1 ), "P1 PFP1 texture count" );
	checks.Equal( int( fragment.alphaTest ), 6, "P1 PFP1 alpha test" );
	checks.Equal( int( fragment.alphaReference ), 128, "P1 PFP1 alpha reference" );
	checks.Equal( fragment.bufferColor, 0xFF000000u, "P1 PFP1 buffer colour" );
	if ( fragment.stages.size() == 2 && fragment.textures.size() == 1 )
	{
		checks.Equal( int( fragment.textures[0].samplerBinding ), 1, "P1 PFP1 sampler binding" );
		checks.Equal(
		    int( fragment.stages[0].rgbSources[0] ), 3, "P1 PFP1 stage 0 texture source" );
		checks.That( fragment.stages[1].constantKind == ConstantKind::kDrawConstants,
		    "P1 PFP1 stage 1 constant from the draw constants" );
		checks.Equal( fragment.specializations.size(), std::size_t( 1 ), "P1 PFP1 specialization" );
		checks.Equal( int( fragment.stages[1].rgbScale ), 1, "P1 PFP1 stage 1 scale" );
		checks.Equal( fragment.stages[1].constantColor, 0x11223344u, "P1 PFP1 constant colour" );
	}
	checks.That( WriteFragmentProgram( fragment ) == fragmentBytes.v,
	    "P1 the writer reproduces the hand-built PFP1" );

	const Bytes vertexBytes = HandVertex();
	VertexProgram vertex;
	const auto vproblem = ReadVertexProgram( vertexBytes.v, 32, vertex );
	checks.That(
	    !vproblem, std::string( "P1 hand-built PVS1 reads: " ) + std::string( vproblem.rule ) );
	checks.Equal( vertex.uniforms.size(), std::size_t( 2 ), "P1 PVS1 uniform count" );
	checks.Equal( int( vertex.drawConstantRegister ), 8, "P1 PVS1 draw-constant register" );
	checks.Equal( int( vertex.vertexIndexRegister ), 0, "P1 PVS1 vertex index register" );
	checks.Equal( vertex.dvlb.size(), StubDvlb().size(), "P1 PVS1 DVLB view" );
	checks.That( WriteVertexProgram( vertex, vertex.dvlb ) == vertexBytes.v,
	    "P1 the writer reproduces the hand-built PVS1" );

	// P2: each mutant names its defect.
	struct Mutant
	{
		const char *name;
		std::size_t at;
		std::uint8_t value;
		std::string_view rule;
	};
	const Mutant fragmentMutants[] = {
	    { "magic", 0, 0x51, "PFP1 magic" },
	    { "version", 4, 2, "PFP1 version" },
	    { "zero stages", 6, 0, "PFP1 stage count outside 1 to 6" },
	    { "four textures", 7, 4, "PFP1 more than three texture units" },
	    { "alpha test code", 8, 9, "PFP1 alpha test function" },
	    { "texture group 4", 16, 4, "PFP1 texture group out of range" },
	    { "absent texture unit", 20, 4, "PFP1 source unknown or naming an absent texture unit" },
	    { "fragment lighting source", 20, 1,
	        "PFP1 source unknown or naming an absent texture unit" },
	    { "first stage reads previous", 20, 0xF, "PFP1 first stage reads the previous stage" },
	    { "alpha operand", 29, 8, "PFP1 operand out of range" },
	    { "combine function", 32, 10, "PFP1 combine function out of range" },
	    { "alpha dot3", 33, 7, "PFP1 dot3 in the alpha combiner" },
	    { "scale 8x", 34, 3, "PFP1 scale out of range" },
	    { "constant kind", 36, 3, "PFP1 constant kind" },
	    { "reserved stage byte", 39, 1, "PFP1 reserved byte set" },
	    { "constant past the block", 68, 4, "PFP1 constant outside the draw-constant block" },
	    { "unaligned constant", 68, 2, "PFP1 constant outside the draw-constant block" },
	    { "buffer write beyond stages", 10, 0x04, "PFP1 buffer write names an absent stage" },
	    { "specialization stage", 84, 2, "PFP1 specialization names an absent stage" },
	    { "specialization reserved", 85, 1, "PFP1 reserved byte set" },
	    { "specialization count", 11, 2, "PFP1 size does not match its counts" },
	};
	for ( const Mutant &mutant : fragmentMutants )
	{
		Bytes bytes = fragmentBytes;
		bytes.v[mutant.at] = std::byte( mutant.value );
		FragmentProgram out;
		const auto rule = ReadFragmentProgram( bytes.v, 16, out ).rule;
		checks.That( rule == mutant.rule, std::string( "P2 PFP1 mutant refused by its rule: " ) +
		                                      mutant.name + " -> " + std::string( rule ) );
	}
	{
		Bytes truncated = fragmentBytes;
		truncated.v.pop_back();
		FragmentProgram out;
		checks.That( ReadFragmentProgram( truncated.v, 16, out ).rule ==
		                 "PFP1 size does not match its counts",
		    "P2 PFP1 truncated" );
		checks.That( ReadFragmentProgram( fragmentBytes.v, 12, out ).rule ==
		                 "PFP1 constant outside the draw-constant block",
		    "P2 PFP1 constant against a smaller block" );
		Bytes uniformGroup = fragmentBytes;
		uniformGroup.v[36] = std::byte( 2 ); // stage 0 constant from a uniform buffer
		uniformGroup.v[37] = std::byte( 4 ); // in group 4
		checks.That( ReadFragmentProgram( uniformGroup.v, 16, out ).rule ==
		                 "PFP1 uniform-buffer constant group or offset",
		    "P2 PFP1 uniform-buffer constant in group 4" );
	}

	const Mutant vertexMutants[] = {
	    { "magic", 0, 0x51, "PVS1 magic" },
	    { "version", 4, 2, "PVS1 version" },
	    { "vertex index register v16", 8, 16, "PVS1 vertex index register outside v0 to v15" },
	    { "reserved", 10, 1, "PVS1 reserved byte set" },

	    { "group 4", 16, 4, "PVS1 uniform group out of range" },
	    { "overlap", 22, 2, "PVS1 uniform registers out of range or overlapping" },
	    { "past c95", 22, 94, "PVS1 uniform registers out of range or overlapping" },
	    { "binding twice", 20, 0, "PVS1 binding listed twice" },
	    { "draw constants over a uniform", 6, 2,
	        "PVS1 draw-constant registers out of range or overlapping" },
	    { "DVLB magic", 24, 0, "PVS1 DVLB magic" },
	    { "two DVLEs", 28, 2, "PVS1 DVLB must hold exactly one DVLE" },
	    { "DVLB size", 12, 8, "PVS1 DVLB size does not match the artifact" },
	};
	for ( const Mutant &mutant : vertexMutants )
	{
		Bytes bytes = vertexBytes;
		bytes.v[mutant.at] = std::byte( mutant.value );
		VertexProgram out;
		const auto rule = ReadVertexProgram( bytes.v, 32, out ).rule;
		checks.That( rule == mutant.rule, std::string( "P2 PVS1 mutant refused by its rule: " ) +
		                                      mutant.name + " -> " + std::string( rule ) );
	}
	{
		VertexProgram out;
		checks.That( ReadVertexProgram( vertexBytes.v, 0, out ).rule ==
		                 "PVS1 reads draw constants the pipeline does not declare",
		    "P2 PVS1 draw constants without a block" );
	}

	// P3
	const auto vertexBindings = BindingsOf( vertex );
	checks.Equal( vertexBindings.size(), std::size_t( 2 ), "P3 vertex bindings" );
	bool uniforms = true;
	for ( const auto &binding : vertexBindings )
		uniforms = uniforms && binding.kind == BindingKind::kUniformBuffer;
	checks.That( uniforms, "P3 vertex bindings are uniform buffers" );
	FragmentProgram shared = fragment;
	shared.textures.push_back( shared.textures[0] ); // the same texture on two units
	const auto fragmentBindings = BindingsOf( shared );
	checks.Equal( fragmentBindings.size(), std::size_t( 2 ), "P3 fragment bindings once each" );
	if ( fragmentBindings.size() == 2 )
	{
		checks.That( fragmentBindings[0].kind == BindingKind::kSampledTexture &&
		                 fragmentBindings[0].group == 2 && fragmentBindings[0].binding == 0,
		    "P3 texture binding" );
		checks.That(
		    fragmentBindings[1].kind == BindingKind::kSampler && fragmentBindings[1].binding == 1,
		    "P3 sampler binding" );
	}
	FragmentProgram fromUniform = fragment;
	fromUniform.stages[0].constantKind = ConstantKind::kUniformBuffer;
	fromUniform.stages[0].constantGroup = 2;
	fromUniform.stages[0].constantBinding = 3;
	const auto withUniform = BindingsOf( fromUniform );
	checks.That( withUniform.size() == 3 && withUniform[2].kind == BindingKind::kUniformBuffer &&
	                 withUniform[2].group == 2 && withUniform[2].binding == 3,
	    "P3 a uniform-buffer constant is reflected as a uniform buffer" );
	checks.Equal( DrawConstantBytesOf( fragment ), 16u, "P3 the draw-constant bytes read" );

	// P7: specialization and constant packing (D20, D16).
	const SpecializationConstant green[] = { { ShaderStage::kFragment, 7, 1 } };
	const SpecializationConstant other[] = { { ShaderStage::kFragment, 7, 2 },
	    { ShaderStage::kVertex, 7, 1 }, { ShaderStage::kFragment, 99, 1 } };
	const auto specialized = Specialize( fragment, green );
	const auto unspecialized = Specialize( fragment, other );
	checks.That( specialized.size() == 2 && specialized[1].constantKind == ConstantKind::kLiteral &&
	                 specialized[1].constantColor == 0xFF00FF00u,
	    "P7 a matching specialization makes the stage's constant its literal" );
	checks.That(
	    unspecialized.size() == 2 && unspecialized[1].constantKind == ConstantKind::kDrawConstants,
	    "P7 another value, another stage or an unknown id leaves the stage as described" );
	const float red[4] = { 1.0f, 0.2f, 0.0f, 1.0f };
	const float wild[4] = { -1.0f, 2.0f, 0.5f, NAN };
	checks.Equal( PackConstant( red ), 0xFF0033FFu, "P7 1, 0.2, 0, 1 packs to 255, 51, 0, 255" );
	checks.Equal( PackConstant( wild ), 0x0080FF00u, "P7 clamped and NaN as 0" );
}

// -- P4 ------------------------------------------------------------------

using Rgba = std::array<double, 4>;

double FactorValue( std::uint8_t code, const Rgba &src, const Rgba &dst, int channel )
{
	switch ( code )
	{
	case factor::kZero:
		return 0.0;
	case factor::kOne:
		return 1.0;
	case factor::kSrcColor:
		return src[channel];
	case factor::kOneMinusSrcColor:
		return 1.0 - src[channel];
	case factor::kDstColor:
		return dst[channel];
	case factor::kOneMinusDstColor:
		return 1.0 - dst[channel];
	case factor::kSrcAlpha:
		return src[3];
	case factor::kOneMinusSrcAlpha:
		return 1.0 - src[3];
	case factor::kDstAlpha:
		return dst[3];
	case factor::kOneMinusDstAlpha:
		return 1.0 - dst[3];
	}
	return NAN;
}

Rgba Gpu( const BlendFactors &f, const Rgba &src, const Rgba &dst )
{
	Rgba out{};
	for ( int c = 0; c < 4; ++c )
	{
		const bool alpha = c == 3;
		out[c] =
		    src[c] * FactorValue( alpha ? f.alphaSource : f.colorSource, src, dst, c ) +
		    dst[c] * FactorValue( alpha ? f.alphaDestination : f.colorDestination, src, dst, c );
	}
	return out;
}

// The port's formulas (pipeline.h's BlendMode comments). Alpha of modes that
// keep the destination's is dst.a; the rest blend alpha like colour.
Rgba Port( BlendMode mode, const Rgba &s, const Rgba &d )
{
	Rgba o{};
	for ( int c = 0; c < 4; ++c )
	{
		switch ( mode )
		{
		case BlendMode::kOpaque:
			o[c] = s[c];
			break;
		case BlendMode::kAlpha:
			o[c] = s[c] * s[3] + d[c] * ( 1 - s[3] );
			break;
		case BlendMode::kPremultiplied:
			o[c] = s[c] + d[c] * ( 1 - s[3] );
			break;
		case BlendMode::kAdditive:
			o[c] = s[c] + d[c];
			break;
		case BlendMode::kTransmittance:
			o[c] = c == 3 ? d[3] : s[c] + d[c] * s[3];
			break;
		case BlendMode::kModulate2x:
			o[c] = c == 3 ? d[3] : 2 * s[c] * d[c];
			break;
		case BlendMode::kAlphaAdditive:
			o[c] = s[c] * s[3] + d[c];
			break;
		}
	}
	return o;
}

void BlendCases( testing::Checks &checks )
{
	const Rgba samples[] = { { 0.2, 0.5, 0.9, 0.25 }, { 1.0, 0.0, 0.3, 0.75 },
	    { 0.6, 0.4, 0.1, 0.0 }, { 0.05, 0.95, 0.5, 1.0 } };
	const BlendMode modes[] = { BlendMode::kOpaque, BlendMode::kAlpha, BlendMode::kPremultiplied,
	    BlendMode::kAdditive, BlendMode::kTransmittance, BlendMode::kModulate2x,
	    BlendMode::kAlphaAdditive };
	for ( BlendMode mode : modes )
	{
		const auto factors = Blend( mode );
		bool equal = factors.has_value();
		for ( const Rgba &s : samples )
			for ( const Rgba &d : samples )
				if ( equal )
				{
					const Rgba gpu = Gpu( *factors, s, d ), port = Port( mode, s, d );
					for ( int c = 0; c < 4; ++c )
						equal = equal && std::fabs( gpu[c] - port[c] ) < 1e-12;
				}
		checks.That( equal,
		    "P4 blend mode " + std::to_string( int( mode ) ) + " evaluates to the port's formula" );
	}
}

// -- P5 ------------------------------------------------------------------

bool GpuTest( std::uint8_t code, int a, int b )
{
	switch ( code )
	{
	case test::kNever:
		return false;
	case test::kAlways:
		return true;
	case test::kEqual:
		return a == b;
	case test::kNotEqual:
		return a != b;
	case test::kLess:
		return a < b;
	case test::kLessEqual:
		return a <= b;
	case test::kGreater:
		return a > b;
	case test::kGreaterEqual:
		return a >= b;
	}
	return false;
}

bool PortTest( CompareOp op, int a, int b )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return false;
	case CompareOp::kLess:
		return a < b;
	case CompareOp::kLessEqual:
		return a <= b;
	case CompareOp::kEqual:
		return a == b;
	case CompareOp::kGreaterEqual:
		return a >= b;
	case CompareOp::kGreater:
		return a > b;
	case CompareOp::kAlways:
		return true;
	case CompareOp::kNotEqual:
		return a != b;
	}
	return false;
}

int GpuStencil( std::uint8_t code, int old, int ref )
{
	switch ( code )
	{
	case stencil::kKeep:
		return old;
	case stencil::kZero:
		return 0;
	case stencil::kReplace:
		return ref;
	case stencil::kIncrement:
		return old < 255 ? old + 1 : 255;
	case stencil::kDecrement:
		return old > 0 ? old - 1 : 0;
	case stencil::kInvert:
		return ~old & 255;
	case stencil::kIncrementWrap:
		return ( old + 1 ) & 255;
	case stencil::kDecrementWrap:
		return ( old - 1 ) & 255;
	}
	return -1;
}

int PortStencil( StencilOp op, int old, int ref )
{
	switch ( op )
	{
	case StencilOp::kKeep:
		return old;
	case StencilOp::kZero:
		return 0;
	case StencilOp::kReplace:
		return ref;
	case StencilOp::kIncrementClamp:
		return old < 255 ? old + 1 : 255;
	case StencilOp::kDecrementClamp:
		return old > 0 ? old - 1 : 0;
	case StencilOp::kInvert:
		return ~old & 255;
	case StencilOp::kIncrementWrap:
		return ( old + 1 ) & 255;
	case StencilOp::kDecrementWrap:
		return ( old - 1 ) & 255;
	}
	return -1;
}

void CompareCases( testing::Checks &checks )
{
	for ( int op = 0; op <= int( CompareOp::kNotEqual ); ++op )
	{
		bool equal = true;
		for ( int a = 0; a < 3; ++a )
			for ( int b = 0; b < 3; ++b )
				equal = equal && GpuTest( TestFunction( CompareOp( op ) ), a, b ) ==
				                     PortTest( CompareOp( op ), a, b );
		checks.That( equal, "P5 compare op " + std::to_string( op ) + " matches the GPU test" );
	}
	for ( int op = 0; op <= int( StencilOp::kDecrementWrap ); ++op )
	{
		bool equal = true;
		for ( int old : { 0, 1, 128, 255 } )
			equal = equal && GpuStencil( StencilOperation( StencilOp( op ) ), old, 7 ) ==
			                     PortStencil( StencilOp( op ), old, 7 );
		checks.That( equal, "P5 stencil op " + std::to_string( op ) + " matches the GPU's" );
	}
}

// -- P6 ------------------------------------------------------------------

void FactCases( testing::Checks &checks )
{
	RasterState raster;
	raster.cull = render::device::CullMode::kNone;
	checks.Equal( int( render::device::pica_format::CullMode( raster ) ), int( cull::kNone ), "P6 cull none" );
	raster.cull = render::device::CullMode::kBack;
	checks.Equal(
	    int( render::device::pica_format::CullMode( raster ) ), int( cull::kBackCcw ), "P6 cull back, CCW front" );
	raster.frontCounterClockwise = false;
	checks.Equal( int( render::device::pica_format::CullMode( raster ) ), int( cull::kFrontCcw ),
	    "P6 cull back, CW front culls the CCW faces" );
	raster.cull = render::device::CullMode::kFront;
	checks.Equal( int( render::device::pica_format::CullMode( raster ) ), int( cull::kBackCcw ),
	    "P6 cull front, CW front culls the CW faces" );

	checks.That( SampledFormat( Format::kRGBA8Unorm ) == texel::kRGBA8, "P6 RGBA8 sampled" );
	checks.That( SampledFormat( Format::kR8Unorm ) == texel::kRGBA8,
	    "P6 R8 stored and sampled as RGBA8, so it reads (r, 0, 0, 1)" );
	checks.That( SampledFormat( Format::kETC1Rgb ) == texel::kETC1 &&
	                 SampledFormat( Format::kETC1A4 ) == texel::kETC1A4 &&
	                 !ColorBufferFormat( Format::kETC1Rgb ),
	    "P6 ETC1 and ETC1A4 sampled, not rendered" );
	for ( Format refused : { Format::kRGBA8Srgb, Format::kBGRA8Srgb, Format::kBC1Unorm,
	          Format::kRGBA16Float, Format::kR32Float, Format::kRG11B10Float } )
		checks.That( !SampledFormat( refused ) && !ColorBufferFormat( refused ),
		    "P6 format refused: " + std::to_string( int( refused ) ) );
	checks.That( !ColorBufferFormat( Format::kR8Unorm ), "P6 R8 is not a colour buffer" );
	checks.That( DepthBufferFormat( Format::kD24UnormS8 ) == depthbuffer::kDepth24Stencil8,
	    "P6 D24S8 depth" );
	checks.That( !DepthBufferFormat( Format::kD32Float ), "P6 D32 refused" );

	const DeviceFacts &facts = AdapterFacts();
	checks.That( facts.capabilities == CapabilitySet{ Capability::kTextureCompressionETC1 },
	    "P6 ETC1 (D40) alone of the optional capabilities" );
	checks.That( facts.artifactFormat == ArtifactFormat::kPica, "P6 kPica artifacts" );
	checks.Equal( facts.limits.maxTextureDimension2D, 1024u, "P6 1024 texture limit" );
	checks.Equal( facts.limits.maxColorAttachments, 1u, "P6 one colour attachment" );
	checks.Equal( facts.limits.sampleCounts, 1u, "P6 one sample" );

	checks.That( Describe().id == "pica", "P6 descriptor id" );
	const auto created = Create( PicaAdapterOptions{} );
	checks.That( !created && created.Error().status == DeviceStatus::kUnavailable,
	    "P6 a host build refuses creation (only the 3DS has a device)" );
	PicaAdapterOptions small;
	small.commandListBytes = 1024;
	const auto tooSmall = Create( small );
	checks.That( !tooSmall && tooSmall.Error().status == DeviceStatus::kInvalidDescription,
	    "P6 a command buffer under 4096 bytes is invalid" );
	DeviceRequest request;
	request.required = { Capability::kCompute };
	const auto refused = Describe().create( request );
	checks.That( !refused && refused.Error().status == DeviceStatus::kUnsupported &&
	                 refused.Error().nativeCode == std::int32_t( Capability::kCompute ),
	    "P6 a required capability is refused by name" );
}

// -- P8 ------------------------------------------------------------------

void LayoutCases( testing::Checks &checks )
{
	bool seen[64] = {};
	bool bijective = true;
	for ( std::uint32_t y = 0; y < 8; ++y )
		for ( std::uint32_t x = 0; x < 8; ++x )
		{
			const std::uint32_t at = MortonOffset( x, y );
			bijective = bijective && at < 64 && !seen[at];
			if ( at < 64 )
				seen[at] = true;
		}
	checks.That( bijective, "P8 the 8x8 Morton order visits every texel once" );
	checks.That( MortonOffset( 1, 0 ) == 1 && MortonOffset( 0, 1 ) == 2 &&
	                 MortonOffset( 2, 0 ) == 4 && MortonOffset( 0, 4 ) == 32 &&
	                 MortonOffset( 7, 7 ) == 63,
	    "P8 x in the low bit, y above it, per 3dbrew" );
	checks.Equal( TiledIndex( 9, 0, 16 ), 64u + 1u, "P8 the second tile follows the first" );
	checks.Equal( TiledIndex( 0, 8, 16 ), 128u, "P8 the second tile row follows the first" );

	TextureLayout small;
	checks.That( LayoutOf( Format::kRGBA8Unorm, 4, 4, 1, small ) &&
	                 small.levels[0].storedWidth == 8 && small.levels[0].storedHeight == 8 &&
	                 small.sampledLevels == 0 && small.bytes == 256,
	    "P8 a 4x4 texture takes one tile and cannot be sampled" );
	TextureLayout chain;
	const bool made = LayoutOf( Format::kRGBA8Unorm, 64, 32, 7, chain );
	checks.That( made && chain.sampledLevels == 3, "P8 64x32: levels down to 16x8 are sampled" );
	checks.That( made && chain.levels[1].offset == 64u * 32 * 4 &&
	                 chain.levels[2].offset == 64u * 32 * 4 + 32 * 16 * 4 &&
	                 chain.levels[6].width == 1 && chain.levels[6].storedWidth == 8,
	    "P8 levels follow each other; the smallest are padded to a tile" );
	TextureLayout odd;
	checks.That( LayoutOf( Format::kRGBA8Unorm, 400, 240, 1, odd ) && odd.sampledLevels == 0 &&
	                 odd.levels[0].storedWidth == 400 && odd.levels[0].storedHeight == 240,
	    "P8 a 400x240 target is stored but not sampled" );
	checks.That( !LayoutOf( Format::kRGBA16Float, 8, 8, 1, small ) &&
	                 !LayoutOf( Format::kBC1Unorm, 8, 8, 1, small ),
	    "P8 formats the PICA cannot hold have no layout" );

	// RGBA8 is the word 0xRRGGBBAA; BGRA8 stores the same word.
	const std::byte rgba[4] = {
	    std::byte( 0x11 ), std::byte( 0x22 ), std::byte( 0x33 ), std::byte( 0x44 ) };
	const std::byte bgra[4] = {
	    std::byte( 0x33 ), std::byte( 0x22 ), std::byte( 0x11 ), std::byte( 0x44 ) };
	checks.Equal( StoreTexel( Format::kRGBA8Unorm, rgba ), 0x11223344u, "P8 RGBA8 word" );
	checks.Equal( StoreTexel( Format::kBGRA8Unorm, bgra ), 0x11223344u, "P8 BGRA8 word" );
	checks.Equal(
	    StoreTexel( Format::kR8Unorm, rgba ), 0x110000FFu, "P8 R8 as RGBA8 (r, 0, 0, 1)" );
	const float half[4] = { 0.5f, 0.0f, 1.0f, 1.0f };
	checks.Equal( ClearWord( Format::kRGBA8Unorm, half ), 0x8000FFFFu, "P8 a clear colour's word" );
	checks.Equal( DepthClearWord( 1.0f, 0x5A ), 0x5AFFFFFFu, "P8 a depth clear's word" );

	for ( Format format :
	    { Format::kRGBA8Unorm, Format::kBGRA8Unorm, Format::kR8Unorm, Format::kD24UnormS8 } )
	{
		TextureLayout layout;
		(void)LayoutOf( format, 24, 16, 1, layout );
		std::vector<std::byte> stored( layout.bytes, std::byte( 0xEE ) );
		const std::uint32_t texel = PortTexelBytes( format );
		std::vector<std::byte> rows( 5 * 7 * texel );
		for ( std::size_t i = 0; i < rows.size(); ++i )
			rows[i] = std::byte( ( i * 37 + 11 ) & 0xFF );
		if ( format == Format::kD24UnormS8 )
			for ( std::size_t i = 0; i < 35; ++i )
			{
				const float depth = float( i ) / 34.0f;
				std::memcpy( rows.data() + i * 4, &depth, 4 );
			}
		CopyIn( format, layout.levels[0], stored.data(), 3, 9, 5, 7, rows.data() );
		std::vector<std::byte> back( rows.size() );
		CopyOut( format, layout.levels[0], stored.data(), 3, 9, 5, 7, back.data() );
		bool same = true;
		for ( std::size_t i = 0; i < rows.size(); i += texel )
		{
			if ( format == Format::kD24UnormS8 )
			{
				float a = 0, b = 0;
				std::memcpy( &a, rows.data() + i, 4 );
				std::memcpy( &b, back.data() + i, 4 );
				same = same && std::fabs( a - b ) <= 1.0f / 16777215.0f;
			}
			else
				same = same && std::memcmp( rows.data() + i, back.data() + i, texel ) == 0;
		}
		// The texel at (3, 9) is stored at the tile index the layout names.
		const std::size_t first =
		    std::size_t( TiledIndex( 3, 9, layout.levels[0].storedWidth ) ) * 4;
		const bool placed = format == Format::kD24UnormS8 ||
		                    std::to_integer<int>( stored[first + 3] ) ==
		                        std::to_integer<int>( rows[format == Format::kBGRA8Unorm ? 2 : 0] );
		checks.That( same && placed,
		    "P8 a region copies in and out unchanged through the stored form: format " +
		        std::to_string( int( format ) ) );
	}

	// ETC (D40): the stored blocks in the order the 3DS reads them, as
	// materialsystem/shaderapipica/pica_texture.cpp writes them (proven on
	// the device): tiles row-major, each tile's four blocks in Z order, at
	// consecutive offsets; an ETC1 word stored little-endian, the port's in
	// the specification's byte order; ETC1A4 blocks the same 16 bytes.
	for ( Format format : { Format::kETC1Rgb, Format::kETC1A4 } )
	{
		const std::uint32_t width = 16, height = 24,
		                    blockBytes = format == Format::kETC1Rgb ? 8 : 16;
		TextureLayout layout;
		const bool made = LayoutOf( format, width, height, 1, layout );
		checks.That( made && layout.bytes == std::uint64_t( width ) * height / 16 * blockBytes &&
		                 layout.sampledLevels == 0,
		    "P8 an ETC level takes whole blocks (16x24: stored, not sampled)" );
		std::vector<std::byte> port( std::size_t( width / 4 ) * ( height / 4 ) * blockBytes );
		for ( std::size_t i = 0; i < port.size(); ++i )
			port[i] = std::byte( ( i * 29 + 7 ) & 0xFF );
		std::vector<std::byte> stored( layout.bytes, std::byte( 0xEE ) );
		CopyIn( format, layout.levels[0], stored.data(), 0, 0, width, height, port.data() );
		bool ordered = true;
		std::size_t at = 0;
		for ( std::uint32_t ty = 0; ty < height; ty += 8 )
			for ( std::uint32_t tx = 0; tx < width; tx += 8 )
				for ( std::uint32_t b = 0; b < 4; ++b, at += blockBytes )
				{
					const std::uint32_t bx = ( tx + ( b & 1 ) * 4 ) / 4,
					                    by = ( ty + ( b >> 1 ) * 4 ) / 4;
					const std::byte *source =
					    &port[( std::size_t( by ) * ( width / 4 ) + bx ) * blockBytes];
					for ( std::uint32_t k = 0; k < blockBytes; ++k )
					{
						const std::byte want =
						    format == Format::kETC1Rgb ? source[7 - k] : source[k];
						ordered = ordered && stored[at + k] == want;
					}
				}
		std::vector<std::byte> back( port.size() );
		CopyOut( format, layout.levels[0], stored.data(), 0, 0, width, height, back.data() );
		checks.That(
		    ordered, "P8 ETC blocks stored in the 3DS's tile and Z order, byte order converted: " +
		                 std::to_string( int( format ) ) );
		checks.That(
		    back == port, "P8 ETC blocks copy out unchanged: " + std::to_string( int( format ) ) );
		// A region of whole blocks moves only those blocks.
		std::vector<std::byte> one( blockBytes, std::byte( 0x5A ) );
		std::vector<std::byte> before = stored;
		CopyIn( format, layout.levels[0], stored.data(), 4, 8, 4, 4, one.data() );
		std::size_t changed = 0;
		for ( std::size_t i = 0; i < stored.size(); ++i )
			changed += stored[i] != before[i];
		checks.That(
		    changed > 0 && changed <= blockBytes, "P8 a one-block ETC copy writes one block" );
	}
}

} // namespace

int main()
{
	testing::Checks checks;
	ArtifactCases( checks );
	BlendCases( checks );
	CompareCases( checks );
	FactCases( checks );
	LayoutCases( checks );
	return checks.Report();
}
