//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite lightmap-basis (RFC 0016 K11 "Model assembly",
//			the indirect diffuse term of static surfaces): the lightmap basis
//			(render/shaders/common/lightmap_basis.glsl) evaluated on the GPU by
//			a check kernel (lightmap_basis_check.comp) and judged against an
//			oracle in this file that shares no code with it:
//			- flat: the page sampled with linear filtering equals the
//			  bilinear interpolation of its half-float texels, within the
//			  filter's sub-texel precision (2^-7 of the neighbours' spread,
//			  plus 1e-5 relative), at 128 random coordinates; at texel
//			  centres it is the texel;
//			- directional (tools/quality/lightmap_directional.py's model):
//			  E0 * clamp( 1 + beta . ( n - N ), 0, 4 ) within 1e-5 relative
//			  + 1e-6 at texel centres, with gains beyond both clamps
//			  exercised; n = N and a zero gradient page are the flat light
//			  bitwise (the term's neutral value);
//			- RNM (lightmappedgeneric_ps2_3_x.h): each bump basis
//			  direction's clamped cosine squared, and the three bumped pages
//			  at one, two and three page offsets weighted by them over their
//			  sum, within 1e-5 relative + 1e-6;
//			- the directional page split (render.pass.world's SplitLightmapLayer):
//			  the flat and gradient pages the kernel samples are split from
//			  one 2:1 LMAP layer, so the flat checks fail if the whole layer
//			  is staged as flat light (sp_gi_chamber_01's defect);
//			- the baked layer rule (BakedLightmapLayer): the total layer, or
//			  the indirect layer when the core owns the surface's direct
//			  light; a map without the layer has none.
//			Tolerances were fixed before the first run.
//
//			Seeded (--sensitivity): the smooth normal ignored and the gain
//			unclamped (directional.), the RNM weights unsquared and the
//			bumped pages read from the flat one (rnm.), each a check kernel
//			built from lightmap_basis.glsl; the whole directional layer
//			staged as the flat page (flat.), a loader defect.
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/device/device.h"
#include "spv/lightmap_basis_check_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;

constexpr std::uint32_t kPage = 16; // the flat page's width and height
constexpr std::uint32_t kRnmWidth = 4 * kPage;
constexpr float kRnmOffset = float( kPage ) / float( kRnmWidth );
constexpr float kGainMax = 4.0f;
constexpr float kFilterPrecision = 1.0f / 128.0f; // of the neighbours' spread
constexpr float kRelative = 1e-5f;
constexpr float kAbsolute = 1e-6f;
constexpr std::uint32_t kFilterCases = 128;

// The loader defect the sensitivity run seeds: no kernel of its own, so a
// marker module the run recognises and replaces by the control kernel.
const std::uint32_t kWholePageMarker[1] = { 0 };

struct float3
{
	float x = 0, y = 0, z = 0;
};

float Dot( float3 a, float3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float3 Normalize( float3 v )
{
	const float length = std::sqrt( Dot( v, v ) );
	return { v.x / length, v.y / length, v.z / length };
}

// A deterministic generator (the suite's values never change between runs).
class Random
{
public:
	float Next()
	{
		m_State = m_State * 6364136223846793005ull + 1442695040888963407ull;
		return float( ( m_State >> 40 ) & 0xffffff ) / float( 0x1000000 );
	}
	float Range( float low, float high ) { return low + ( high - low ) * Next(); }
	float3 Direction()
	{
		for ( ;; )
		{
			const float3 v{ Range( -1, 1 ), Range( -1, 1 ), Range( -1, 1 ) };
			const float length = Dot( v, v );
			if ( length > 1e-3f && length <= 1.0f )
				return Normalize( v );
		}
	}

private:
	std::uint64_t m_State = 0x1234567ull;
};

// An RGBA page as the oracle reads it: the texels' half-float values.
struct Page
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::array<float, 3>> texels; // row 0 at the top

	std::array<float, 3> At( int x, int y ) const
	{
		x = std::clamp( x, 0, int( width ) - 1 ); // clamp to edge
		y = std::clamp( y, 0, int( height ) - 1 );
		return texels[std::size_t( y ) * width + std::size_t( x )];
	}
};

// A page of random values rounded to half floats, with its texel bytes.
Page MakePage( std::uint32_t width, std::uint32_t height, float low, float high, Random &random,
    std::vector<std::byte> &bytes )
{
	Page page{ width, height, {} };
	page.texels.resize( std::size_t( width ) * height );
	bytes.resize( page.texels.size() * 8 );
	for ( std::size_t i = 0; i < page.texels.size(); ++i )
	{
		std::uint16_t half[4];
		for ( int c = 0; c < 3; ++c )
		{
			half[c] = FloatToHalf( random.Range( low, high ) );
			page.texels[i][std::size_t( c )] = HalfToFloat( half[c] );
		}
		half[3] = FloatToHalf( 1.0f );
		std::memcpy( bytes.data() + i * 8, half, 8 );
	}
	return page;
}

// Bilinear interpolation at `uv`, and each channel's neighbour spread.
std::array<float, 3> Bilinear( const Page &page, float u, float v, std::array<float, 3> *spread )
{
	const float x = u * float( page.width ) - 0.5f;
	const float y = v * float( page.height ) - 0.5f;
	const int x0 = int( std::floor( x ) );
	const int y0 = int( std::floor( y ) );
	const float fx = x - float( x0 );
	const float fy = y - float( y0 );
	const std::array<float, 3> a = page.At( x0, y0 ), b = page.At( x0 + 1, y0 ),
	                           c = page.At( x0, y0 + 1 ), d = page.At( x0 + 1, y0 + 1 );
	std::array<float, 3> out{};
	for ( std::size_t k = 0; k < 3; ++k )
	{
		const double top = double( a[k] ) + ( double( b[k] ) - a[k] ) * fx;
		const double bottom = double( c[k] ) + ( double( d[k] ) - c[k] ) * fx;
		out[k] = float( top + ( bottom - top ) * fy );
		if ( spread )
			( *spread )[k] =
			    std::max( { a[k], b[k], c[k], d[k] } ) - std::min( { a[k], b[k], c[k], d[k] } );
	}
	return out;
}

// The kernel's case record (lightmap_basis_check.comp's Case).
struct GpuCase
{
	float uv[4];
	float normal[4];
	float smoothNormal[4];
	float tangentNormal[4];
};

struct Case
{
	enum class Kind
	{
		kFilter, // a random coordinate: the flat page's filtering
		kCentre  // a texel centre: the basis's arithmetic
	};
	Kind kind = Kind::kCentre;
	int texelX = 0, texelY = 0;
	float u = 0, v = 0;
	float3 normal, smoothNormal, tangentNormal;
	bool sameNormal = false;
};

// One dispatch: the four outputs per case.
struct Outputs
{
	std::vector<std::array<float, 4>> values; // 4 per case
	const float *Flat( std::size_t i ) const { return values[i * 4 + 0].data(); }
	const float *Directional( std::size_t i ) const { return values[i * 4 + 1].data(); }
	const float *Rnm( std::size_t i ) const { return values[i * 4 + 2].data(); }
	const float *Weights( std::size_t i ) const { return values[i * 4 + 3].data(); }
};

bool Near( float expected, float actual, float relative, float absolute )
{
	return std::isfinite( actual ) &&
	       std::fabs( expected - actual ) <= relative * std::fabs( expected ) + absolute;
}

std::string Describe( std::size_t index, const float *expected, const float *actual )
{
	char text[160];
	std::snprintf( text, sizeof( text ),
	    "case %zu: expected (%.6g %.6g %.6g), got (%.6g %.6g %.6g)", index, expected[0],
	    expected[1], expected[2], actual[0], actual[1], actual[2] );
	return text;
}

// One dispatch of the check kernel over its three pages.
std::optional<std::string> Dispatch( CheckKernel &kernel, resources::TextureCache &cache,
    TextureId flat, TextureId gradient, TextureId rnm, std::span<const GpuCase> cases,
    Outputs &out )
{
	const TextureId textures[] = { flat, gradient, rnm };
	std::vector<std::byte> bytes;
	if ( std::optional<std::string> why = kernel.Run( cache, textures,
	         std::uint32_t( cases.size() ), std::as_bytes( cases ), cases.size() * 4 * 16, bytes ) )
		return why;
	out.values.resize( cases.size() * 4 );
	std::memcpy( out.values.data(), bytes.data(), bytes.size() );
	return std::nullopt;
}

std::optional<std::string> Stage( resources::TextureCache &cache, const std::string &name,
    std::uint32_t width, std::uint32_t height, std::span<const std::byte> texels, TextureId &out )
{
	TextureDesc desc;
	desc.format = Format::kRGBA16Float;
	desc.width = width;
	desc.height = height;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto staged = cache.Stage( name, desc, texels );
	if ( !staged )
		return "the page " + name + " was refused";
	out = staged.Value().texture;
	return std::nullopt;
}

void PolicyChecks( Results &results )
{
	using mapcontainer::WorldLightmapLayer;
	results.That(
	    BakedLightmapLayer( false ) == WorldLightmapLayer::Total, "policy.unowned-reads-total" );
	results.That(
	    BakedLightmapLayer( true ) == WorldLightmapLayer::Indirect, "policy.owned-reads-indirect" );
	mapcontainer::WorldLightmapLayout layered{};
	layered.layerCount = 3;
	layered.roles[0] = WorldLightmapLayer::Total;
	layered.roles[1] = WorldLightmapLayer::Direct;
	layered.roles[2] = WorldLightmapLayer::Indirect;
	results.That( mapcontainer::WorldLightmapLayerIndex( layered, BakedLightmapLayer( true ) ) == 2,
	    "policy.owned-layer-of-three" );
	mapcontainer::WorldLightmapLayout single{};
	single.layerCount = 1;
	single.roles[0] = WorldLightmapLayer::Total;
	results.That( mapcontainer::WorldLightmapLayerIndex( single, BakedLightmapLayer( true ) ) < 0,
	    "policy.owned-without-indirect-has-none" );
	results.That( mapcontainer::WorldLightmapLayerIndex( single, BakedLightmapLayer( false ) ) == 0,
	    "policy.unowned-single-page" );
}

std::optional<std::string> BasisChecks( IRenderDevice2 &device,
    std::span<const std::uint32_t> module, bool wholePage, Results &results )
{
	Random random;
	// The directional layer as the map pipeline writes it: E0 on the left,
	// beta on the right, and the oracle's own copies of both.
	std::vector<std::byte> e0Bytes, betaBytes, rnmBytes;
	const Page e0 = MakePage( kPage, kPage, 0.05f, 2.0f, random, e0Bytes );
	// LMAP v3 stores beta components in [-2, 2] (beta / 4 + 0.5).
	const Page beta = MakePage( kPage, kPage, -2.0f, 2.0f, random, betaBytes );
	const Page rnm = MakePage( kRnmWidth, kPage, 0.0f, 3.0f, random, rnmBytes );
	std::vector<std::byte> layer( std::size_t( 2 * kPage ) * kPage * 8 );
	for ( std::uint32_t y = 0; y < kPage; ++y )
	{
		const std::size_t row = std::size_t( kPage ) * 8;
		std::memcpy( layer.data() + y * 2 * row, e0Bytes.data() + y * row, row );
		std::memcpy( layer.data() + y * 2 * row + row, betaBytes.data() + y * row, row );
	}
	const LightmapLayerPages pages = SplitLightmapLayer( layer, 2 * kPage, kPage );
	results.That( pages.Directional() && pages.width == kPage && pages.height == kPage,
	    "page-split.directional-halves", "the 2:1 layer did not split into two pages" );
	results.That( pages.flat == e0Bytes, "page-split.flat-is-left-half" );
	// The gradient page stores beta / 4 + 0.5; the oracle reads beta as the
	// shader does (g * 4 - 2), from the stored halves.
	Page stored{ kPage, kPage, {} };
	stored.texels.resize( beta.texels.size() );
	bool storedNear = pages.gradient.size() == betaBytes.size();
	for ( std::size_t i = 0; storedNear && i < stored.texels.size(); ++i )
	{
		std::uint16_t half[4];
		std::memcpy( half, pages.gradient.data() + i * 8, sizeof( half ) );
		for ( std::size_t c = 0; c < 3; ++c )
		{
			stored.texels[i][c] = HalfToFloat( half[c] ) * 4.0f - 2.0f;
			storedNear = storedNear && std::fabs( stored.texels[i][c] - beta.texels[i][c] ) < 2e-3f;
		}
		storedNear = storedNear && half[3] == 0x3c00u; // the flat texel's alpha, 1
	}
	results.That( storedNear, "page-split.gradient-is-right-half-biased" );
	const LightmapLayerPages square = SplitLightmapLayer( e0Bytes, kPage, kPage );
	results.That(
	    !square.Directional() && square.flat == e0Bytes, "page-split.flat-page-unchanged" );
	results.That( SplitLightmapLayer( std::span( e0Bytes ).first( 8 ), kPage, kPage ).flat.empty(),
	    "page-split.short-layer-refused" );

	resources::TextureCache cache( device );
	TextureId flatPage, gradientPage, rnmPage, zeroPage;
	// A zero gradient as stored: 0.5 in RGB (exact in half), alpha 1.
	std::vector<std::byte> zeros( std::size_t( kPage ) * kPage * 8 );
	for ( std::size_t i = 0; i < zeros.size(); i += 8 )
	{
		const std::uint16_t half[4] = { 0x3800u, 0x3800u, 0x3800u, 0x3c00u };
		std::memcpy( zeros.data() + i, half, sizeof( half ) );
	}
	// The seeded loader defect stages the whole layer as the flat light.
	if ( std::optional<std::string> why =
	         wholePage ? Stage( cache, "flat", 2 * kPage, kPage, layer, flatPage )
	                   : Stage( cache, "flat", pages.width, pages.height, pages.flat, flatPage ) )
		return why;
	if ( std::optional<std::string> why =
	         Stage( cache, "gradient", kPage, kPage, pages.gradient, gradientPage ) )
		return why;
	if ( std::optional<std::string> why =
	         Stage( cache, "rnm", kRnmWidth, kPage, rnmBytes, rnmPage ) )
		return why;
	if ( std::optional<std::string> why = Stage( cache, "zero", kPage, kPage, zeros, zeroPage ) )
		return why;

	// The cases: random coordinates for the filter, texel centres for the
	// arithmetic (a third of them with n = N).
	std::vector<Case> cases;
	for ( std::uint32_t i = 0; i < kFilterCases; ++i )
	{
		Case c;
		c.kind = Case::Kind::kFilter;
		c.u = random.Range( 0.5f / kPage, 1.0f - 0.5f / kPage );
		c.v = random.Range( 0.5f / kPage, 1.0f - 0.5f / kPage );
		c.normal = c.smoothNormal = c.tangentNormal = { 0, 0, 1 };
		cases.push_back( c );
	}
	for ( std::uint32_t y = 0; y < kPage; ++y )
	{
		for ( std::uint32_t x = 0; x < kPage; ++x )
		{
			Case c;
			c.texelX = int( x );
			c.texelY = int( y );
			c.u = ( float( x ) + 0.5f ) / float( kPage );
			c.v = ( float( y ) + 0.5f ) / float( kPage );
			c.smoothNormal = random.Direction();
			c.sameNormal = ( x + y ) % 3 == 0;
			c.normal = c.sameNormal ? c.smoothNormal : random.Direction();
			// Tangent-space normals toward +z, some beyond a basis direction.
			float3 t = random.Direction();
			t.z = std::fabs( t.z ) + 0.2f;
			c.tangentNormal = Normalize( t );
			cases.push_back( c );
		}
	}
	std::vector<GpuCase> gpu( cases.size() );
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		// The RNM coordinate is the flat region's texel in the RNM page.
		const float rnmU = c.kind == Case::Kind::kCentre
		                       ? ( float( c.texelX ) + 0.5f ) / float( kRnmWidth )
		                       : c.u * kRnmOffset;
		gpu[i] = { { c.u, c.v, kRnmOffset, rnmU }, { c.normal.x, c.normal.y, c.normal.z, 0 },
		    { c.smoothNormal.x, c.smoothNormal.y, c.smoothNormal.z, 0 },
		    { c.tangentNormal.x, c.tangentNormal.y, c.tangentNormal.z, 0 } };
	}
	// The kernel reads the RNM coordinate from uv.xy: a second case list
	// with it in place, dispatched on the same pages.
	std::vector<GpuCase> gpuRnm = gpu;
	for ( GpuCase &c : gpuRnm )
		c.uv[0] = c.uv[3];

	CheckKernel kernel( device );
	if ( std::optional<std::string> why = kernel.Create(
	         module.empty() ? std::span<const std::uint32_t>( spirv::kLightmapBasisCheck ) : module,
	         3, 1, "render_lab.lightmap-basis-check" ) )
		return why;
	Outputs main, rnmRun, zero;
	if ( std::optional<std::string> why =
	         Dispatch( kernel, cache, flatPage, gradientPage, rnmPage, gpu, main ) )
		return why;
	if ( std::optional<std::string> why =
	         Dispatch( kernel, cache, flatPage, gradientPage, rnmPage, gpuRnm, rnmRun ) )
		return why;
	if ( std::optional<std::string> why =
	         Dispatch( kernel, cache, flatPage, zeroPage, rnmPage, gpu, zero ) )
		return why;

	std::size_t filterFailures = 0, centreFailures = 0, directionalFailures = 0,
	            sameNormalFailures = 0, zeroFailures = 0, weightFailures = 0, rnmFailures = 0;
	std::size_t clampedHigh = 0, clampedLow = 0;
	std::string filterFirst, centreFirst, directionalFirst, sameFirst, zeroFirst, weightFirst,
	    rnmFirst;
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		if ( c.kind == Case::Kind::kFilter )
		{
			std::array<float, 3> spread{};
			const std::array<float, 3> expected = Bilinear( e0, c.u, c.v, &spread );
			bool ok = true;
			for ( std::size_t k = 0; k < 3; ++k )
				ok = ok && Near( expected[k], main.Flat( i )[k], kRelative,
				               kFilterPrecision * spread[k] + kAbsolute );
			if ( !ok && filterFailures++ == 0 )
				filterFirst = Describe( i, expected.data(), main.Flat( i ) );
			continue;
		}
		const std::array<float, 3> texel = e0.At( c.texelX, c.texelY );
		const std::array<float, 3> gradient = stored.At( c.texelX, c.texelY );
		bool ok = true;
		for ( std::size_t k = 0; k < 3; ++k )
			ok = ok && Near( texel[k], main.Flat( i )[k], kRelative, kAbsolute );
		if ( !ok && centreFailures++ == 0 )
			centreFirst = Describe( i, texel.data(), main.Flat( i ) );

		// Directional, in double precision.
		const double gain = 1.0 + double( gradient[0] ) * ( c.normal.x - c.smoothNormal.x ) +
		                    double( gradient[1] ) * ( c.normal.y - c.smoothNormal.y ) +
		                    double( gradient[2] ) * ( c.normal.z - c.smoothNormal.z );
		clampedHigh += gain > kGainMax + 0.05 ? 1 : 0;
		clampedLow += gain < -0.05 ? 1 : 0;
		const double clamped = std::clamp( gain, 0.0, double( kGainMax ) );
		const float directional[3] = {
		    float( texel[0] * clamped ), float( texel[1] * clamped ), float( texel[2] * clamped ) };
		ok = true;
		for ( std::size_t k = 0; k < 3; ++k )
			ok = ok && Near( directional[k], main.Directional( i )[k], kRelative, kAbsolute );
		if ( !ok && directionalFailures++ == 0 )
			directionalFirst = Describe( i, directional, main.Directional( i ) );
		if ( c.sameNormal &&
		     std::memcmp( main.Directional( i ), main.Flat( i ), 3 * sizeof( float ) ) != 0 &&
		     sameNormalFailures++ == 0 )
			sameFirst = Describe( i, main.Flat( i ), main.Directional( i ) );
		if ( std::memcmp( zero.Directional( i ), zero.Flat( i ), 3 * sizeof( float ) ) != 0 &&
		     zeroFailures++ == 0 )
			zeroFirst = Describe( i, zero.Flat( i ), zero.Directional( i ) );

		// RNM: weights, then the bumped pages at one to three offsets.
		const float3 basis[3] = { { 0.81649661f, 0.0f, 0.57735026f },
		    { -0.40824834f, 0.70710677f, 0.57735026f },
		    { -0.40824822f, -0.70710683f, 0.57735026f } };
		double weights[3];
		for ( int b = 0; b < 3; ++b )
		{
			const double cosine =
			    std::clamp( double( Dot( c.tangentNormal, basis[b] ) ), 0.0, 1.0 );
			weights[b] = cosine * cosine;
		}
		const float expectedWeights[3] = {
		    float( weights[0] ), float( weights[1] ), float( weights[2] ) };
		ok = true;
		for ( std::size_t k = 0; k < 3; ++k )
			ok = ok && Near( expectedWeights[k], main.Weights( i )[k], kRelative, kAbsolute );
		if ( !ok && weightFailures++ == 0 )
			weightFirst = Describe( i, expectedWeights, main.Weights( i ) );
		const double sum = weights[0] + weights[1] + weights[2];
		float expectedRnm[3];
		for ( std::size_t k = 0; k < 3; ++k )
		{
			double light = 0.0;
			for ( int b = 0; b < 3; ++b )
				light += weights[b] * rnm.At( c.texelX + int( kPage ) * ( b + 1 ), c.texelY )[k];
			expectedRnm[k] = float( light / sum );
		}
		ok = true;
		for ( std::size_t k = 0; k < 3; ++k )
			ok = ok && Near( expectedRnm[k], rnmRun.Rnm( i )[k], kRelative, kAbsolute );
		if ( !ok && rnmFailures++ == 0 )
			rnmFirst = Describe( i, expectedRnm, rnmRun.Rnm( i ) );
	}
	auto count = []( std::size_t failures, const std::string &first )
	{
		return failures == 0 ? std::string()
		                     : std::to_string( failures ) + " cases differ; first " + first;
	};
	results.That( filterFailures == 0, "flat.filtered", count( filterFailures, filterFirst ) );
	results.That( centreFailures == 0, "flat.texel-centres", count( centreFailures, centreFirst ) );
	results.That( directionalFailures == 0, "directional.oracle",
	    count( directionalFailures, directionalFirst ) );
	results.That( clampedHigh > 0 && clampedLow > 0, "directional.clamps-exercised",
	    std::to_string( clampedHigh ) + " above, " + std::to_string( clampedLow ) + " below" );
	results.That( sameNormalFailures == 0, "directional.smooth-normal-is-flat-bitwise",
	    count( sameNormalFailures, sameFirst ) );
	results.That( zeroFailures == 0, "directional.zero-gradient-is-flat-bitwise",
	    count( zeroFailures, zeroFirst ) );
	results.That( weightFailures == 0, "rnm.weights", count( weightFailures, weightFirst ) );
	results.That( rnmFailures == 0, "rnm.oracle", count( rnmFailures, rnmFirst ) );
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	const bool wholePage = module.data() == kWholePageMarker;
	if ( wholePage )
		module = {};
	PolicyChecks( results );
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	if ( std::optional<std::string> why = BasisChecks( *device, module, wholePage, results ) )
		return why;
	(void)device->WaitIdle();
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kBasisSeeded[] = {
    { "no-smooth-normal", spirv::kLightmapBasisNoSmoothNormal, "directional." },
    { "no-gain-clamp", spirv::kLightmapBasisNoGainClamp, "directional." },
    { "gradient-unbiased", spirv::kLightmapBasisGradientUnbiased, "directional." },
    { "rnm-unsquared", spirv::kLightmapBasisRnmUnsquared, "rnm." },
    { "rnm-offset-from-zero", spirv::kLightmapBasisRnmOffsetFromZero, "rnm." },
    { "whole-page", kWholePageMarker, "flat." } };

} // namespace

int RunLightmapBasisSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "lightmap-basis", kBasisSeeded, RunOnce );
}

} // namespace render::lab
