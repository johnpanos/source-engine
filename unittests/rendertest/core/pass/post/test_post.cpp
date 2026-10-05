//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.post (RFC 0016 K8 "Post and screen effects") with real
//			pixels, on render.device.vulkan, or on render.device.gl with
//			RENDERTEST_POST_GL. A 64x32 linear scene (a gray ramp, values
//			above white and a bright block) goes through the bloom chain and
//			render.pass.output, and both images are judged against a CPU
//			oracle written here from the fxc sources (Downsample_nohdr_ps2x,
//			BlurFilter_ps2x, Engine_Post_ps2x), sharing no code with the GLSL:
//
//			C1 the claims: each legacy shader of the chain is claimed with
//			   its role, $bloomamount and the r_bloomtint value; Counter-
//			   Strike's shape, Engine_Post's software AA and screenspace
//			   programs other than bloomadd are refused by name;
//			B1 the bloom image (8-bit, quarter size) equals the oracle within
//			   two levels: the downsample shapes sRGB-encoded clipped taps,
//			   the blurs are the 13-tap kernel, the vertical one stepping by
//			   1 / width (BlurFilterY's quirk) and scaled by the bloom amount;
//			B2 the quirk is a discriminating case: the oracle with a 1 /
//			   height step differs from it by more than ten levels somewhere;
//			O1 the output adds the bloom to the tone-mapped frame in its sRGB
//			   encoding (within two levels); without a bloom it is the legacy
//			   clip alone, byte for byte as before;
//			P1 non-finite parameters and an empty scene are refused;
//			S1 seeded programs (shaping linear values, the vertical step by
//			   1 / height, the bloom added before the tone map) each fail.
//
//=============================================================================//

#include "render/pass/output/output.h"
#include "render/pass/post/post.h"
#include "spv/post_defects_spv.h"
#include "testing/checks.h"

#if defined( RENDERTEST_POST_GL )
#include "render/device/gl/provider.h"
#else
#include "render/device/vulkan/provider.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <span>
#include <thread>
#include <vector>

namespace
{

using namespace render;
namespace post = render::pass::post;
namespace output = render::pass::output;

constexpr std::uint32_t kWidth = 64;
constexpr std::uint32_t kHeight = 32;
constexpr std::uint32_t kQuarterWidth = kWidth / 4;
constexpr std::uint32_t kQuarterHeight = kHeight / 4;
constexpr float kExposure = 1.5f;
constexpr float kAmount = 1.75f;
constexpr float kTint[4] = { 0.3f, 0.59f, 0.11f, 2.2f };

using Rgb = std::array<double, 3>;

// --- the oracle -----------------------------------------------------------------

double SrgbEncode( double c )
{
	c = std::clamp( c, 0.0, 1.0 );
	return c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow( c, 1.0 / 2.4 ) - 0.055;
}

double SrgbDecode( double c )
{
	c = std::max( c, 0.0 );
	return c <= 0.04045 ? c / 12.92 : std::pow( ( c + 0.055 ) / 1.055, 2.4 );
}

double Quantize( double c )
{
	return std::round( std::clamp( c, 0.0, 1.0 ) * 255.0 ) / 255.0;
}

struct Image
{
	std::uint32_t width = 0, height = 0;
	std::vector<Rgb> texels;
	Rgb At( int x, int y ) const
	{
		x = std::clamp( x, 0, int( width ) - 1 );
		y = std::clamp( y, 0, int( height ) - 1 );
		return texels[std::size_t( y ) * width + x];
	}
	// A bilinear sample at uv, clamped to the edge.
	Rgb Sample( double u, double v ) const
	{
		const double x = u * width - 0.5, y = v * height - 0.5;
		const int x0 = int( std::floor( x ) ), y0 = int( std::floor( y ) );
		const double fx = x - x0, fy = y - y0;
		Rgb out{};
		for ( int c = 0; c < 3; ++c )
			out[c] = ( At( x0, y0 )[c] * ( 1 - fx ) + At( x0 + 1, y0 )[c] * fx ) * ( 1 - fy ) +
			         ( At( x0, y0 + 1 )[c] * ( 1 - fx ) + At( x0 + 1, y0 + 1 )[c] * fx ) * fy;
		return out;
	}
};

std::vector<float> MakeScene()
{
	std::vector<float> rgba( std::size_t( kWidth ) * kHeight * 4, 1.0f );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			float *t = &rgba[( std::size_t( y ) * kWidth + x ) * 4];
			const float ramp = float( x ) / ( kWidth - 1 ) * 1.2f;
			t[0] = ramp;
			t[1] = ramp * 0.5f;
			t[2] = float( y ) / kHeight * 0.25f;
			// A bright block: values above white that clip, and bloom.
			if ( x >= 20 && x < 32 && y >= 8 && y < 20 )
			{
				t[0] = 4.0f;
				t[1] = 3.0f;
				t[2] = 2.0f;
			}
		}
	return rgba;
}

Image OracleBloom( const std::vector<float> &scene, bool heightStep )
{
	Image encoded{ kWidth, kHeight, {} };
	for ( std::size_t i = 0; i < std::size_t( kWidth ) * kHeight; ++i )
		encoded.texels.push_back(
		    { SrgbEncode( scene[i * 4] * kExposure ), SrgbEncode( scene[i * 4 + 1] * kExposure ),
		        SrgbEncode( scene[i * 4 + 2] * kExposure ) } );
	auto shape = []( Rgb p )
	{
		const double lum = p[0] * kTint[0] + p[1] * kTint[1] + p[2] * kTint[2];
		for ( double &c : p )
			c = std::pow( c, double( kTint[3] ) ) * lum;
		return p;
	};
	Image down{ kQuarterWidth, kQuarterHeight, {} };
	for ( std::uint32_t y = 0; y < kQuarterHeight; ++y )
		for ( std::uint32_t x = 0; x < kQuarterWidth; ++x )
		{
			Rgb sum{};
			for ( int ty : { 0, 2 } )
				for ( int tx : { 0, 2 } )
				{
					Rgb tap{};
					for ( int dy = 0; dy < 2; ++dy )
						for ( int dx = 0; dx < 2; ++dx )
							for ( int c = 0; c < 3; ++c )
								tap[c] += encoded.At(
								              int( x * 4 ) + tx + dx, int( y * 4 ) + ty + dy )[c] /
								          4;
					const Rgb shaped = shape( tap );
					for ( int c = 0; c < 3; ++c )
						sum[c] += shaped[c] / 4;
				}
			for ( double &c : sum )
				c = Quantize( c );
			down.texels.push_back( sum );
		}
	auto blur = [&]( const Image &source, double du, double dv, double scale )
	{
		Image out{ source.width, source.height, {} };
		for ( std::uint32_t y = 0; y < source.height; ++y )
			for ( std::uint32_t x = 0; x < source.width; ++x )
			{
				const double u = ( x + 0.5 ) / source.width, v = ( y + 0.5 ) / source.height;
				Rgb sum{};
				for ( int tap = 0; tap < 7; ++tap )
				{
					const double o = post::kBlurOffsets[tap];
					const Rgb a = source.Sample( u + du * o, v + dv * o );
					const Rgb b = source.Sample( u - du * o, v - dv * o );
					for ( int c = 0; c < 3; ++c )
						sum[c] += ( tap == 0 ? a[c] : a[c] + b[c] ) * post::kBlurWeights[tap];
				}
				for ( double &c : sum )
					c = Quantize( c * scale );
				out.texels.push_back( sum );
			}
		return out;
	};
	const Image x = blur( down, 1.0 / kQuarterWidth, 0.0, 1.0 );
	return blur( x, 0.0, heightStep ? 1.0 / kQuarterHeight : 1.0 / kQuarterWidth, kAmount );
}

// The output on an 8-bit UNORM target (the shader's sRGB encoding).
std::vector<double> OracleOutput( const std::vector<float> &scene, const Image *bloom )
{
	std::vector<double> out;
	for ( std::uint32_t y = 0; y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			const float *t = &scene[( std::size_t( y ) * kWidth + x ) * 4];
			const Rgb b =
			    bloom ? bloom->Sample( ( x + 0.5 ) / kWidth, ( y + 0.5 ) / kHeight ) : Rgb{};
			for ( int c = 0; c < 3; ++c )
			{
				double encoded = SrgbEncode( t[c] * kExposure ) + b[c];
				// Decoded to linear, then the output's sRGB encoding clips at white.
				out.push_back( SrgbEncode( SrgbDecode( encoded ) ) * 255.0 );
			}
		}
	return out;
}

// --- running ------------------------------------------------------------------

bool Wait( device::IRenderDevice2 &device, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct Frame
{
	bool ok = false;
	std::vector<std::uint8_t> bloom;  // quarter-size RGBA8
	std::vector<std::uint8_t> output; // full-size RGBA8
};

struct Programs
{
	std::span<const std::uint32_t> post;
	std::span<const std::uint32_t> output;
};

Frame Run( device::IRenderDevice2 &device, const std::vector<float> &scene, bool withBloom,
    const Programs &programs, const post::BloomParams &params )
{
	Frame frame;
	auto bloomRenderer = post::BloomRenderer::CreateWithFragment( device, programs.post );
	auto outputRenderer = output::OutputRenderer::CreateWithFragment(
	    device, device::Format::kRGBA8Unorm, programs.output );
	if ( !bloomRenderer || !outputRenderer )
	{
		std::printf( "render.pass.post: renderer creation failed (post %d, output %d)\n",
		    bloomRenderer ? -1 : int( bloomRenderer.Error() ),
		    outputRenderer ? -1 : int( outputRenderer.Error() ) );
		return frame;
	}
	device::TextureDesc sceneDesc;
	sceneDesc.format = device::Format::kRGBA32Float;
	sceneDesc.width = kWidth;
	sceneDesc.height = kHeight;
	sceneDesc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	device::TextureDesc targetDesc;
	targetDesc.format = device::Format::kRGBA8Unorm;
	targetDesc.width = kWidth;
	targetDesc.height = kHeight;
	targetDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	device::BufferDesc readDesc;
	readDesc.size = std::uint64_t( kWidth ) * kHeight * 4;
	readDesc.usages = { device::ResourceUsage::kCopyDestination };
	readDesc.memory = device::MemoryKind::kReadback;
	device::BufferDesc bloomReadDesc = readDesc;
	bloomReadDesc.size = std::uint64_t( kQuarterWidth ) * kQuarterHeight * 4;
	auto sceneTexture = device.CreateTexture( sceneDesc );
	auto target = device.CreateTexture( targetDesc );
	auto read = device.CreateBuffer( readDesc );
	auto bloomRead = device.CreateBuffer( bloomReadDesc );
	auto upload = device.CreateUploadBuffer( std::as_bytes( std::span<const float>( scene ) ) );
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !( sceneTexture && target && read && bloomRead && upload && encoder ) )
		std::printf(
		    "render.pass.post: setup failed (scene %d target %d read %d bloom %d upload %d "
		    "encoder %d)\n",
		    int( sceneTexture.HasValue() ), int( target.HasValue() ), int( read.HasValue() ),
		    int( bloomRead.HasValue() ), int( upload.HasValue() ), int( encoder.HasValue() ) );
	else
	{
		device::CommandEncoder &e = encoder.Value();
		e.TransitionBuffer( read.Value(), device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		e.TransitionBuffer( bloomRead.Value(), device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		e.TransitionTexture( sceneTexture.Value(), device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( upload.Value(), sceneTexture.Value(), { 0, 0, 0, kWidth, kHeight } );
		e.TransitionTexture( sceneTexture.Value(), device::ResourceUsage::kCopyDestination,
		    device::ResourceUsage::kSampled );
		device::TextureId bloom;
		bool recorded = true;
		if ( withBloom )
		{
			auto made = bloomRenderer.Value()->Record( e,
			    { sceneTexture.Value(), device::ResourceUsage::kSampled, kWidth, kHeight },
			    params );
			recorded = made.HasValue();
			if ( made )
				bloom = made.Value();
		}
		e.TransitionTexture( target.Value(), device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kColorAttachment );
		output::OutputDirectTargets direct;
		direct.scene = sceneTexture.Value();
		direct.sceneWidth = kWidth;
		direct.sceneHeight = kHeight;
		direct.target = target.Value();
		direct.width = kWidth;
		direct.height = kHeight;
		direct.bloom = bloom;
		output::OutputParams outputParams;
		outputParams.exposure = params.exposure;
		recorded = recorded && outputRenderer.Value()->Record( e, direct, outputParams ).HasValue();
		e.TransitionTexture( target.Value(), device::ResourceUsage::kColorAttachment,
		    device::ResourceUsage::kCopySource );
		e.CopyTextureToBuffer( target.Value(), read.Value(), { 0, 0, 0, kWidth, kHeight } );
		if ( bloom.IsValid() )
		{
			e.TransitionTexture(
			    bloom, device::ResourceUsage::kSampled, device::ResourceUsage::kCopySource );
			e.CopyTextureToBuffer(
			    bloom, bloomRead.Value(), { 0, 0, 0, kQuarterWidth, kQuarterHeight } );
			e.TransitionTexture(
			    bloom, device::ResourceUsage::kCopySource, device::ResourceUsage::kSampled );
		}
		device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
		auto submitted = device.Submit( device::QueueKind::kGraphics, encoders, {} );
		if ( !recorded || !submitted )
			std::printf( "render.pass.post: recorded %d submitted %d\n", int( recorded ),
			    int( submitted.HasValue() ) );
		if ( recorded && submitted && Wait( device, submitted.Value() ) )
		{
			bloomRenderer.Value()->Collect( submitted.Value() );
			outputRenderer.Value()->Collect( submitted.Value() );
			std::vector<std::byte> raw( readDesc.size ), rawBloom( bloomReadDesc.size );
			frame.ok =
			    device.ReadBuffer( read.Value(), 0, raw ).HasValue() &&
			    ( !withBloom || device.ReadBuffer( bloomRead.Value(), 0, rawBloom ).HasValue() );
			frame.output.resize( raw.size() );
			std::memcpy( frame.output.data(), raw.data(), raw.size() );
			frame.bloom.resize( rawBloom.size() );
			std::memcpy( frame.bloom.data(), rawBloom.data(), rawBloom.size() );
		}
	}
	for ( auto *resource : { &sceneTexture, &target } )
		if ( *resource )
			(void)device.Release( resource->Value(), device::CompletionToken() );
	for ( auto *resource : { &read, &bloomRead, &upload } )
		if ( *resource )
			(void)device.Release( resource->Value(), device::CompletionToken() );
	bloomRenderer.Value().reset();
	outputRenderer.Value().reset();
	(void)device.WaitIdle();
	(void)device.Poll();
	return frame;
}

// The largest per-channel difference in levels, RGB only.
int WorstBloom( const std::vector<std::uint8_t> &bytes, const Image &oracle )
{
	int worst = 0;
	for ( std::size_t i = 0; i < oracle.texels.size(); ++i )
		for ( int c = 0; c < 3; ++c )
			worst =
			    std::max( worst, std::abs( int( bytes[i * 4 + c] ) -
			                               int( std::lround( oracle.texels[i][c] * 255.0 ) ) ) );
	return worst;
}

int WorstOutput( const std::vector<std::uint8_t> &bytes, const std::vector<double> &oracle )
{
	int worst = 0;
	for ( std::size_t i = 0; i < oracle.size() / 3; ++i )
		for ( int c = 0; c < 3; ++c )
			worst = std::max( worst,
			    std::abs( int( bytes[i * 4 + c] ) - int( std::lround( oracle[i * 3 + c] ) ) ) );
	return worst;
}

post::BloomParams Params()
{
	post::BloomParams params;
	params.exposure = kExposure;
	std::copy( kTint, kTint + 4, params.tint );
	params.bloomAmount = kAmount;
	return params;
}

void Claims( testing::Checks &checks )
{
	using V = post::PostDrawVariable;
	const V tint[] = { { "$bloomtint", "[0.25 0.5 0.125 2]" } };
	auto down = post::ClaimPostDraw( "Downsample_nohdr", tint );
	checks.That( down && down.Value().role == post::PostRole::kDownsample &&
	                 down.Value().tint[0] == 0.25f && down.Value().tint[3] == 2.0f,
	    "C1.downsample-is-claimed-with-the-cvar-tint" );
	const V grey[] = { { "$bloomtintenable", "0" } };
	auto plain = post::ClaimPostDraw( "Downsample_nohdr", grey );
	checks.That( plain && plain.Value().tint[0] == 0.333f && plain.Value().tint[3] == 1.0f,
	    "C1.bloomtintenable-0-is-third-grey-exponent-1" );
	const V cstrike[] = { { "$cstrike", "1" } };
	checks.That(
	    !post::ClaimPostDraw( "Downsample_nohdr", cstrike ), "C1.cstrike-shape-is-refused" );
	checks.That( post::ClaimPostDraw( "BlurFilterX", {} ).HasValue(), "C1.blur-x-is-claimed" );
	const V amount[] = { { "$bloomamount", "0.625" } };
	auto y = post::ClaimPostDraw( "BlurFilterY", amount );
	checks.That( y && y.Value().role == post::PostRole::kBlurY && y.Value().bloomAmount == 0.625f,
	    "C1.blur-y-carries-bloomamount" );
	const V aa[] = { { "$AAINTERNAL1", "[1 0 1 0]" } };
	checks.That(
	    !post::ClaimPostDraw( "Engine_Post_dx9", aa ), "C1.engine-post-software-aa-is-refused" );
	const V noAa[] = { { "$AAINTERNAL1", "[0 0 0 0]" }, { "$bloomenable", "1" } };
	auto add = post::ClaimPostDraw( "Engine_Post_dx9", noAa );
	checks.That( add && add.Value().role == post::PostRole::kAdd && add.Value().bloomEnabled,
	    "C1.engine-post-is-the-add" );
	const V bloomadd[] = { { "$PIXSHADER", "bloomadd_ps20" } };
	auto p2 = post::ClaimPostDraw( "screenspace_general_dx9", bloomadd );
	checks.That( p2 && p2.Value().role == post::PostRole::kAdd, "C1.portal2-bloomadd-is-the-add" );
	const V other[] = { { "$PIXSHADER", "luminance_compare_ps20" } };
	checks.That( !post::ClaimPostDraw( "screenspace_general_dx9", other ),
	    "C1.another-screenspace-program-is-refused" );
	checks.That( !post::ClaimPostDraw( "VertexLitGeneric", {} ), "C1.a-surface-shader-is-refused" );
}

} // namespace

int main()
{
	testing::Checks checks;
	Claims( checks );
	std::atomic<std::uint64_t> messages{ 0 };
#if defined( RENDERTEST_POST_GL )
	namespace gl = render::device::gl;
	gl::GlAdapterOptions options;
	options.validation = true;
	options.validationCounter = &messages;
	auto created = gl::Create( options );
	const char *backend = "gl";
	const bool judged = true;
#else
	namespace vulkan = render::device::vulkan;
	const bool judged = vulkan::ValidationLayerAvailable();
	vulkan::VulkanAdapterOptions options;
	options.validation = judged;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	auto created = vulkan::Create( options );
	const char *backend = "vulkan";
#endif
	if ( !checks.That( created.HasValue(), "device.created" ) )
		return checks.Report();
	{
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		const std::vector<float> scene = MakeScene();
		const Image bloomOracle = OracleBloom( scene, false );
		const Image heightOracle = OracleBloom( scene, true );
		const std::vector<double> withOracle = OracleOutput( scene, &bloomOracle );
		const std::vector<double> plainOracle = OracleOutput( scene, nullptr );

		std::vector<std::uint8_t> heightBytes;
		for ( const Rgb &t : heightOracle.texels )
			for ( double c : { t[0], t[1], t[2], 1.0 } )
				heightBytes.push_back( std::uint8_t( std::lround( c * 255.0 ) ) );
		checks.That( WorstBloom( heightBytes, bloomOracle ) > 10,
		    "B2.the-height-step-differs-by-more-than-ten-levels" );

		const Frame frame = Run( *device, scene, true, {}, Params() );
		if ( checks.That( frame.ok, "B1.the-chain-runs" ) )
		{
			const int bloomError = WorstBloom( frame.bloom, bloomOracle );
			const int outputError = WorstOutput( frame.output, withOracle );
			std::printf( "render.pass.post (%s): bloom within %d levels, output within %d\n",
			    backend, bloomError, outputError );
			checks.That( bloomError <= 2, "B1.the-bloom-image-matches-the-oracle" );
			checks.That( outputError <= 2, "O1.the-output-adds-the-bloom-after-the-tone-map" );
			checks.That( WorstOutput( frame.output, plainOracle ) > 10, "O1.the-bloom-is-visible" );
		}
		const Frame plain = Run( *device, scene, false, {}, Params() );
		checks.That( plain.ok && WorstOutput( plain.output, plainOracle ) <= 1,
		    "O1.without-a-bloom-the-output-is-the-legacy-clip" );

		// P1: refusals.
		{
			auto renderer = post::BloomRenderer::Create( *device );
			auto encoder = device->BeginEncoder( device::QueueKind::kGraphics );
			if ( checks.That( renderer && encoder, "P1.setup" ) )
			{
				post::BloomParams bad = Params();
				bad.exposure = NAN;
				checks.That( renderer.Value()
				                     ->Record( encoder.Value(),
				                         { device::TextureId(), {}, kWidth, kHeight }, Params() )
				                     .Error() == post::PostStatus::kInvalidTarget,
				    "P1.an-empty-scene-is-refused" );
				post::BloomSource source{ device::TextureId(), {}, 0, 0 };
				checks.That( renderer.Value()->Record( encoder.Value(), source, bad ).Error() ==
				                 post::PostStatus::kInvalidTarget,
				    "P1.a-zero-extent-is-refused" );
				device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
				auto submitted = device->Submit( device::QueueKind::kGraphics, encoders, {} );
				if ( submitted )
					(void)Wait( *device, submitted.Value() );
				renderer.Value().reset();
			}
		}

#if !defined( RENDERTEST_POST_GL )
		// S1: the seeded programs (SPIR-V replacements) each fail.
		namespace seeded = rendertest::post::spirv;
		struct Seeded
		{
			const char *name;
			Programs programs;
			bool bloomCheck;
		};
		const Seeded defects[] = {
		    { "S1.shaping-linear-values-is-detected", { seeded::kPostLinearShape, {} }, true },
		    { "S1.the-vertical-step-by-height-is-detected", { seeded::kPostBlurYHeightStep, {} },
		        true },
		    { "S1.the-bloom-before-the-tone-map-is-detected",
		        { {}, seeded::kOutputBloomBeforeToneMap }, false } };
		for ( const Seeded &defect : defects )
		{
			const Frame bad = Run( *device, scene, true, defect.programs, Params() );
			const int error = !bad.ok             ? 255
			                  : defect.bloomCheck ? WorstBloom( bad.bloom, bloomOracle )
			                                      : WorstOutput( bad.output, withOracle );
			std::printf( "render.pass.post: %s -> %d levels\n", defect.name, error );
			checks.That( error > 2, defect.name );
		}
#endif
		(void)device->WaitIdle();
	}
	if ( judged )
		checks.That( messages.load() == 0, "validation.no-messages" );
	return checks.Report();
}
