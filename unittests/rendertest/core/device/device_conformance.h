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

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <vector>

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
	// Draws produce pixels, so the conventions section runs.
	bool rasterizes = false;
	// D15: a compute artifact in the adapter's format (layout( local_size_x =
	// 64 ), set 3 binding 0 a storage buffer of uints, values[i] =
	// values[i] * 2 + 1), so claimed compute capabilities can be exercised.
	std::span<const std::uint32_t> doubleCompute;
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
	    Code( shaders::kFullScreenVertex ), "main", {} } };
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
    IRenderDevice2 &device, std::span<const std::uint32_t> vertex, bool depth )
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
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device.Facts().artifactFormat,
	                                          Code( vertex.data(), vertex.size() ), "main", {} },
	    { ShaderStage::kFragment, device.Facts().artifactFormat, Code( shaders::kColorFragment ),
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
	const ColorPipeline good = MakeColorPipeline( *device, shaders::kFullScreenVertex, false );
	s.That( good.ok, "D4", "a pipeline whose reflected bindings are in its layouts is created" );

	const std::size_t live = device->LiveResourceCount();
	auto frame = device->CreateBindGroupLayout( { BindGroupRole::kFrame, {} } );
	const BindGroupLayoutId onlyFrame[] = { frame ? frame.Value() : BindGroupLayoutId{} };
	static const ReflectedBinding used[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, device->Facts().artifactFormat,
	                                          Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat, Code( shaders::kColorFragment ),
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

inline void UploadReuse( Suite &s )
{
	auto device = s.Create();
	if ( !device )
		return;
	// More upload bytes in flight than a small ring holds, all submitted
	// before any completes. A range reused early corrupts an earlier write.
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
	const ColorPipeline pipeline = MakeColorPipeline( device, vertex, true );
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
		stages[0] = { ShaderStage::kVertex, device->Facts().artifactFormat,
		    Code( vertex.data(), vertex.size() ), "main", {} };
		stages[1] = { ShaderStage::kFragment, device->Facts().artifactFormat,
		    Code( shaders::kConstantFragment ), "main", {}, reflected };
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
	                                          Code( shaders::kFullScreenVertex ), "main", {} },
	    { ShaderStage::kFragment, device->Facts().artifactFormat,
	        Code( shaders::kConstantFragment ), "main", {}, 16 } };
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
	    Code( s.m_Driver.doubleCompute.data(), s.m_Driver.doubleCompute.size() ), "main", used } };
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

} // namespace detail

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
	detail::UploadReuse( suite );
	detail::EncoderThreads( suite );
	detail::UsageStates( suite );
	detail::ConcurrentEncoders( suite );
	detail::Conventions( suite );
	detail::DrawConstants( suite );
	detail::ColorWriteMasks( suite );
	detail::CapabilityHonesty( suite );
}

} // namespace rendertest

#endif // RENDERTEST_CORE_DEVICE_CONFORMANCE_H
