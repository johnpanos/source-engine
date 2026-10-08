//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy.depth-alpha (RFC 0016 K9) on render.device.vulkan
//			with real pixels: render::legacy::DepthAlphaPass writes D3D9's
//			destination-alpha depth (WRITE_DEPTH_TO_DESTALPHA) into a frame
//			copy from the scene's depth, judged against a C++ reference that
//			shares no code with the GLSL.
//
//			A1 inside the copied rectangle the alpha is the reference's
//			   projected z over the range (within one level), for two depths;
//			A2 the colour channels are untouched everywhere, and alpha is
//			   untouched outside the rectangle;
//			A3 the two depths give different alphas (the check is not vacuous);
//			R1 an invalid range, a rectangle past the target and a missing
//			   depth are refused without recording;
//			S1 a seeded program that ignores the range fails A1;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "render/device/vulkan/provider.h"
#include "render/legacy/depth_alpha.h"
#include "spv/legacy_depth_alpha_defects_spv.h"
#include "testing/checks.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;

constexpr std::uint32_t kWidth = 64;
constexpr std::uint32_t kHeight = 32;
// The depth cleared over the whole target, and a rectangle cleared nearer.
constexpr float kFarDepth = 0.5f;
constexpr float kNearDepth = 0.9f;
constexpr std::uint32_t kNearX = 8, kNearY = 4, kNearW = 16, kNearH = 8;
// The copied rectangle: covers part of the near rectangle and the rest.
constexpr std::uint32_t kCopyX = 4, kCopyY = 2, kCopyW = 40, kCopyH = 20;
constexpr std::uint8_t kColor[4] = { 51, 102, 153, 255 };

// D3D's perspective z column (row vectors): near 4, far 4096.
constexpr float kNear = 4.0f, kFar = 4096.0f;
constexpr float kZScale = kFar / ( kFar - kNear );
constexpr float kZOffset = -kNear * kFar / ( kFar - kNear );
constexpr float kRange = 192.0f;

// The reference: D3D's depth is zScale + zOffset / view z, so the view z a
// depth came from is zOffset / ( depth - zScale ); then its projected z over
// the range.
int ReferenceAlpha( float depth )
{
	const double viewZ = double( kZOffset ) / ( double( depth ) - double( kZScale ) );
	const double projected = viewZ * kZScale + kZOffset;
	const double alpha = std::clamp( projected / kRange, 0.0, 1.0 );
	return int( std::lround( alpha * 255.0 ) );
}

legacy::DepthAlphaCopy Request( IRenderDevice2 &device, TextureId color, TextureId depth )
{
	legacy::DepthAlphaCopy copy;
	copy.device = &device;
	copy.target = color;
	copy.targetFormat = Format::kRGBA8Unorm;
	copy.targetWidth = kWidth;
	copy.targetHeight = kHeight;
	copy.x = kCopyX;
	copy.y = kCopyY;
	copy.width = kCopyW;
	copy.height = kCopyH;
	copy.depth = depth;
	copy.depthUsage = ResourceUsage::kDepthWrite;
	copy.projection[0] = kZScale;
	copy.projection[1] = kZOffset;
	copy.projection[2] = 1.0f;
	copy.projection[3] = 0.0f;
	copy.range = kRange;
	return copy;
}

struct Frame
{
	bool recorded = false;
	bool read = false;
	bool alphaInside = true;
	bool colorKept = true;
	bool alphaOutside = true;
	CompletionToken token;
};

// The fixture frame: depths by region clears, the colour cleared, the pass
// over the copy's rectangle, the target read back.
Frame RunFrame( IRenderDevice2 &device, legacy::DepthAlphaPass &pass, TextureId color,
    TextureId depth, BufferId readback, std::uint64_t readbackBytes )
{
	Frame frame;
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return frame;
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture( color, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	e.TransitionTexture( depth, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	ColorAttachment attachment;
	attachment.texture = color;
	attachment.clear = { kColor[0] / 255.0f, kColor[1] / 255.0f, kColor[2] / 255.0f, 1.0f };
	DepthAttachment depthAttachment;
	depthAttachment.texture = depth;
	depthAttachment.clearDepth = kFarDepth;
	RenderingDesc rendering;
	rendering.colors = { &attachment, 1 };
	rendering.depth = depthAttachment;
	rendering.width = kWidth;
	rendering.height = kHeight;
	e.BeginRendering( rendering );
	ClearRegion nearRegion;
	nearRegion.x = kNearX;
	nearRegion.y = kNearY;
	nearRegion.width = kNearW;
	nearRegion.height = kNearH;
	nearRegion.depth = true;
	nearRegion.depthValue = kNearDepth;
	e.ClearRegion( nearRegion );
	e.EndRendering();
	e.TransitionTexture( color, ResourceUsage::kColorAttachment, ResourceUsage::kCopyDestination );
	frame.recorded = pass.Record( e, Request( device, color, depth ) );
	e.TransitionTexture( color, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( color, readback, { 0, 0, 0, kWidth, kHeight } );
	auto submitted = device.Submit( QueueKind::kGraphics, { &e, 1 }, {} );
	std::vector<std::byte> raw( readbackBytes );
	frame.read = submitted.HasValue() && device.WaitIdle().HasValue() &&
	             device.ReadBuffer( readback, 0, raw ).HasValue();
	if ( submitted )
		frame.token = submitted.Value();
	if ( !frame.read )
		return frame;
	const int nearAlpha = ReferenceAlpha( kNearDepth );
	const int farAlpha = ReferenceAlpha( kFarDepth );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			const auto *px = reinterpret_cast<const std::uint8_t *>(
			    raw.data() + ( std::size_t( y ) * kWidth + x ) * 4 );
			for ( int c = 0; c < 3; ++c )
				frame.colorKept = frame.colorKept && px[c] == kColor[c];
			const bool copied =
			    x >= kCopyX && x < kCopyX + kCopyW && y >= kCopyY && y < kCopyY + kCopyH;
			const bool nearer =
			    x >= kNearX && x < kNearX + kNearW && y >= kNearY && y < kNearY + kNearH;
			if ( copied )
				frame.alphaInside = frame.alphaInside &&
				                    std::abs( int( px[3] ) - ( nearer ? nearAlpha : farAlpha ) ) <= 1;
			else
				frame.alphaOutside = frame.alphaOutside && px[3] == kColor[3];
		}
	return frame;
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
		auto pass = legacy::DepthAlphaPass::Create( *device );
		if ( !checks.That( pass != nullptr, "pass.is-created" ) )
			return checks.Report();

		TextureDesc colorDesc;
		colorDesc.format = Format::kRGBA8Unorm;
		colorDesc.width = kWidth;
		colorDesc.height = kHeight;
		colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopyDestination,
			ResourceUsage::kCopySource };
		TextureDesc depthDesc = colorDesc;
		depthDesc.format = Format::kD32Float;
		depthDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
		auto color = device->CreateTexture( colorDesc );
		auto depth = device->CreateTexture( depthDesc );
		BufferDesc readbackDesc;
		readbackDesc.size = std::uint64_t( kWidth ) * kHeight * 4;
		readbackDesc.usages = { ResourceUsage::kCopyDestination };
		readbackDesc.memory = MemoryKind::kReadback;
		auto readback = device->CreateBuffer( readbackDesc );
		if ( !checks.That( color && depth && readback, "fixture.resources" ) )
			return checks.Report();

		// R1: refusals record nothing.
		{
			auto encoder = device->BeginEncoder( QueueKind::kGraphics );
			legacy::DepthAlphaCopy bad = Request( *device, color.Value(), depth.Value() );
			bad.range = 0.0f;
			const bool zeroRange = !pass->Record( encoder.Value(), bad );
			bad = Request( *device, color.Value(), depth.Value() );
			bad.x = kWidth - 8;
			const bool past = !pass->Record( encoder.Value(), bad );
			bad = Request( *device, color.Value(), TextureId() );
			const bool noDepth = !pass->Record( encoder.Value(), bad );
			checks.That( zeroRange && past && noDepth,
			    "R1.invalid-range-rectangle-and-depth-are-refused" );
			auto submitted =
			    device->Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
			checks.That( submitted.HasValue(), "R1.the-empty-encoder-submits" );
		}

		const Frame frame = RunFrame( *device, *pass, color.Value(), depth.Value(),
		    readback.Value(), readbackDesc.size );
		checks.That( frame.recorded, "A1.the-pass-records" );
		if ( checks.That( frame.read, "fixture.frame-is-read-back" ) )
		{
			checks.That( frame.alphaInside, "A1.alpha-is-the-projected-depth-over-the-range" );
			checks.That( frame.colorKept, "A2.colour-is-untouched" );
			checks.That( frame.alphaOutside, "A2.alpha-outside-the-copy-is-untouched" );
		}
		const int nearAlpha = ReferenceAlpha( kNearDepth );
		const int farAlpha = ReferenceAlpha( kFarDepth );
		checks.That( std::abs( nearAlpha - farAlpha ) > 4, "A3.the-two-depths-differ" );
		std::printf( "depth alpha: near %d far %d of 255\n", nearAlpha, farAlpha );
		pass->Collect( frame.token );

		// S1: a seeded program that ignores the range is caught by A1.
		auto seeded = legacy::DepthAlphaPass::CreateWithFragment(
		    *device, rendertest::legacy::spirv::kDepthAlphaIgnoresRange );
		if ( checks.That( seeded != nullptr, "S1.the-seeded-pass-is-created" ) )
		{
			const Frame bad = RunFrame( *device, *seeded, color.Value(), depth.Value(),
			    readback.Value(), readbackDesc.size );
			checks.That( bad.recorded && bad.read && !bad.alphaInside,
			    "S1.a-pass-that-ignores-the-range-fails-A1" );
			seeded->Collect( bad.token );
			seeded.reset();
		}
		pass.reset();
		(void)device->Release( color.Value(), {} );
		(void)device->Release( depth.Value(), {} );
		(void)device->Release( readback.Value(), {} );
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.That( messages.load() == 0, "validation.no-messages" );
	else
		std::printf( "render.legacy.depth-alpha: Khronos validation layer not installed; not judged\n" );
	return checks.Report();
}
