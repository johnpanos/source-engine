//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.output (RFC 0016 "Output", render.output.v1) on
//			render.device.vulkan with real pixels: render.pass.output takes a
//			chart of known linear values (grays from 1e-4 to 64 times SDR
//			white, seeded colors, negatives) through exposure, the tone map
//			and the output encoding into each kind of target, and the results
//			are judged against the C++ reference (output_oracle.h), which
//			shares no code with the GLSL.
//
//			L1 the legacy point: on an 8-bit target at SDR (peak 1, headroom
//			   1) the pass is exactly the legacy clip and sRGB encoding, byte
//			   for byte the encoding alone applied to clamp(exposure * x, 0, 1);
//			L2 the 8-bit encodings are within one level of the sRGB oracle, and
//			   an sRGB-view target matches the shader's own encoding;
//			E1 extended range, headroom at least the peak: the clip alone
//			   (within one half-float step of clamp(exposure * x, 0, peak));
//			E2 extended range, headroom below the peak (1.2, 2, 4.5 against a
//			   peak of 16): the oracle's EETF within 0.3 percent, the peak
//			   lands on the headroom, a gray ramp stays monotonic, values below
//			   the knee pass unchanged and colors keep their channel ratios;
//			D1 a debug view (toneMap false) gets the encoding alone: linear
//			   values untouched on the half-float target, exposure ignored;
//			P1 invalid parameters, targets and extents are refused before any
//			   pass is added;
//			S1 six seeded fragment programs (compressing when the display
//			   covers the peak, per-channel mapping, the knee at the peak,
//			   the headroom ignored, a tone-mapped debug view, the sRGB curve
//			   on a linear target) are each detected by the checks above;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "output_oracle.h"
#include "render/device/vulkan/provider.h"
#include "render/graph/executor.h"
#include "render/pass/output/output.h"
#include "spv/output_defects_spv.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <random>
#include <span>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace
{

using namespace render;
using render::pass::output::OutputParams;
using render::pass::output::OutputRenderer;
using render::pass::output::OutputStatus;
using render::pass::output::OutputTargets;
using rendertest::output::Rgb;
namespace oracle = rendertest::output;

constexpr std::uint32_t kWidth = 64;
constexpr std::uint32_t kHeight = 32;
constexpr std::size_t kTexels = std::size_t( kWidth ) * kHeight;

// --- half floats (IEEE 754 binary16), written here, not shared ------------

float HalfBitsToFloat( std::uint16_t h )
{
	const int sign = ( h >> 15 ) & 1;
	const int exponent = ( h >> 10 ) & 31;
	const int mantissa = h & 1023;
	double value;
	if ( exponent == 0 )
		value = std::ldexp( double( mantissa ), -24 );
	else if ( exponent == 31 )
		value = mantissa ? NAN : INFINITY;
	else
		value = std::ldexp( double( mantissa | 1024 ), exponent - 25 );
	return float( sign ? -value : value );
}

// The binary16 value nearest `value` (ties to even, the default rounding
// mode of nearbyint), as bits; beyond the largest half, infinity.
std::uint16_t NearestHalfBits( float value )
{
	const std::uint16_t sign = std::signbit( value ) ? 0x8000 : 0;
	const double magnitude = std::fabs( double( value ) );
	if ( magnitude >= 65520.0 )
		return std::uint16_t( sign | 0x7c00 );
	if ( magnitude < std::ldexp( 1.0, -14 ) )
		return std::uint16_t( sign | std::uint16_t( std::nearbyint( magnitude * 16777216.0 ) ) );
	int exponent = 0;
	(void)std::frexp( magnitude, &exponent );
	int unbiased = exponent - 1;
	double mantissa = std::nearbyint( magnitude / std::ldexp( 1.0, unbiased - 10 ) );
	if ( mantissa >= 2048.0 )
	{
		mantissa = 1024.0;
		++unbiased;
	}
	if ( unbiased > 15 )
		return std::uint16_t( sign | 0x7c00 );
	return std::uint16_t(
	    sign | ( std::uint16_t( unbiased + 15 ) << 10 ) | std::uint16_t( mantissa - 1024.0 ) );
}

// Within one binary16 step of the nearest half to `expected` (the device may
// round a color attachment's conversion to nearest or toward zero).
bool WithinOneHalfStep( std::uint16_t actual, float expected )
{
	const std::uint16_t nearest = NearestHalfBits( expected );
	const int a = ( actual & 0x8000 ) ? -int( actual & 0x7fff ) : int( actual );
	const int n = ( nearest & 0x8000 ) ? -int( nearest & 0x7fff ) : int( nearest );
	return std::abs( a - n ) <= 1;
}

// --- the chart --------------------------------------------------------------

struct Chart
{
	std::vector<float> rgba; // kTexels x RGBA
	std::size_t grays = 0;   // the first `grays` texels are a rising gray ramp
	Rgb At( std::size_t i ) const { return { rgba[i * 4], rgba[i * 4 + 1], rgba[i * 4 + 2] }; }
};

Chart MakeChart( std::uint32_t seed )
{
	Chart chart;
	chart.rgba.assign( kTexels * 4, 1.0f );
	std::size_t i = 0;
	auto put = [&]( float r, float g, float b )
	{
		chart.rgba[i * 4 + 0] = r;
		chart.rgba[i * 4 + 1] = g;
		chart.rgba[i * 4 + 2] = b;
		++i;
	};
	// A rising gray ramp from 1e-4 to 64 times white, then exact landmarks.
	for ( int k = 0; k < 400; ++k )
	{
		const float v = float( 1e-4 * std::pow( 64.0 / 1e-4, k / 399.0 ) );
		put( v, v, v );
	}
	chart.grays = i;
	for ( float v : { 0.0f, 0.25f, 0.5f, 1.0f, 1.2f, 2.0f, 4.0f, 4.5f, 8.0f, 16.0f, 20.0f, 49.0f,
	          -1.0f, -0.001f } )
		put( v, v, v );
	std::mt19937 random( seed );
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	while ( i < kTexels )
	{
		const double magnitude = std::exp( std::log( 1e-3 ) + unit( random ) * std::log( 4e4 ) );
		double c[3] = { unit( random ), unit( random ), unit( random ) };
		if ( unit( random ) < 0.2 )
			c[int( unit( random ) * 3 ) % 3] = 0.0;
		if ( unit( random ) < 0.05 )
			c[int( unit( random ) * 3 ) % 3] = -0.5;
		const double peak = std::max( { c[0], c[1], c[2], 1e-6 } );
		put( float( c[0] / peak * magnitude ), float( c[1] / peak * magnitude ),
		    float( c[2] / peak * magnitude ) );
	}
	return chart;
}

// --- running the pass ---------------------------------------------------------

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

struct Result
{
	bool ok = false;
	bool refused = false;
	OutputStatus status = OutputStatus::kDevice;
	std::vector<std::uint8_t> bytes;   // 8-bit targets: RGBA bytes
	std::vector<std::uint16_t> halves; // half-float targets: RGBA halves
};

std::uint32_t TexelBytes( device::Format format )
{
	return format == device::Format::kRGBA16Float ? 8 : 4;
}

Result Run( device::IRenderDevice2 &device, OutputRenderer &renderer, device::Format format,
    const std::vector<float> &chart, const OutputParams &params,
    std::uint32_t targetWidth = kWidth )
{
	Result result;
	device::TextureDesc sceneDesc;
	sceneDesc.format = device::Format::kRGBA32Float;
	sceneDesc.width = kWidth;
	sceneDesc.height = kHeight;
	sceneDesc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	device::TextureDesc targetDesc;
	targetDesc.format = format;
	targetDesc.width = targetWidth;
	targetDesc.height = kHeight;
	targetDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	device::BufferDesc readbackDesc;
	readbackDesc.size = std::uint64_t( targetWidth ) * kHeight * TexelBytes( format );
	readbackDesc.usages = { device::ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	auto scene = device.CreateTexture( sceneDesc );
	auto target = device.CreateTexture( targetDesc );
	auto readback = device.CreateBuffer( readbackDesc );
	if ( !scene || !target || !readback )
		return result;

	graph::GraphBuilder builder;
	const graph::ResourceRef sceneRef = builder.ImportTexture( "scene", scene.Value(), sceneDesc,
	    device::ResourceUsage::kUndefined, device::ResourceUsage::kSampled );
	const graph::ResourceRef targetRef = builder.ImportTexture( "target", target.Value(),
	    targetDesc, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopySource );
	const graph::ResourceRef readbackRef = builder.ImportBuffer( "readback", readback.Value(),
	    readbackDesc, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
	device::BufferDesc stagingDesc;
	stagingDesc.size = chart.size() * sizeof( float );
	const graph::ResourceRef staging = builder.CreateBuffer( "chart", stagingDesc );
	const std::vector<float> *bytes = &chart;
	builder.AddPass( "chart-upload", graph::PassKind::kCopy )
	    .Write( staging, device::ResourceUsage::kCopyDestination )
	    .Execute(
	        [bytes, staging]( graph::RecordContext &context )
	        {
		        context.Encoder().WriteBuffer( context.Buffer( staging ), 0,
		            std::as_bytes( std::span<const float>( *bytes ) ) );
	        } );
	builder.AddPass( "chart-copy", graph::PassKind::kCopy )
	    .Read( staging, device::ResourceUsage::kCopySource )
	    .Write( sceneRef, device::ResourceUsage::kCopyDestination )
	    .Execute(
	        [staging, sceneRef]( graph::RecordContext &context )
	        {
		        context.Encoder().CopyBufferToTexture( context.Buffer( staging ),
		            context.Texture( sceneRef ), { 0, 0, 0, kWidth, kHeight } );
	        } );
	OutputTargets targets;
	targets.scene = sceneRef;
	targets.target = targetRef;
	targets.width = kWidth;
	targets.height = kHeight;
	auto added = renderer.AddPass( builder, targets, params );
	if ( !added )
	{
		result.refused = true;
		result.status = added.Error();
	}
	else
	{
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( targetRef, device::ResourceUsage::kCopySource )
		    .Write( readbackRef, device::ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [targetRef, readbackRef, targetWidth]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyTextureToBuffer( context.Texture( targetRef ),
			            context.Buffer( readbackRef ), { 0, 0, 0, targetWidth, kHeight } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			if ( executed && Wait( device, executed.Value().token ) )
			{
				renderer.Collect( executed.Value().token );
				std::vector<std::byte> raw( readbackDesc.size );
				result.ok = device.ReadBuffer( readback.Value(), 0, raw ).HasValue() &&
				            renderer.RecordFailures() == 0;
				if ( format == device::Format::kRGBA16Float )
				{
					result.halves.resize( raw.size() / 2 );
					std::memcpy( result.halves.data(), raw.data(), raw.size() );
				}
				else
				{
					result.bytes.resize( raw.size() );
					std::memcpy( result.bytes.data(), raw.data(), raw.size() );
				}
			}
		}
	}
	(void)device.Release( scene.Value(), device::CompletionToken() );
	(void)device.Release( target.Value(), device::CompletionToken() );
	(void)device.Release( readback.Value(), device::CompletionToken() );
	(void)device.Poll();
	return result;
}

OutputParams Params( float exposure, float peak, float headroom, bool toneMap = true )
{
	OutputParams params;
	params.exposure = exposure;
	params.scenePeak = peak;
	params.headroom = headroom;
	params.toneMap = toneMap;
	return params;
}

// Makes a renderer for a target format: the real pass or a seeded one.
using Factory = std::function<std::unique_ptr<OutputRenderer>( device::Format )>;

// --- the judged properties ----------------------------------------------------

struct Battery
{
	device::IRenderDevice2 &device;
	const Chart &chart;
	Factory make;
	// name -> passed, in order.
	std::vector<std::pair<std::string, bool>> results;

	void Record( const std::string &name, bool passed ) { results.emplace_back( name, passed ); }

	void Legacy()
	{
		auto unorm = make( device::Format::kRGBA8Unorm );
		auto srgbView = make( device::Format::kRGBA8Srgb );
		if ( !unorm || !srgbView )
		{
			Record( "L1.renderers", false );
			return;
		}
		bool bitwise = true;
		bool oracle = true;
		for ( float exposure : { 1.0f, 0.5f, 1.37f } )
		{
			const Result mapped = Run( device, *unorm, device::Format::kRGBA8Unorm, chart.rgba,
			    Params( exposure, 1.0f, 1.0f ) );
			// The legacy chain: the clip, then the encoding alone.
			std::vector<float> clipped = chart.rgba;
			for ( std::size_t i = 0; i < kTexels; ++i )
				for ( int c = 0; c < 3; ++c )
					clipped[i * 4 + c] =
					    std::min( std::max( chart.rgba[i * 4 + c] * exposure, 0.0f ), 1.0f );
			const Result legacy = Run( device, *unorm, device::Format::kRGBA8Unorm, clipped,
			    Params( 1.0f, 1.0f, 1.0f, false ) );
			if ( !mapped.ok || !legacy.ok || mapped.bytes != legacy.bytes )
				bitwise = false;
			for ( std::size_t i = 0; mapped.ok && i < kTexels; ++i )
			{
				const Rgb in = chart.At( i );
				for ( int c = 0; c < 3; ++c )
				{
					const double expected =
					    255.0 * oracle::SrgbEncode( std::clamp( in[c] * exposure, 0.0, 1.0 ) );
					if ( std::fabs( mapped.bytes[i * 4 + c] - expected ) > 1.0 )
						oracle = false;
				}
			}
		}
		Record( "L1.sdr-is-the-legacy-clip-and-srgb-bitwise", bitwise );
		Record( "L2.sdr-srgb-within-one-level-of-the-oracle", oracle );
		const Result shader = Run(
		    device, *unorm, device::Format::kRGBA8Unorm, chart.rgba, Params( 1.0f, 1.0f, 1.0f ) );
		const Result hardware = Run(
		    device, *srgbView, device::Format::kRGBA8Srgb, chart.rgba, Params( 1.0f, 1.0f, 1.0f ) );
		bool same = shader.ok && hardware.ok && shader.bytes.size() == hardware.bytes.size();
		for ( std::size_t i = 0; same && i < shader.bytes.size(); ++i )
			if ( ( i & 3 ) != 3 &&
			     std::abs( int( shader.bytes[i] ) - int( hardware.bytes[i] ) ) > 1 )
				same = false;
		Record( "L2.an-srgb-view-target-matches-the-shader-encoding", same );
	}

	void Extended()
	{
		auto half = make( device::Format::kRGBA16Float );
		if ( !half )
		{
			Record( "E.renderer", false );
			return;
		}
		// E1: the display covers the peak.
		bool clip = true;
		for ( const auto &[exposure, peak, headroom] : { std::tuple{ 1.0f, 4.0f, 8.0f },
		          std::tuple{ 2.0f, 4.0f, 4.0f }, std::tuple{ 1.0f, 1.0f, 1.2f } } )
		{
			const Result r = Run( device, *half, device::Format::kRGBA16Float, chart.rgba,
			    Params( exposure, peak, headroom ) );
			for ( std::size_t i = 0; r.ok && i < kTexels; ++i )
				for ( int c = 0; c < 3; ++c )
				{
					const float expected =
					    std::min( std::max( chart.rgba[i * 4 + c] * exposure, 0.0f ), peak );
					if ( !WithinOneHalfStep( r.halves[i * 4 + c], expected ) )
						clip = false;
				}
			clip = clip && r.ok;
		}
		Record( "E1.the-clip-alone-when-the-display-covers-the-peak", clip );

		// E2: compression below the peak.
		bool matches = true, peakLands = true, monotonic = true, belowKnee = true, hue = true;
		const float peak = 16.0f;
		for ( float headroom : { 1.2f, 2.0f, 4.5f } )
		{
			const Result r = Run( device, *half, device::Format::kRGBA16Float, chart.rgba,
			    Params( 1.0f, peak, headroom ) );
			if ( !r.ok )
			{
				matches = peakLands = monotonic = belowKnee = hue = false;
				continue;
			}
			const double knee = oracle::KneeValue( peak, headroom );
			float previous = -1.0f;
			for ( std::size_t i = 0; i < kTexels; ++i )
			{
				const Rgb in = chart.At( i );
				const Rgb expected = oracle::ToneMap( in, 1.0, peak, headroom );
				Rgb actual;
				for ( int c = 0; c < 3; ++c )
					actual[c] = HalfBitsToFloat( r.halves[i * 4 + c] );
				for ( int c = 0; c < 3; ++c )
				{
					const double tolerance =
					    3e-3 * std::max( 1.0, expected[c] ) + 1e-3 * expected[c];
					if ( std::fabs( actual[c] - expected[c] ) > tolerance )
						matches = false;
				}
				const double clippedPeak = std::max( { std::clamp( in[0], 0.0, double( peak ) ),
				    std::clamp( in[1], 0.0, double( peak ) ),
				    std::clamp( in[2], 0.0, double( peak ) ) } );
				if ( clippedPeak >= peak &&
				     std::fabs( std::max( { actual[0], actual[1], actual[2] } ) - headroom ) >
				         3e-3 * headroom )
					peakLands = false;
				if ( clippedPeak > 0.0 && clippedPeak < knee * 0.999 )
					for ( int c = 0; c < 3; ++c )
						if ( !WithinOneHalfStep( r.halves[i * 4 + c],
						         float( std::clamp( in[c], 0.0, double( peak ) ) ) ) )
							belowKnee = false;
				if ( i < chart.grays )
				{
					if ( actual[0] < previous )
						monotonic = false;
					previous = float( actual[0] );
				}
				// Channel ratios against the brightest channel, where it is
				// resolvable in half precision.
				const int top =
				    int( std::max_element( actual.begin(), actual.end() ) - actual.begin() );
				if ( clippedPeak > knee && actual[top] > 0.05 )
					for ( int c = 0; c < 3; ++c )
					{
						const double inRatio =
						    std::clamp( in[c], 0.0, double( peak ) ) / clippedPeak;
						const double outRatio = actual[c] / actual[top];
						if ( std::fabs( inRatio - outRatio ) > 4e-3 )
							hue = false;
					}
			}
		}
		Record( "E2.compression-matches-the-bt2390-oracle", matches );
		Record( "E2.the-scene-peak-lands-on-the-headroom", peakLands );
		Record( "E2.a-gray-ramp-stays-monotonic", monotonic );
		Record( "E2.values-below-the-knee-pass-unchanged", belowKnee );
		Record( "E2.colors-keep-their-channel-ratios", hue );
	}

	void DebugView()
	{
		auto half = make( device::Format::kRGBA16Float );
		auto unorm = make( device::Format::kRGBA8Unorm );
		if ( !half || !unorm )
		{
			Record( "D1.renderers", false );
			return;
		}
		const Result r = Run( device, *half, device::Format::kRGBA16Float, chart.rgba,
		    Params( 3.0f, 1.0f, 1.2f, false ) );
		bool untouched = r.ok;
		for ( std::size_t i = 0; r.ok && i < kTexels; ++i )
			for ( int c = 0; c < 3; ++c )
				if ( !WithinOneHalfStep(
				         r.halves[i * 4 + c], std::max( chart.rgba[i * 4 + c], 0.0f ) ) )
					untouched = false;
		Record( "D1.a-debug-view-reaches-a-linear-target-untouched", untouched );
		const Result a = Run( device, *unorm, device::Format::kRGBA8Unorm, chart.rgba,
		    Params( 1.0f, 1.0f, 1.0f, false ) );
		const Result b = Run( device, *unorm, device::Format::kRGBA8Unorm, chart.rgba,
		    Params( 4.0f, 1.0f, 1.0f, false ) );
		Record( "D1.a-debug-view-ignores-exposure", a.ok && b.ok && a.bytes == b.bytes );
	}

	bool AllPassed() const
	{
		for ( const auto &[name, passed] : results )
			if ( !passed )
				return false;
		return true;
	}
};

} // namespace

int main()
{
	testing::Checks checks;
	namespace vulkan = render::device::vulkan;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	std::uint32_t seed = 1;
	if ( const char *text = std::getenv( "CONFORMANCE_SEED" ) )
		seed = std::uint32_t( std::strtoul( text, nullptr, 10 ) );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<device::IRenderDevice2> device = std::move( created ).Value();
		const Chart chart = MakeChart( seed );

		Factory real = [&]( device::Format format ) -> std::unique_ptr<OutputRenderer>
		{
			auto made = OutputRenderer::Create( *device, format );
			return made ? std::move( made ).Value() : nullptr;
		};
		Battery battery{ *device, chart, real, {} };
		battery.Legacy();
		battery.Extended();
		battery.DebugView();
		for ( const auto &[name, passed] : battery.results )
			checks.That( passed, name.c_str() );

		// P1: refusals before any pass.
		auto half = real( device::Format::kRGBA16Float );
		auto unorm = real( device::Format::kRGBA8Unorm );
		auto refused = [&]( OutputRenderer &renderer, device::Format format,
		                   const OutputParams &params, OutputStatus status,
		                   std::uint32_t width = kWidth )
		{
			const Result r = Run( *device, renderer, format, chart.rgba, params, width );
			return r.refused && r.status == status;
		};
		checks.That( half && unorm, "P1.renderers" );
		if ( half && unorm )
		{
			checks.That( refused( *half, device::Format::kRGBA16Float, Params( 1, 4, 0.5f ),
			                 OutputStatus::kInvalidParams ),
			    "P1.a-headroom-below-one-is-refused" );
			checks.That( refused( *unorm, device::Format::kRGBA8Unorm, Params( 1, 4, 2.0f ),
			                 OutputStatus::kInvalidParams ),
			    "P1.an-8-bit-target-refuses-headroom-above-one" );
			checks.That( refused( *half, device::Format::kRGBA16Float, Params( 1, 0.0f, 1 ),
			                 OutputStatus::kInvalidParams ) &&
			                 refused( *half, device::Format::kRGBA16Float, Params( 1, 60.0f, 1 ),
			                     OutputStatus::kInvalidParams ),
			    "P1.a-scene-peak-outside-pq-is-refused" );
			checks.That( refused( *half, device::Format::kRGBA16Float, Params( NAN, 4, 1 ),
			                 OutputStatus::kInvalidParams ),
			    "P1.a-non-finite-exposure-is-refused" );
			checks.That( refused( *half, device::Format::kRGBA16Float, Params( 1, 4, 1 ),
			                 OutputStatus::kSizeMismatch, kWidth / 2 ),
			    "P1.a-target-of-another-extent-is-refused" );
			checks.That( refused( *half, device::Format::kRGBA8Unorm, Params( 1, 1, 1 ),
			                 OutputStatus::kInvalidTarget ),
			    "P1.a-target-of-another-format-is-refused" );
		}
		auto unsupported = OutputRenderer::Create( *device, device::Format::kR32Float );
		checks.That( !unsupported && unsupported.Error() == OutputStatus::kInvalidTarget,
		    "P1.a-target-format-without-an-encoding-is-refused" );

		// S1: each seeded program fails the battery.
		struct Seeded
		{
			const char *name;
			std::span<const std::uint32_t> code;
		};
		const Seeded seeded[] = { { "S1.compressing-when-the-display-covers-the-peak-is-detected",
		                              rendertest::output::spirv::kOutputAlwaysCompress },
		    { "S1.per-channel-mapping-is-detected", rendertest::output::spirv::kOutputPerChannel },
		    { "S1.the-knee-at-the-peak-is-detected", rendertest::output::spirv::kOutputKneeAtPeak },
		    { "S1.an-ignored-headroom-is-detected",
		        rendertest::output::spirv::kOutputHeadroomIgnored },
		    { "S1.a-tone-mapped-debug-view-is-detected",
		        rendertest::output::spirv::kOutputDebugViewToneMapped },
		    { "S1.the-srgb-curve-on-a-linear-target-is-detected",
		        rendertest::output::spirv::kOutputSrgbOnLinear } };
		for ( const Seeded &defect : seeded )
		{
			Factory bad = [&]( device::Format format ) -> std::unique_ptr<OutputRenderer>
			{
				auto made = OutputRenderer::CreateWithFragment( *device, format, defect.code );
				return made ? std::move( made ).Value() : nullptr;
			};
			Battery probe{ *device, chart, bad, {} };
			probe.Legacy();
			probe.Extended();
			probe.DebugView();
			std::string failed;
			for ( const auto &[name, passed] : probe.results )
				if ( !passed )
					failed += ( failed.empty() ? "" : ", " ) + name;
			std::printf( "render.output: %s -> %s\n", defect.name,
			    failed.empty() ? "(nothing failed)" : failed.c_str() );
			checks.That( !probe.AllPassed(), defect.name );
		}
		half.reset();
		unorm.reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.That( messages.load() == 0, "validation.no-messages" );
	else
		std::printf( "render.output: Khronos validation layer not installed; not judged\n" );
	return checks.Report();
}
