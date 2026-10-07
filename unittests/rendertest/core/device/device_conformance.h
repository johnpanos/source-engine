//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared render.device.v2 suite (RFC 0016). One suite runs
//			against every adapter that claims the port (null, Vulkan, later
//			OpenGL) and against the deliberately bad adapters of
//			test_device_negative.cpp, which must each fail the clause they
//			break. Contract: unittests/rendertest/contracts/render.device.v2.md.
//
//			Check names are "<driver>.<clause> <what>", so a sensitivity run can
//			find the clause a broken adapter failed.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_DEVICE_CONFORMANCE_H
#define RENDERTEST_CORE_DEVICE_CONFORMANCE_H

#include "render/device/device.h"
#include "testing/checks.h"
#include "test_shaders.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <vector>

#if defined( __linux__ )
#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif

namespace rendertest
{

using namespace render::device;

struct DeviceDriver
{
	std::string name;
	std::function<std::unique_ptr<IRenderDevice2>()> create;
	// Makes token complete: a manual adapter runs its work, a real one waits.
	// False if it did not complete in time.
	std::function<bool( IRenderDevice2 &, CompletionToken )> complete;
	// Optional: forces a device loss. Without it the loss clause is skipped
	// (and reported as such), never passed.
	std::function<void( IRenderDevice2 & )> lose;
	// Tokens stay incomplete until complete() (manual adapters), so ordering
	// before completion is observable.
	bool holdsCompletion = false;
	// Optional: holds the queue (true), so work submitted from then on does
	// not start until it is released (false): the latest schedule a token
	// allows. D10 records its uploads while held, so a range handed out again
	// before its token completes is overwritten before anything reads it,
	// and D5 checks a released resource while its token is still pending,
	// however fast the driver runs. Unset: both rely on the queue being
	// slower than the recording (a manual adapter holds by construction).
	std::function<void( IRenderDevice2 &, bool )> hold;
	// Draws produce pixels, so the conventions section runs.
	bool rasterizes = false;
	// D15: a compute fixture (layout( local_size_x = 64 ), set 3 binding 0 a
	// storage buffer of uints, values[i] = values[i] * 2 + 1), so claimed
	// compute capabilities can be exercised; its SPIR-V, mapped through
	// `artifact` like every fixture.
	std::span<const std::uint32_t> doubleCompute;
	// The suite's fixtures (test_shaders.h, SPIR-V) in the adapter's artifact
	// format: given a fixture's SPIR-V, its artifact (the OpenGL adapter's
	// GLSL 4.50, from the generated spv/device_fixtures_glsl.h). Unset: the
	// adapter takes the SPIR-V itself.
	std::function<std::span<const std::byte>( std::span<const std::uint32_t> spirv )> artifact;
};

namespace detail
{

inline std::span<const std::byte> Code( const std::uint32_t *words, std::size_t count )
{
	return { reinterpret_cast<const std::byte *>( words ), count * sizeof( std::uint32_t ) };
}

template <std::size_t N> std::span<const std::byte> Code( const std::uint32_t ( &words )[N] )
{
	return Code( words, N );
}

inline std::vector<std::byte> Pattern( std::size_t size, std::uint8_t seed )
{
	std::vector<std::byte> bytes( size );
	for ( std::size_t i = 0; i < size; ++i )
		bytes[i] = static_cast<std::byte>( ( i * 31u + seed * 7u + ( i >> 8 ) ) & 0xffu );
	return bytes;
}

class Suite
{
public:
	Suite( testing::Checks &checks, const DeviceDriver &driver )
	    : m_Checks( checks ), m_Driver( driver )
	{
	}

	// A fixture in the adapter's artifact format (DeviceDriver::artifact).
	std::span<const std::byte> Code( std::span<const std::uint32_t> spirv ) const
	{
		if ( m_Driver.artifact )
			return m_Driver.artifact( spirv );
		return detail::Code( spirv.data(), spirv.size() );
	}

	bool That( bool condition, const char *clause, const char *what )
	{
		return m_Checks.That( condition, m_Driver.name + "." + clause + " " + what );
	}

	std::unique_ptr<IRenderDevice2> Create()
	{
		std::unique_ptr<IRenderDevice2> device = m_Driver.create();
		That( device != nullptr, "setup", "the adapter creates a device" );
		return device;
	}

	BufferId Buffer( IRenderDevice2 &device, std::uint64_t size, UsageSet usages,
	    MemoryKind memory = MemoryKind::kDeviceLocal )
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = usages;
		desc.memory = memory;
		auto created = device.CreateBuffer( desc );
		That( created.HasValue(), "setup", "a valid buffer is created" );
		return created ? created.Value() : BufferId{};
	}

	std::optional<CompletionToken> Run( IRenderDevice2 &device, CommandEncoder &encoder,
	    std::span<const CompletionToken> waits = {} )
	{
		auto token = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, SubmitWaits{ waits } );
		if ( !token )
			return std::nullopt;
		return token.Value();
	}

	bool Finish( IRenderDevice2 &device, CompletionToken token )
	{
		return m_Driver.complete( device, token );
	}

	// Copies size bytes of buffer (in kCopySource) to a readback buffer and
	// reads them after completion.
	std::vector<std::byte> ReadBack( IRenderDevice2 &device, BufferId buffer, std::uint64_t size )
	{
		std::vector<std::byte> out( size );
		const BufferId readback =
		    Buffer( device, size, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return {};
		encoder.Value().TransitionBuffer(
		    readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.Value().CopyBuffer( buffer, readback, { 0, 0, size } );
		const std::optional<CompletionToken> token = Run( device, encoder.Value() );
		if ( !token || !Finish( device, *token ) || !device.ReadBuffer( readback, 0, out ) )
			out.clear();
		(void)device.Release( readback, token.value_or( CompletionToken{} ) );
		return out;
	}

	testing::Checks &m_Checks;
	const DeviceDriver &m_Driver;
};

inline void Facts( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const DeviceFacts before = device->Facts();
	const DeviceFacts *address = &device->Facts();
	s.That(
	    before.limits.maxBindGroups == kMaxBindGroups, "D1", "reports exactly four bind groups" );
	s.That( ( before.limits.sampleCounts & 1u ) != 0, "D1", "supports single-sample targets" );
	s.That( !before.diagnosticBackend.empty(), "D1", "names its diagnostic backend" );
	s.That( before.limits.maxColorAttachments >= 1 && before.limits.maxTextureDimension2D >= 1024,
	    "D1", "reports usable limits" );
	(void)s.Buffer( *device, 256, { ResourceUsage::kCopyDestination } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( encoder )
	{
		const std::optional<CompletionToken> token = s.Run( *device, encoder.Value() );
		if ( token )
			(void)s.Finish( *device, *token );
	}
	(void)device->Poll();
	s.That( device->Facts() == before && &device->Facts() == address, "D1",
	    "facts are immutable after creation and use" );
}

inline void InvalidDescriptions( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const std::size_t live = device->LiveResourceCount();
	auto invalid = [&]( const auto &result, DeviceOperation operation )
	{
		return !result && result.Error().status == DeviceStatus::kInvalidDescription &&
		       result.Error().operation == operation;
	};

	BufferDesc empty;
	empty.usages = { ResourceUsage::kVertex };
	s.That( invalid( device->CreateBuffer( empty ), DeviceOperation::kCreateBuffer ), "D2",
	    "a zero-size buffer fails with kInvalidDescription" );
	BufferDesc noUsage;
	noUsage.size = 64;
	s.That( invalid( device->CreateBuffer( noUsage ), DeviceOperation::kCreateBuffer ), "D2",
	    "a buffer without usages fails" );

	TextureDesc texture;
	texture.width = 4;
	texture.height = 4;
	texture.usages = { ResourceUsage::kSampled };
	s.That( invalid( device->CreateTexture( texture ), DeviceOperation::kCreateTexture ), "D2",
	    "a texture without a format fails" );
	texture.format = Format::kRGBA8Unorm;
	texture.mipLevels = 4;
	s.That( invalid( device->CreateTexture( texture ), DeviceOperation::kCreateTexture ), "D2",
	    "more mips than the extent allows fails" );
	texture.mipLevels = 1;
	texture.dimension = TextureDimension::kCube;
	texture.depthOrLayers = 4;
	s.That( invalid( device->CreateTexture( texture ), DeviceOperation::kCreateTexture ), "D2",
	    "a cube whose layers are not a multiple of six fails" );

	const BindingDesc duplicate[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex } },
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } } };
	s.That( invalid( device->CreateBindGroupLayout( { BindGroupRole::kDraw, duplicate } ),
	            DeviceOperation::kCreateBindGroupLayout ),
	    "D2", "a layout that declares one binding twice fails" );

	s.That( device->LiveResourceCount() == live, "D2", "failed creations leave nothing live" );
}

inline void BindGroupLimit( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	std::vector<BindGroupLayoutId> layouts;
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( layout )
			layouts.push_back( layout.Value() );
	}
	s.That( layouts.size() == kMaxBindGroups, "D3", "one empty layout per role is created" );
	layouts.push_back( layouts.empty() ? BindGroupLayoutId{} : layouts.back() );
	const std::size_t live = device->LiveResourceCount();

	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	    s.Code( shaders::kFullScreenVertex ), "main", {} } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	auto fifth = device->CreatePipeline( desc );
	s.That( !fifth && fifth.Error().status == DeviceStatus::kTooManyBindGroups, "D3",
	    "a fifth bind group fails with kTooManyBindGroups" );
	s.That(
	    device->LiveResourceCount() == live, "D3", "the rejected pipeline leaves nothing live" );
	desc.layouts = std::span<const BindGroupLayoutId>( layouts.data(), kMaxBindGroups );
	s.That( device->CreatePipeline( desc ).HasValue(), "D3", "four bind groups are accepted" );
}

struct ColorPipeline
{
	BindGroupLayoutId layouts[kMaxBindGroups];
	PipelineId pipeline;
	bool ok = false;
};

inline ColorPipeline MakeColorPipeline(
    const Suite &s, IRenderDevice2 &device, std::span<const std::uint32_t> vertex, bool depth )
{
	ColorPipeline result;
	static const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		const BindGroupRole r = static_cast<BindGroupRole>( role );
		auto layout = device.CreateBindGroupLayout(
		    { r, r == BindGroupRole::kMaterial ? std::span<const BindingDesc>( material )
		                                       : std::span<const BindingDesc>() } );
		if ( !layout )
			return result;
		result.layouts[role] = layout.Value();
	}
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, device.Facts().artifactFormat, s.Code( vertex ), "main", {} },
	    { ShaderStage::kFragment, device.Facts().artifactFormat, s.Code( shaders::kColorFragment ),
	        "main", used } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = result.layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	if ( depth )
	{
		desc.depthFormat = Format::kD32Float;
		desc.depthStencil = { true, true, CompareOp::kLess };
	}
	auto pipeline = device.CreatePipeline( desc );
	if ( pipeline )
	{
		result.pipeline = pipeline.Value();
		result.ok = true;
	}
	return result;
}

inline void PipelineLayouts( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const ColorPipeline good = MakeColorPipeline( s, *device, shaders::kFullScreenVertex, false );
	s.That( good.ok, "D4", "a pipeline whose reflected bindings are in its layouts is created" );

	const std::size_t live = device->LiveResourceCount();
	auto frame = device->CreateBindGroupLayout( { BindGroupRole::kFrame, {} } );
	const BindGroupLayoutId onlyFrame[] = { frame ? frame.Value() : BindGroupLayoutId{} };
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          s.Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat, s.Code( shaders::kColorFragment ),
	        "main", used } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = onlyFrame;
	desc.colorFormats = colors;
	auto mismatch = device->CreatePipeline( desc );
	s.That( !mismatch && mismatch.Error().status == DeviceStatus::kLayoutMismatch, "D4",
	    "a reflected binding missing from the layouts fails with kLayoutMismatch" );

	const ArtifactFormat foreign = device->Facts().artifactFormat == ArtifactFormat::kSpirv
	                                   ? ArtifactFormat::kGlsl450
	                                   : ArtifactFormat::kSpirv;
	const ShaderArtifactView wrong[] = {
	    { ShaderStage::kVertex, foreign, Code( shaders::kFullScreenVertex ), "main", {} } };
	desc.stages = wrong;
	auto format = device->CreatePipeline( desc );
	s.That( !format && format.Error().status == DeviceStatus::kUnsupported, "D4",
	    "an artifact in a format the device does not accept fails with kUnsupported" );
	s.That(
	    device->LiveResourceCount() == live + 1, "D4", "rejected pipelines leave nothing live" );
}

inline void Release( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const BufferId buffer = s.Buffer( *device, 256, { ResourceUsage::kCopyDestination } );
	const std::size_t live = device->LiveResourceCount();
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	const std::vector<std::byte> bytes = Pattern( 256, 1 );
	encoder.Value().TransitionBuffer(
	    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.Value().WriteBuffer( buffer, 0, bytes );
	// With the queue held the token is still incomplete below, so the
	// liveness check runs however fast the driver is.
	if ( s.m_Driver.hold )
		s.m_Driver.hold( *device, true );
	const std::optional<CompletionToken> token = s.Run( *device, encoder.Value() );
	s.That( token.has_value(), "D5", "the submission that uses the resource is accepted" );
	if ( !token )
		return;
	s.That( device->Release( buffer, *token ).HasValue(), "D5", "release is accepted" );
	if ( !device->IsComplete( *token ) )
	{
		(void)device->Poll();
		s.That( device->IsComplete( *token ) || device->LiveResourceCount() == live, "D5",
		    "a released resource stays live until its token completes" );
	}
	if ( s.m_Driver.hold )
		s.m_Driver.hold( *device, false );
	if ( s.m_Driver.holdsCompletion )
		s.That( !device->IsComplete( *token ), "D5", "the manual adapter held the token" );
	s.That( s.Finish( *device, *token ), "D5", "the token completes" );
	(void)device->Poll();
	s.That( device->LiveResourceCount() == live - 1, "D5",
	    "Poll frees the resource after its token completes" );
	auto again = device->Release( buffer, {} );
	s.That( !again && again.Error().status == DeviceStatus::kInvalidHandle, "D5",
	    "a released handle is never valid again" );

	// Release forbids new use; it does not cancel work already submitted.
	const BufferId source =
	    s.Buffer( *device, 128, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId target =
	    s.Buffer( *device, 128, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto copy = device->BeginEncoder( QueueKind::kGraphics );
	if ( !copy )
		return;
	const std::vector<std::byte> payload = Pattern( 128, 9 );
	copy.Value().TransitionBuffer(
	    source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	copy.Value().WriteBuffer( source, 0, payload );
	copy.Value().TransitionBuffer(
	    source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	copy.Value().TransitionBuffer(
	    target, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	copy.Value().CopyBuffer( source, target, { 0, 0, 128 } );
	copy.Value().TransitionBuffer(
	    target, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> copied = s.Run( *device, copy.Value() );
	if ( !copied )
		return;
	(void)device->Release( source, *copied );
	(void)s.Finish( *device, *copied );
	(void)device->Poll();
	s.That( s.ReadBack( *device, target, 128 ) == payload, "D5",
	    "work submitted before a release still runs" );
}

inline void Order( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	CompletionToken tokens[3];
	bool ok = true;
	for ( CompletionToken &token : tokens )
	{
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		std::optional<CompletionToken> submitted =
		    encoder ? s.Run( *device, encoder.Value() ) : std::nullopt;
		ok &= submitted.has_value();
		token = submitted.value_or( CompletionToken{} );
	}
	s.That( ok, "D6", "three submissions are accepted" );
	// At every point: a complete token implies every earlier one is.
	auto ordered = [&]
	{
		for ( int later = 1; later < 3; ++later )
		{
			for ( int earlier = 0; earlier < later; ++earlier )
			{
				if ( device->IsComplete( tokens[later] ) && !device->IsComplete( tokens[earlier] ) )
					return false;
			}
		}
		return true;
	};
	s.That( ordered(), "D6", "no later submission reports complete before an earlier one" );
	s.That( tokens[1].value == tokens[0].value + 1 && tokens[2].value == tokens[1].value + 1 &&
	            tokens[0].epoch == device->Epoch(),
	    "D6", "token values rise by one per submission in the current epoch" );
	s.That( s.Finish( *device, tokens[0] ), "D6", "the first submission completes" );
	s.That( ordered(), "D6", "completion stays ordered after the first completes" );
	s.That( s.Finish( *device, tokens[1] ), "D6", "the middle submission completes" );
	s.That( device->IsComplete( tokens[0] ) && device->IsComplete( tokens[1] ), "D6",
	    "completing a submission completes every earlier one on its queue" );
	if ( s.m_Driver.holdsCompletion )
		s.That( !device->IsComplete( tokens[2] ), "D6", "later submissions stay pending" );
	s.That( s.Finish( *device, tokens[2] ), "D6", "the last submission completes" );
	s.That( device->IsComplete( CompletionToken{} ), "D6", "the default token is complete" );
}

inline void Epochs( Suite &s )
{
	if ( !s.m_Driver.lose )
	{
		std::printf(
		    "SKIP %s.D7 device loss cannot be forced on this adapter\n", s.m_Driver.name.c_str() );
		return;
	}
	auto device = s.Create();
	if ( !device )
		return;
	(void)s.Buffer( *device, 64, { ResourceUsage::kVertex } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	const std::optional<CompletionToken> old =
	    encoder ? s.Run( *device, encoder.Value() ) : std::nullopt;
	const std::uint32_t epoch = device->Epoch();
	s.m_Driver.lose( *device );
	s.That( device->State() == DeviceState::kLost, "D7", "a lost device reports kLost" );
	auto lost = device->BeginEncoder( QueueKind::kGraphics );
	s.That( !lost && lost.Error().status == DeviceStatus::kDeviceLost, "D7",
	    "a lost device refuses new work" );
	s.That( device->Recover().HasValue() && device->State() == DeviceState::kAvailable &&
	            device->Epoch() == epoch + 1,
	    "D7", "recovery starts a new epoch" );
	s.That( device->LiveResourceCount() == 0, "D7", "no resource survives the lost epoch" );
	s.That( old && device->IsComplete( *old ), "D7", "old-epoch tokens are complete" );
	auto fresh = device->BeginEncoder( QueueKind::kGraphics );
	if ( !fresh || !old )
		return;
	const CompletionToken waits[] = { *old };
	auto stale =
	    device->Submit( QueueKind::kGraphics, { &fresh.Value(), 1 }, SubmitWaits{ waits } );
	s.That( !stale && stale.Error().status == DeviceStatus::kStaleEpoch, "D7",
	    "waiting on an old-epoch token fails with kStaleEpoch" );
}

inline void EncoderOrderAndErrors( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const BufferId buffer =
	    s.Buffer( *device, 64, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const std::vector<std::byte> first = Pattern( 64, 2 );
	const std::vector<std::byte> second = Pattern( 64, 3 );

	auto a = device->BeginEncoder( QueueKind::kGraphics );
	auto b = device->BeginEncoder( QueueKind::kGraphics );
	if ( !a || !b )
		return;
	a.Value().TransitionBuffer(
	    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	a.Value().WriteBuffer( buffer, 0, first );
	b.Value().WriteBuffer( buffer, 0, second );
	b.Value().TransitionBuffer(
	    buffer, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	CommandEncoder pair[] = { std::move( a.Value() ), std::move( b.Value() ) };
	auto token = device->Submit( QueueKind::kGraphics, pair, {} );
	s.That( token.HasValue(), "D8", "two encoders submit together" );
	s.That( !pair[0].IsOpen() && !pair[1].IsOpen(), "D8", "submitted encoders are closed" );
	if ( token )
		(void)s.Finish( *device, token.Value() );
	s.That( s.ReadBack( *device, buffer, 64 ) == second, "D8",
	    "encoders run in the order given to Submit" );

	auto good = device->BeginEncoder( QueueKind::kGraphics );
	auto bad = device->BeginEncoder( QueueKind::kGraphics );
	if ( !good || !bad )
		return;
	good.Value().TransitionBuffer(
	    buffer, ResourceUsage::kCopySource, ResourceUsage::kCopyDestination );
	good.Value().WriteBuffer( buffer, 0, first );
	good.Value().TransitionBuffer(
	    buffer, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	bad.Value().Draw( 3 ); // outside rendering
	s.That( bad.Value().Backend() && bad.Value().Backend()->HasError(), "D8",
	    "a draw outside rendering is recorded as an error" );
	CommandEncoder mixed[] = { std::move( good.Value() ), std::move( bad.Value() ) };
	auto rejected = device->Submit( QueueKind::kGraphics, mixed, {} );
	s.That( !rejected && rejected.Error().status == DeviceStatus::kInvalidState, "D8",
	    "a submission with an erroring encoder fails with kInvalidState" );
	(void)device->WaitIdle();
	s.That( s.ReadBack( *device, buffer, 64 ) == second, "D8",
	    "nothing of a rejected submission runs" );
}

inline void DataLands( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const std::vector<std::byte> bytes = Pattern( 1000, 4 );
	const BufferId source =
	    s.Buffer( *device, 1024, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId target =
	    s.Buffer( *device, 1024, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( source, 16, bytes );
	e.TransitionBuffer( source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( target, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBuffer( source, target, { 16, 8, 1000 } );
	e.TransitionBuffer( target, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	s.That( token.has_value(), "D9", "a write and a copy submit" );
	if ( token )
		(void)s.Finish( *device, *token );
	std::vector<std::byte> read = s.ReadBack( *device, target, 1008 );
	s.That( read.size() == 1008 && std::memcmp( read.data() + 8, bytes.data(), 1000 ) == 0, "D9",
	    "written and copied bytes read back unchanged" );

	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = 4;
	desc.height = 4;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource,
	    ResourceUsage::kColorAttachment };
	auto texture = device->CreateTexture( desc );
	s.That( texture.HasValue(), "D9", "an RGBA8 texture is created" );
	if ( !texture )
		return;
	const BufferId pixels =
	    s.Buffer( *device, 64, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto clear = device->BeginEncoder( QueueKind::kGraphics );
	if ( !clear )
		return;
	CommandEncoder &c = clear.Value();
	c.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	c.ClearTexture( texture.Value(), { 0.2f, 0.4f, 0.6f, 1.0f } );
	c.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	c.TransitionBuffer( pixels, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	c.CopyTextureToBuffer( texture.Value(), pixels, { 0, 0, 0, 4, 4 } );
	c.TransitionBuffer( pixels, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> cleared = s.Run( *device, c );
	s.That( cleared.has_value(), "D9", "a clear and a texture copy submit" );
	if ( cleared )
		(void)s.Finish( *device, *cleared );
	const std::vector<std::byte> texels = s.ReadBack( *device, pixels, 64 );
	bool all = texels.size() == 64;
	for ( std::size_t i = 0; all && i < 64; i += 4 )
	{
		all = texels[i] == std::byte{ 51 } && texels[i + 1] == std::byte{ 102 } &&
		      texels[i + 2] == std::byte{ 153 } && texels[i + 3] == std::byte{ 255 };
	}
	s.That( all, "D9", "a cleared texture copies out as its clear color" );
}

// Creation publishes an owned snapshot directly in copy-source usage. No
// encoder write or transfer through the adapter's ring is required.
inline void InitializedUploads( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const auto baseline = device->LiveResourceCount();
	const auto empty = device->CreateUploadBuffer( {} );
	s.That( !empty && empty.Error().status == DeviceStatus::kInvalidDescription &&
	            device->LiveResourceCount() == baseline,
	    "D25", "empty uploads fail without allocating" );
	std::vector<std::byte> bytes = Pattern( 4096, 51 );
	const auto first = device->CreateUploadBuffer( bytes );
	bytes = Pattern( 4096, 52 );
	const auto second = device->CreateUploadBuffer( bytes );
	std::fill( bytes.begin(), bytes.end(), std::byte{ 0 } );
	s.That( first && second, "D25", "initialized uploads are created" );
	if ( !first || !second )
		return;
	s.That( s.ReadBack( *device, first.Value(), 4096 ) == Pattern( 4096, 51 ), "D25",
	    "source mutation and another creation do not change the first snapshot" );
	s.That( s.ReadBack( *device, second.Value(), 4096 ) == Pattern( 4096, 52 ), "D25",
	    "each snapshot can be copied without an encoder write" );
	auto invalid = device->BeginEncoder( QueueKind::kGraphics );
	if ( invalid )
	{
		invalid.Value().TransitionBuffer(
		    first.Value(), ResourceUsage::kCopySource, ResourceUsage::kCopyDestination );
		invalid.Value().WriteBuffer( first.Value(), 0, bytes );
		s.That( !s.Run( *device, invalid.Value() ), "D25", "upload snapshots reject mutation" );
	}
	(void)device->Release( first.Value(), {} );
	(void)device->Release( second.Value(), {} );
	(void)device->Poll();
	s.That( device->LiveResourceCount() == baseline, "D25", "snapshots release without leaks" );
}

inline void UploadReuse( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	// More upload bytes in flight than a small ring holds, all submitted
	// before any completes. A range reused early corrupts an earlier write:
	// with the queue held, always (the copy out of the range cannot have run).
	// A device destroyed while held drops its held work.
	if ( s.m_Driver.hold )
		s.m_Driver.hold( *device, true );
	constexpr std::size_t kBuffers = 24;
	constexpr std::size_t kSize = 96 * 1024;
	std::vector<BufferId> buffers;
	std::vector<CompletionToken> tokens;
	for ( std::size_t i = 0; i < kBuffers; ++i )
	{
		const BufferId buffer = s.Buffer(
		    *device, kSize, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return;
		encoder.Value().TransitionBuffer(
		    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.Value().WriteBuffer( buffer, 0, Pattern( kSize, static_cast<std::uint8_t>( i ) ) );
		encoder.Value().TransitionBuffer(
		    buffer, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		const std::optional<CompletionToken> token = s.Run( *device, encoder.Value() );
		if ( !token )
		{
			s.That( false, "D10", "every upload submission is accepted" );
			return;
		}
		buffers.push_back( buffer );
		tokens.push_back( *token );
	}
	if ( s.m_Driver.hold )
		s.m_Driver.hold( *device, false );
	s.That( s.Finish( *device, tokens.back() ), "D10", "the uploads complete" );
	bool intact = true;
	for ( std::size_t i = 0; i < kBuffers; ++i )
		intact &= s.ReadBack( *device, buffers[i], kSize ) ==
		          Pattern( kSize, static_cast<std::uint8_t>( i ) );
	s.That( intact, "D10", "no upload range is reused before its token completes" );
}

inline void EncoderThreads( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.BeginLabel( "owner" );
	e.EndLabel();
	s.That( e.SequenceViolations() == 0, "D11", "recording on one thread is not a violation" );
	std::thread other(
	    [&e]
	    {
		    e.BeginLabel( "intruder" );
		    e.EndLabel();
	    } );
	other.join();
	s.That( e.SequenceViolations() > 0, "D11",
	    "recording one encoder from a second thread is diagnosed" );
}

inline void UsageStates( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = 4;
	desc.height = 4;
	desc.usages = {
	    ResourceUsage::kCopyDestination, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	auto texture = device->CreateTexture( desc );
	if ( !texture )
		return;
	auto missing = device->BeginEncoder( QueueKind::kGraphics );
	if ( !missing )
		return;
	missing.Value().ClearTexture( texture.Value(), {} );
	auto cleared = s.Run( *device, missing.Value() );
	s.That( !cleared, "D12", "a clear without a transition to kCopyDestination is rejected" );

	auto wrongBefore = device->BeginEncoder( QueueKind::kGraphics );
	if ( !wrongBefore )
		return;
	wrongBefore.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	wrongBefore.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kSampled, ResourceUsage::kCopySource );
	s.That( !s.Run( *device, wrongBefore.Value() ), "D12",
	    "a transition from a usage the resource is not in is rejected" );

	auto undeclared = device->BeginEncoder( QueueKind::kGraphics );
	if ( !undeclared )
		return;
	undeclared.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	s.That( !s.Run( *device, undeclared.Value() ), "D12",
	    "a transition to a usage the texture was not created for is rejected" );

	auto right = device->BeginEncoder( QueueKind::kGraphics );
	if ( !right )
		return;
	right.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	right.Value().ClearTexture( texture.Value(), {} );
	right.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	const std::optional<CompletionToken> token = s.Run( *device, right.Value() );
	s.That( token.has_value(), "D12", "declared transitions around a clear are accepted" );

	// The next submission continues from the usage the previous one left,
	// whether or not the GPU has run it yet.
	auto next = device->BeginEncoder( QueueKind::kGraphics );
	if ( !next )
		return;
	next.Value().TransitionTexture(
	    texture.Value(), ResourceUsage::kSampled, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> chained = s.Run( *device, next.Value() );
	s.That( chained.has_value(), "D12",
	    "a submission continues from the usage an unfinished one left" );
	if ( chained )
		(void)s.Finish( *device, *chained );
	else if ( token )
		(void)s.Finish( *device, *token );
}

// D14: distinct encoders recorded concurrently on several threads (one
// thread per encoder), submitted together; every write lands and no encoder
// reports a sequence violation. Parallel native recording is a capability;
// CPU-side recording of different encoders is not.
inline void ConcurrentEncoders( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	constexpr int kThreads = 8;
	constexpr std::size_t kSize = 4096;
	std::vector<BufferId> buffers;
	std::vector<CommandEncoder> encoders;
	for ( int i = 0; i < kThreads; ++i )
	{
		buffers.push_back( s.Buffer(
		    *device, kSize, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } ) );
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return;
		encoders.push_back( std::move( encoder ).Value() );
	}
	std::vector<std::thread> threads;
	for ( int i = 0; i < kThreads; ++i )
	{
		threads.emplace_back(
		    [&, i]
		    {
			    CommandEncoder &e = encoders[i];
			    e.TransitionBuffer(
			        buffers[i], ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			    for ( int chunk = 0; chunk < 16; ++chunk )
			    {
				    const std::vector<std::byte> bytes =
				        Pattern( kSize / 16, static_cast<std::uint8_t>( i * 16 + chunk ) );
				    e.WriteBuffer( buffers[i], chunk * ( kSize / 16 ), bytes );
			    }
			    e.TransitionBuffer(
			        buffers[i], ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		    } );
	}
	for ( std::thread &thread : threads )
		thread.join();
	std::uint32_t violations = 0;
	for ( const CommandEncoder &e : encoders )
		violations += e.SequenceViolations();
	s.That( violations == 0, "D14", "one thread per encoder is not a sequence violation" );
	auto token = device->Submit( QueueKind::kGraphics, encoders, {} );
	s.That( token.HasValue(), "D14", "concurrently recorded encoders submit together" );
	if ( !token || !s.Finish( *device, token.Value() ) )
		return;
	bool intact = true;
	for ( int i = 0; i < kThreads; ++i )
	{
		const std::vector<std::byte> read = s.ReadBack( *device, buffers[i], kSize );
		for ( int chunk = 0; chunk < 16 && intact; ++chunk )
		{
			const std::vector<std::byte> expected =
			    Pattern( kSize / 16, static_cast<std::uint8_t>( i * 16 + chunk ) );
			intact = read.size() == kSize && std::memcmp( read.data() + chunk * ( kSize / 16 ),
			                                     expected.data(), expected.size() ) == 0;
		}
	}
	s.That( intact, "D14", "every concurrently recorded write lands" );
}

// Renders with the port's conventions and reads the pixels back. Returns the
// RGBA8 texels of a size x size target and the depth texels, or empty.
struct Rendered
{
	std::vector<std::byte> color;
	std::vector<float> depth;
};

inline Rendered RenderFixture(
    Suite &s, IRenderDevice2 &device, std::span<const std::uint32_t> vertex, std::uint32_t size )
{
	Rendered out;
	const ColorPipeline pipeline = MakeColorPipeline( s, device, vertex, true );
	if ( !pipeline.ok )
		return out;
	const float color[4] = { 1.0f, 0.2f, 0.0f, 1.0f };
	const BufferId uniform =
	    s.Buffer( device, 256, { ResourceUsage::kCopyDestination, ResourceUsage::kUniform } );
	const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
	auto group = device.CreateBindGroup( { pipeline.layouts[2], entry } );
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = size;
	target.height = size;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto colorTarget = device.CreateTexture( target );
	target.format = Format::kD32Float;
	target.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kCopySource };
	auto depthTarget = device.CreateTexture( target );
	const std::uint64_t bytes = static_cast<std::uint64_t>( size ) * size * 4;
	const BufferId colorOut =
	    s.Buffer( device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId depthOut =
	    s.Buffer( device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !group || !colorTarget || !depthTarget || !encoder )
		return out;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( uniform, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( uniform, 0, { reinterpret_cast<const std::byte *>( color ), sizeof( color ) } );
	e.TransitionBuffer( uniform, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
	e.TransitionTexture(
	    colorTarget.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	e.TransitionTexture(
	    depthTarget.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	const ColorAttachment attachments[] = {
	    { colorTarget.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = attachments;
	rendering.depth = DepthAttachment{ depthTarget.Value(), LoadOp::kClear, StoreOp::kStore, 1.0f };
	rendering.width = size;
	rendering.height = size;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.pipeline );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.SetViewport( { 0, 0, static_cast<float>( size ), static_cast<float>( size ), 0, 1 } );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    colorTarget.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    depthTarget.Value(), ResourceUsage::kDepthWrite, ResourceUsage::kCopySource );
	e.TransitionBuffer( colorOut, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.TransitionBuffer( depthOut, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( colorTarget.Value(), colorOut, { 0, 0, 0, size, size } );
	e.CopyTextureToBuffer( depthTarget.Value(), depthOut, { 0, 0, 0, size, size } );
	e.TransitionBuffer( colorOut, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( depthOut, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( device, e );
	s.That( token.has_value(), "D13", "the fixture draw submits" );
	if ( !token || !s.Finish( device, *token ) )
		return out;
	out.color = s.ReadBack( device, colorOut, bytes );
	const std::vector<std::byte> depth = s.ReadBack( device, depthOut, bytes );
	out.depth.resize( depth.size() / 4 );
	if ( !depth.empty() )
		std::memcpy( out.depth.data(), depth.data(), depth.size() );
	return out;
}

inline void Conventions( Suite &s )
{
	if ( !s.m_Driver.rasterizes )
	{
		std::printf( "SKIP %s.D13 the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	auto device = s.Create();
	if ( !device )
		return;
	constexpr std::uint32_t kSize = 8;
	auto isDrawn = []( const std::vector<std::byte> &color, std::size_t texel )
	{
		return color[texel * 4] == std::byte{ 255 } && color[texel * 4 + 1] == std::byte{ 51 } &&
		       color[texel * 4 + 2] == std::byte{ 0 };
	};
	const Rendered full = RenderFixture( s, *device, shaders::kFullScreenVertex, kSize );
	bool covered = full.color.size() == kSize * kSize * 4;
	for ( std::size_t i = 0; covered && i < kSize * kSize; ++i )
		covered = isDrawn( full.color, i );
	s.That( covered, "D13", "a full-screen triangle writes the material color everywhere" );
	bool depth = full.depth.size() == kSize * kSize;
	for ( std::size_t i = 0; depth && i < full.depth.size(); ++i )
		depth = full.depth[i] > 0.2499f && full.depth[i] < 0.2501f;
	s.That( depth, "D13", "clip z 0.25 stores depth 0.25 (clip depth runs 0 to 1)" );

	const Rendered top = RenderFixture( s, *device, shaders::kTopHalfVertex, kSize );
	bool upper = top.color.size() == kSize * kSize * 4;
	for ( std::uint32_t y = 0; upper && y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; upper && x < kSize; ++x )
			upper = isDrawn( top.color, y * kSize + x ) == ( y < kSize / 2 );
	}
	s.That( upper, "D13", "clip y 0 to 1 lands in the top rows (clip Y up, row 0 at the top)" );
}

// D16 draw constants: the pipeline's block rules, the encoder's (a block is
// undefined after SetPipeline, and a draw needs every word of it; writes stay
// inside it) and, on rasterizing adapters, real pixels: two draws in one pass
// with different constants each show their own.
inline void DrawConstants( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	BindGroupLayoutId layouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( !s.That( layout.HasValue(), "D16", "an empty layout is created" ) )
			return;
		layouts[role] = layout.Value();
	}
	const Format colors[] = { Format::kRGBA8Unorm };
	auto describe = [&]( std::span<const std::uint32_t> vertex, std::uint32_t reflected,
	                    std::uint32_t declared, ShaderArtifactView( &stages )[2] )
	{
		stages[0] = {
		    ShaderStage::kVertex, device->Facts().artifactFormat, s.Code( vertex ), "main", {} };
		stages[1] = { ShaderStage::kFragment, device->Facts().artifactFormat,
		    s.Code( shaders::kConstantFragment ), "main", {}, reflected };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		desc.colorFormats = colors;
		desc.raster.cull = CullMode::kNone;
		desc.drawConstantBytes = declared;
		return desc;
	};
	ShaderArtifactView stages[2];
	auto status = [&]( std::uint32_t reflected, std::uint32_t declared )
	{
		auto created = device->CreatePipeline(
		    describe( shaders::kFullScreenVertex, reflected, declared, stages ) );
		return created ? DeviceStatus::kInternal : created.Error().status;
	};
	s.That( status( 16, kMaxDrawConstantBytes + 4 ) == DeviceStatus::kInvalidDescription, "D16",
	    "a block over the maximum fails kInvalidDescription" );
	s.That( status( 0, 6 ) == DeviceStatus::kInvalidDescription, "D16",
	    "a block that is not a multiple of four fails kInvalidDescription" );
	s.That( status( 16, 0 ) == DeviceStatus::kLayoutMismatch, "D16",
	    "a stage reading more than the block fails kLayoutMismatch" );
	auto full = device->CreatePipeline( describe( shaders::kFullScreenVertex, 16, 16, stages ) );
	ShaderArtifactView topStages[2];
	auto top = device->CreatePipeline( describe( shaders::kTopHalfVertex, 16, 16, topStages ) );
	if ( !s.That( full.HasValue() && top.HasValue(), "D16",
	         "pipelines with a 16-byte block are created" ) )
		return;

	constexpr std::uint32_t kSize = 8;
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto color = device->CreateTexture( target );
	if ( !s.That( color.HasValue(), "D16", "the target is created" ) )
		return;
	const float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	const float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	auto bytes = []( const float ( &value )[4] )
	{
		return std::span<const std::byte>(
		    reinterpret_cast<const std::byte *>( value ), sizeof( value ) );
	};
	// Records a pass drawing the full-screen pipeline; `body` sets its constants.
	auto pass = [&]( CommandEncoder &e, const std::function<void( CommandEncoder & )> &body )
	{
		const ColorAttachment attachments[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 1 }, {} } };
		RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.width = kSize;
		rendering.height = kSize;
		e.BeginRendering( rendering );
		e.SetViewport( { 0, 0, float( kSize ), float( kSize ), 0, 1 } );
		body( e );
		e.EndRendering();
	};
	auto rejected = [&]( const std::function<void( CommandEncoder & )> &body )
	{
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return false;
		encoder.Value().TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		pass( encoder.Value(), body );
		CommandEncoder list[] = { std::move( encoder ).Value() };
		auto token = device->Submit( QueueKind::kGraphics, list, {} );
		return !token && token.Error().status == DeviceStatus::kInvalidState;
	};
	s.That( rejected(
	            [&]( CommandEncoder &e )
	            {
		            e.SetPipeline( full.Value() );
		            e.Draw( 3 );
	            } ),
	    "D16", "a draw with the block unset fails the submission" );
	s.That( rejected(
	            [&]( CommandEncoder &e )
	            {
		            e.SetPipeline( full.Value() );
		            e.SetDrawConstants( 0, bytes( red ).first( 8 ) );
		            e.Draw( 3 );
	            } ),
	    "D16", "a draw with part of the block unset fails the submission" );
	s.That( rejected(
	            [&]( CommandEncoder &e )
	            {
		            e.SetPipeline( full.Value() );
		            e.SetDrawConstants( 8, bytes( red ) );
		            e.Draw( 3 );
	            } ),
	    "D16", "a write past the block fails the submission" );
	s.That( rejected(
	            [&]( CommandEncoder &e )
	            {
		            e.SetPipeline( full.Value() );
		            e.SetDrawConstants( 0, bytes( red ) );
		            e.SetPipeline( top.Value() );
		            e.Draw( 3 );
	            } ),
	    "D16", "binding a pipeline leaves the block undefined" );

	const std::uint64_t outBytes = std::uint64_t( kSize ) * kSize * 4;
	const BufferId out = s.Buffer(
	    *device, outBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	pass( e,
	    [&]( CommandEncoder &inner )
	    {
		    inner.SetPipeline( full.Value() );
		    inner.SetDrawConstants( 0, bytes( red ) );
		    inner.Draw( 3 );
		    inner.SetPipeline( top.Value() );
		    inner.SetDrawConstants( 0, bytes( green ).first( 8 ) );
		    inner.SetDrawConstants( 8, bytes( green ).subspan( 8 ) );
		    inner.Draw( 3 );
	    } );
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( color.Value(), out, { 0, 0, 0, kSize, kSize } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	s.That( token.has_value(), "D16", "draws with their blocks set submit" );
	if ( !token || !s.m_Driver.rasterizes )
	{
		if ( !s.m_Driver.rasterizes )
			std::printf(
			    "SKIP %s.D16 pixels: the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	if ( !s.Finish( *device, *token ) )
		return;
	const std::vector<std::byte> pixels = s.ReadBack( *device, out, outBytes );
	// Half the rows green (the second draw), the rest red (the first). Which
	// half is D13's business, not this clause's.
	auto rowColor = [&]( std::uint32_t y ) -> int
	{
		int color = -1;
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const std::byte *p = &pixels[( y * kSize + x ) * 4];
			const int here = p[0] == std::byte{ 0 } && p[1] == std::byte{ 255 }   ? 1
			                 : p[0] == std::byte{ 255 } && p[1] == std::byte{ 0 } ? 0
			                                                                      : -2;
			if ( here < 0 || ( color >= 0 && here != color ) )
				return -1;
			color = here;
		}
		return color;
	};
	std::uint32_t greenRows = 0, redRows = 0;
	for ( std::uint32_t y = 0; pixels.size() == outBytes && y < kSize; ++y )
	{
		const int color = rowColor( y );
		greenRows += color == 1;
		redRows += color == 0;
	}
	s.That( greenRows == kSize / 2 && redRows == kSize / 2, "D16",
	    "each draw shows its own constants (half the rows green over a red full screen)" );
}

// D17 color write masks: one mask per color format, each within the four
// channels (else kInvalidDescription), and, on rasterizing adapters, the
// channels a mask leaves out keep what the attachment held.
inline void ColorWriteMasks( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	BindGroupLayoutId layouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( !s.That( layout.HasValue(), "D17", "an empty layout is created" ) )
			return;
		layouts[role] = layout.Value();
	}
	const Format colors[] = { Format::kRGBA8Unorm };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          s.Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat,
	        s.Code( shaders::kConstantFragment ), "main", {}, 16 } };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	desc.drawConstantBytes = 16;
	auto status = [&]( std::span<const std::uint8_t> masks )
	{
		desc.colorWriteMasks = masks;
		auto created = device->CreatePipeline( desc );
		return created ? DeviceStatus::kInternal : created.Error().status;
	};
	const std::uint8_t two[] = { kColorWriteAll, kColorWriteAll };
	const std::uint8_t wide[] = { kColorWriteAll + 1 };
	s.That( status( two ) == DeviceStatus::kInvalidDescription, "D17",
	    "a mask count other than the color formats' fails kInvalidDescription" );
	s.That( status( wide ) == DeviceStatus::kInvalidDescription, "D17",
	    "a mask outside the four channels fails kInvalidDescription" );
	const std::uint8_t redAlpha[] = { kColorWriteRed | kColorWriteAlpha };
	desc.colorWriteMasks = redAlpha;
	auto masked = device->CreatePipeline( desc );
	if ( !s.That( masked.HasValue(), "D17", "a pipeline writing red and alpha is created" ) )
		return;

	constexpr std::uint32_t kSize = 8;
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto color = device->CreateTexture( target );
	if ( !s.That( color.HasValue(), "D17", "the target is created" ) )
		return;
	const std::uint64_t outBytes = std::uint64_t( kSize ) * kSize * 4;
	const BufferId out = s.Buffer(
	    *device, outBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	// Cleared blue at half alpha; the draw is opaque yellow through the mask.
	const ColorAttachment attachments[] = {
	    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 0.5f }, {} } };
	RenderingDesc rendering;
	rendering.colors = attachments;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetViewport( { 0, 0, float( kSize ), float( kSize ), 0, 1 } );
	e.SetPipeline( masked.Value() );
	const float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	e.SetDrawConstants( 0, std::as_bytes( std::span( yellow ) ) );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( color.Value(), out, { 0, 0, 0, kSize, kSize } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	s.That( token.has_value(), "D17", "a masked draw submits" );
	if ( !token || !s.m_Driver.rasterizes )
	{
		if ( !s.m_Driver.rasterizes )
			std::printf(
			    "SKIP %s.D17 pixels: the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	if ( !s.Finish( *device, *token ) )
		return;
	const std::vector<std::byte> pixels = s.ReadBack( *device, out, outBytes );
	bool kept = pixels.size() == outBytes;
	for ( std::size_t i = 0; kept && i < outBytes; i += 4 )
	{
		// Red and alpha from the draw, green and blue from the clear (127 or
		// 128 for alpha 0.5 is the clear's, which the mask must not reach).
		kept = pixels[i] == std::byte{ 255 } && pixels[i + 1] == std::byte{ 0 } &&
		       pixels[i + 2] == std::byte{ 255 } && pixels[i + 3] == std::byte{ 255 };
	}
	s.That( kept, "D17", "a red-and-alpha mask writes red and alpha and keeps green and blue" );
}

// D21 transmittance blending: kTransmittance writes src + dst * a in color and
// keeps the destination's alpha. On rasterizing adapters, over a half-float
// target cleared to ( 0.8, 0.6, 0.4, 0.25 ), a draw of ( 0.01, 0.02, 0.03,
// 0.05 ) gives ( 0.05, 0.05, 0.05, 0.25 ): the small transmittance applied at
// the target's precision (a premultiplied blend would give 0.77, 0.59, 0.41).
inline void ColorBlend( Suite &s, bool modulate = false )
{
	const char *clause = modulate ? "D17.modulate2x" : "D21";
	auto device = s.Create();
	if ( !device )
		return;
	BindGroupLayoutId layouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( !s.That( layout.HasValue(), clause, "an empty layout is created" ) )
			return;
		layouts[role] = layout.Value();
	}
	const Format colors[] = { Format::kRGBA16Float };
	const BlendMode blends[] = { modulate ? BlendMode::kModulate2x : BlendMode::kTransmittance };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          s.Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat,
	        s.Code( shaders::kConstantFragment ), "main", {}, 16 } };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.raster.cull = CullMode::kNone;
	desc.drawConstantBytes = 16;
	auto pipeline = device->CreatePipeline( desc );
	if ( !s.That(
	         pipeline.HasValue(), clause, "the requested color-blending pipeline is created" ) )
		return;

	constexpr std::uint32_t kSize = 8;
	TextureDesc target;
	target.format = Format::kRGBA16Float;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto color = device->CreateTexture( target );
	if ( !s.That( color.HasValue(), clause, "the half-float target is created" ) )
		return;
	const std::uint64_t outBytes = std::uint64_t( kSize ) * kSize * 8;
	const BufferId out = s.Buffer(
	    *device, outBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	const ColorAttachment attachments[] = {
	    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0.8f, 0.6f, 0.4f, 0.25f }, {} } };
	RenderingDesc rendering;
	rendering.colors = attachments;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetViewport( { 0, 0, float( kSize ), float( kSize ), 0, 1 } );
	e.SetPipeline( pipeline.Value() );
	const float source[4] = { 0.01f, 0.02f, 0.03f, 0.05f };
	e.SetDrawConstants( 0, std::as_bytes( std::span( source ) ) );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( color.Value(), out, { 0, 0, 0, kSize, kSize } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	s.That( token.has_value(), clause, "the color-blended draw submits" );
	if ( !token || !s.m_Driver.rasterizes )
	{
		if ( !s.m_Driver.rasterizes )
			std::printf(
			    "SKIP %s.D21 pixels: the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	if ( !s.Finish( *device, *token ) )
		return;
	const std::vector<std::byte> pixels = s.ReadBack( *device, out, outBytes );
	// Half floats decoded here (normal numbers only: the values are 0.01 to 1).
	const auto half = [&]( std::size_t offset )
	{
		std::uint16_t bits = 0;
		std::memcpy( &bits, pixels.data() + offset, sizeof( bits ) );
		const int exponent = ( bits >> 10 ) & 31;
		const double mantissa = 1.0 + ( bits & 1023 ) / 1024.0;
		return ( bits & 0x8000 ? -1.0 : 1.0 ) * std::ldexp( mantissa, exponent - 15 );
	};
	const double want[4] = { modulate ? 2.0 * 0.01 * 0.8 : 0.01 + 0.8 * 0.05,
	    modulate ? 2.0 * 0.02 * 0.6 : 0.02 + 0.6 * 0.05,
	    modulate ? 2.0 * 0.03 * 0.4 : 0.03 + 0.4 * 0.05, 0.25 };
	bool matched = pixels.size() == outBytes;
	double worst = 0.0;
	for ( std::size_t i = 0; matched && i < outBytes; i += 8 )
	{
		for ( int c = 0; c < 4; ++c )
		{
			const double error = std::fabs( half( i + 2 * c ) - want[c] ) / want[c];
			worst = std::max( worst, error );
			// A half float's relative precision is 2^-11; the blend rounds
			// once more.
			matched &= error <= 2.0 / 1024.0;
		}
	}
	s.That( matched, clause,
	    "the independent color equation matches within 2^-10, destination alpha kept" );
	if ( !matched )
		std::printf( "INFO %s.D21 worst relative error %.5f\n", s.m_Driver.name.c_str(), worst );
}

// D20 specialization constants: a stage lists each id at most once (else
// kInvalidDescription); an id the stage does not declare is ignored; on
// rasterizing adapters a constant's value reaches the shader (specialized.frag
// draws red by default, green with constant 7 at 1).
inline void SpecializationConstants( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	BindGroupLayoutId layouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( !s.That( layout.HasValue(), "D20", "an empty layout is created" ) )
			return;
		layouts[role] = layout.Value();
	}
	const Format colors[] = { Format::kRGBA8Unorm };
	auto pipelineWith = [&]( std::span<const SpecializationConstant> constants )
	{
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, device->Facts().artifactFormat,
		        s.Code( shaders::kFullScreenVertex ), "main", {} },
		    { ShaderStage::kFragment, device->Facts().artifactFormat,
		        s.Code( shaders::kSpecializedFragment ), "main", {} } };
		PipelineDesc desc;
		desc.stages = stages;
		desc.constants = constants;
		desc.layouts = layouts;
		desc.colorFormats = colors;
		desc.raster.cull = CullMode::kNone;
		return device->CreatePipeline( desc );
	};
	const SpecializationConstant twice[] = {
	    { ShaderStage::kFragment, 7, 1 }, { ShaderStage::kFragment, 7, 0 } };
	auto duplicate = pipelineWith( twice );
	s.That( !duplicate && duplicate.Error().status == DeviceStatus::kInvalidDescription, "D20",
	    "an id listed twice in a stage fails kInvalidDescription" );
	const SpecializationConstant undeclared[] = { { ShaderStage::kFragment, 99, 1 } };
	s.That( pipelineWith( undeclared ).HasValue(), "D20", "an undeclared id is ignored" );
	auto plain = pipelineWith( {} );
	const SpecializationConstant green[] = { { ShaderStage::kFragment, 7, 1 } };
	auto specialized = pipelineWith( green );
	if ( !s.That( plain.HasValue() && specialized.HasValue(), "D20",
	         "pipelines with and without the constant are created" ) )
		return;

	constexpr std::uint32_t kSize = 8;
	const std::uint64_t outBytes = std::uint64_t( kSize ) * kSize * 4;
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	PipelineId pipelines[2] = { plain.Value(), specialized.Value() };
	BufferId outs[2];
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	for ( int i = 0; i < 2; ++i )
	{
		auto color = device->CreateTexture( target );
		if ( !s.That( color.HasValue(), "D20", "a target is created" ) )
			return;
		outs[i] = s.Buffer(
		    *device, outBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		const ColorAttachment attachments[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 1 }, {} } };
		RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.width = kSize;
		rendering.height = kSize;
		e.BeginRendering( rendering );
		e.SetViewport( { 0, 0, float( kSize ), float( kSize ), 0, 1 } );
		e.SetPipeline( pipelines[i] );
		e.Draw( 3 );
		e.EndRendering();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		e.TransitionBuffer( outs[i], ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( color.Value(), outs[i], { 0, 0, 0, kSize, kSize } );
		e.TransitionBuffer( outs[i], ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	}
	const std::optional<CompletionToken> token = s.Run( *device, e );
	s.That( token.has_value(), "D20", "the two draws submit" );
	if ( !token || !s.m_Driver.rasterizes )
	{
		if ( !s.m_Driver.rasterizes )
			std::printf(
			    "SKIP %s.D20 pixels: the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	if ( !s.Finish( *device, *token ) )
		return;
	bool drawn[2] = { true, true };
	for ( int i = 0; i < 2; ++i )
	{
		const std::vector<std::byte> pixels = s.ReadBack( *device, outs[i], outBytes );
		drawn[i] = pixels.size() == outBytes;
		for ( std::size_t p = 0; drawn[i] && p < outBytes; p += 4 )
		{
			drawn[i] = pixels[p] == std::byte( i == 0 ? 255 : 0 ) &&
			           pixels[p + 1] == std::byte( i == 0 ? 0 : 255 ) &&
			           pixels[p + 2] == std::byte{ 0 };
		}
	}
	s.That( drawn[0], "D20", "without the constant the shader's default applies (red)" );
	s.That( drawn[1], "D20", "the constant's value reaches the shader (green)" );
}

// D18 external images: the exporter exists exactly when kExternalImages is
// claimed; a plain texture never takes kExternal; an exported image's memory,
// mapped through its description (offset, stride), holds what the port reads
// back from the texture; descriptions outside the rules fail by status.
inline void ExternalImagesClause( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	IExternalImages *exporter = device->ExternalImages();
	s.That(
	    device->Facts().capabilities.Has( Capability::kExternalImages ) == ( exporter != nullptr ),
	    "D18", "the exporter is present exactly when kExternalImages is claimed" );
	constexpr std::uint32_t kWidth = 16;
	constexpr std::uint32_t kHeight = 8;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = kWidth;
	desc.height = kHeight;
	desc.usages = {
	    ResourceUsage::kCopyDestination, ResourceUsage::kCopySource, ResourceUsage::kExternal };
	auto plain = device->CreateTexture( desc );
	s.That( !plain && plain.Error().status == DeviceStatus::kInvalidDescription, "D18",
	    "CreateTexture refuses kExternal" );
	if ( !exporter )
		return;
	auto statusOf = [&]( const TextureDesc &candidate )
	{
		auto made = exporter->CreateExported( candidate );
		if ( !made )
			return made.Error().status;
		exporter->CloseHandle( made.Value().handle );
		(void)device->Release( made.Value().texture, CompletionToken{} );
		return DeviceStatus::kInternal;
	};
	TextureDesc noExternal = desc;
	noExternal.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	TextureDesc mips = desc;
	mips.mipLevels = 2;
	TextureDesc wide = desc;
	wide.format = Format::kRGBA16Float;
	s.That( statusOf( noExternal ) == DeviceStatus::kInvalidDescription &&
	            statusOf( mips ) == DeviceStatus::kInvalidDescription,
	    "D18", "an export without kExternal, or of several mips, fails kInvalidDescription" );
	s.That( statusOf( wide ) == DeviceStatus::kUnsupported, "D18",
	    "an export of a format outside the four fails kUnsupported" );

	auto image = exporter->CreateExported( desc );
	if ( !s.That( image.HasValue() && image.Value().handle >= 0 && image.Value().fourcc != 0 &&
	                  image.Value().stride >= kWidth * 4,
	         "D18", "an image is exported with a handle and a plane description" ) )
		return;
	const ExternalImage exported = image.Value();
	const std::uint64_t bytes = std::uint64_t( kWidth ) * kHeight * 4;
	const std::vector<std::byte> pattern = Pattern( bytes, 5 );
	const BufferId source =
	    s.Buffer( *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId out =
	    s.Buffer( *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( source, 0, pattern );
	e.TransitionBuffer( source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    exported.texture, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( source, exported.texture, { 0, 0, 0, kWidth, kHeight } );
	e.TransitionTexture(
	    exported.texture, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( exported.texture, out, { 0, 0, 0, kWidth, kHeight } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture( exported.texture, ResourceUsage::kCopySource, ResourceUsage::kExternal );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	const bool finished = token && s.Finish( *device, *token );
	s.That( finished, "D18", "the texture is written and handed to kExternal" );
	const std::vector<std::byte> readback =
	    finished ? s.ReadBack( *device, out, bytes ) : std::vector<std::byte>();
	s.That( readback == pattern, "D18", "the port reads back what it wrote" );
#if defined( __linux__ )
	const int fd = int( exported.handle );
	const std::size_t length = std::size_t( exported.offset ) +
	                           std::size_t( exported.stride ) * ( kHeight - 1 ) + kWidth * 4;
	void *mapped = ::mmap( nullptr, length, PROT_READ, MAP_SHARED, fd, 0 );
	bool equal = mapped != MAP_FAILED && readback.size() == bytes;
	if ( mapped != MAP_FAILED )
	{
		dma_buf_sync sync{ DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
		(void)::ioctl( fd, DMA_BUF_IOCTL_SYNC, &sync );
		const auto *base = static_cast<const std::byte *>( mapped ) + exported.offset;
		for ( std::uint32_t y = 0; equal && y < kHeight; ++y )
			equal = std::memcmp( base + std::size_t( y ) * exported.stride,
			            readback.data() + std::size_t( y ) * kWidth * 4, kWidth * 4 ) == 0;
		sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
		(void)::ioctl( fd, DMA_BUF_IOCTL_SYNC, &sync );
		::munmap( mapped, length );
	}
	s.That( equal, "D18",
	    "the exported memory, read at its offset and stride, equals the texture's readback" );
	exporter->CloseHandle( exported.handle );
#else
	exporter->CloseHandle( exported.handle );
	std::printf(
	    "SKIP %s.D18 memory: no dmabuf mapping on this platform\n", s.m_Driver.name.c_str() );
#endif
	(void)device->Release( exported.texture, token.value_or( CompletionToken{} ) );
	(void)device->Release( source, token.value_or( CompletionToken{} ) );
	(void)device->Release( out, token.value_or( CompletionToken{} ) );
}

// D15 capability honesty: every capability an executing adapter claims
// works. kCompute and kStorageBuffers: a dispatch writes a storage buffer;
// kAsyncCompute: the same dispatch runs from a compute-queue encoder;
// kAsyncTransfer: a transfer-queue encoder copies a buffer. The port has no
// operation for transient aliasing, parallel native recording or ray query
// yet, so an adapter that executes work cannot honestly claim them. A
// recording adapter (null) runs no shaders and is skipped.
inline bool DoublesOn( Suite &s, IRenderDevice2 &device, QueueKind queue, const char *what )
{
	constexpr std::uint32_t kCount = 64;
	static const BindingDesc storage[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, storage } );
	if ( !s.That( layout.HasValue() && !s.m_Driver.doubleCompute.empty(), "D15", what ) )
		return false;
	const BindGroupLayoutId layouts[] = { {}, {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 3, 0, BindingKind::kStorageBuffer } };
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, device.Facts().artifactFormat,
	    s.Code( s.m_Driver.doubleCompute ), "main", used } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	desc.layouts = layouts;
	auto pipeline = device.CreatePipeline( desc );
	const BufferId data = s.Buffer( device, kCount * 4,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite,
	        ResourceUsage::kCopySource } );
	const BindGroupEntry entry[] = { { 0, data, 0, 0, {}, {} } };
	auto group = device.CreateBindGroup( { layout.Value(), entry } );
	auto encoder = device.BeginEncoder( queue );
	if ( !s.That( pipeline.HasValue() && group.HasValue() && encoder.HasValue(), "D15", what ) )
		return false;
	std::vector<std::uint32_t> values( kCount );
	for ( std::uint32_t i = 0; i < kCount; ++i )
		values[i] = i;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( data, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( data, 0, std::as_bytes( std::span<const std::uint32_t>( values ) ) );
	e.TransitionBuffer( data, ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	e.Dispatch( 1 );
	e.TransitionBuffer( data, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	auto token = device.Submit( queue, { &e, 1 }, {} );
	if ( !s.That( token.HasValue() && s.Finish( device, token.Value() ), "D15", what ) )
		return false;
	const std::vector<std::byte> read = s.ReadBack( device, data, kCount * 4 );
	bool doubled = read.size() == kCount * 4;
	for ( std::uint32_t i = 0; doubled && i < kCount; ++i )
	{
		std::uint32_t value = 0;
		std::memcpy( &value, read.data() + i * 4, 4 );
		doubled = value == 2 * i + 1;
	}
	return s.That( doubled, "D15", what );
}

// D19 block-compressed formats: without kTextureCompressionBC a kBC* texture
// fails kUnsupported. With it, a BC1 and a BC3 texture of 12x6 texels and two
// mips (the second 6x3, so not a multiple of the block) take whole blocks
// (RegionBytes); uploaded mip by mip and copied back, their bytes are
// unchanged. A copy that splits a block, a clear, an attachment usage and a
// multisampled description fail.
inline void BlockCompressedFormats( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.format = Format::kBC1Unorm;
	desc.width = 12;
	desc.height = 6;
	desc.mipLevels = 2;
	desc.usages = {
	    ResourceUsage::kSampled, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	if ( !device->Facts().capabilities.Has( Capability::kTextureCompressionBC ) )
	{
		auto made = device->CreateTexture( desc );
		s.That( !made && made.Error().status == DeviceStatus::kUnsupported, "D19",
		    "without kTextureCompressionBC a block-compressed texture fails kUnsupported" );
		return;
	}
	s.That( RegionBytes( Format::kBC1Unorm, 12, 6 ) == 48 &&
	            RegionBytes( Format::kBC3Unorm, 12, 6 ) == 96 &&
	            RegionBytes( Format::kBC1Unorm, 6, 3 ) == 16 &&
	            RegionBytes( Format::kBC5Unorm, 1, 1 ) == 16 &&
	            RegionBytes( Format::kBC6HUfloat, 12, 6 ) == 96 &&
	            RegionBytes( Format::kBC7Unorm, 6, 3 ) == 32 &&
	            IsBlockCompressed( Format::kBC7Srgb ),
	    "D19", "a region takes whole blocks" );
	TextureDesc attachment = desc;
	attachment.usages = { ResourceUsage::kSampled, ResourceUsage::kColorAttachment };
	TextureDesc multisampled = desc;
	multisampled.mipLevels = 1;
	multisampled.sampleCount = 4;
	auto badAttachment = device->CreateTexture( attachment );
	auto badSamples = device->CreateTexture( multisampled );
	s.That( !badAttachment && badAttachment.Error().status == DeviceStatus::kInvalidDescription &&
	            !badSamples && badSamples.Error().status == DeviceStatus::kInvalidDescription,
	    "D19", "an attachment usage or several samples fail kInvalidDescription" );

	for ( const Format format :
	    { Format::kBC1Unorm, Format::kBC3Unorm, Format::kBC6HUfloat, Format::kBC7Unorm } )
	{
		TextureDesc blocks = desc;
		blocks.format = format;
		auto texture = device->CreateTexture( blocks );
		if ( !s.That( texture.HasValue(), "D19", "a block-compressed texture is created" ) )
			return;
		const std::uint64_t first = RegionBytes( format, 12, 6 );
		const std::uint64_t bytes = first + RegionBytes( format, 6, 3 );
		const std::vector<std::byte> pattern = Pattern( bytes, 19 );
		const BufferId source = s.Buffer(
		    *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		const BufferId out = s.Buffer(
		    *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return;
		CommandEncoder &e = encoder.Value();
		const TextureId id = texture.Value();
		e.TransitionBuffer( source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( source, 0, pattern );
		e.TransitionBuffer( source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		e.TransitionTexture( id, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( source, id, { 0, 0, 0, 12, 6 } );
		e.CopyBufferToTexture( source, id, { first, 1, 0, 6, 3 } );
		e.TransitionTexture( id, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( id, out, { 0, 0, 0, 12, 6 } );
		e.CopyTextureToBuffer( id, out, { first, 1, 0, 6, 3 } );
		e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		const std::optional<CompletionToken> token = s.Run( *device, e );
		const bool finished = token && s.Finish( *device, *token );
		const std::string what = std::string( format == Format::kBC1Unorm     ? "BC1"
		                                      : format == Format::kBC3Unorm   ? "BC3"
		                                      : format == Format::kBC6HUfloat ? "BC6H"
		                                                                      : "BC7" ) +
		                         ": both mips copy in and back unchanged";
		s.That( finished && s.ReadBack( *device, out, bytes ) == pattern, "D19", what.c_str() );

		// A copy that splits a block, and a clear, fail their submissions.
		auto refused = [&]( auto &&record )
		{
			auto bad = device->BeginEncoder( QueueKind::kGraphics );
			if ( !bad )
				return false;
			record( bad.Value() );
			auto submitted = device->Submit( QueueKind::kGraphics, { &bad.Value(), 1 }, {} );
			return !submitted && submitted.Error().status == DeviceStatus::kInvalidState;
		};
		const bool split = refused(
		    [&]( CommandEncoder &bad )
		    {
			    bad.TransitionTexture(
			        id, ResourceUsage::kCopySource, ResourceUsage::kCopyDestination );
			    bad.CopyBufferToTexture( source, id, { 0, 0, 0, 3, 4 } );
		    } );
		const bool cleared = refused(
		    [&]( CommandEncoder &bad )
		    {
			    bad.TransitionTexture(
			        id, ResourceUsage::kCopySource, ResourceUsage::kCopyDestination );
			    bad.ClearTexture( id, { 1.0f, 0.0f, 0.0f, 1.0f } );
		    } );
		s.That( split && cleared, "D19", "a copy that splits a block, and a clear, are refused" );
		(void)device->Release( id, token.value_or( CompletionToken{} ) );
		(void)device->Release( source, token.value_or( CompletionToken{} ) );
		(void)device->Release( out, token.value_or( CompletionToken{} ) );
	}
}

// D27 packed unsigned floats (kRG11B10Float): PackRG11B10Float rounds to
// nearest even, maps negatives and NaN to 0 and clamps past the largest
// finite value; a 4x4 colour attachment cleared to a colour copies out as
// that packing.
inline void PackedFloatTargets( Suite &s )
{
	s.That(
	    PackRG11B10Float( 1.0f, 1.0f, 1.0f ) == ( 0x3c0u | ( 0x3c0u << 11 ) | ( 0x1e0u << 22 ) ) &&
	        PackRG11B10Float( 0.5f, 0.0f, 2.0f ) == ( 0x380u | ( 0x200u << 22 ) ) &&
	        PackRG11B10Float( -1.0f, std::nanf( "" ), 0.0f ) == 0u &&
	        PackRG11B10Float( 1.0e9f, 0.0f, 1.0e9f ) == ( 0x7bfu | ( 0x3dfu << 22 ) ) &&
	        BytesPerTexel( Format::kRG11B10Float ) == 4,
	    "D27", "the packing rounds, zeroes negatives and NaN, and clamps" );
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.format = Format::kRG11B10Float;
	desc.width = 4;
	desc.height = 4;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource,
	    ResourceUsage::kColorAttachment, ResourceUsage::kSampled };
	auto texture = device->CreateTexture( desc );
	if ( !s.That( texture.HasValue(), "D27", "a packed-float colour attachment is created" ) )
		return;
	const BufferId pixels =
	    s.Buffer( *device, 64, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto clear = device->BeginEncoder( QueueKind::kGraphics );
	if ( !clear )
		return;
	CommandEncoder &c = clear.Value();
	c.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	c.ClearTexture( texture.Value(), { 0.25f, 3.0f, 0.5f, 1.0f } );
	c.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	c.TransitionBuffer( pixels, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	c.CopyTextureToBuffer( texture.Value(), pixels, { 0, 0, 0, 4, 4 } );
	c.TransitionBuffer( pixels, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, c );
	if ( token )
		(void)s.Finish( *device, *token );
	const std::vector<std::byte> texels = s.ReadBack( *device, pixels, 64 );
	const std::uint32_t expected = PackRG11B10Float( 0.25f, 3.0f, 0.5f );
	bool all = token.has_value() && texels.size() == 64;
	for ( std::size_t i = 0; all && i < 64; i += 4 )
	{
		std::uint32_t texel = 0;
		std::memcpy( &texel, texels.data() + i, 4 );
		all = texel == expected;
	}
	s.That( all, "D27", "a cleared packed-float target copies out as its packed clear colour" );
	(void)device->Release( texture.Value(), token.value_or( CompletionToken{} ) );
	(void)device->Release( pixels, token.value_or( CompletionToken{} ) );
}

// D22 region copies: a buffer-to-texture copy at (x, y) writes that
// rectangle alone, a texture-to-buffer copy at (x, y) reads it back, and a
// region past the mip's edge is refused at submission.
inline void RegionCopies( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = 8;
	desc.height = 8;
	desc.usages = {
	    ResourceUsage::kSampled, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto texture = device->CreateTexture( desc );
	if ( !s.That( texture.HasValue(), "D22", "an RGBA8 texture is created" ) )
		return;
	const TextureId id = texture.Value();
	constexpr std::uint32_t kX = 4, kY = 5, kW = 3, kH = 2;
	const std::vector<std::byte> whole = Pattern( 8 * 8 * 4, 22 );
	const std::vector<std::byte> region = Pattern( kW * kH * 4, 23 );
	const std::uint64_t regionAt = 8 * 8 * 4;
	const BufferId source = s.Buffer( *device, regionAt + region.size(),
	    { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId out = s.Buffer( *device, regionAt + region.size(),
	    { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( source, 0, whole );
	e.WriteBuffer( source, regionAt, region );
	e.TransitionBuffer( source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture( id, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( source, id, { 0, 0, 0, 8, 8 } );
	TextureBufferCopy part;
	part.bufferOffset = regionAt;
	part.width = kW;
	part.height = kH;
	part.x = kX;
	part.y = kY;
	e.CopyBufferToTexture( source, id, part );
	e.TransitionTexture( id, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( id, out, { 0, 0, 0, 8, 8 } );
	e.CopyTextureToBuffer( id, out, part );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	const bool finished = token && s.Finish( *device, *token );
	const std::vector<std::byte> read = s.ReadBack( *device, out, regionAt + region.size() );
	bool placed = finished && read.size() == regionAt + region.size();
	for ( std::uint32_t y = 0; placed && y < 8; ++y )
		for ( std::uint32_t x = 0; placed && x < 8; ++x )
		{
			const bool inside = x >= kX && x < kX + kW && y >= kY && y < kY + kH;
			const std::byte *want =
			    inside ? &region[( ( y - kY ) * kW + ( x - kX ) ) * 4] : &whole[( y * 8 + x ) * 4];
			placed = std::memcmp( &read[( y * 8 + x ) * 4], want, 4 ) == 0;
		}
	s.That( placed, "D22", "a copy at (x, y) writes its rectangle alone" );
	s.That( placed && std::memcmp( read.data() + regionAt, region.data(), region.size() ) == 0,
	    "D22", "a copy out at (x, y) reads that rectangle" );
	auto bad = device->BeginEncoder( QueueKind::kGraphics );
	if ( bad )
	{
		TextureBufferCopy past = part;
		past.x = 6; // 6 + 3 > 8
		bad.Value().TransitionTexture(
		    id, ResourceUsage::kCopySource, ResourceUsage::kCopyDestination );
		bad.Value().CopyBufferToTexture( source, id, past );
		auto submitted = device->Submit( QueueKind::kGraphics, { &bad.Value(), 1 }, {} );
		s.That( !submitted && submitted.Error().status == DeviceStatus::kInvalidState, "D22",
		    "a region past the mip's edge is refused" );
	}
	(void)device->Release( id, token.value_or( CompletionToken{} ) );
	(void)device->Release( source, token.value_or( CompletionToken{} ) );
	(void)device->Release( out, token.value_or( CompletionToken{} ) );
}

// D37 texture copies: a region copied between two single-sample textures of
// one format lands at the same (x, y) and writes nothing else; a format
// mismatch or a region past the edge is refused.
inline void TextureCopies( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = 8;
	desc.height = 8;
	desc.usages = {
	    ResourceUsage::kSampled, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto from = device->CreateTexture( desc );
	auto to = device->CreateTexture( desc );
	TextureDesc otherDesc = desc;
	otherDesc.format = Format::kRGBA16Float;
	auto other = device->CreateTexture( otherDesc );
	if ( !s.That( from.HasValue() && to.HasValue() && other.HasValue(), "D37",
	         "the copy textures are created" ) )
		return;
	constexpr std::uint32_t kX = 4, kY = 5, kW = 3, kH = 2;
	const std::vector<std::byte> source = Pattern( 8 * 8 * 4, 37 );
	const std::vector<std::byte> target = Pattern( 8 * 8 * 4, 38 );
	const BufferId upload = s.Buffer( *device, 2 * 8 * 8 * 4,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId out = s.Buffer(
	    *device, 8 * 8 * 4, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( upload, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( upload, 0, source );
	e.WriteBuffer( upload, 8 * 8 * 4, target );
	e.TransitionBuffer( upload, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture( from.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.TransitionTexture( to.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( upload, from.Value(), { 0, 0, 0, 8, 8 } );
	e.CopyBufferToTexture( upload, to.Value(), { 8 * 8 * 4, 0, 0, 8, 8 } );
	e.TransitionTexture(
	    from.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	TextureCopy region;
	region.width = kW;
	region.height = kH;
	region.x = kX;
	region.y = kY;
	e.CopyTexture( from.Value(), to.Value(), region );
	e.TransitionTexture( to.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( to.Value(), out, { 0, 0, 0, 8, 8 } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	const bool finished = token && s.Finish( *device, *token );
	const std::vector<std::byte> read = s.ReadBack( *device, out, 8 * 8 * 4 );
	bool placed = finished && read.size() == 8 * 8 * 4;
	for ( std::uint32_t y = 0; placed && y < 8; ++y )
		for ( std::uint32_t x = 0; placed && x < 8; ++x )
		{
			const bool inside = x >= kX && x < kX + kW && y >= kY && y < kY + kH;
			const std::byte *want = inside ? &source[( y * 8 + x ) * 4] : &target[( y * 8 + x ) * 4];
			placed = std::memcmp( &read[( y * 8 + x ) * 4], want, 4 ) == 0;
		}
	s.That( placed, "D37", "a texture copy at (x, y) writes that rectangle of the source alone" );
	auto refused = [&]( TextureId destination, const TextureCopy &copy )
	{
		auto bad = device->BeginEncoder( QueueKind::kGraphics );
		if ( !bad )
			return false;
		bad.Value().TransitionTexture(
		    destination, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		bad.Value().CopyTexture( from.Value(), destination, copy );
		auto submitted = device->Submit( QueueKind::kGraphics, { &bad.Value(), 1 }, {} );
		return !submitted && submitted.Error().status == DeviceStatus::kInvalidState;
	};
	TextureCopy past = region;
	past.x = 6; // 6 + 3 > 8
	s.That( refused( other.Value(), region ), "D37", "a copy between two formats is refused" );
	s.That( refused( to.Value(), past ), "D37", "a region past the mip's edge is refused" );
	const CompletionToken done = token.value_or( CompletionToken{} );
	(void)device->Release( from.Value(), done );
	(void)device->Release( to.Value(), done );
	(void)device->Release( other.Value(), done );
	(void)device->Release( upload, done );
	(void)device->Release( out, done );
}

// D23 timestamps: with kTimestamps, timestamps around work (one inside
// rendering) land in their readback buffer once the submission completes,
// do not decrease in recording order, and a later submission's are no
// earlier; one into device-local memory or at an unaligned offset is
// refused. Without the capability a submission with one fails kUnsupported.
inline void Timestamps( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const bool claimed = device->Facts().capabilities.Has( Capability::kTimestamps );
	s.That( claimed == ( device->Facts().timestampPeriodNs > 0.0 ), "D23",
	    "the timestamp period is set exactly when timestamps are claimed" );
	const BufferId times =
	    s.Buffer( *device, 64, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = 4;
	desc.height = 4;
	desc.usages = { ResourceUsage::kColorAttachment };
	auto texture = device->CreateTexture( desc );
	if ( !s.That( texture.HasValue(), "D23", "an RGBA8 attachment is created" ) )
		return;
	const std::vector<std::byte> unset( 64, std::byte{ 0xff } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( times, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( times, 0, unset );
	e.WriteTimestamp( times, 0 );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	const ColorAttachment colors[] = {
	    { texture.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = 4;
	rendering.height = 4;
	e.BeginRendering( rendering );
	e.WriteTimestamp( times, 8 );
	e.EndRendering();
	e.WriteTimestamp( times, 16 );
	auto first = device->Submit( QueueKind::kGraphics, { &e, 1 }, {} );
	if ( !claimed )
	{
		s.That( !first && first.Error().status == DeviceStatus::kUnsupported, "D23",
		    "without the capability a timestamp fails its submission kUnsupported" );
		(void)device->Release( texture.Value(), CompletionToken{} );
		(void)device->Release( times, CompletionToken{} );
		return;
	}
	s.That( first.HasValue(), "D23", "timestamps around work, one inside rendering, submit" );
	std::optional<CompletionToken> last = first ? std::optional( first.Value() ) : std::nullopt;
	auto second = device->BeginEncoder( QueueKind::kGraphics );
	if ( second )
	{
		second.Value().WriteTimestamp( times, 24 );
		auto submitted = device->Submit( QueueKind::kGraphics, { &second.Value(), 1 }, {} );
		if ( submitted )
			last = submitted.Value();
	}
	std::uint64_t t[4] = {};
	const bool finished = last && s.Finish( *device, *last ) &&
	                      device->ReadBuffer( times, 0, std::as_writable_bytes( std::span( t ) ) );
	const std::uint64_t kUnset = ~std::uint64_t( 0 );
	s.That( finished && t[0] != kUnset && t[1] != kUnset && t[2] != kUnset && t[3] != kUnset, "D23",
	    "every timestamp lands in its buffer" );
	s.That( finished && t[0] <= t[1] && t[1] <= t[2], "D23",
	    "timestamps do not decrease in recording order" );
	s.That( finished && t[2] <= t[3], "D23", "a later submission's timestamp is no earlier" );
	const BufferId local = s.Buffer( *device, 64, { ResourceUsage::kCopyDestination } );
	auto refuse = [&]( BufferId buffer, std::uint64_t offset, const char *what )
	{
		auto bad = device->BeginEncoder( QueueKind::kGraphics );
		if ( !bad )
			return;
		bad.Value().TransitionBuffer(
		    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		bad.Value().WriteTimestamp( buffer, offset );
		auto submitted = device->Submit( QueueKind::kGraphics, { &bad.Value(), 1 }, {} );
		s.That(
		    !submitted && submitted.Error().status == DeviceStatus::kInvalidState, "D23", what );
	};
	refuse( local, 0, "a timestamp into device-local memory is refused" );
	refuse( times, 4, "a timestamp at an unaligned offset is refused" );
	(void)device->Release( texture.Value(), last.value_or( CompletionToken{} ) );
	(void)device->Release( times, last.value_or( CompletionToken{} ) );
	(void)device->Release( local, last.value_or( CompletionToken{} ) );
}

inline void CapabilityHonesty( Suite &s )
{
	if ( !s.m_Driver.rasterizes )
	{
		std::printf( "SKIP %s.D15 the adapter executes no shaders\n", s.m_Driver.name.c_str() );
		return;
	}
	auto device = s.Create();
	if ( !device )
		return;
	const CapabilitySet claimed = device->Facts().capabilities;
	if ( claimed.Has( Capability::kCompute ) || claimed.Has( Capability::kStorageBuffers ) )
		(void)DoublesOn( s, *device, QueueKind::kGraphics,
		    "claimed compute and storage buffers: a dispatch writes its storage buffer" );
	if ( claimed.Has( Capability::kAsyncCompute ) )
		(void)DoublesOn( s, *device, QueueKind::kCompute,
		    "claimed async compute: a compute-queue encoder runs the dispatch" );
	if ( claimed.Has( Capability::kAsyncTransfer ) )
	{
		const std::vector<std::byte> bytes = Pattern( 256, 11 );
		const BufferId source = s.Buffer(
		    *device, 256, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		const BufferId target = s.Buffer(
		    *device, 256, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		auto encoder = device->BeginEncoder( QueueKind::kTransfer );
		bool copied = encoder.HasValue();
		if ( copied )
		{
			CommandEncoder &e = encoder.Value();
			e.TransitionBuffer(
			    source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.WriteBuffer( source, 0, bytes );
			e.TransitionBuffer(
			    source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
			e.TransitionBuffer(
			    target, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.CopyBuffer( source, target, { 0, 0, 256 } );
			e.TransitionBuffer(
			    target, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
			auto token = device->Submit( QueueKind::kTransfer, { &e, 1 }, {} );
			copied = token.HasValue() && s.Finish( *device, token.Value() ) &&
			         s.ReadBack( *device, target, 256 ) == bytes;
		}
		s.That( copied, "D15", "claimed async transfer: a transfer-queue encoder copies a buffer" );
	}
	for ( const Capability without :
	    { Capability::kTransientAliasing, Capability::kParallelRecording, Capability::kRayQuery } )
	{
		if ( claimed.Has( without ) )
		{
			const std::string what = std::string( "claims " ) + CapabilityName( without ) +
			                         ", which no port operation implements yet";
			s.That( false, "D15", what.c_str() );
		}
	}
}

// Raster clauses: work on real pixels and data that every executing adapter
// must get right, beyond the numbered clauses' minimal fixtures. Check names
// are "<driver>.compute", "<driver>.sampled" and "<driver>.multisample".

// Copies a texture in kCopySource out through a buffer and reads it back.
inline std::vector<std::byte> ReadTexture(
    Suite &s, IRenderDevice2 &device, CommandEncoder &e, TextureId texture, std::uint32_t size )
{
	const std::uint64_t bytes = std::uint64_t( size ) * size * 4;
	const BufferId out =
	    s.Buffer( device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyTextureToBuffer( texture, out, { 0, 0, 0, size, size } );
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( device, e );
	if ( !token || !s.Finish( device, *token ) )
		return {};
	return s.ReadBack( device, out, bytes );
}

inline bool Near( std::byte actual, int expected )
{
	const int value = static_cast<int>( actual );
	return value >= expected - 1 && value <= expected + 1;
}

inline bool Raster( Suite &s, bool condition, const char *area, const char *what )
{
	return s.m_Checks.That( condition, s.m_Driver.name + "." + area + " " + what );
}

// Two dispatches on one storage buffer (the second reads what the first
// wrote, in the same usage, so the adapter must order them).
inline void ComputeClauses( Suite &s, IRenderDevice2 &device )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
	{
		std::printf( "SKIP %s.compute the adapter claims no compute\n", s.m_Driver.name.c_str() );
		return;
	}
	constexpr std::uint32_t kCount = 256;
	static const BindingDesc storage[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, storage } );
	if ( !layout )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 3, 0, BindingKind::kStorageBuffer } };
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, device.Facts().artifactFormat,
	    s.Code( shaders::kDoubleCompute ), "main", used } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	desc.layouts = layouts;
	auto pipeline = device.CreatePipeline( desc );
	Raster( s, pipeline.HasValue(), "compute", "a compute pipeline is created" );
	const BufferId data = s.Buffer( device, kCount * 4,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite,
	        ResourceUsage::kCopySource } );
	const BindGroupEntry entry[] = { { 0, data, 0, 0, {}, {} } };
	auto group = device.CreateBindGroup( { layout.Value(), entry } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !pipeline || !group || !encoder )
		return;
	std::vector<std::uint32_t> values( kCount );
	for ( std::uint32_t i = 0; i < kCount; ++i )
		values[i] = i;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( data, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( data, 0, std::as_bytes( std::span<const std::uint32_t>( values ) ) );
	e.TransitionBuffer( data, ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	e.Dispatch( kCount / 64 );
	e.Dispatch( kCount / 64 );
	e.TransitionBuffer( data, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( device, e );
	Raster( s, token && s.Finish( device, *token ), "compute", "two dispatches submit" );
	const std::vector<std::byte> read = s.ReadBack( device, data, kCount * 4 );
	bool ordered = read.size() == kCount * 4;
	for ( std::uint32_t i = 0; ordered && i < kCount; ++i )
	{
		std::uint32_t value = 0;
		std::memcpy( &value, read.data() + i * 4, 4 );
		ordered = value == 4 * i + 3;
	}
	Raster( s, ordered, "compute", "the second dispatch reads the first one's writes" );
}

// A 4x4 texture of `format` holding `texels`, drawn full screen with a point
// sampler into an RGBA8 target: the pixels must equal `expected`.
inline bool SampleTexture( Suite &s, IRenderDevice2 &device, Format format,
    const std::vector<std::byte> &texels, const std::vector<std::byte> &expected, const char *what,
    std::optional<SamplerDesc> sampling = {},
    std::span<const std::uint32_t> fragment = shaders::kSampledFragment,
    const char *clause = "sampled" )
{
	constexpr std::uint32_t kSize = 4;
	static const BindingDesc material[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !layout )
		return false;
	const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
	static const ReflectedBinding used[] = {
	    { 2, 0, BindingKind::kSampledTexture }, { 2, 1, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device.Facts().artifactFormat,
	                                          s.Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device.Facts().artifactFormat, s.Code( fragment ), "main",
	        used } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	auto pipeline = device.CreatePipeline( desc );

	TextureDesc image;
	image.format = format;
	image.width = kSize;
	image.height = kSize;
	image.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto texture = device.CreateTexture( image );
	image.format = Format::kRGBA8Unorm;
	image.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto target = device.CreateTexture( image );
	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( sampling.value_or( point ) );
	Raster( s, pipeline && texture && target && sampler, clause,
	    "a sampling pipeline, texture and sampler are created" );
	if ( !pipeline || !texture || !target || !sampler )
		return false;
	const BindGroupEntry entries[] = {
	    { 0, {}, 0, 0, texture.Value(), {} }, { 1, {}, 0, 0, {}, sampler.Value() } };
	auto group = device.CreateBindGroup( { layout.Value(), entries } );
	const BufferId staging = s.Buffer(
	    device, texels.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !group || !encoder )
		return false;
	CommandEncoder &e = encoder.Value();
	e.TransitionBuffer( staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( staging, 0, texels );
	e.TransitionBuffer( staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( staging, texture.Value(), { 0, 0, 0, kSize, kSize } );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	const ColorAttachment attachment[] = {
	    { target.Value(), LoadOp::kClear, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = attachment;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.Draw( 3 );
	e.EndRendering();
	e.TransitionTexture(
	    target.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	const std::vector<std::byte> pixels = ReadTexture( s, device, e, target.Value(), kSize );
	return Raster( s, pixels == expected, clause, what );
}

inline void ComparisonSampling( Suite &s )
{
	auto owned = s.Create();
	if ( !owned )
		return;
	IRenderDevice2 &device = *owned;
	for ( CompareOp op : { CompareOp::kNever, CompareOp::kLess, CompareOp::kLessEqual,
	          CompareOp::kEqual, CompareOp::kGreaterEqual, CompareOp::kGreater, CompareOp::kAlways,
	          CompareOp::kNotEqual } )
	{
		SamplerDesc desc;
		desc.comparison = op;
		auto sampler = device.CreateSampler( desc );
		s.That( sampler.HasValue(), "D24", "a comparison sampler is created" );
		if ( sampler )
			(void)device.Release( sampler.Value(), {} );
	}
	SamplerDesc invalid;
	invalid.comparison = static_cast<CompareOp>( 255 );
	auto refused = device.CreateSampler( invalid );
	s.That( !refused && refused.Error().status == DeviceStatus::kInvalidDescription, "D24",
	    "an invalid comparison operation fails without a resource" );
	if ( !s.m_Driver.rasterizes )
	{
		std::printf(
		    "SKIP %s.D24 pixels: the adapter executes no shaders\n", s.m_Driver.name.c_str() );
		return;
	}
	// D24: each result is an analytical depth comparison, then a half-texel
	// bilinear average. Testing less and less-equal separately catches a
	// direction/equality error; a coordinate outside the image tests clamping.
	std::vector<float> depth( 16 );
	for ( std::size_t i = 0; i < depth.size(); ++i )
		depth[i] = float( i % 4 + 1 ) * 0.25f;
	const auto bytes = std::as_bytes( std::span( depth ) );
	for ( CompareOp compare : { CompareOp::kLessEqual, CompareOp::kLess, CompareOp::kGreater } )
	{
		SamplerDesc sampling;
		sampling.mipFilter = Filter::kNearest;
		sampling.address = AddressMode::kClampToEdge;
		sampling.comparison = compare;
		std::vector<std::byte> expected( 64 );
		auto lit = [&]( float stored )
		{
			return compare == CompareOp::kLessEqual ? 0.5f <= stored
			       : compare == CompareOp::kLess    ? 0.5f < stored
			                                        : 0.5f > stored;
		};
		for ( std::size_t i = 0; i < depth.size(); ++i )
		{
			const float a = float( lit( depth[i] ) );
			const float b = float( lit( depth[std::min( i + 1, i / 4 * 4 + 3 )] ) );
			expected[i * 4] = std::byte( std::lround( ( a + b ) * 127.5f ) );
			expected[i * 4 + 1] = std::byte( lit( depth[i] ) ? 255 : 0 );
			expected[i * 4 + 2] = std::byte( lit( depth[0] ) ? 255 : 0 );
			expected[i * 4 + 3] = std::byte( 255 );
		}
		(void)SampleTexture( s, device, Format::kD32Float, { bytes.begin(), bytes.end() }, expected,
		    "D24 comparison before filtering, equality and edge clamp", sampling,
		    shaders::kComparisonFragment, "D24" );
	}
	(void)device.WaitIdle();
}

// A texture uploaded through a buffer, sampled with a nearest sampler at
// texel centers: every texel lands on the pixel with the same coordinates
// (row 0 at the top for textures and framebuffers alike).
inline void SampledClauses( Suite &s, IRenderDevice2 &device )
{
	const std::vector<std::byte> texels = Pattern( 4 * 4 * 4, 5 );
	(void)SampleTexture( s, device, Format::kRGBA8Unorm, texels, texels,
	    "texels sampled at their centers land on the same pixels (row 0 on top)" );
	if ( !device.Facts().capabilities.Has( Capability::kTextureCompressionBC ) )
		return;
	// D19: one BC1 block in its three-color mode (color0 <= color1): each row
	// is color0 (blue), color1 (red) and two transparent blacks, which D3D9's
	// DXT1 keeps (one-bit alpha).
	const std::uint8_t block[8] = { 0x1f, 0x00, 0x00, 0xf8, 0xf4, 0xf4, 0xf4, 0xf4 };
	std::vector<std::byte> bc1( 8 );
	std::memcpy( bc1.data(), block, sizeof( block ) );
	static const std::uint8_t row[16] = { 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0 };
	std::vector<std::byte> decoded;
	for ( int y = 0; y < 4; ++y )
	{
		for ( const std::uint8_t value : row )
			decoded.push_back( std::byte( value ) );
	}
	(void)SampleTexture( s, device, Format::kBC1Unorm, bc1, decoded,
	    "D19 a BC1 block decodes as D3D9's DXT1, one-bit alpha kept" );
}

// An indexed draw from a vertex buffer into a 4x multisampled target that
// resolves: a counter-clockwise quad on the left half blends additively over
// the clear color; a clockwise quad on the right half is culled as a back face.
inline void MultisampleClauses( Suite &s, IRenderDevice2 &device )
{
	constexpr std::uint32_t kSize = 8;
	if ( !( device.Facts().limits.sampleCounts & 4u ) )
	{
		std::printf(
		    "SKIP %s.multisample the adapter has no 4x sample count\n", s.m_Driver.name.c_str() );
		return;
	}
	static const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !layout )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device.Facts().artifactFormat,
	                                          s.Code( shaders::kPositionVertex ), "main", {} },
	    { ShaderStage::kFragment, device.Facts().artifactFormat, s.Code( shaders::kColorFragment ),
	        "main", used } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat2, 0, 0 } };
	const VertexBufferLayout buffers[] = { { 8, false } };
	const Format colors[] = { Format::kRGBA8Unorm };
	const BlendMode blends[] = { BlendMode::kAdditive };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.raster = { CullMode::kBack, true };
	desc.sampleCount = 4;
	auto pipeline = device.CreatePipeline( desc );

	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.sampleCount = 4;
	target.usages = { ResourceUsage::kColorAttachment };
	auto multisampled = device.CreateTexture( target );
	target.sampleCount = 1;
	target.usages = { ResourceUsage::kResolveDestination, ResourceUsage::kCopySource };
	auto resolved = device.CreateTexture( target );
	Raster( s, pipeline && multisampled && resolved, "multisample",
	    "a 4x pipeline and its targets are created" );
	if ( !pipeline || !multisampled || !resolved )
		return;

	// Left half counter-clockwise (front), right half clockwise (back), clip Y up.
	const float vertices[] = { -1, -1, 0, -1, 0, 1, -1, 1, 0, -1, 0, 1, 1, 1, 1, -1 };
	const std::uint16_t indices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };
	const float color[4] = { 0.4f, 0.2f, 0.0f, 1.0f };
	const UsageSet upload{ ResourceUsage::kCopyDestination };
	const BufferId vertexBuffer =
	    s.Buffer( device, sizeof( vertices ), UsageSet( upload ).Add( ResourceUsage::kVertex ) );
	const BufferId indexBuffer =
	    s.Buffer( device, sizeof( indices ), UsageSet( upload ).Add( ResourceUsage::kIndex ) );
	const BufferId uniform =
	    s.Buffer( device, 256, UsageSet( upload ).Add( ResourceUsage::kUniform ) );
	const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
	auto group = device.CreateBindGroup( { layout.Value(), entry } );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !group || !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	auto fill = [&]( BufferId buffer, std::span<const std::byte> bytes, ResourceUsage usage )
	{
		e.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( buffer, 0, bytes );
		e.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, usage );
	};
	fill( vertexBuffer, std::as_bytes( std::span( vertices ) ), ResourceUsage::kVertex );
	fill( indexBuffer, std::as_bytes( std::span( indices ) ), ResourceUsage::kIndex );
	fill( uniform, std::as_bytes( std::span( color ) ), ResourceUsage::kUniform );
	e.TransitionTexture(
	    multisampled.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	e.TransitionTexture(
	    resolved.Value(), ResourceUsage::kUndefined, ResourceUsage::kResolveDestination );
	const ColorAttachment attachment[] = { { multisampled.Value(), LoadOp::kClear,
	    StoreOp::kDiscard, { 0.2f, 0.0f, 0.0f, 1.0f }, resolved.Value() } };
	RenderingDesc rendering;
	rendering.colors = attachment;
	rendering.width = kSize;
	rendering.height = kSize;
	e.BeginRendering( rendering );
	e.SetPipeline( pipeline.Value() );
	e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
	e.SetVertexBuffer( 0, vertexBuffer );
	e.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint16 );
	e.DrawIndexed( 12 );
	e.EndRendering();
	e.TransitionTexture(
	    resolved.Value(), ResourceUsage::kResolveDestination, ResourceUsage::kCopySource );
	const std::vector<std::byte> pixels = ReadTexture( s, device, e, resolved.Value(), kSize );
	bool left = pixels.size() == kSize * kSize * 4;
	bool right = left;
	for ( std::uint32_t y = 0; left && y < kSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kSize; ++x )
		{
			const std::byte *p = pixels.data() + ( y * kSize + x ) * 4;
			if ( x < kSize / 2 )
				left &=
				    Near( p[0], 153 ) && Near( p[1], 51 ) && Near( p[2], 0 ) && Near( p[3], 255 );
			else
				right &=
				    Near( p[0], 51 ) && Near( p[1], 0 ) && Near( p[2], 0 ) && Near( p[3], 255 );
		}
	}
	Raster( s, left, "multisample",
	    "the front-facing quad blends additively and resolves on the left" );
	Raster( s, right, "multisample", "the back-facing quad is culled (CCW front, Y up)" );
}

// D38 line fill: without kFillModeLines a pipeline with RasterState::fill
// kLines fails kUnsupported. With it, the suite's full-screen triangle (whose
// edges lie on or past the target's border) drawn as lines leaves the
// interior at the clear color, where the same pipeline filled covers it.
inline void FillModeLines( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	BindGroupLayoutId layouts[kMaxBindGroups];
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		auto layout = device->CreateBindGroupLayout( { static_cast<BindGroupRole>( role ), {} } );
		if ( !s.That( layout.HasValue(), "D38", "an empty layout is created" ) )
			return;
		layouts[role] = layout.Value();
	}
	const Format colors[] = { Format::kRGBA8Unorm };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          s.Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat,
	        s.Code( shaders::kConstantFragment ), "main", {}, 16 } };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	desc.drawConstantBytes = 16;
	PipelineDesc lines = desc;
	lines.raster.fill = FillMode::kLines;
	if ( !device->Facts().capabilities.Has( Capability::kFillModeLines ) )
	{
		auto refused = device->CreatePipeline( lines );
		s.That( !refused && refused.Error().status == DeviceStatus::kUnsupported, "D38",
		    "without kFillModeLines a line-fill pipeline fails kUnsupported" );
		return;
	}
	auto solid = device->CreatePipeline( desc );
	auto wire = device->CreatePipeline( lines );
	if ( !s.That( solid.HasValue() && wire.HasValue(), "D38",
	         "filled and line-fill pipelines are created" ) )
		return;
	constexpr std::uint32_t kSize = 8;
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	const std::uint64_t outBytes = std::uint64_t( kSize ) * kSize * 4;
	auto draw = [&]( PipelineId pipeline ) -> std::vector<std::byte>
	{
		auto color = device->CreateTexture( target );
		if ( !color )
			return {};
		const BufferId out = s.Buffer(
		    *device, outBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return {};
		CommandEncoder &e = encoder.Value();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		const ColorAttachment attachments[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 1 }, {} } };
		RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.width = kSize;
		rendering.height = kSize;
		e.BeginRendering( rendering );
		e.SetViewport( { 0, 0, float( kSize ), float( kSize ), 0, 1 } );
		e.SetPipeline( pipeline );
		const float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		e.SetDrawConstants( 0, std::as_bytes( std::span( red ) ) );
		e.Draw( 3 );
		e.EndRendering();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( color.Value(), out, { 0, 0, 0, kSize, kSize } );
		e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		const std::optional<CompletionToken> token = s.Run( *device, e );
		if ( !token || !s.m_Driver.rasterizes || !s.Finish( *device, *token ) )
			return {};
		return s.ReadBack( *device, out, outBytes );
	};
	const std::vector<std::byte> filled = draw( solid.Value() );
	const std::vector<std::byte> edges = draw( wire.Value() );
	if ( !s.m_Driver.rasterizes )
	{
		std::printf( "SKIP %s.D38 pixels: the adapter does not rasterize\n", s.m_Driver.name.c_str() );
		return;
	}
	// The center pixel (4, 4): red when filled, the blue clear as lines.
	const std::size_t center = ( 4 * kSize + 4 ) * 4;
	s.That( filled.size() == outBytes && filled[center] == std::byte{ 255 } &&
	            filled[center + 2] == std::byte{ 0 },
	    "D38", "the filled triangle covers the center" );
	s.That( edges.size() == outBytes && edges[center] == std::byte{ 0 } &&
	            edges[center + 2] == std::byte{ 255 },
	    "D38", "the line-fill triangle leaves its interior at the clear color" );
}

// D36 cube arrays: without kCubeArrays a kCube texture of more than six layers
// fails kUnsupported, and a cube whose layers are not a multiple of six fails
// kInvalidDescription. With it, a two-cube array (12 faces of 4x4 RGBA8, two
// mips) takes every face by layer index: distinct bytes copied in face by face
// and back read unchanged, so no layer aliases another.
inline void CubeArrays( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	TextureDesc desc;
	desc.dimension = TextureDimension::kCube;
	desc.format = Format::kRGBA8Unorm;
	desc.width = desc.height = 4;
	desc.depthOrLayers = 12;
	desc.mipLevels = 2;
	desc.usages = {
	    ResourceUsage::kSampled, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	TextureDesc odd = desc;
	odd.depthOrLayers = 7;
	auto bad = device->CreateTexture( odd );
	s.That( !bad && bad.Error().status == DeviceStatus::kInvalidDescription, "D36",
	    "a cube of seven layers fails kInvalidDescription" );
	if ( !device->Facts().capabilities.Has( Capability::kCubeArrays ) )
	{
		auto made = device->CreateTexture( desc );
		s.That( !made && made.Error().status == DeviceStatus::kUnsupported, "D36",
		    "without kCubeArrays a cube array fails kUnsupported" );
		return;
	}
	auto texture = device->CreateTexture( desc );
	if ( !s.That( texture.HasValue(), "D36", "a cube array is created" ) )
		return;
	const std::uint64_t face0 = 4 * 4 * 4, face1 = 2 * 2 * 4;
	const std::uint64_t perMip0 = 12 * face0, bytes = perMip0 + 12 * face1;
	const std::vector<std::byte> pattern = Pattern( bytes, 32 );
	const BufferId source =
	    s.Buffer( *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId out =
	    s.Buffer( *device, bytes, { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	auto encoder = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return;
	CommandEncoder &e = encoder.Value();
	const TextureId id = texture.Value();
	e.TransitionBuffer( source, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( source, 0, pattern );
	e.TransitionBuffer( source, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture( id, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	for ( std::uint32_t layer = 0; layer < 12; ++layer )
	{
		e.CopyBufferToTexture( source, id, { layer * face0, 0, layer, 4, 4 } );
		e.CopyBufferToTexture( source, id, { perMip0 + layer * face1, 1, layer, 2, 2 } );
	}
	e.TransitionTexture( id, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionBuffer( out, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	for ( std::uint32_t layer = 0; layer < 12; ++layer )
	{
		e.CopyTextureToBuffer( id, out, { layer * face0, 0, layer, 4, 4 } );
		e.CopyTextureToBuffer( id, out, { perMip0 + layer * face1, 1, layer, 2, 2 } );
	}
	e.TransitionBuffer( out, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	const std::optional<CompletionToken> token = s.Run( *device, e );
	const bool finished = token && s.Finish( *device, *token );
	s.That( finished && s.ReadBack( *device, out, bytes ) == pattern, "D36",
	    "all twelve faces of both mips copy in and back unchanged, by layer" );
	(void)device->Release( id, token.value_or( CompletionToken{} ) );
	(void)device->Release( source, token.value_or( CompletionToken{} ) );
	(void)device->Release( out, token.value_or( CompletionToken{} ) );
}

// D30 multi-draw indirect and D31 indirect count. Every adapter: a claimed
// capability accepts well-formed calls, and refuses records outside the
// buffer, a short stride, a buffer not in kIndirect and (D31) a count outside
// its buffer with kInvalidState; an unclaimed one fails the submission
// kUnsupported. Adapters that rasterize also check pixels: two records draw
// both halves of the target, a count of 1 of at most 2 draws the left half,
// and a draw count of zero draws nothing.
inline void IndirectDraws( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	const CapabilitySet claimed = device->Facts().capabilities;
	const bool multi = claimed.Has( Capability::kMultiDrawIndirect );
	const bool counted = claimed.Has( Capability::kDrawIndirectCount );
	static const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	auto layout = device->CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !s.That( layout.HasValue(), "D30", "a material layout is created" ) )
		return;
	const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          s.Code( shaders::kPositionVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat,
	        s.Code( shaders::kColorFragment ), "main", used } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat2, 0, 0 } };
	const VertexBufferLayout buffers[] = { { 8, false } };
	const Format colors[] = { Format::kRGBA8Unorm };
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.colorFormats = colors;
	desc.raster.cull = CullMode::kNone;
	auto pipeline = device->CreatePipeline( desc );
	constexpr std::uint32_t kSize = 8;
	TextureDesc target;
	target.format = Format::kRGBA8Unorm;
	target.width = kSize;
	target.height = kSize;
	target.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	auto color = device->CreateTexture( target );
	if ( !s.That( pipeline.HasValue() && color.HasValue(), "D30",
	         "the pipeline and its target are created" ) )
		return;

	// The left half (indices 0-5) and the right half (6-11).
	const float vertices[] = { -1, -1, 0, -1, 0, 1, -1, 1, 0, -1, 1, -1, 1, 1, 0, 1 };
	const std::uint16_t indices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };
	const DrawIndexedIndirectCommand records[] = { { 6, 1, 0, 0, 0 }, { 6, 1, 6, 0, 0 } };
	const float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	const UsageSet upload{ ResourceUsage::kCopyDestination };
	const BufferId vertexBuffer =
	    s.Buffer( *device, sizeof( vertices ), UsageSet( upload ).Add( ResourceUsage::kVertex ) );
	const BufferId indexBuffer =
	    s.Buffer( *device, sizeof( indices ), UsageSet( upload ).Add( ResourceUsage::kIndex ) );
	const BufferId uniform =
	    s.Buffer( *device, 256, UsageSet( upload ).Add( ResourceUsage::kUniform ) );
	const BufferId indirect =
	    s.Buffer( *device, sizeof( records ), UsageSet( upload ).Add( ResourceUsage::kIndirect ) );
	const BufferId count = s.Buffer( *device, 4, UsageSet( upload ).Add( ResourceUsage::kIndirect ) );
	const BindGroupEntry entry[] = { { 0, uniform, 0, 16, {}, {} } };
	auto group = device->CreateBindGroup( { layout.Value(), entry } );
	if ( !s.That( group.HasValue(), "D30", "the material group is created" ) )
		return;

	// Records one submission: uploads (the indirect buffer is left in
	// kCopyDestination when `indirectReady` is false), one pass with `draw`,
	// and optionally the target's readback. Returns the submission's status
	// (kInternal for success) and the pixels.
	auto run = [&]( std::uint32_t drawCountValue, bool indirectReady,
	               const std::function<void( CommandEncoder & )> &draw,
	               std::vector<std::byte> *pixels ) -> DeviceStatus
	{
		auto encoder = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return DeviceStatus::kUnavailable;
		CommandEncoder &e = encoder.Value();
		auto fill = [&]( BufferId buffer, std::span<const std::byte> bytes, ResourceUsage usage )
		{
			e.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.WriteBuffer( buffer, 0, bytes );
			if ( usage != ResourceUsage::kCopyDestination )
				e.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, usage );
		};
		fill( vertexBuffer, std::as_bytes( std::span( vertices ) ), ResourceUsage::kVertex );
		fill( indexBuffer, std::as_bytes( std::span( indices ) ), ResourceUsage::kIndex );
		fill( uniform, std::as_bytes( std::span( red ) ), ResourceUsage::kUniform );
		fill( indirect, std::as_bytes( std::span( records ) ),
		    indirectReady ? ResourceUsage::kIndirect : ResourceUsage::kCopyDestination );
		fill( count, std::as_bytes( std::span( &drawCountValue, 1 ) ), ResourceUsage::kIndirect );
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		const ColorAttachment attachments[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
		RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.width = kSize;
		rendering.height = kSize;
		e.BeginRendering( rendering );
		e.SetPipeline( pipeline.Value() );
		e.SetBindGroup( BindGroupRole::kMaterial, group.Value() );
		e.SetVertexBuffer( 0, vertexBuffer );
		e.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint16 );
		draw( e );
		e.EndRendering();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		if ( pixels )
		{
			*pixels = ReadTexture( s, *device, e, color.Value(), kSize );
			return pixels->empty() ? DeviceStatus::kInvalidState : DeviceStatus::kInternal;
		}
		auto token = device->Submit( QueueKind::kGraphics, { &e, 1 }, {} );
		if ( !token )
			return token.Error().status;
		return s.Finish( *device, token.Value() ) ? DeviceStatus::kInternal
		                                          : DeviceStatus::kUnavailable;
	};
	auto direct = [&]( std::uint32_t n, std::uint32_t stride )
	{
		return [=]( CommandEncoder &e )
		{
			e.DrawIndexedIndirect( indirect, 0, n, stride );
		};
	};
	auto viaCount = [&]( std::uint64_t countOffset )
	{
		return [=]( CommandEncoder &e )
		{
			e.DrawIndexedIndirectCount( indirect, 0, count, countOffset, 2, 20 );
		};
	};
	constexpr DeviceStatus kOk = DeviceStatus::kInternal;

	if ( multi )
	{
		s.That( run( 0, true, direct( 2, 20 ), nullptr ) == kOk, "D30",
		    "two records in kIndirect submit" );
		s.That( run( 0, true, direct( 0, 20 ), nullptr ) == kOk, "D30",
		    "a draw count of zero submits" );
		s.That( run( 0, true, direct( 3, 20 ), nullptr ) == DeviceStatus::kInvalidState, "D30",
		    "records past the buffer's end fail kInvalidState" );
		s.That( run( 0, true, direct( 1, 16 ), nullptr ) == DeviceStatus::kInvalidState, "D30",
		    "a stride under 20 fails kInvalidState" );
		s.That( run( 0, false, direct( 1, 20 ), nullptr ) == DeviceStatus::kInvalidState, "D30",
		    "an indirect buffer not in kIndirect fails kInvalidState" );
	}
	else
		s.That( run( 0, true, direct( 1, 20 ), nullptr ) == DeviceStatus::kUnsupported, "D30",
		    "without the capability an indirect draw fails its submission kUnsupported" );
	if ( counted )
	{
		s.That( run( 1, true, viaCount( 0 ), nullptr ) == kOk, "D31",
		    "an indirect draw with a count in kIndirect submits" );
		s.That( run( 1, true, viaCount( 4 ), nullptr ) == DeviceStatus::kInvalidState, "D31",
		    "a count outside its buffer fails kInvalidState" );
	}
	else
		s.That( run( 1, true, viaCount( 0 ), nullptr ) == DeviceStatus::kUnsupported, "D31",
		    "without the capability a counted indirect draw fails kUnsupported" );

	if ( !s.m_Driver.rasterizes )
		return;
	auto halves = [&]( const std::vector<std::byte> &pixels, bool left, bool right )
	{
		if ( pixels.size() != kSize * kSize * 4 )
			return false;
		bool ok = true;
		for ( std::uint32_t y = 0; y < kSize; ++y )
		{
			for ( std::uint32_t x = 0; x < kSize; ++x )
			{
				const bool lit = x < kSize / 2 ? left : right;
				ok = ok && Near( pixels[( y * kSize + x ) * 4], lit ? 255 : 0 );
			}
		}
		return ok;
	};
	std::vector<std::byte> pixels;
	if ( multi )
	{
		(void)run( 0, true, direct( 2, 20 ), &pixels );
		s.That( halves( pixels, true, true ), "D30", "two records draw both halves" );
		(void)run( 0, true, direct( 0, 20 ), &pixels );
		s.That( halves( pixels, false, false ), "D30", "a draw count of zero draws nothing" );
	}
	if ( counted )
	{
		(void)run( 1, true, viaCount( 0 ), &pixels );
		s.That( halves( pixels, true, false ), "D31",
		    "a GPU count of 1 of at most 2 draws only the first record" );
	}
	(void)device->WaitIdle();
}

} // namespace detail

// The raster clauses (compute, sampling, multisampling and facing) on one
// device of an adapter that executes work.
inline void RunRasterConformance( testing::Checks &checks, const DeviceDriver &driver )
{
	detail::Suite suite( checks, driver );
	std::unique_ptr<IRenderDevice2> device = suite.Create();
	if ( !device )
		return;
	detail::ComputeClauses( suite, *device );
	detail::SampledClauses( suite, *device );
	detail::MultisampleClauses( suite, *device );
	(void)device->WaitIdle();
	(void)device->Poll();
}

inline void RunDeviceConformance( testing::Checks &checks, const DeviceDriver &driver )
{
	detail::Suite suite( checks, driver );
	detail::Facts( suite );
	detail::InvalidDescriptions( suite );
	detail::BindGroupLimit( suite );
	detail::PipelineLayouts( suite );
	detail::Release( suite );
	detail::Order( suite );
	detail::Epochs( suite );
	detail::EncoderOrderAndErrors( suite );
	detail::DataLands( suite );
	detail::InitializedUploads( suite );
	detail::UploadReuse( suite );
	detail::EncoderThreads( suite );
	detail::UsageStates( suite );
	detail::ConcurrentEncoders( suite );
	detail::Conventions( suite );
	detail::DrawConstants( suite );
	detail::ColorWriteMasks( suite );
	detail::SpecializationConstants( suite );
	detail::ColorBlend( suite );
	detail::ColorBlend( suite, true );
	detail::ExternalImagesClause( suite );
	detail::BlockCompressedFormats( suite );
	detail::PackedFloatTargets( suite );
	detail::RegionCopies( suite );
	detail::TextureCopies( suite );
	detail::Timestamps( suite );
	detail::IndirectDraws( suite );
	detail::CubeArrays( suite );
	detail::FillModeLines( suite );
	detail::ComparisonSampling( suite );
	detail::CapabilityHonesty( suite );
}

} // namespace rendertest

#endif // RENDERTEST_CORE_DEVICE_CONFORMANCE_H
