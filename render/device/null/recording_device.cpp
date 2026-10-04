//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.null (RFC 0016): the recording adapter of
//			render.device.v2. See public/render/device/null/provider.h.
//
//			Model: Submit validates every encoder against the device's usage
//			state as of the previous submission, then queues the batch. The
//			batch runs (its data effects apply and its commands are recorded)
//			when its token completes. Uploads are copied into a ring at record
//			time and read from it when the batch runs, so a ring range reused
//			before its token completes corrupts the destination, as on a GPU.
//
//			Resource state is tracked per resource, not per subresource.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/device/validation.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace render::device::null
{

namespace
{

constexpr std::size_t kQueueCount = static_cast<std::size_t>( QueueKind::kCount );

foundation::Unexpected<DeviceError> Fail( DeviceStatus status, DeviceOperation operation )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, 0 } );
}

std::uint16_t ToHalf( float value )
{
	const std::uint32_t bits = std::bit_cast<std::uint32_t>( value );
	const std::uint32_t sign = ( bits >> 16 ) & 0x8000u;
	const std::int32_t exponent = static_cast<std::int32_t>( ( bits >> 23 ) & 0xffu ) - 127 + 15;
	std::uint32_t mantissa = bits & 0x7fffffu;
	if ( exponent <= 0 )
		return static_cast<std::uint16_t>( sign );
	if ( exponent >= 31 )
		return static_cast<std::uint16_t>( sign | 0x7c00u );
	mantissa += 0x1000u; // round to nearest
	if ( mantissa & 0x800000u )
		return static_cast<std::uint16_t>( sign | ( ( exponent + 1 ) << 10 ) );
	return static_cast<std::uint16_t>( sign | ( exponent << 10 ) | ( mantissa >> 13 ) );
}

std::uint8_t Unorm8( float value )
{
	const float clamped = std::clamp( value, 0.0f, 1.0f );
	return static_cast<std::uint8_t>( std::lround( clamped * 255.0f ) );
}

float LinearToSrgb( float value )
{
	const float c = std::clamp( value, 0.0f, 1.0f );
	return c <= 0.0031308f ? c * 12.92f : 1.055f * std::pow( c, 1.0f / 2.4f ) - 0.055f;
}

// The bytes one texel of format holds for color.
std::vector<std::byte> EncodeTexel( Format format, const ClearColor &color )
{
	std::vector<std::byte> out( BytesPerTexel( format ) );
	auto put8 = [&]( std::size_t index, std::uint8_t value )
	{
		out[index] = static_cast<std::byte>( value );
	};
	auto putBytes = [&]( std::size_t offset, const void *data, std::size_t size )
	{
		std::memcpy( out.data() + offset, data, size );
	};
	switch ( format )
	{
	case Format::kR8Unorm:
		put8( 0, Unorm8( color.r ) );
		break;
	case Format::kRGBA8Unorm:
		put8( 0, Unorm8( color.r ) );
		put8( 1, Unorm8( color.g ) );
		put8( 2, Unorm8( color.b ) );
		put8( 3, Unorm8( color.a ) );
		break;
	case Format::kRGBA8Srgb:
		put8( 0, Unorm8( LinearToSrgb( color.r ) ) );
		put8( 1, Unorm8( LinearToSrgb( color.g ) ) );
		put8( 2, Unorm8( LinearToSrgb( color.b ) ) );
		put8( 3, Unorm8( color.a ) );
		break;
	case Format::kBGRA8Unorm:
		put8( 0, Unorm8( color.b ) );
		put8( 1, Unorm8( color.g ) );
		put8( 2, Unorm8( color.r ) );
		put8( 3, Unorm8( color.a ) );
		break;
	case Format::kBGRA8Srgb:
		put8( 0, Unorm8( LinearToSrgb( color.b ) ) );
		put8( 1, Unorm8( LinearToSrgb( color.g ) ) );
		put8( 2, Unorm8( LinearToSrgb( color.r ) ) );
		put8( 3, Unorm8( color.a ) );
		break;
	case Format::kRG16Float:
	{
		const std::uint16_t h[2] = { ToHalf( color.r ), ToHalf( color.g ) };
		putBytes( 0, h, sizeof( h ) );
		break;
	}
	case Format::kRGB10A2Unorm:
	{
		const auto pack = []( float value, unsigned maximum )
		{
			return static_cast<std::uint32_t>(
			    std::lround( std::clamp( value, 0.0f, 1.0f ) * maximum ) );
		};
		const std::uint32_t packed = pack( color.r, 1023 ) | ( pack( color.g, 1023 ) << 10 ) |
		                             ( pack( color.b, 1023 ) << 20 ) | ( pack( color.a, 3 ) << 30 );
		putBytes( 0, &packed, sizeof( packed ) );
		break;
	}
	case Format::kRGBA16Float:
	{
		const std::uint16_t h[4] = {
		    ToHalf( color.r ), ToHalf( color.g ), ToHalf( color.b ), ToHalf( color.a ) };
		putBytes( 0, h, sizeof( h ) );
		break;
	}
	case Format::kRGBA16Unorm:
	{
		const auto unorm16 = []( float value )
		{
			return static_cast<std::uint16_t>(
			    std::lround( std::clamp( value, 0.0f, 1.0f ) * 65535.0f ) );
		};
		const std::uint16_t u[4] = {
		    unorm16( color.r ), unorm16( color.g ), unorm16( color.b ), unorm16( color.a ) };
		putBytes( 0, u, sizeof( u ) );
		break;
	}
	case Format::kR32Float:
	case Format::kD32Float:
	case Format::kD32FloatS8:
		putBytes( 0, &color.r, sizeof( float ) );
		break;
	case Format::kRGBA32Float:
	{
		const float f[4] = { color.r, color.g, color.b, color.a };
		putBytes( 0, f, sizeof( f ) );
		break;
	}
	case Format::kD24UnormS8:
	{
		const std::uint32_t depth = static_cast<std::uint32_t>(
		    std::lround( std::clamp( color.r, 0.0f, 1.0f ) * 16777215.0f ) );
		putBytes( 0, &depth, sizeof( depth ) );
		break;
	}
	case Format::kBC1Unorm: // never cleared (D19)
	case Format::kBC1Srgb:
	case Format::kBC2Unorm:
	case Format::kBC2Srgb:
	case Format::kBC3Unorm:
	case Format::kBC3Srgb:
	case Format::kBC4Unorm:
	case Format::kBC5Unorm:
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return out;
}

struct Buffer
{
	BufferDesc desc;
	std::vector<std::byte> data;
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;
};

struct Texture
{
	TextureDesc desc;
	std::uint32_t layers = 1;
	std::vector<std::vector<std::byte>> subresources; // mip * layers + layer
	ResourceUsage usage = ResourceUsage::kUndefined;
	bool released = false;

	std::uint32_t Width( std::uint32_t mip ) const { return std::max( 1u, desc.width >> mip ); }
	std::uint32_t Height( std::uint32_t mip ) const { return std::max( 1u, desc.height >> mip ); }
	std::uint32_t Depth( std::uint32_t mip ) const
	{
		return desc.dimension == TextureDimension::k3D ? std::max( 1u, desc.depthOrLayers >> mip )
		                                               : 1u;
	}

	std::vector<std::byte> &Subresource( std::uint32_t mip, std::uint32_t layer )
	{
		if ( subresources.empty() )
		{
			for ( std::uint32_t m = 0; m < desc.mipLevels; ++m )
			{
				for ( std::uint32_t l = 0; l < layers; ++l )
					subresources.emplace_back( static_cast<std::size_t>( RegionBytes(
					                               desc.format, Width( m ), Height( m ) ) ) *
					                               Depth( m ),
					    std::byte{ 0 } );
			}
		}
		return subresources[mip * layers + layer];
	}
};

struct Layout
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::vector<BindingDesc> bindings;
	bool released = false;
};

struct Simple
{
	bool released = false;
	std::uint32_t drawConstantBytes = 0; // pipelines (D16)
};

struct Command
{
	RecordedOp op = RecordedOp::kDraw;
	std::uint64_t a = 0;
	std::uint64_t b = 0;
	ResourceUsage before = ResourceUsage::kUndefined;
	ResourceUsage after = ResourceUsage::kUndefined;
	SubresourceRange range;
	ClearColor color;
	BufferCopy copy;
	TextureBufferCopy textureCopy;
	std::uint64_t count = 0;
	std::uint64_t ringOffset = 0;
	std::vector<std::byte> deferred; // an upload that found the ring full
	bool fromRing = false;
	std::vector<ColorAttachment> colors;
	std::optional<DepthAttachment> depth;
	std::uint32_t slot = 0;
};

class RecordingDevice;

class NullEncoder final : public IEncoderBackend
{
public:
	NullEncoder( RecordingDevice &device, QueueKind queue ) : m_Device( device ), m_Queue( queue )
	{
	}
	~NullEncoder() override;

	void TransitionTexture( TextureId texture, ResourceUsage before, ResourceUsage after,
	    const SubresourceRange &range ) override
	{
		Command command;
		command.op = RecordedOp::kTransitionTexture;
		command.a = texture.value;
		command.before = before;
		command.after = after;
		command.range = range;
		Push( std::move( command ) );
	}
	void TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after ) override
	{
		Command command;
		command.op = RecordedOp::kTransitionBuffer;
		command.a = buffer.value;
		command.before = before;
		command.after = after;
		Push( std::move( command ) );
	}
	void ClearTexture(
	    TextureId texture, const ClearColor &color, const SubresourceRange &range ) override
	{
		NotRendering();
		Command command;
		command.op = RecordedOp::kClearTexture;
		command.a = texture.value;
		command.color = color;
		command.range = range;
		Push( std::move( command ) );
	}
	void WriteBuffer(
	    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes ) override;
	void CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy ) override
	{
		NotRendering();
		Command command;
		command.op = RecordedOp::kCopyBuffer;
		command.a = source.value;
		command.b = destination.value;
		command.copy = copy;
		command.count = copy.size;
		Push( std::move( command ) );
	}
	void CopyTextureToBuffer(
	    TextureId source, BufferId destination, const TextureBufferCopy &copy ) override
	{
		NotRendering();
		Command command;
		command.op = RecordedOp::kCopyTextureToBuffer;
		command.a = source.value;
		command.b = destination.value;
		command.textureCopy = copy;
		Push( std::move( command ) );
	}
	void CopyBufferToTexture(
	    BufferId source, TextureId destination, const TextureBufferCopy &copy ) override
	{
		NotRendering();
		Command command;
		command.op = RecordedOp::kCopyBufferToTexture;
		command.a = source.value;
		command.b = destination.value;
		command.textureCopy = copy;
		Push( std::move( command ) );
	}
	void BeginRendering( const RenderingDesc &desc ) override
	{
		NotRendering();
		m_Rendering = true;
		Command command;
		command.op = RecordedOp::kBeginRendering;
		command.colors.assign( desc.colors.begin(), desc.colors.end() );
		command.depth = desc.depth;
		command.a = desc.colors.empty() ? ( desc.depth ? desc.depth->texture.value : 0 )
		                                : desc.colors[0].texture.value;
		if ( desc.width == 0 || desc.height == 0 || ( desc.colors.empty() && !desc.depth ) )
			m_Error = true;
		Push( std::move( command ) );
	}
	void EndRendering() override
	{
		if ( !m_Rendering )
			m_Error = true;
		m_Rendering = false;
		Command command;
		command.op = RecordedOp::kEndRendering;
		Push( std::move( command ) );
	}
	void SetPipeline( PipelineId pipeline ) override
	{
		Command command;
		command.op = RecordedOp::kSetPipeline;
		command.a = pipeline.value;
		Push( std::move( command ) );
	}
	void SetBindGroup( BindGroupRole role, BindGroupId group ) override
	{
		Command command;
		command.op = RecordedOp::kSetBindGroup;
		command.a = group.value;
		command.slot = static_cast<std::uint32_t>( role );
		if ( command.slot >= kMaxBindGroups )
			m_Error = true;
		Push( std::move( command ) );
	}
	void SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset ) override
	{
		Command command;
		command.op = RecordedOp::kSetVertexBuffer;
		command.a = buffer.value;
		command.slot = slot;
		command.count = offset;
		Push( std::move( command ) );
	}
	void SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat ) override
	{
		Command command;
		command.op = RecordedOp::kSetIndexBuffer;
		command.a = buffer.value;
		command.count = offset;
		Push( std::move( command ) );
	}
	void SetViewport( const Viewport & ) override
	{
		Command command;
		command.op = RecordedOp::kSetViewport;
		Push( std::move( command ) );
	}
	void Draw( std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t,
	    std::uint32_t ) override
	{
		Drawing();
		Command command;
		command.op = RecordedOp::kDraw;
		command.count = static_cast<std::uint64_t>( vertexCount ) * instanceCount;
		Push( std::move( command ) );
	}
	void DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t,
	    std::int32_t, std::uint32_t ) override
	{
		Drawing();
		Command command;
		command.op = RecordedOp::kDrawIndexed;
		command.count = static_cast<std::uint64_t>( indexCount ) * instanceCount;
		Push( std::move( command ) );
	}
	void Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z ) override
	{
		NotRendering();
		Command command;
		command.op = RecordedOp::kDispatch;
		command.count = static_cast<std::uint64_t>( x ) * y * z;
		Push( std::move( command ) );
	}
	void SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes ) override
	{
		Command command;
		command.op = RecordedOp::kSetDrawConstants;
		command.slot = offset;
		command.count = bytes.size();
		command.deferred.assign( bytes.begin(), bytes.end() );
		Push( std::move( command ) );
	}
	void BeginLabel( std::string_view ) override
	{
		++m_Labels;
		Command command;
		command.op = RecordedOp::kBeginLabel;
		Push( std::move( command ) );
	}
	void EndLabel() override
	{
		if ( m_Labels == 0 )
			m_Error = true;
		else
			--m_Labels;
		Command command;
		command.op = RecordedOp::kEndLabel;
		Push( std::move( command ) );
	}
	void WriteTimestamp( BufferId buffer, std::uint64_t offset ) override
	{
		Command command;
		command.op = RecordedOp::kWriteTimestamp;
		command.a = buffer.value;
		command.copy.destinationOffset = offset;
		Push( std::move( command ) );
	}
	bool HasError() const override { return m_Error; }

	bool Complete() const { return !m_Error && !m_Rendering && m_Labels == 0; }
	RecordingDevice &Device() const { return m_Device; }
	QueueKind Queue() const { return m_Queue; }
	std::vector<Command> &Commands() { return m_Commands; }
	std::vector<std::uint64_t> &RingAllocations() { return m_RingAllocations; }
	void MarkSubmitted() { m_Submitted = true; }

private:
	void Push( Command command ) { m_Commands.push_back( std::move( command ) ); }
	void NotRendering()
	{
		if ( m_Rendering )
			m_Error = true;
	}
	void Drawing()
	{
		if ( !m_Rendering )
			m_Error = true;
	}

	RecordingDevice &m_Device;
	QueueKind m_Queue;
	std::vector<Command> m_Commands;
	std::vector<std::uint64_t> m_RingAllocations;
	bool m_Rendering = false;
	bool m_Error = false;
	bool m_Submitted = false;
	std::uint32_t m_Labels = 0;
};

// The upload ring. Allocations retire in order once their token completes
// or their encoder was destroyed without submitting.
class UploadRing
{
public:
	explicit UploadRing( std::uint64_t capacity ) : m_Bytes( capacity ) {}

	std::optional<std::uint64_t> Allocate( std::uint64_t size, std::uint64_t id )
	{
		size = ( size + 15u ) & ~std::uint64_t( 15u );
		const std::uint64_t capacity = m_Bytes.size();
		if ( size == 0 || size >= capacity )
			return std::nullopt;
		std::uint64_t offset = 0;
		if ( m_Live.empty() )
		{
			offset = 0;
		}
		else
		{
			const std::uint64_t tail = m_Live.front().offset;
			if ( m_Head >= tail )
			{
				if ( capacity - m_Head >= size )
					offset = m_Head;
				else if ( tail > size )
					offset = 0;
				else
					return std::nullopt;
			}
			else if ( tail - m_Head > size )
			{
				offset = m_Head;
			}
			else
			{
				return std::nullopt;
			}
		}
		m_Live.push_back( { offset, size, id, {}, false, false } );
		m_Head = offset + size;
		return offset;
	}

	void Submit( std::uint64_t id, CompletionToken token )
	{
		for ( Allocation &allocation : m_Live )
		{
			if ( allocation.id == id )
			{
				allocation.token = token;
				allocation.submitted = true;
			}
		}
	}

	void Abandon( std::uint64_t id )
	{
		for ( Allocation &allocation : m_Live )
		{
			if ( allocation.id == id && !allocation.submitted )
				allocation.abandoned = true;
		}
	}

	template <typename IsComplete> void Retire( const IsComplete &complete )
	{
		while ( !m_Live.empty() &&
		        ( m_Live.front().abandoned ||
		            ( m_Live.front().submitted && complete( m_Live.front().token ) ) ) )
			m_Live.pop_front();
		if ( m_Live.empty() )
			m_Head = 0;
	}

	void Reset()
	{
		m_Live.clear();
		m_Head = 0;
	}

	std::byte *Data( std::uint64_t offset ) { return m_Bytes.data() + offset; }

private:
	struct Allocation
	{
		std::uint64_t offset;
		std::uint64_t size;
		std::uint64_t id;
		CompletionToken token;
		bool submitted;
		bool abandoned;
	};

	std::vector<std::byte> m_Bytes;
	std::deque<Allocation> m_Live;
	std::uint64_t m_Head = 0;
};

class RecordingDevice final : public IRenderDevice2, public INullDeviceControl
{
public:
	explicit RecordingDevice( const NullOptions &options )
	    : m_Options( options ), m_Ring( options.uploadRingBytes )
	{
		m_Facts.diagnosticBackend = "null";
		m_Facts.adapterName = "null recording device";
		m_Facts.capabilities = options.capabilities;
		m_Facts.limits.maxBindGroups = kMaxBindGroups;
		m_Facts.limits.maxTextureDimension2D = 16384;
		m_Facts.limits.maxColorAttachments = 8;
		m_Facts.limits.maxVertexBuffers = 16;
		m_Facts.limits.uniformBufferAlignment = 256;
		m_Facts.limits.sampleCounts = 0xfu; // 1, 2, 4 and 8
		m_Facts.artifactFormat = options.artifactFormat;
		// D23: one tick per nanosecond; every command advances the clock.
		if ( m_Facts.capabilities.Has( Capability::kTimestamps ) )
			m_Facts.timestampPeriodNs = 1.0;
	}

	// IRenderDevice2 ---------------------------------------------------------

	const DeviceFacts &Facts() const override { return m_Facts; }
	DeviceState State() const override { return m_State; }
	std::uint32_t Epoch() const override { return m_Epoch; }
	MemoryBudgetSnapshot ReadMemoryBudget() const override
	{
		MemoryBudgetSnapshot result;
		result.supported = true;
		result.epoch = m_Epoch;
		HeapMemoryBudget heap;
		// Null has no physical heap or driver allowance. Its reported usage is
		// the exact byte storage currently owned by the recording implementation.
		heap.usageKnown = true;
		for ( const auto &entry : m_Buffers )
			heap.usageBytes += entry.second.data.size();
		for ( const auto &entry : m_Textures )
			for ( const auto &subresource : entry.second.subresources )
				heap.usageBytes += subresource.size();
		result.heaps.push_back( heap );
		return result;
	}

	DeviceResult<BufferId> CreateBuffer( const BufferDesc &desc ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateBuffer );
		if ( auto valid = ValidateBuffer( desc ); !valid )
			return foundation::MakeUnexpected( valid.Error() );
		Buffer buffer;
		buffer.desc = desc;
		buffer.desc.debugName = {};
		buffer.data.assign( desc.size, std::byte{ 0 } );
		const BufferId id{ ++m_NextId };
		m_Buffers.emplace( id.value, std::move( buffer ) );
		return id;
	}

	DeviceResult<BufferId> CreateUploadBuffer( std::span<const std::byte> bytes ) override
	{
		BufferDesc desc;
		desc.size = bytes.size();
		desc.memory = MemoryKind::kUpload;
		desc.usages = { ResourceUsage::kCopySource };
		auto buffer = CreateBuffer( desc );
		if ( !buffer )
			return buffer;
		Buffer &record = m_Buffers.at( buffer.Value().value );
		std::copy( bytes.begin(), bytes.end(), record.data.begin() );
		record.usage = ResourceUsage::kCopySource;
		return buffer;
	}

	DeviceResult<TextureId> CreateTexture( const TextureDesc &desc ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateTexture );
		if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
			return foundation::MakeUnexpected( valid.Error() );
		if ( IsBlockCompressed( desc.format ) &&
		     !m_Facts.capabilities.Has( Capability::kTextureCompressionBC ) )
			return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateTexture );
		Texture texture;
		texture.desc = desc;
		texture.desc.debugName = {};
		texture.layers = desc.dimension == TextureDimension::k3D ? 1u : desc.depthOrLayers;
		// Storage is made on first content access (EnsureStorage), so a frame
		// target that is only transitioned and attached costs no memory.
		const TextureId id{ ++m_NextId };
		m_Textures.emplace( id.value, std::move( texture ) );
		return id;
	}

	DeviceResult<SamplerId> CreateSampler( const SamplerDesc &desc ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateSampler );
		if ( auto valid = ValidateSampler( desc ); !valid )
			return foundation::MakeUnexpected( valid.Error() );
		const SamplerId id{ ++m_NextId };
		m_Samplers.emplace( id.value, Simple{} );
		return id;
	}

	DeviceResult<BindGroupLayoutId> CreateBindGroupLayout(
	    const BindGroupLayoutDesc &desc ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateBindGroupLayout );
		if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
			return foundation::MakeUnexpected( valid.Error() );
		Layout layout;
		layout.role = desc.role;
		layout.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
		const BindGroupLayoutId id{ ++m_NextId };
		m_Layouts.emplace( id.value, std::move( layout ) );
		return id;
	}

	DeviceResult<BindGroupId> CreateBindGroup( const BindGroupDesc &desc ) override
	{
		const DeviceOperation op = DeviceOperation::kCreateBindGroup;
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, op );
		const std::optional<LayoutView> layout = FindLayout( desc.layout );
		if ( !layout )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( auto valid = ValidateBindGroup( desc, *layout ); !valid )
			return foundation::MakeUnexpected( valid.Error() );
		for ( const BindGroupEntry &entry : desc.entries )
		{
			if ( ( entry.buffer.IsValid() && !LiveBuffer( entry.buffer.value ) ) ||
			     ( entry.texture.IsValid() && !LiveTexture( entry.texture.value ) ) ||
			     ( entry.sampler.IsValid() && !Live( m_Samplers, entry.sampler.value ) ) )
				return Fail( DeviceStatus::kInvalidHandle, op );
		}
		const BindGroupId id{ ++m_NextId };
		m_BindGroups.emplace( id.value, Simple{} );
		return id;
	}

	DeviceResult<PipelineId> CreatePipeline( const PipelineDesc &desc ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreatePipeline );
		auto valid = ValidatePipeline( desc, m_Facts,
		    [this]( BindGroupLayoutId id )
		    {
			    return FindLayout( id );
		    } );
		if ( !valid )
			return foundation::MakeUnexpected( valid.Error() );
		const PipelineId id{ ++m_NextId };
		m_Pipelines.emplace( id.value, Simple{ false, desc.drawConstantBytes } );
		return id;
	}

	DeviceResult<void> Release( ResourceId resource, CompletionToken releaseAfter ) override
	{
		bool *released = ReleasedFlag( resource );
		if ( !released || *released )
			return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
		*released = true;
		m_Releases.push_back( { resource, releaseAfter } );
		return {};
	}

	DeviceResult<CommandEncoder> BeginEncoder( QueueKind queue ) override
	{
		if ( m_State != DeviceState::kAvailable )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
		if ( !QueueSupported( queue ) )
			return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
		return CommandEncoder( queue, std::make_unique<NullEncoder>( *this, queue ) );
	}

	DeviceResult<CompletionToken> Submit(
	    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits ) override;

	bool IsComplete( CompletionToken token ) const override
	{
		if ( !token.NamesSubmission() || token.epoch < m_Epoch )
			return true;
		if ( token.epoch > m_Epoch || static_cast<std::size_t>( token.queue ) >= kQueueCount )
			return false;
		return token.value <= m_Completed[static_cast<std::size_t>( token.queue )];
	}

	std::size_t Poll() override
	{
		if ( m_Options.completion == CompletionMode::kOnPoll )
			CompleteAll();
		return Collect();
	}

	DeviceResult<void> ReadBuffer(
	    BufferId id, std::uint64_t offset, std::span<std::byte> out ) override
	{
		const DeviceOperation op = DeviceOperation::kReadBuffer;
		Buffer *buffer = LiveBuffer( id.value );
		if ( !buffer )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( buffer->desc.memory != MemoryKind::kReadback || offset > buffer->data.size() ||
		     out.size() > buffer->data.size() - offset )
			return Fail( DeviceStatus::kInvalidDescription, op );
		std::memcpy( out.data(), buffer->data.data() + offset, out.size() );
		return {};
	}

	DeviceResult<void> WaitIdle() override
	{
		CompleteAll();
		return {};
	}

	DeviceResult<void> Recover() override
	{
		if ( m_State == DeviceState::kAvailable )
			return {};
		++m_Epoch;
		m_Buffers.clear();
		m_Textures.clear();
		m_Samplers.clear();
		m_Layouts.clear();
		m_BindGroups.clear();
		m_Pipelines.clear();
		m_Releases.clear();
		for ( auto &pending : m_Pending )
			pending.clear();
		for ( std::size_t i = 0; i < kQueueCount; ++i )
			m_Completed[i] = m_Submitted[i];
		{
			std::lock_guard<std::mutex> lock( m_RingLock );
			m_Ring.Reset();
		}
		m_State = DeviceState::kAvailable;
		return {};
	}

	std::size_t LiveResourceCount() const override
	{
		return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
		       m_BindGroups.size() + m_Pipelines.size();
	}

	// INullDeviceControl -----------------------------------------------------

	void CompleteThrough( QueueKind queue, std::uint64_t value ) override
	{
		const std::size_t index = static_cast<std::size_t>( queue );
		if ( index >= kQueueCount )
			return;
		value = std::min( value, m_Submitted[index] );
		auto &pending = m_Pending[index];
		while ( !pending.empty() && pending.front().token.value <= value )
		{
			Execute( pending.front() );
			pending.pop_front();
		}
		m_Completed[index] = std::max( m_Completed[index], value );
	}

	void CompleteAll() override
	{
		for ( std::size_t i = 0; i < kQueueCount; ++i )
			CompleteThrough( static_cast<QueueKind>( i ), m_Submitted[i] );
	}

	void LoseDevice() override { m_State = DeviceState::kLost; }
	std::span<const RecordedCommand> Recorded() const override { return m_Recorded; }
	void ClearRecorded() override { m_Recorded.clear(); }
	std::uint64_t DeferredUploads() const override { return m_DeferredUploads; }

	// For NullEncoder --------------------------------------------------------

	// Copies bytes into the ring (or a deferred block) for the command.
	// Distinct encoders may record on different threads at once (port clause
	// D14), so ring staging is serialized; submission, completion and Poll
	// stay on the device owner's sequence.
	void StageUpload( NullEncoder &encoder, Command &command, std::span<const std::byte> bytes )
	{
		std::lock_guard<std::mutex> lock( m_RingLock );
		m_Ring.Retire(
		    [this]( CompletionToken token )
		    {
			    return IsComplete( token );
		    } );
		const std::uint64_t id = ++m_NextAllocation;
		if ( const std::optional<std::uint64_t> offset = m_Ring.Allocate( bytes.size(), id ) )
		{
			std::memcpy( m_Ring.Data( *offset ), bytes.data(), bytes.size() );
			command.ringOffset = *offset;
			command.fromRing = true;
			encoder.RingAllocations().push_back( id );
			return;
		}
		++m_DeferredUploads;
		command.deferred.assign( bytes.begin(), bytes.end() );
	}

	void AbandonUploads( const std::vector<std::uint64_t> &allocations )
	{
		std::lock_guard<std::mutex> lock( m_RingLock );
		for ( std::uint64_t id : allocations )
			m_Ring.Abandon( id );
	}

private:
	struct PendingRelease
	{
		ResourceId resource;
		CompletionToken token;
	};

	struct Batch
	{
		CompletionToken token;
		std::vector<Command> commands;
	};

	template <typename Map> static bool Live( const Map &map, std::uint64_t id )
	{
		const auto found = map.find( id );
		return found != map.end() && !found->second.released;
	}

	Buffer *LiveBuffer( std::uint64_t id )
	{
		const auto found = m_Buffers.find( id );
		return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
	}

	Texture *LiveTexture( std::uint64_t id )
	{
		const auto found = m_Textures.find( id );
		return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
	}

	// Submitted work runs even if its resources were released after
	// submission: release forbids new use, it does not cancel work in flight.
	Buffer *ExistingBuffer( std::uint64_t id )
	{
		const auto found = m_Buffers.find( id );
		return found != m_Buffers.end() ? &found->second : nullptr;
	}

	Texture *ExistingTexture( std::uint64_t id )
	{
		const auto found = m_Textures.find( id );
		return found != m_Textures.end() ? &found->second : nullptr;
	}

	std::optional<LayoutView> FindLayout( BindGroupLayoutId id ) const
	{
		const auto found = m_Layouts.find( id.value );
		if ( found == m_Layouts.end() || found->second.released )
			return std::nullopt;
		return LayoutView{ found->second.role, found->second.bindings };
	}

	bool *ReleasedFlag( ResourceId resource )
	{
		auto flag = [&]( auto &map ) -> bool *
		{
			const auto found = map.find( resource.value );
			return found == map.end() ? nullptr : &found->second.released;
		};
		switch ( resource.kind )
		{
		case ResourceKind::kBuffer:
			return flag( m_Buffers );
		case ResourceKind::kTexture:
			return flag( m_Textures );
		case ResourceKind::kSampler:
			return flag( m_Samplers );
		case ResourceKind::kPipeline:
			return flag( m_Pipelines );
		case ResourceKind::kBindGroupLayout:
			return flag( m_Layouts );
		case ResourceKind::kBindGroup:
			return flag( m_BindGroups );
		case ResourceKind::kNone:
			break;
		}
		return nullptr;
	}

	bool QueueSupported( QueueKind queue ) const
	{
		switch ( queue )
		{
		case QueueKind::kGraphics:
			return true;
		case QueueKind::kCompute:
			return m_Facts.capabilities.Has( Capability::kAsyncCompute );
		case QueueKind::kTransfer:
			return m_Facts.capabilities.Has( Capability::kAsyncTransfer );
		case QueueKind::kCount:
			break;
		}
		return false;
	}

	std::size_t Collect()
	{
		std::size_t freed = 0;
		for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
		{
			if ( !IsComplete( it->token ) )
			{
				++it;
				continue;
			}
			Erase( it->resource );
			it = m_Releases.erase( it );
			++freed;
		}
		std::lock_guard<std::mutex> lock( m_RingLock );
		m_Ring.Retire(
		    [this]( CompletionToken token )
		    {
			    return IsComplete( token );
		    } );
		return freed;
	}

	void Erase( ResourceId resource )
	{
		switch ( resource.kind )
		{
		case ResourceKind::kBuffer:
			m_Buffers.erase( resource.value );
			break;
		case ResourceKind::kTexture:
			m_Textures.erase( resource.value );
			break;
		case ResourceKind::kSampler:
			m_Samplers.erase( resource.value );
			break;
		case ResourceKind::kPipeline:
			m_Pipelines.erase( resource.value );
			break;
		case ResourceKind::kBindGroupLayout:
			m_Layouts.erase( resource.value );
			break;
		case ResourceKind::kBindGroup:
			m_BindGroups.erase( resource.value );
			break;
		case ResourceKind::kNone:
			break;
		}
	}

	// Checks one encoder's commands against the usage state in states, which
	// it updates. Returns false on the first invalid command.
	bool Validate(
	    std::vector<Command> &commands, std::unordered_map<std::uint64_t, ResourceUsage> &states )
	{
		auto state = [&]( std::uint64_t id, ResourceUsage current ) -> ResourceUsage &
		{
			return states.try_emplace( id, current ).first->second;
		};
		auto texture = [&]( std::uint64_t id, ResourceUsage required ) -> bool
		{
			Texture *t = LiveTexture( id );
			return t && state( id, t->usage ) == required;
		};
		auto buffer = [&]( std::uint64_t id, ResourceUsage required ) -> bool
		{
			Buffer *b = LiveBuffer( id );
			return b && state( id, b->usage ) == required;
		};
		DrawConstantCoverage constants; // D16
		for ( Command &command : commands )
		{
			switch ( command.op )
			{
			case RecordedOp::kTransitionTexture:
			{
				Texture *t = LiveTexture( command.a );
				if ( !t || !t->desc.usages.Has( command.after ) )
					return false;
				ResourceUsage &current = state( command.a, t->usage );
				if ( command.before != ResourceUsage::kUndefined && command.before != current )
					return false;
				current = command.after;
				break;
			}
			case RecordedOp::kTransitionBuffer:
			{
				Buffer *b = LiveBuffer( command.a );
				if ( !b || !b->desc.usages.Has( command.after ) )
					return false;
				ResourceUsage &current = state( command.a, b->usage );
				if ( command.before != ResourceUsage::kUndefined && command.before != current )
					return false;
				current = command.after;
				break;
			}
			case RecordedOp::kClearTexture:
			{
				// A block-compressed texture is written by copies only (D19).
				if ( !texture( command.a, ResourceUsage::kCopyDestination ) ||
				     IsBlockCompressed( LiveTexture( command.a )->desc.format ) )
					return false;
				break;
			}
			case RecordedOp::kWriteBuffer:
			{
				Buffer *b = LiveBuffer( command.a );
				if ( !buffer( command.a, ResourceUsage::kCopyDestination ) ||
				     command.copy.destinationOffset + command.count > b->data.size() )
					return false;
				break;
			}
			case RecordedOp::kCopyBuffer:
			{
				if ( !buffer( command.a, ResourceUsage::kCopySource ) ||
				     !buffer( command.b, ResourceUsage::kCopyDestination ) )
					return false;
				const Buffer *src = LiveBuffer( command.a );
				const Buffer *dst = LiveBuffer( command.b );
				if ( command.copy.sourceOffset + command.copy.size > src->data.size() ||
				     command.copy.destinationOffset + command.copy.size > dst->data.size() )
					return false;
				break;
			}
			case RecordedOp::kCopyTextureToBuffer:
				if ( !texture( command.a, ResourceUsage::kCopySource ) ||
				     !buffer( command.b, ResourceUsage::kCopyDestination ) ||
				     !TextureCopyFits( *LiveTexture( command.a ), *LiveBuffer( command.b ),
				         command.textureCopy ) )
					return false;
				break;
			case RecordedOp::kCopyBufferToTexture:
				if ( !buffer( command.a, ResourceUsage::kCopySource ) ||
				     !texture( command.b, ResourceUsage::kCopyDestination ) ||
				     !TextureCopyFits( *LiveTexture( command.b ), *LiveBuffer( command.a ),
				         command.textureCopy ) )
					return false;
				break;
			case RecordedOp::kBeginRendering:
				for ( const ColorAttachment &color : command.colors )
				{
					if ( !texture( color.texture.value, ResourceUsage::kColorAttachment ) )
						return false;
					if ( color.resolve.IsValid() &&
					     !texture( color.resolve.value, ResourceUsage::kResolveDestination ) )
						return false;
				}
				if ( command.depth &&
				     !texture( command.depth->texture.value, ResourceUsage::kDepthWrite ) &&
				     !texture( command.depth->texture.value, ResourceUsage::kDepthRead ) )
					return false;
				break;
			case RecordedOp::kSetPipeline:
				if ( !Live( m_Pipelines, command.a ) )
					return false;
				constants.Bind( m_Pipelines.find( command.a )->second.drawConstantBytes );
				break;
			case RecordedOp::kSetDrawConstants:
				if ( !constants.Write( command.slot, command.count ) )
					return false;
				break;
			case RecordedOp::kDraw:
			case RecordedOp::kDrawIndexed:
			case RecordedOp::kDispatch:
				if ( !constants.Ready() )
					return false;
				break;
			case RecordedOp::kSetBindGroup:
				if ( !Live( m_BindGroups, command.a ) )
					return false;
				break;
			case RecordedOp::kSetVertexBuffer:
				if ( !buffer( command.a, ResourceUsage::kVertex ) )
					return false;
				break;
			case RecordedOp::kSetIndexBuffer:
				if ( !buffer( command.a, ResourceUsage::kIndex ) )
					return false;
				break;
			case RecordedOp::kWriteTimestamp:
			{
				// D23: kReadback memory in kCopyDestination, 8-byte aligned.
				const std::uint64_t offset = command.copy.destinationOffset;
				if ( !buffer( command.a, ResourceUsage::kCopyDestination ) )
					return false;
				const Buffer *b = LiveBuffer( command.a );
				if ( b->desc.memory != MemoryKind::kReadback || offset % 8 != 0 ||
				     offset > b->data.size() || b->data.size() - offset < 8 )
					return false;
				break;
			}
			default:
				break;
			}
		}
		return true;
	}

	static bool TextureCopyFits(
	    const Texture &texture, const Buffer &buffer, const TextureBufferCopy &copy )
	{
		if ( copy.mip >= texture.desc.mipLevels || copy.layer >= texture.layers ||
		     copy.width == 0 || copy.height == 0 )
			return false;
		const Format format = texture.desc.format;
		if ( !CopyRegionAligned( format, texture.Width( copy.mip ), texture.Height( copy.mip ),
		         copy.x, copy.y, copy.width, copy.height, copy.bufferOffset ) )
			return false;
		const std::uint64_t bytes = RegionBytes( format, copy.width, copy.height );
		return copy.bufferOffset + bytes <= buffer.data.size();
	}

	void Execute( Batch &batch )
	{
		for ( Command &command : batch.commands )
		{
			m_Clock += 10; // each command takes 10 ns of the null GPU's time
			Apply( command );
			RecordedCommand recorded;
			recorded.op = command.op;
			recorded.resource = command.a;
			recorded.before = command.before;
			recorded.after = command.after;
			recorded.count = command.count;
			recorded.submission = batch.token.value;
			m_Recorded.push_back( recorded );
		}
	}

	void FillSubresource(
	    Texture &texture, std::uint32_t mip, std::uint32_t layer, const ClearColor &color )
	{
		const std::vector<std::byte> texel = EncodeTexel( texture.desc.format, color );
		std::vector<std::byte> &data = texture.Subresource( mip, layer );
		for ( std::size_t offset = 0; offset + texel.size() <= data.size(); offset += texel.size() )
			std::memcpy( data.data() + offset, texel.data(), texel.size() );
	}

	void Apply( Command &command )
	{
		switch ( command.op )
		{
		case RecordedOp::kTransitionTexture:
			if ( Texture *t = ExistingTexture( command.a ) )
			{
				if ( command.before == ResourceUsage::kUndefined )
				{
					// Contents are discarded: a GPU may leave anything behind.
					for ( auto &subresource : t->subresources )
						std::fill( subresource.begin(), subresource.end(), std::byte{ 0xcd } );
				}
			}
			break;
		case RecordedOp::kTransitionBuffer:
			break;
		case RecordedOp::kClearTexture:
			if ( Texture *t = ExistingTexture( command.a ) )
			{
				const SubresourceRange &r = command.range;
				for ( std::uint32_t mip = r.baseMip;
				    mip < std::min( r.baseMip + r.mipCount, t->desc.mipLevels ); ++mip )
				{
					for ( std::uint32_t layer = r.baseLayer;
					    layer < std::min( r.baseLayer + r.layerCount, t->layers ); ++layer )
						FillSubresource( *t, mip, layer, command.color );
				}
			}
			break;
		case RecordedOp::kWriteBuffer:
			if ( Buffer *b = ExistingBuffer( command.a ) )
			{
				const std::byte *source =
				    command.fromRing ? m_Ring.Data( command.ringOffset ) : command.deferred.data();
				std::memcpy(
				    b->data.data() + command.copy.destinationOffset, source, command.count );
			}
			break;
		case RecordedOp::kCopyBuffer:
		{
			Buffer *src = ExistingBuffer( command.a );
			Buffer *dst = ExistingBuffer( command.b );
			if ( src && dst )
				std::memmove( dst->data.data() + command.copy.destinationOffset,
				    src->data.data() + command.copy.sourceOffset, command.copy.size );
			break;
		}
		case RecordedOp::kCopyTextureToBuffer:
		case RecordedOp::kCopyBufferToTexture:
		{
			const bool toBuffer = command.op == RecordedOp::kCopyTextureToBuffer;
			Texture *t = ExistingTexture( toBuffer ? command.a : command.b );
			Buffer *b = ExistingBuffer( toBuffer ? command.b : command.a );
			if ( !t || !b )
				break;
			const TextureBufferCopy &copy = command.textureCopy;
			// Rows of blocks (of texels, for an uncompressed format).
			const FormatBlock block = BlockOf( t->desc.format );
			std::vector<std::byte> &data = t->Subresource( copy.mip, copy.layer );
			const std::size_t pitch = RegionBytes( t->desc.format, t->Width( copy.mip ), 1 );
			const std::size_t row = RegionBytes( t->desc.format, copy.width, 1 );
			const std::uint32_t rows = ( copy.height + block.height - 1 ) / block.height;
			// The region's first block: its block row, then its block column.
			const std::size_t origin =
			    ( copy.y / block.height ) * pitch + RegionBytes( t->desc.format, copy.x, 1 );
			for ( std::uint32_t y = 0; y < rows; ++y )
			{
				std::byte *image = data.data() + origin + y * pitch;
				std::byte *linear = b->data.data() + copy.bufferOffset + y * row;
				if ( toBuffer )
					std::memcpy( linear, image, row );
				else
					std::memcpy( image, linear, row );
			}
			command.count = static_cast<std::uint64_t>( row ) * rows;
			break;
		}
		case RecordedOp::kWriteTimestamp:
			if ( Buffer *b = ExistingBuffer( command.a ) )
			{
				const std::uint64_t tick = m_Clock;
				std::memcpy(
				    b->data.data() + command.copy.destinationOffset, &tick, sizeof( tick ) );
				command.count = tick;
			}
			break;
		case RecordedOp::kBeginRendering:
			for ( const ColorAttachment &color : command.colors )
			{
				Texture *t = ExistingTexture( color.texture.value );
				if ( t && color.load == LoadOp::kClear )
					FillSubresource( *t, 0, 0, color.clear );
			}
			if ( command.depth && command.depth->load == LoadOp::kClear )
			{
				if ( Texture *t = ExistingTexture( command.depth->texture.value ) )
					FillSubresource( *t, 0, 0, { command.depth->clearDepth, 0, 0, 0 } );
			}
			break;
		default:
			break;
		}
	}

	NullOptions m_Options;
	DeviceFacts m_Facts;
	DeviceState m_State = DeviceState::kAvailable;
	std::uint32_t m_Epoch = 1;
	std::uint64_t m_NextId = 0;
	std::uint64_t m_NextAllocation = 0;
	std::uint64_t m_DeferredUploads = 0;
	std::unordered_map<std::uint64_t, Buffer> m_Buffers;
	std::unordered_map<std::uint64_t, Texture> m_Textures;
	std::unordered_map<std::uint64_t, Simple> m_Samplers;
	std::unordered_map<std::uint64_t, Layout> m_Layouts;
	std::unordered_map<std::uint64_t, Simple> m_BindGroups;
	std::unordered_map<std::uint64_t, Simple> m_Pipelines;
	std::vector<PendingRelease> m_Releases;
	std::uint64_t m_Submitted[kQueueCount] = {};
	std::uint64_t m_Completed[kQueueCount] = {};
	std::deque<Batch> m_Pending[kQueueCount];
	std::vector<RecordedCommand> m_Recorded;
	std::uint64_t m_Clock = 0; // D23: the null GPU's time, in ticks
	UploadRing m_Ring;
	std::mutex m_RingLock; // guards m_Ring, m_NextAllocation, m_DeferredUploads
};

NullEncoder::~NullEncoder()
{
	if ( !m_Submitted )
		m_Device.AbandonUploads( m_RingAllocations );
}

void NullEncoder::WriteBuffer(
    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes )
{
	NotRendering();
	Command command;
	command.op = RecordedOp::kWriteBuffer;
	command.a = buffer.value;
	command.copy.destinationOffset = offset;
	command.count = bytes.size();
	if ( bytes.empty() )
		m_Error = true;
	else
		m_Device.StageUpload( *this, command, bytes );
	Push( std::move( command ) );
}

DeviceResult<CompletionToken> RecordingDevice::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	// Encoders are consumed whatever the outcome.
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );

	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( !QueueSupported( queue ) )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		const std::size_t index = static_cast<std::size_t>( wait.queue );
		if ( index >= kQueueCount || wait.epoch > m_Epoch || wait.value > m_Submitted[index] )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<NullEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		NullEncoder *encoder = dynamic_cast<NullEncoder *>( backend.get() );
		if ( !encoder || &encoder->Device() != this || encoder->Queue() != queue )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( !encoder->Complete() || !Validate( encoder->Commands(), states ) )
			return Fail( DeviceStatus::kInvalidState, op );
		recorded.push_back( encoder );
	}
	// D23: timestamps need the capability, and each buffer ends the
	// submission in kCopyDestination.
	for ( NullEncoder *encoder : recorded )
	{
		for ( const Command &command : encoder->Commands() )
		{
			if ( command.op != RecordedOp::kWriteTimestamp )
				continue;
			if ( !m_Facts.capabilities.Has( Capability::kTimestamps ) )
				return Fail( DeviceStatus::kUnsupported, op );
			const auto found = states.find( command.a );
			if ( found == states.end() || found->second != ResourceUsage::kCopyDestination )
				return Fail( DeviceStatus::kInvalidState, op );
		}
	}
	// Usage state is the state after every accepted submission, whether or
	// not it has run yet: the next submission continues from it.
	for ( const auto &[id, usage] : states )
	{
		if ( Texture *t = LiveTexture( id ) )
			t->usage = usage;
		else if ( Buffer *b = LiveBuffer( id ) )
			b->usage = usage;
	}

	const std::size_t index = static_cast<std::size_t>( queue );
	const CompletionToken token{ queue, m_Epoch, ++m_Submitted[index] };
	Batch batch;
	batch.token = token;
	for ( NullEncoder *encoder : recorded )
	{
		std::lock_guard<std::mutex> lock( m_RingLock );
		for ( std::uint64_t allocation : encoder->RingAllocations() )
		{
			if ( m_Options.unsafeUploadReuse )
				m_Ring.Abandon( allocation );
			else
				m_Ring.Submit( allocation, token );
		}
		encoder->MarkSubmitted();
		for ( Command &command : encoder->Commands() )
			batch.commands.push_back( std::move( command ) );
	}
	m_Pending[index].push_back( std::move( batch ) );
	return token;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	NullOptions options;
	if ( const std::optional<Capability> missing =
	         FirstMissing( options.capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return Create( options );
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "null", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const NullOptions &options )
{
	if ( options.uploadRingBytes < 256 )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateDevice );
	return std::unique_ptr<IRenderDevice2>( std::make_unique<RecordingDevice>( options ) );
}

INullDeviceControl *Control( IRenderDevice2 &device )
{
	return dynamic_cast<RecordingDevice *>( &device );
}

} // namespace render::device::null
