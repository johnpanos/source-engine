//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal: the device, its facts, resources,
//			submission and completion. See metal_device.h.
//
//=============================================================================//

#include "metal_device.h"

#include <TargetConditionals.h>

#include <cstdio>
#include <cstring>

namespace render::device::metal
{

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, nativeCode } );
}

MTLPixelFormat PixelFormatOf( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return MTLPixelFormatR8Unorm;
	case Format::kRGBA8Unorm:
		return MTLPixelFormatRGBA8Unorm;
	case Format::kRGBA8Srgb:
		return MTLPixelFormatRGBA8Unorm_sRGB;
	case Format::kBGRA8Unorm:
		return MTLPixelFormatBGRA8Unorm;
	case Format::kBGRA8Srgb:
		return MTLPixelFormatBGRA8Unorm_sRGB;
	case Format::kRG16Float:
		return MTLPixelFormatRG16Float;
	case Format::kRGBA16Float:
		return MTLPixelFormatRGBA16Float;
	case Format::kR32Float:
		return MTLPixelFormatR32Float;
	case Format::kRGBA32Float:
		return MTLPixelFormatRGBA32Float;
	case Format::kD32Float:
		return MTLPixelFormatDepth32Float;
	case Format::kD32FloatS8:
		return MTLPixelFormatDepth32Float_Stencil8;
	case Format::kRGBA16Unorm:
		return MTLPixelFormatRGBA16Unorm;
	case Format::kBC1Unorm:
		return MTLPixelFormatBC1_RGBA;
	case Format::kBC1Srgb:
		return MTLPixelFormatBC1_RGBA_sRGB;
	case Format::kBC2Unorm:
		return MTLPixelFormatBC2_RGBA;
	case Format::kBC2Srgb:
		return MTLPixelFormatBC2_RGBA_sRGB;
	case Format::kBC3Unorm:
		return MTLPixelFormatBC3_RGBA;
	case Format::kBC3Srgb:
		return MTLPixelFormatBC3_RGBA_sRGB;
	case Format::kBC4Unorm:
		return MTLPixelFormatBC4_RUnorm;
	case Format::kBC5Unorm:
		return MTLPixelFormatBC5_RGUnorm;
	case Format::kBC6HUfloat:
		return MTLPixelFormatBC6H_RGBUfloat;
	case Format::kBC7Unorm:
		return MTLPixelFormatBC7_RGBAUnorm;
	case Format::kBC7Srgb:
		return MTLPixelFormatBC7_RGBAUnorm_sRGB;
	case Format::kRGB10A2Unorm:
		return MTLPixelFormatRGB10A2Unorm;
	case Format::kRG11B10Float:
		return MTLPixelFormatRG11B10Float;
	// Apple GPUs have no 24-bit depth (Depth24Unorm_Stencil8 is an Intel/AMD
	// Mac format): kD24UnormS8 is refused by name.
	case Format::kD24UnormS8:
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return MTLPixelFormatInvalid;
}

bool IsSrgb( Format format )
{
	switch ( format )
	{
	case Format::kRGBA8Srgb:
	case Format::kBGRA8Srgb:
	case Format::kBC1Srgb:
	case Format::kBC2Srgb:
	case Format::kBC3Srgb:
	case Format::kBC7Srgb:
		return true;
	default:
		return false;
	}
}

// Device ------------------------------------------------------------------------

MetalDevice::MetalDevice( const MetalAdapterOptions &options ) : m_Options( options ) {}

MetalDevice::~MetalDevice()
{
	if ( m_LastCommitted )
		[m_LastCommitted waitUntilCompleted]; // reviewed idle wait: teardown
}

DeviceResult<void> MetalDevice::Initialize()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	if ( m_Options.uploadRingBytes < 4096 )
		return Fail( DeviceStatus::kInvalidDescription, op );
	m_Device = MTLCreateSystemDefaultDevice();
	if ( !m_Device )
		return Fail( DeviceStatus::kUnavailable, op );
	// MSL 3.0 artifacts, argument buffers of samplers and textures (tier 2).
	if ( ![m_Device supportsFamily:MTLGPUFamilyMetal3] ||
	     m_Device.argumentBuffersSupport < MTLArgumentBuffersTier2 )
		return Fail( DeviceStatus::kUnsupported, op );
	m_AdapterName = m_Device.name ? std::string( m_Device.name.UTF8String ) : "Metal";
	QueryFacts();
	return CreateQueueObjects();
}

DeviceResult<void> MetalDevice::CreateQueueObjects()
{
	m_Queue = [m_Device newCommandQueue];
	m_RingBuffer = [m_Device newBufferWithLength:m_Options.uploadRingBytes
	                                     options:MTLResourceStorageModeShared |
	                                             MTLResourceCPUCacheModeWriteCombined];
	if ( !m_Queue || !m_RingBuffer )
		return Fail( DeviceStatus::kOutOfMemory, DeviceOperation::kCreateDevice );
	m_RingBuffer.label = @"render.device.metal upload ring";
	m_Ring.Reset( m_Options.uploadRingBytes );
	return {};
}

void MetalDevice::QueryFacts()
{
	CapabilitySet have{ Capability::kCompute, Capability::kStorageBuffers,
	    Capability::kMultiDrawIndirect, Capability::kIndirectFirstInstance };
	// BC on Apple silicon Macs, and on iOS devices that report it (M-series
	// iPads); an iPhone decodes BC on the CPU instead (RFC 0016 R91-VK11).
	if ( m_Device.supportsBCTextureCompression )
		have.Add( Capability::kTextureCompressionBC );
	// Cube arrays: every Metal 3 GPU (Apple4 and later, Mac2).
	have.Add( Capability::kCubeArrays );
	// Not claimed: parallel recording (CPU lists replayed at Submit), async
	// queues (one queue), transient aliasing, ray query, external images,
	// draw-indirect count, and timestamps (Apple GPUs sample counters only at
	// stage boundaries, not between commands).
	CapabilitySet claimed;
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( Capability::kCount ); ++bit )
	{
		const auto capability = static_cast<Capability>( bit );
		if ( have.Has( capability ) && m_Options.allowed.Has( capability ) )
			claimed.Add( capability );
	}
	Limits limits;
	limits.maxBindGroups = kMaxBindGroups;
	limits.maxTextureDimension2D = 16384;
	limits.maxColorAttachments = 8;
	limits.maxVertexBuffers = kMaxVertexSlots;
	limits.uniformBufferAlignment = 256;
	limits.sampleCounts = 1;
	for ( std::uint32_t log2 = 1; log2 <= 3; ++log2 )
	{
		if ( [m_Device supportsTextureSampleCount:( 1u << log2 )] )
			limits.sampleCounts |= 1u << log2;
	}
	m_Facts.diagnosticBackend = "metal";
	m_Facts.adapterName = m_AdapterName;
	m_Facts.capabilities = claimed;
	m_Facts.limits = limits;
	m_Facts.artifactFormat = ArtifactFormat::kMsl;
	m_Facts.timestampPeriodNs = 0.0;
}

BufferRecord *MetalDevice::LiveBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
}

TextureRecord *MetalDevice::LiveTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
}

BufferRecord *MetalDevice::ExistingBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() ? &found->second : nullptr;
}

TextureRecord *MetalDevice::ExistingTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() ? &found->second : nullptr;
}

const PipelineRecord *MetalDevice::ExistingPipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	return found != m_Pipelines.end() ? &found->second : nullptr;
}

BindGroupRecord *MetalDevice::ExistingBindGroup( std::uint64_t id )
{
	const auto found = m_BindGroups.find( id );
	return found != m_BindGroups.end() ? &found->second : nullptr;
}

id<MTLSamplerState> MetalDevice::Sampler( std::uint64_t id ) const
{
	const auto found = m_Samplers.find( id );
	return found != m_Samplers.end() ? found->second.sampler : nil;
}

// Recorded resources (recording::IRecordedResources) --------------------------------

std::optional<recording::TextureView> MetalDevice::Texture( std::uint64_t id ) const
{
	const auto found = m_Textures.find( id );
	if ( found == m_Textures.end() || found->second.released )
		return std::nullopt;
	return recording::TextureView{ found->second.desc, found->second.layers, found->second.usage };
}

std::optional<recording::BufferView> MetalDevice::Buffer( std::uint64_t id ) const
{
	const auto found = m_Buffers.find( id );
	if ( found == m_Buffers.end() || found->second.released )
		return std::nullopt;
	return recording::BufferView{ found->second.desc, found->second.usage };
}

std::optional<recording::PipelineView> MetalDevice::Pipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	if ( found == m_Pipelines.end() || found->second.released )
		return std::nullopt;
	const PipelineRecord &p = found->second;
	return recording::PipelineView{ p.kind, p.colorFormats, p.depthFormat, p.sampleCount,
	    p.vertexBuffers, p.drawConstantBytes, p.layouts, p.layoutHasBindings };
}

std::optional<BindGroupLayoutId> MetalDevice::BindGroup( std::uint64_t id ) const
{
	const auto group = m_BindGroups.find( id );
	if ( group == m_BindGroups.end() || group->second.released )
		return std::nullopt;
	auto live = [&]( const auto &map, std::uint64_t key )
	{
		const auto found = map.find( key );
		return found != map.end() && !found->second.released;
	};
	for ( const BindGroupEntry &entry : group->second.entries )
	{
		if ( ( entry.buffer.IsValid() && !live( m_Buffers, entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !live( m_Textures, entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() && !live( m_Samplers, entry.sampler.value ) ) )
			return std::nullopt;
	}
	return group->second.layout;
}

std::optional<LayoutView> MetalDevice::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

bool *MetalDevice::ReleasedFlag( ResourceId resource )
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

namespace
{

NSString *Label( std::string_view name )
{
	return [[NSString alloc] initWithBytes:name.data()
	                                length:name.size()
	                              encoding:NSUTF8StringEncoding];
}

MTLCompareFunction CompareOf( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return MTLCompareFunctionNever;
	case CompareOp::kLess:
		return MTLCompareFunctionLess;
	case CompareOp::kLessEqual:
		return MTLCompareFunctionLessEqual;
	case CompareOp::kEqual:
		return MTLCompareFunctionEqual;
	case CompareOp::kGreaterEqual:
		return MTLCompareFunctionGreaterEqual;
	case CompareOp::kGreater:
		return MTLCompareFunctionGreater;
	case CompareOp::kAlways:
		return MTLCompareFunctionAlways;
	case CompareOp::kNotEqual:
		return MTLCompareFunctionNotEqual;
	}
	return MTLCompareFunctionAlways;
}

} // namespace

DeviceResult<BufferId> MetalDevice::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Device-local memory is private; the CPU writes upload buffers and reads
	// readback buffers through shared storage (unified memory on Apple GPUs).
	const MTLResourceOptions options = desc.memory == MemoryKind::kDeviceLocal
	                                       ? MTLResourceStorageModePrivate
	                                       : MTLResourceStorageModeShared;
	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.buffer = [m_Device newBufferWithLength:desc.size options:options];
	if ( !record.buffer )
		return Fail( DeviceStatus::kOutOfMemory, op );
	if ( !desc.debugName.empty() )
		record.buffer.label = Label( desc.debugName );
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BufferId> MetalDevice::CreateUploadBuffer( std::span<const std::byte> bytes )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( bytes.empty() )
		return Fail( DeviceStatus::kInvalidDescription, op );
	BufferRecord record;
	record.desc.size = bytes.size();
	record.desc.memory = MemoryKind::kUpload;
	record.desc.usages = { ResourceUsage::kCopySource };
	// Initialized before this returns (D25): the bytes are copied now.
	record.buffer = [m_Device newBufferWithBytes:bytes.data()
	                                      length:bytes.size()
	                                     options:MTLResourceStorageModeShared];
	if ( !record.buffer )
		return Fail( DeviceStatus::kOutOfMemory, op );
	record.usage = ResourceUsage::kCopySource;
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<TextureId> MetalDevice::CreateTexture( const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Presentation belongs to a render.presentation.v1 bridge.
	if ( desc.usages.Has( ResourceUsage::kPresent ) || desc.usages.Has( ResourceUsage::kExternal ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( IsBlockCompressed( desc.format ) &&
	     !m_Facts.capabilities.Has( Capability::kTextureCompressionBC ) )
		return Fail( DeviceStatus::kUnsupported, op );
	const MTLPixelFormat format = PixelFormatOf( desc.format );
	if ( format == MTLPixelFormatInvalid )
		return Fail( DeviceStatus::kUnsupported, op );
	const bool attachment = desc.usages.Has( ResourceUsage::kColorAttachment ) ||
	                        desc.usages.Has( ResourceUsage::kDepthWrite ) ||
	                        desc.usages.Has( ResourceUsage::kDepthRead ) ||
	                        desc.usages.Has( ResourceUsage::kResolveDestination );
	const bool storage = desc.usages.Has( ResourceUsage::kStorageRead ) ||
	                     desc.usages.Has( ResourceUsage::kStorageWrite );
	// Shader writes take no sRGB, depth or compressed format.
	if ( ( attachment && desc.dimension == TextureDimension::k3D ) ||
	     ( storage && ( IsDepthFormat( desc.format ) || IsSrgb( desc.format ) ||
	                      IsBlockCompressed( desc.format ) ) ) )
		return Fail( DeviceStatus::kUnsupported, op );

	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
	descriptor.pixelFormat = format;
	descriptor.width = desc.width;
	descriptor.height = desc.height;
	descriptor.mipmapLevelCount = desc.mipLevels;
	descriptor.sampleCount = desc.sampleCount;
	descriptor.storageMode = MTLStorageModePrivate;
	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	switch ( desc.dimension )
	{
	case TextureDimension::k2D:
		record.layers = desc.depthOrLayers;
		if ( desc.sampleCount > 1 )
			descriptor.textureType = desc.depthOrLayers > 1 ? MTLTextureType2DMultisampleArray
			                                                : MTLTextureType2DMultisample;
		else
			descriptor.textureType =
			    desc.depthOrLayers > 1 ? MTLTextureType2DArray : MTLTextureType2D;
		descriptor.arrayLength = desc.depthOrLayers;
		break;
	case TextureDimension::kCube:
		record.layers = desc.depthOrLayers;
		descriptor.textureType =
		    desc.depthOrLayers > 6 ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
		descriptor.arrayLength = desc.depthOrLayers / 6;
		break;
	case TextureDimension::k3D:
		record.layers = 1;
		descriptor.textureType = MTLTextureType3D;
		descriptor.depth = desc.depthOrLayers;
		break;
	}
	MTLTextureUsage usage = MTLTextureUsageShaderRead;
	// Texture clears are render passes: a clearable color or depth format is
	// a render target too.
	if ( attachment || ( desc.usages.Has( ResourceUsage::kCopyDestination ) &&
	                       !IsBlockCompressed( desc.format ) &&
	                       desc.dimension != TextureDimension::k3D ) )
		usage |= MTLTextureUsageRenderTarget;
	if ( storage )
		usage |= MTLTextureUsageShaderWrite;
	descriptor.usage = usage;
	record.texture = [m_Device newTextureWithDescriptor:descriptor];
	if ( !record.texture )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( !desc.debugName.empty() )
		record.texture.label = Label( desc.debugName );
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<SamplerId> MetalDevice::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateSampler( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	MTLSamplerDescriptor *descriptor = [MTLSamplerDescriptor new];
	auto filter = []( Filter f )
	{ return f == Filter::kLinear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest; };
	descriptor.minFilter = filter( desc.minFilter );
	descriptor.magFilter = filter( desc.magFilter );
	descriptor.mipFilter =
	    desc.mipFilter == Filter::kLinear ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
	const auto addressOf = []( AddressMode mode )
	{
		return mode == AddressMode::kClampToEdge      ? MTLSamplerAddressModeClampToEdge
		       : mode == AddressMode::kMirroredRepeat ? MTLSamplerAddressModeMirrorRepeat
		                                              : MTLSamplerAddressModeRepeat;
	};
	descriptor.sAddressMode = addressOf( desc.address );
	descriptor.tAddressMode = addressOf( AddressV( desc ) );
	descriptor.rAddressMode = addressOf( desc.address );
	descriptor.maxAnisotropy = std::clamp( desc.maxAnisotropy, 1u, 16u );
	// D24: compare the reference with each stored depth.
	descriptor.compareFunction =
	    desc.comparison ? CompareOf( *desc.comparison ) : MTLCompareFunctionNever;
	// Samplers are encoded into argument buffers.
	descriptor.supportArgumentBuffers = YES;
	SamplerRecord record;
	record.sampler = [m_Device newSamplerStateWithDescriptor:descriptor];
	if ( !record.sampler )
		return Fail( DeviceStatus::kInternal, op );
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupLayoutId> MetalDevice::CreateBindGroupLayout(
    const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Binding n is argument id n; an array of count c takes ids n..n+c-1,
	// so it may not reach a later binding's id.
	for ( const BindingDesc &binding : desc.bindings )
	{
		for ( const BindingDesc &other : desc.bindings )
		{
			if ( other.binding > binding.binding &&
			     other.binding < binding.binding + binding.count )
				return Fail( DeviceStatus::kUnsupported, op );
		}
	}
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupId> MetalDevice::CreateBindGroup( const BindGroupDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const std::optional<LayoutView> layout = FindLayout( desc.layout );
	if ( !layout )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( auto valid = ValidateBindGroup( desc, *layout ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindGroupEntry &entry : desc.entries )
	{
		const auto sampler = m_Samplers.find( entry.sampler.value );
		if ( ( entry.buffer.IsValid() && !LiveBuffer( entry.buffer.value ) ) ||
		     ( entry.texture.IsValid() && !LiveTexture( entry.texture.value ) ) ||
		     ( entry.sampler.IsValid() &&
		         ( sampler == m_Samplers.end() || sampler->second.released ) ) )
			return Fail( DeviceStatus::kInvalidHandle, op );
	}
	BindGroupRecord record;
	record.layout = desc.layout;
	record.role = static_cast<std::uint32_t>( layout->role );
	record.bindings.assign( layout->bindings.begin(), layout->bindings.end() );
	record.entries.assign( desc.entries.begin(), desc.entries.end() );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

id<MTLBuffer> MetalDevice::ArgumentBuffer(
    BindGroupRecord &group, std::uint64_t pipeline, std::uint32_t stage, const StageGroup &view )
{
	const ArgumentKey key{ pipeline, stage };
	if ( const auto found = group.encoded.find( key ); found != group.encoded.end() )
		return found->second;
	id<MTLBuffer> buffer = [m_Device newBufferWithLength:std::max<NSUInteger>(
	                                                         view.encoder.encodedLength, 16 )
	                                             options:MTLResourceStorageModeShared];
	[view.encoder setArgumentBuffer:buffer offset:0];
	for ( const BindGroupEntry &entry : group.entries )
	{
		// Only the ids the stage declares exist in its encoder.
		if ( std::find( view.ids.begin(), view.ids.end(), entry.binding ) == view.ids.end() )
			continue;
		if ( entry.buffer.IsValid() )
			[view.encoder setBuffer:ExistingBuffer( entry.buffer.value )->buffer
			                 offset:entry.offset
			                atIndex:entry.binding];
		else if ( entry.texture.IsValid() )
			[view.encoder setTexture:ExistingTexture( entry.texture.value )->texture
			                 atIndex:entry.binding];
		else if ( entry.sampler.IsValid() )
			[view.encoder setSamplerState:Sampler( entry.sampler.value ) atIndex:entry.binding];
	}
	group.encoded.emplace( key, buffer );
	return buffer;
}

DeviceResult<void> MetalDevice::Release( ResourceId resource, CompletionToken releaseAfter )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	m_Releases.push_back( { resource, releaseAfter } );
	return {};
}

void MetalDevice::Erase( ResourceId resource )
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
		// Groups encoded for its stages are encoded for no other pipeline.
		for ( auto &[_, group] : m_BindGroups )
		{
			for ( std::uint32_t stage = 0; stage < kStageCount; ++stage )
				group.encoded.erase( ArgumentKey{ resource.value, stage } );
		}
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

std::size_t MetalDevice::LiveResourceCount() const
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

MemoryBudgetSnapshot MetalDevice::ReadMemoryBudget() const
{
	MemoryBudgetSnapshot result;
	if ( State() != DeviceState::kAvailable )
		return result;
	result.supported = true;
	result.epoch = m_Epoch;
	// One unified heap: the driver's allocation figure, and its recommended
	// working set as the allowance.
	HeapMemoryBudget heap;
	heap.deviceLocal = true;
	heap.usageKnown = true;
	heap.usageBytes = m_Device.currentAllocatedSize;
	heap.budgetKnown = true;
	heap.budgetBytes = m_Device.recommendedMaxWorkingSetSize;
	result.heaps.push_back( heap );
	return result;
}

// Encoders, submission and completion -------------------------------------------

DeviceResult<CommandEncoder> MetalDevice::BeginEncoder( QueueKind queue )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder( queue, std::make_unique<RecordingEncoder>(
	    static_cast<recording::IUploadStager &>( *this ), this, queue ) );
}

void MetalDevice::OnCompleted( std::uint32_t epoch, std::uint64_t value, bool failed, bool lost )
{
	if ( failed )
		m_Errors.fetch_add( 1 );
	if ( lost )
		m_State.store( DeviceState::kLost );
	std::lock_guard<std::mutex> lock( m_CompletionLock );
	if ( epoch != m_Epoch )
		return;
	// The queue completes in order; should a handler run early, its value
	// waits until every earlier one has completed (D6).
	m_CompletedEarly.insert( value );
	std::uint64_t completed = m_Completed.load( std::memory_order_relaxed );
	while ( m_CompletedEarly.erase( completed + 1 ) )
		++completed;
	m_Completed.store( completed, std::memory_order_release );
}

DeviceResult<CompletionToken> MetalDevice::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	// Encoders are consumed whatever the outcome.
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );

	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		// One queue in order: a wait on an earlier graphics submission holds.
		if ( wait.queue != QueueKind::kGraphics || wait.epoch > m_Epoch ||
		     wait.value > m_Submitted )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<RecordingEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		RecordingEncoder *encoder = dynamic_cast<RecordingEncoder *>( backend.get() );
		if ( !encoder || encoder->Owner() != this || encoder->Queue() != queue )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( !encoder->Complete() || !recording::Validate( encoder->Commands(), states, *this ) )
			return Fail( DeviceStatus::kInvalidState, op );
		recorded.push_back( encoder );
	}
	if ( const std::optional<DeviceStatus> refused =
	         recording::CheckSubmission( recorded, m_Facts.capabilities, states ) )
		return Fail( *refused, op );
	for ( const auto &[id, usage] : states )
	{
		if ( TextureRecord *t = LiveTexture( id ) )
			t->usage = usage;
		else if ( BufferRecord *b = LiveBuffer( id ) )
			b->usage = usage;
	}
	const CompletionToken token{ queue, m_Epoch, ++m_Submitted };
	{
		std::lock_guard<std::mutex> ring( m_RingLock );
		for ( RecordingEncoder *encoder : recorded )
		{
			for ( std::uint64_t allocation : encoder->RingAllocations() )
				m_Ring.Submit( allocation, token );
			encoder->MarkSubmitted();
		}
	}
	MTLCommandBufferDescriptor *descriptor = [MTLCommandBufferDescriptor new];
	if ( m_Options.validation )
		descriptor.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
	id<MTLCommandBuffer> commands = [m_Queue commandBufferWithDescriptor:descriptor];
	if ( m_Options.validation )
		commands.label = [NSString stringWithFormat:@"render.device.metal submission %llu",
		                           static_cast<unsigned long long>( token.value )];
	Execute( commands, recorded );
	const std::uint32_t epoch = token.epoch;
	const std::uint64_t value = token.value;
	MetalDevice *self = this;
	[commands addCompletedHandler:^( id<MTLCommandBuffer> buffer ) {
	  const bool failed = buffer.status == MTLCommandBufferStatusError;
	  const NSInteger code = failed ? buffer.error.code : 0;
	  bool lost = failed && ( code == MTLCommandBufferErrorAccessRevoked ||
	                            code == MTLCommandBufferErrorNotPermitted );
#if TARGET_OS_OSX
	  lost |= failed && code == MTLCommandBufferErrorDeviceRemoved; // an eGPU unplugged
#endif
	  if ( failed )
		  std::fprintf( stderr, "render.device.metal: submission %llu failed: %s\n",
		      static_cast<unsigned long long>( value ),
		      buffer.error.localizedDescription.UTF8String );
	  self->OnCompleted( epoch, value, failed, lost );
	}];
	[commands commit];
	m_LastCommitted = commands;
	return token;
}

bool MetalDevice::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() )
		return true;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( token.epoch < m_Epoch )
		return true;
	if ( token.epoch > m_Epoch || token.queue != QueueKind::kGraphics )
		return false;
	return token.value <= m_Completed.load( std::memory_order_acquire );
}

std::size_t MetalDevice::Collect()
{
	const std::uint64_t completed = m_Completed.load( std::memory_order_acquire );
	std::size_t freed = 0;
	for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
	{
		const CompletionToken &token = it->token;
		const bool done = !token.NamesSubmission() || token.epoch < m_Epoch ||
		                  ( token.epoch == m_Epoch && token.value <= completed );
		if ( !done )
		{
			++it;
			continue;
		}
		Erase( it->resource );
		it = m_Releases.erase( it );
		++freed;
	}
	std::lock_guard<std::mutex> ring( m_RingLock );
	m_Ring.Retire( m_Epoch, completed );
	return freed;
}

std::size_t MetalDevice::Poll()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return Collect();
}

DeviceResult<void> MetalDevice::ReadBuffer(
    BufferId id, std::uint64_t offset, std::span<std::byte> out )
{
	const DeviceOperation op = DeviceOperation::kReadBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	BufferRecord *buffer = LiveBuffer( id.value );
	if ( !buffer )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( buffer->desc.memory != MemoryKind::kReadback || offset > buffer->desc.size ||
	     out.size() > buffer->desc.size - offset )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	std::memcpy(
	    out.data(), static_cast<const std::byte *>( [buffer->buffer contents] ) + offset, out.size() );
	return {};
}

DeviceResult<void> MetalDevice::WaitIdle()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	// The queue runs in order: the last committed buffer completing means
	// every earlier one has.
	if ( m_LastCommitted )
		[m_LastCommitted waitUntilCompleted];
	return {};
}

DeviceResult<void> MetalDevice::Recover()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() == DeviceState::kAvailable )
		return {};
	m_State.store( DeviceState::kRecovering );
	if ( m_LastCommitted )
		[m_LastCommitted waitUntilCompleted];
	m_LastCommitted = nil;
	m_Buffers.clear();
	m_Textures.clear();
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Pipelines.clear();
	m_Releases.clear();
	{
		std::lock_guard<std::mutex> completion( m_CompletionLock );
		++m_Epoch;
		m_CompletedEarly.clear();
		m_Completed.store( 0 );
	}
	m_Submitted = 0;
	// A new device object: the old one may be the removed GPU.
	m_Device = MTLCreateSystemDefaultDevice();
	if ( !m_Device || !CreateQueueObjects() )
	{
		m_State.store( DeviceState::kFatal );
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateDevice );
	}
	m_State.store( DeviceState::kAvailable );
	return {};
}

namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	MetalAdapterOptions options;
	options.validation = request.validation;
	auto created = Create( options );
	if ( !created )
		return created;
	if ( FirstMissing( created.Value()->Facts().capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return created;
}

MetalDevice *Of( const IRenderDevice2 &device )
{
	return dynamic_cast<MetalDevice *>( const_cast<IRenderDevice2 *>( &device ) );
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "metal", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const MetalAdapterOptions &options )
{
	auto device = std::make_unique<MetalDevice>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
}

std::uint64_t CommandBufferErrors( const IRenderDevice2 &device )
{
	const MetalDevice *metal = Of( device );
	return metal ? metal->CommandBufferErrors() : 0;
}

bool SimulateDeviceLoss( IRenderDevice2 &device )
{
	MetalDevice *metal = Of( device );
	if ( metal )
		metal->SimulateLoss();
	return metal != nullptr;
}

} // namespace render::device::metal
