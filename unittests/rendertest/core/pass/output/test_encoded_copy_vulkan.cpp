//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.output.encoded-copy (RFC 0016, R91 cutover part f) on
//			render.device.vulkan with real pixels: render::pass::output::
//			EncodedCopy draws a frame whose texels already hold the output
//			encoding (the core shader API's RGBA8 colour target) into a target
//			of another colour format, the BGRA8 back buffer a desktop
//			presentation shows, without decoding or encoding it again.
//
//			C1 every texel of a BGRA8 target read back equals the RGBA8
//			   source's red, green and blue (each channel distinct, so an
//			   order swap shows), with alpha 1;
//			C2 the same copy into an RGBA8 target equals the source;
//			C3 a source left in kCopySource and a target in kCopySource are
//			   returned to those usages (the frame reads back from them);
//			R1 an invalid request records nothing;
//			S1 a seeded program that swaps red and blue fails C1;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "render/device/vulkan/provider.h"
#include "render/pass/output/encoded_copy.h"
#include "spv/output_defects_spv.h"
#include "testing/checks.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using render::pass::output::EncodedCopy;
using render::pass::output::EncodedCopyTargets;

constexpr std::uint32_t kWidth = 48;
constexpr std::uint32_t kHeight = 24;

// A pattern whose channels differ at every texel and cover the byte range.
std::uint8_t Channel( std::uint32_t x, std::uint32_t y, int c )
{
	return std::uint8_t( ( x * 5 + y * 11 + std::uint32_t( c ) * 83 + 7 ) & 0xFF );
}

struct Result
{
	bool recorded = false;
	bool read = false;
	bool texels = true;
	bool alphaOne = true;
};

// Uploads the pattern into `source` (left in kCopySource), copies it into a
// target of `targetFormat` (left in kCopySource) and reads the target back.
Result Run( IRenderDevice2 &device, EncodedCopy &copy, Format targetFormat )
{
	Result result;
	TextureDesc sourceDesc;
	sourceDesc.format = Format::kRGBA8Unorm;
	sourceDesc.width = kWidth;
	sourceDesc.height = kHeight;
	sourceDesc.usages = { ResourceUsage::kSampled, ResourceUsage::kCopyDestination,
		ResourceUsage::kCopySource };
	TextureDesc targetDesc = sourceDesc;
	targetDesc.format = targetFormat;
	targetDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	BufferDesc uploadDesc;
	uploadDesc.size = std::uint64_t( kWidth ) * kHeight * 4;
	uploadDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	BufferDesc readbackDesc = uploadDesc;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	readbackDesc.memory = MemoryKind::kReadback;
	auto source = device.CreateTexture( sourceDesc );
	auto target = device.CreateTexture( targetDesc );
	auto upload = device.CreateBuffer( uploadDesc );
	auto readback = device.CreateBuffer( readbackDesc );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !source || !target || !upload || !readback || !encoder )
		return result;

	std::vector<std::byte> pattern( std::size_t( uploadDesc.size ) );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
			for ( int c = 0; c < 4; ++c )
				pattern[( std::size_t( y ) * kWidth + x ) * 4 + std::size_t( c )] =
				    std::byte( c == 3 ? 37 : Channel( x, y, c ) );
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( upload.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( upload.Value(), 0, pattern );
	e.TransitionBuffer( upload.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    source.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( upload.Value(), source.Value(), { 0, 0, 0, kWidth, kHeight, 0, 0 } );
	e.TransitionTexture(
	    source.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture( target.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopySource );

	EncodedCopyTargets targets;
	targets.source = source.Value();
	targets.sourceUsage = ResourceUsage::kCopySource;
	targets.target = target.Value();
	targets.targetFormat = targetFormat;
	targets.targetUsage = ResourceUsage::kCopySource;
	targets.width = kWidth;
	targets.height = kHeight;
	result.recorded = copy.Record( e, targets );

	e.TransitionBuffer(
	    readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( target.Value(), readback.Value(), { 0, 0, 0, kWidth, kHeight, 0, 0 } );
	auto submitted = device.Submit( QueueKind::kGraphics, { &e, 1 }, {} );
	std::vector<std::byte> raw( std::size_t( readbackDesc.size ) );
	result.read = submitted.HasValue() && device.WaitIdle().HasValue() &&
	              device.ReadBuffer( readback.Value(), 0, raw ).HasValue();
	if ( submitted )
		copy.Collect( submitted.Value() );
	const bool bgra = targetFormat == Format::kBGRA8Unorm;
	for ( std::uint32_t y = 0; result.read && y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			const auto *px = reinterpret_cast<const std::uint8_t *>(
			    raw.data() + ( std::size_t( y ) * kWidth + x ) * 4 );
			const std::uint8_t r = bgra ? px[2] : px[0];
			const std::uint8_t g = px[1];
			const std::uint8_t b = bgra ? px[0] : px[2];
			result.texels = result.texels && r == Channel( x, y, 0 ) && g == Channel( x, y, 1 ) &&
			                b == Channel( x, y, 2 );
			result.alphaOne = result.alphaOne && px[3] == 255;
		}
	const CompletionToken token = submitted ? submitted.Value() : CompletionToken{};
	(void)device.Release( source.Value(), token );
	(void)device.Release( target.Value(), token );
	(void)device.Release( upload.Value(), token );
	(void)device.Release( readback.Value(), token );
	return result;
}

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
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<IRenderDevice2> device = std::move( created ).Value();
		auto copy = EncodedCopy::Create( *device );
		if ( !checks.That( copy != nullptr, "copy.is-created" ) )
			return checks.Report();

		const Result bgra = Run( *device, *copy, Format::kBGRA8Unorm );
		checks.That( bgra.recorded, "C1.the-copy-records" );
		if ( checks.That( bgra.read, "C3.the-bgra-target-is-read-back" ) )
		{
			checks.That( bgra.texels, "C1.bgra-texels-equal-the-encoded-source" );
			checks.That( bgra.alphaOne, "C1.alpha-is-one" );
		}
		const Result rgba = Run( *device, *copy, Format::kRGBA8Unorm );
		if ( checks.That( rgba.read, "C3.the-rgba-target-is-read-back" ) )
			checks.That( rgba.texels && rgba.alphaOne, "C2.rgba-texels-equal-the-encoded-source" );

		// R1: refusals record nothing.
		{
			auto encoder = device->BeginEncoder( QueueKind::kGraphics );
			EncodedCopyTargets bad;
			bad.width = kWidth;
			bad.height = kHeight;
			bad.targetFormat = Format::kBGRA8Unorm;
			const bool noTextures = !copy->Record( encoder.Value(), bad );
			auto submitted = device->Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
			checks.That( noTextures && submitted.HasValue(),
			    "R1.an-invalid-request-records-nothing" );
		}

		// S1: a seeded program that swaps red and blue is caught by C1.
		auto seeded = EncodedCopy::CreateWithFragment(
		    *device, rendertest::output::spirv::kOutputEncodedCopySwapsRedBlue );
		if ( checks.That( seeded != nullptr, "S1.the-seeded-copy-is-created" ) )
		{
			const Result bad = Run( *device, *seeded, Format::kBGRA8Unorm );
			checks.That( bad.recorded && bad.read && !bad.texels,
			    "S1.a-copy-that-swaps-red-and-blue-fails-C1" );
			seeded.reset();
		}
		copy.reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.That( messages.load() == 0, "validation.no-messages" );
	else
		std::printf( "render.output.encoded-copy: Khronos validation layer not installed; not judged\n" );
	return checks.Report();
}
