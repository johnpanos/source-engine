//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu: the device, its facts, resources,
//			submission and completion. See webgpu_device.h.
//
//=============================================================================//

#include "webgpu_device.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>

namespace render::device::webgpu
{

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, nativeCode } );
}

WGPUStringView View( std::string_view text )
{
	return WGPUStringView{ text.data(), text.size() };
}

std::string_view Text( WGPUStringView view )
{
	if ( !view.data )
		return {};
	return view.length == WGPU_STRLEN ? std::string_view( view.data )
	                                  : std::string_view( view.data, view.length );
}

WGPUTextureFormat TextureFormatOf( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return WGPUTextureFormat_R8Unorm;
	case Format::kRGBA8Unorm:
		return WGPUTextureFormat_RGBA8Unorm;
	case Format::kRGBA8Srgb:
		return WGPUTextureFormat_RGBA8UnormSrgb;
	case Format::kBGRA8Unorm:
		return WGPUTextureFormat_BGRA8Unorm;
	case Format::kBGRA8Srgb:
		return WGPUTextureFormat_BGRA8UnormSrgb;
	case Format::kRG16Float:
		return WGPUTextureFormat_RG16Float;
	case Format::kRGBA16Float:
		return WGPUTextureFormat_RGBA16Float;
	case Format::kR32Float:
		return WGPUTextureFormat_R32Float;
	case Format::kRGBA32Float:
		return WGPUTextureFormat_RGBA32Float;
	case Format::kD32Float:
		return WGPUTextureFormat_Depth32Float;
	case Format::kD32FloatS8:
		return WGPUTextureFormat_Depth32FloatStencil8;
	case Format::kRGBA16Unorm:
		return WGPUTextureFormat_RGBA16Unorm;
	case Format::kBC1Unorm:
		return WGPUTextureFormat_BC1RGBAUnorm;
	case Format::kBC1Srgb:
		return WGPUTextureFormat_BC1RGBAUnormSrgb;
	case Format::kBC2Unorm:
		return WGPUTextureFormat_BC2RGBAUnorm;
	case Format::kBC2Srgb:
		return WGPUTextureFormat_BC2RGBAUnormSrgb;
	case Format::kBC3Unorm:
		return WGPUTextureFormat_BC3RGBAUnorm;
	case Format::kBC3Srgb:
		return WGPUTextureFormat_BC3RGBAUnormSrgb;
	case Format::kBC4Unorm:
		return WGPUTextureFormat_BC4RUnorm;
	case Format::kBC5Unorm:
		return WGPUTextureFormat_BC5RGUnorm;
	case Format::kBC6HUfloat:
		return WGPUTextureFormat_BC6HRGBUfloat;
	case Format::kBC7Unorm:
		return WGPUTextureFormat_BC7RGBAUnorm;
	case Format::kBC7Srgb:
		return WGPUTextureFormat_BC7RGBAUnormSrgb;
	case Format::kRGB10A2Unorm:
		return WGPUTextureFormat_RGB10A2Unorm;
	case Format::kRG11B10Float:
		return WGPUTextureFormat_RG11B10Ufloat;
	// WebGPU's depth24plus is no 24-bit unorm a buffer can copy: refused by
	// name. ETC1 and ETC1A4 (the 3DS's), packed RGBA4: no WebGPU format here.
	case Format::kD24UnormS8:
	case Format::kETC1Rgb:
	case Format::kETC1A4:
	case Format::kRGBA4Unorm:
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return WGPUTextureFormat_Undefined;
}

namespace
{

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

bool IsFloatFormat( Format format )
{
	switch ( format )
	{
	case Format::kRG16Float:
	case Format::kRGBA16Float:
	case Format::kR32Float:
	case Format::kRGBA32Float:
	case Format::kD32Float:
	case Format::kD32FloatS8:
	case Format::kRG11B10Float:
		return true;
	default:
		return false;
	}
}

WGPUCompareFunction CompareOf( CompareOp op )
{
	switch ( op )
	{
	case CompareOp::kNever:
		return WGPUCompareFunction_Never;
	case CompareOp::kLess:
		return WGPUCompareFunction_Less;
	case CompareOp::kLessEqual:
		return WGPUCompareFunction_LessEqual;
	case CompareOp::kEqual:
		return WGPUCompareFunction_Equal;
	case CompareOp::kGreaterEqual:
		return WGPUCompareFunction_GreaterEqual;
	case CompareOp::kGreater:
		return WGPUCompareFunction_Greater;
	case CompareOp::kAlways:
		return WGPUCompareFunction_Always;
	case CompareOp::kNotEqual:
		return WGPUCompareFunction_NotEqual;
	}
	return WGPUCompareFunction_Always;
}

// A callback's place: the device, the epoch and the submission it belongs to.
struct CallbackTarget
{
	WebGpuDevice *device;
	std::uint32_t epoch;
	std::uint64_t value;
	std::uint64_t buffer;
};

} // namespace

// Device ------------------------------------------------------------------------

WebGpuDevice::WebGpuDevice( const WebGpuAdapterOptions &options ) : m_Options( options )
{
}

WebGpuDevice::~WebGpuDevice()
{
	if ( m_Device )
		(void)WaitIdle(); // reviewed idle wait: teardown
	ClearResources();
	m_EmptyGroup.Reset();
	m_GroupLayouts.clear();
	m_DepthUploads.clear();
	if ( m_Surface )
	{
		wgpuSurfaceUnconfigure( m_Surface );
		wgpuSurfaceRelease( m_Surface );
	}
	if ( m_Queue )
		wgpuQueueRelease( m_Queue );
	if ( m_Device )
		wgpuDeviceRelease( m_Device );
	if ( m_Adapter )
		wgpuAdapterRelease( m_Adapter );
	if ( m_Instance )
	{
		wgpuInstanceProcessEvents( m_Instance ); // callbacks the release cancelled
		wgpuInstanceRelease( m_Instance );
	}
}

void WebGpuDevice::ProcessEvents() const
{
	if ( m_Instance )
		wgpuInstanceProcessEvents( m_Instance );
}

bool WebGpuDevice::WaitFor( WGPUFuture future ) const
{
	WGPUFutureWaitInfo wait{ future, false };
	const WGPUWaitStatus status =
	    wgpuInstanceWaitAny( m_Instance, 1, &wait, std::numeric_limits<std::uint64_t>::max() );
	return status == WGPUWaitStatus_Success && wait.completed;
}

void WebGpuDevice::CountError( std::string_view message )
{
	m_ValidationErrors.fetch_add( 1 );
	if ( m_Options.validationCounter )
		m_Options.validationCounter->fetch_add( 1 );
	std::fprintf( stderr, "render.device.webgpu: %.*s\n", static_cast<int>( message.size() ),
	    message.data() );
}

DeviceResult<void> WebGpuDevice::Initialize()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	// Timed waits: pipeline creation and WaitIdle wait on futures.
	const WGPUInstanceFeatureName timed = WGPUInstanceFeatureName_TimedWaitAny;
	WGPUInstanceDescriptor instance = WGPU_INSTANCE_DESCRIPTOR_INIT;
	instance.requiredFeatureCount = 1;
	instance.requiredFeatures = &timed;
	m_Instance = wgpuCreateInstance( &instance );
	if ( !m_Instance )
		return Fail( DeviceStatus::kUnavailable, op );
	if ( auto opened = OpenDevice(); !opened )
		return opened;
	QueryFacts();
	// The empty group stands in for a pipeline layout's unused roles.
	m_EmptyLayout = CachedLayout( {}, false );
	WGPUBindGroupDescriptor empty = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	empty.layout = m_EmptyLayout->layout.Get();
	m_EmptyGroup.Reset( wgpuDeviceCreateBindGroup( m_Device, &empty ) );
	return {};
}

DeviceResult<void> WebGpuDevice::OpenDevice()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	WGPURequestAdapterOptions adapterOptions = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
	adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;
	WGPURequestAdapterCallbackInfo adapterCallback = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
	adapterCallback.mode = WGPUCallbackMode_WaitAnyOnly;
	adapterCallback.callback = []( WGPURequestAdapterStatus status, WGPUAdapter adapter,
	                               WGPUStringView message, void *userdata1, void * )
	{
		if ( status == WGPURequestAdapterStatus_Success )
			*static_cast<WGPUAdapter *>( userdata1 ) = adapter;
		else
			std::fprintf( stderr, "render.device.webgpu: no adapter: %.*s\n",
			    static_cast<int>( Text( message ).size() ), Text( message ).data() );
	};
	adapterCallback.userdata1 = &m_Adapter;
	if ( !WaitFor( wgpuInstanceRequestAdapter( m_Instance, &adapterOptions, adapterCallback ) ) ||
	     !m_Adapter )
		return Fail( DeviceStatus::kUnavailable, op );

	WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
	if ( wgpuAdapterGetInfo( m_Adapter, &info ) == WGPUStatus_Success )
	{
		// A browser may report only the vendor and architecture.
		m_AdapterName = std::string( Text( info.device ) );
		if ( m_AdapterName.empty() )
			m_AdapterName = std::string( Text( info.description ) );
		if ( m_AdapterName.empty() && info.vendor.length > 0 )
			m_AdapterName =
			    std::string( Text( info.vendor ) ) + " " + std::string( Text( info.architecture ) );
		wgpuAdapterInfoFreeMembers( info );
	}
	if ( m_AdapterName.empty() )
		m_AdapterName = "WebGPU";
	WGPULimits limits = WGPU_LIMITS_INIT;
	if ( wgpuAdapterGetLimits( m_Adapter, &limits ) != WGPUStatus_Success )
		return Fail( DeviceStatus::kUnavailable, op );
	// The port's four groups (plus the draw constants' group, which is the
	// fourth) and its draw constants' dynamic offset.
	if ( limits.maxBindGroups < kMaxBindGroups ||
	     limits.maxDynamicUniformBuffersPerPipelineLayout < 1 )
		return Fail( DeviceStatus::kUnsupported, op );

	// Every optional feature the adapter has that a claim can use.
	const WGPUFeatureName wanted[] = { WGPUFeatureName_TextureCompressionBC,
	    WGPUFeatureName_IndirectFirstInstance, WGPUFeatureName_Float32Filterable,
	    WGPUFeatureName_RG11B10UfloatRenderable, WGPUFeatureName_Depth32FloatStencil8,
	    WGPUFeatureName_Unorm16TextureFormats };
	m_Features.clear();
	for ( WGPUFeatureName feature : wanted )
	{
		if ( wgpuAdapterHasFeature( m_Adapter, feature ) )
			m_Features.push_back( feature );
	}
	WGPUDeviceDescriptor descriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
	descriptor.label = View( "render.device.webgpu" );
	descriptor.requiredFeatureCount = m_Features.size();
	descriptor.requiredFeatures = m_Features.data();
	descriptor.requiredLimits = &limits;
	descriptor.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
	descriptor.deviceLostCallbackInfo.callback =
	    []( WGPUDevice const *, WGPUDeviceLostReason reason, WGPUStringView message,
	        void *userdata1, void * )
	{
		auto *self = static_cast<WebGpuDevice *>( userdata1 );
		// Destruction is not a loss.
		if ( reason == WGPUDeviceLostReason_Destroyed ||
		     reason == WGPUDeviceLostReason_CallbackCancelled )
			return;
		std::fprintf( stderr, "render.device.webgpu: device lost: %.*s\n",
		    static_cast<int>( Text( message ).size() ), Text( message ).data() );
		self->SimulateLoss();
	};
	descriptor.deviceLostCallbackInfo.userdata1 = this;
	descriptor.uncapturedErrorCallbackInfo.callback =
	    []( WGPUDevice const *, WGPUErrorType, WGPUStringView message, void *userdata1, void * )
	{
		static_cast<WebGpuDevice *>( userdata1 )->CountError( Text( message ) );
	};
	descriptor.uncapturedErrorCallbackInfo.userdata1 = this;
	WGPURequestDeviceCallbackInfo deviceCallback = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
	deviceCallback.mode = WGPUCallbackMode_WaitAnyOnly;
	deviceCallback.callback = []( WGPURequestDeviceStatus status, WGPUDevice device,
	                              WGPUStringView message, void *userdata1, void * )
	{
		if ( status == WGPURequestDeviceStatus_Success )
			*static_cast<WGPUDevice *>( userdata1 ) = device;
		else
			std::fprintf( stderr, "render.device.webgpu: no device: %.*s\n",
			    static_cast<int>( Text( message ).size() ), Text( message ).data() );
	};
	deviceCallback.userdata1 = &m_Device;
	if ( !WaitFor( wgpuAdapterRequestDevice( m_Adapter, &descriptor, deviceCallback ) ) ||
	     !m_Device )
		return Fail( DeviceStatus::kUnavailable, op );
	m_Limits = WGPU_LIMITS_INIT;
	(void)wgpuDeviceGetLimits( m_Device, &m_Limits );
	m_Queue = wgpuDeviceGetQueue( m_Device );
	return {};
}

void WebGpuDevice::QueryFacts()
{
	auto has = [&]( WGPUFeatureName feature )
	{
		return std::find( m_Features.begin(), m_Features.end(), feature ) != m_Features.end();
	};
	// Compute, storage buffers and cube arrays are core WebGPU; indexed
	// indirect draws are one draw per record (D30).
	CapabilitySet have{ Capability::kCompute, Capability::kStorageBuffers, Capability::kCubeArrays,
	    Capability::kMultiDrawIndirect };
	if ( has( WGPUFeatureName_IndirectFirstInstance ) )
		have.Add( Capability::kIndirectFirstInstance );
	if ( has( WGPUFeatureName_TextureCompressionBC ) )
		have.Add( Capability::kTextureCompressionBC );
	// Float targets (D39): the float formats are core, but they are sampled
	// through filtering layouts, so 32-bit floats need float32-filterable.
	if ( has( WGPUFeatureName_Float32Filterable ) &&
	     has( WGPUFeatureName_RG11B10UfloatRenderable ) &&
	     has( WGPUFeatureName_Depth32FloatStencil8 ) )
		have.Add( Capability::kFloatTargets );
	// Not claimed: parallel recording (CPU lists replayed at Submit), async
	// queues (one queue), transient aliasing, ray query, external images,
	// draw-indirect count, timestamps between commands, exact occlusion
	// counts (WebGPU's are boolean in practice), line fill, ETC1 (no ETC1A4)
	// and packed RGBA4.
	CapabilitySet claimed;
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( Capability::kCount ); ++bit )
	{
		const auto capability = static_cast<Capability>( bit );
		if ( have.Has( capability ) && m_Options.allowed.Has( capability ) )
			claimed.Add( capability );
	}
	Limits limits;
	limits.maxBindGroups = kMaxBindGroups;
	limits.maxTextureDimension2D = m_Limits.maxTextureDimension2D;
	limits.maxColorAttachments = std::min<std::uint32_t>( m_Limits.maxColorAttachments, 8 );
	limits.maxVertexBuffers = std::min<std::uint32_t>( m_Limits.maxVertexBuffers, kMaxVertexSlots );
	limits.uniformBufferAlignment = m_Limits.minUniformBufferOffsetAlignment;
	limits.sampleCounts = 1u | ( 1u << 2 ); // WebGPU: 1 and 4 samples
	m_Facts.diagnosticBackend = "webgpu";
	m_Facts.adapterName = m_AdapterName;
	m_Facts.capabilities = claimed;
	m_Facts.limits = limits;
	m_Facts.artifactFormat = ArtifactFormat::kWgsl;
	m_Facts.timestampPeriodNs = 0.0;
}

BufferRecord *WebGpuDevice::LiveBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
}

TextureRecord *WebGpuDevice::LiveTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
}

BufferRecord *WebGpuDevice::ExistingBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() ? &found->second : nullptr;
}

TextureRecord *WebGpuDevice::ExistingTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() ? &found->second : nullptr;
}

const PipelineRecord *WebGpuDevice::ExistingPipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	return found != m_Pipelines.end() ? &found->second : nullptr;
}

BindGroupRecord *WebGpuDevice::ExistingBindGroup( std::uint64_t id )
{
	const auto found = m_BindGroups.find( id );
	return found != m_BindGroups.end() ? &found->second : nullptr;
}

const SamplerRecord *WebGpuDevice::ExistingSampler( std::uint64_t id ) const
{
	const auto found = m_Samplers.find( id );
	return found != m_Samplers.end() ? &found->second : nullptr;
}

// Recorded resources (recording::IRecordedResources) --------------------------------

std::optional<recording::TextureView> WebGpuDevice::Texture( std::uint64_t id ) const
{
	const auto found = m_Textures.find( id );
	if ( found == m_Textures.end() || found->second.released )
		return std::nullopt;
	return recording::TextureView{ found->second.desc, found->second.layers, found->second.usage };
}

std::optional<recording::BufferView> WebGpuDevice::Buffer( std::uint64_t id ) const
{
	const auto found = m_Buffers.find( id );
	if ( found == m_Buffers.end() || found->second.released )
		return std::nullopt;
	return recording::BufferView{ found->second.desc, found->second.usage };
}

std::optional<recording::PipelineView> WebGpuDevice::Pipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	if ( found == m_Pipelines.end() || found->second.released )
		return std::nullopt;
	const PipelineRecord &p = found->second;
	return recording::PipelineView{ p.kind, p.colorFormats, p.depthFormat, p.sampleCount,
	    p.vertexBuffers, p.drawConstantBytes, p.layouts, p.layoutHasBindings };
}

std::optional<BindGroupLayoutId> WebGpuDevice::BindGroup( std::uint64_t id ) const
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

bool WebGpuDevice::CanClear( const recording::TextureView &texture ) const
{
	return TextureFormatOf( texture.desc.format ) != WGPUTextureFormat_Undefined;
}

bool WebGpuDevice::CanCopyWithBuffer( const recording::TextureView & ) const
{
	// Depth copies out are WebGPU's; copies in are refused at Submit
	// (Inexpressible).
	return true;
}

std::optional<LayoutView> WebGpuDevice::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

bool *WebGpuDevice::ReleasedFlag( ResourceId resource )
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

// Resources -----------------------------------------------------------------------

DeviceResult<BufferId> WebGpuDevice::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.allocated = ( desc.size + 3 ) & ~std::uint64_t( 3 );
	// Storage lists get a floor that an empty list is bound up to (BuiltGroup:
	// WGSL's minimum binding size); WebGPU zero-fills it.
	if ( desc.usages.Has( ResourceUsage::kStorageRead ) || desc.usages.Has( ResourceUsage::kStorageWrite ) )
		record.allocated = std::max<std::uint64_t>( record.allocated, 256 );
	// Copies in and out are always allowed (uploads, the readback copy, the
	// per-row texture copies); the port's usages add the rest.
	WGPUBufferUsage usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst;
	if ( desc.usages.Has( ResourceUsage::kVertex ) )
		usage |= WGPUBufferUsage_Vertex;
	if ( desc.usages.Has( ResourceUsage::kIndex ) )
		usage |= WGPUBufferUsage_Index;
	if ( desc.usages.Has( ResourceUsage::kIndirect ) )
		usage |= WGPUBufferUsage_Indirect;
	if ( desc.usages.Has( ResourceUsage::kUniform ) )
		usage |= WGPUBufferUsage_Uniform;
	// A copy source may feed a depth texture, which is drawn from storage.
	if ( desc.usages.Has( ResourceUsage::kStorageRead ) ||
	     desc.usages.Has( ResourceUsage::kStorageWrite ) ||
	     desc.usages.Has( ResourceUsage::kCopySource ) )
		usage |= WGPUBufferUsage_Storage;
	WGPUBufferDescriptor descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
	descriptor.label = View( desc.debugName );
	descriptor.usage = usage;
	descriptor.size = record.allocated;
	record.buffer.Reset( wgpuDeviceCreateBuffer( m_Device, &descriptor ) );
	if ( !record.buffer )
		return Fail( DeviceStatus::kOutOfMemory, op );
	if ( desc.memory == MemoryKind::kReadback )
	{
		WGPUBufferDescriptor readback = WGPU_BUFFER_DESCRIPTOR_INIT;
		readback.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
		readback.size = record.allocated;
		record.readback.Reset( wgpuDeviceCreateBuffer( m_Device, &readback ) );
		if ( !record.readback )
			return Fail( DeviceStatus::kOutOfMemory, op );
		record.shadow.assign( desc.size, std::byte{ 0 } );
	}
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BufferId> WebGpuDevice::CreateUploadBuffer( std::span<const std::byte> bytes )
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
	record.allocated = ( bytes.size() + 3 ) & ~std::uint64_t( 3 );
	// Initialized before this returns (D25): mapped at creation, filled and
	// unmapped here.
	WGPUBufferDescriptor descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
	descriptor.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_Storage;
	descriptor.size = record.allocated;
	descriptor.mappedAtCreation = true;
	record.buffer.Reset( wgpuDeviceCreateBuffer( m_Device, &descriptor ) );
	void *mapped = record.buffer ? wgpuBufferGetMappedRange( record.buffer.Get(), 0,
	                                   static_cast<std::size_t>( record.allocated ) )
	                             : nullptr;
	if ( !mapped )
		return Fail( DeviceStatus::kOutOfMemory, op );
	std::memset( mapped, 0, static_cast<std::size_t>( record.allocated ) );
	std::memcpy( mapped, bytes.data(), bytes.size() );
	wgpuBufferUnmap( record.buffer.Get() );
	record.usage = ResourceUsage::kCopySource;
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<TextureId> WebGpuDevice::CreateTexture( const TextureDesc &desc )
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
	if ( const std::optional<Capability> needed = FormatCapability( desc.format );
	    needed && !m_Facts.capabilities.Has( *needed ) )
		return Fail( DeviceStatus::kUnsupported, op );
	const WGPUTextureFormat format = TextureFormatOf( desc.format );
	if ( format == WGPUTextureFormat_Undefined ||
	     ( desc.format == Format::kRGBA16Unorm &&
	         std::find( m_Features.begin(), m_Features.end(),
	             WGPUFeatureName_Unorm16TextureFormats ) == m_Features.end() ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.sampleCount != 1 && desc.sampleCount != 4 )
		return Fail( DeviceStatus::kUnsupported, op );
	const bool attachment = desc.usages.Has( ResourceUsage::kColorAttachment ) ||
	                        desc.usages.Has( ResourceUsage::kDepthWrite ) ||
	                        desc.usages.Has( ResourceUsage::kDepthRead ) ||
	                        desc.usages.Has( ResourceUsage::kResolveDestination );
	const bool storage = desc.usages.Has( ResourceUsage::kStorageRead ) ||
	                     desc.usages.Has( ResourceUsage::kStorageWrite );
	// Shader writes take no sRGB, depth or compressed format.
	if ( storage && ( IsDepthFormat( desc.format ) || IsSrgb( desc.format ) ||
	                    IsBlockCompressed( desc.format ) ) )
		return Fail( DeviceStatus::kUnsupported, op );

	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
	descriptor.label = View( desc.debugName );
	descriptor.format = format;
	// WebGPU sizes a block-compressed texture's first mip in whole blocks;
	// the port does not. Such a texture is made whole blocks larger: copies
	// keep the port's regions, and sampling it is refused by name (its
	// coordinates would cover the padding).
	const FormatBlock block = BlockOf( desc.format );
	const std::uint32_t width = ( desc.width + block.width - 1 ) / block.width * block.width;
	const std::uint32_t height = ( desc.height + block.height - 1 ) / block.height * block.height;
	record.padded = width != desc.width || height != desc.height;
	descriptor.size = { width, height, desc.depthOrLayers };
	descriptor.mipLevelCount = desc.mipLevels;
	descriptor.sampleCount = desc.sampleCount;
	switch ( desc.dimension )
	{
	case TextureDimension::k2D:
	case TextureDimension::kCube:
		descriptor.dimension = WGPUTextureDimension_2D;
		record.layers = desc.depthOrLayers;
		break;
	case TextureDimension::k3D:
		descriptor.dimension = WGPUTextureDimension_3D;
		record.layers = 1;
		break;
	}
	WGPUTextureUsage usage = WGPUTextureUsage_TextureBinding;
	if ( desc.sampleCount == 1 )
		usage |= WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
	// Clears are render passes: a clearable format is a render target too.
	const bool renderable = !IsBlockCompressed( desc.format ) && !IsFloatFormat( desc.format )
	                            ? true
	                            : m_Facts.capabilities.Has( Capability::kFloatTargets ) &&
	                                  !IsBlockCompressed( desc.format );
	if ( attachment || ( desc.usages.Has( ResourceUsage::kCopyDestination ) && renderable ) )
		usage |= WGPUTextureUsage_RenderAttachment;
	if ( attachment && !renderable )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( storage )
		usage |= WGPUTextureUsage_StorageBinding;
	descriptor.usage = usage;
	wgpuDevicePushErrorScope( m_Device, WGPUErrorFilter_Validation );
	record.texture.Reset( wgpuDeviceCreateTexture( m_Device, &descriptor ) );
	bool failed = false;
	WGPUPopErrorScopeCallbackInfo scope = WGPU_POP_ERROR_SCOPE_CALLBACK_INFO_INIT;
	scope.mode = WGPUCallbackMode_WaitAnyOnly;
	scope.callback = []( WGPUPopErrorScopeStatus, WGPUErrorType type, WGPUStringView message,
	                     void *userdata1, void * )
	{
		if ( type != WGPUErrorType_NoError )
		{
			*static_cast<bool *>( userdata1 ) = true;
			std::fprintf( stderr, "render.device.webgpu: texture refused: %.*s\n",
			    static_cast<int>( Text( message ).size() ), Text( message ).data() );
		}
	};
	scope.userdata1 = &failed;
	(void)WaitFor( wgpuDevicePopErrorScope( m_Device, scope ) );
	if ( !record.texture || failed )
		return Fail( DeviceStatus::kUnsupported, op );
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	return id;
}

WGPUTextureView WebGpuDevice::SampledView(
    TextureRecord &texture, WGPUTextureViewDimension dimension, bool depthAspect )
{
	const std::uint64_t key = ( std::uint64_t( 0 ) << 60 ) | ( std::uint64_t( dimension ) << 8 ) |
	                          ( depthAspect ? 1u : 0u );
	if ( const auto found = texture.views.find( key ); found != texture.views.end() )
		return found->second.Get();
	WGPUTextureViewDescriptor descriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
	descriptor.dimension = dimension;
	descriptor.aspect = depthAspect ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
	descriptor.mipLevelCount = texture.desc.mipLevels;
	descriptor.arrayLayerCount = dimension == WGPUTextureViewDimension_2D     ? 1
	                             : dimension == WGPUTextureViewDimension_3D   ? 1
	                             : dimension == WGPUTextureViewDimension_Cube ? 6
	                                                                          : texture.layers;
	TextureViewHandle view( wgpuTextureCreateView( texture.texture.Get(), &descriptor ) );
	const WGPUTextureView raw = view.Get();
	texture.views.emplace( key, std::move( view ) );
	return raw;
}

WGPUTextureView WebGpuDevice::AttachmentView(
    TextureRecord &texture, std::uint32_t mip, std::uint32_t layer )
{
	const std::uint64_t key =
	    ( std::uint64_t( 1 ) << 60 ) | ( std::uint64_t( mip ) << 32 ) | std::uint64_t( layer );
	if ( const auto found = texture.views.find( key ); found != texture.views.end() )
		return found->second.Get();
	WGPUTextureViewDescriptor descriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
	const bool volume = texture.desc.dimension == TextureDimension::k3D;
	descriptor.dimension = volume ? WGPUTextureViewDimension_3D : WGPUTextureViewDimension_2D;
	descriptor.baseMipLevel = mip;
	descriptor.mipLevelCount = 1;
	descriptor.baseArrayLayer = volume ? 0 : layer;
	descriptor.arrayLayerCount = 1;
	TextureViewHandle view( wgpuTextureCreateView( texture.texture.Get(), &descriptor ) );
	const WGPUTextureView raw = view.Get();
	texture.views.emplace( key, std::move( view ) );
	return raw;
}

DeviceResult<SamplerId> WebGpuDevice::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateSampler( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	WGPUSamplerDescriptor descriptor = WGPU_SAMPLER_DESCRIPTOR_INIT;
	auto filter = []( Filter f )
	{
		return f == Filter::kLinear ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
	};
	descriptor.minFilter = filter( desc.minFilter );
	descriptor.magFilter = filter( desc.magFilter );
	descriptor.mipmapFilter = desc.mipFilter == Filter::kLinear ? WGPUMipmapFilterMode_Linear
	                                                            : WGPUMipmapFilterMode_Nearest;
	const auto addressOf = []( AddressMode mode )
	{
		return mode == AddressMode::kClampToEdge      ? WGPUAddressMode_ClampToEdge
		       : mode == AddressMode::kMirroredRepeat ? WGPUAddressMode_MirrorRepeat
		                                              : WGPUAddressMode_Repeat;
	};
	descriptor.addressModeU = addressOf( desc.address );
	descriptor.addressModeV = addressOf( AddressV( desc ) );
	descriptor.addressModeW = addressOf( desc.address );
	// WebGPU allows anisotropy only with every filter linear.
	const bool linear = desc.minFilter == Filter::kLinear && desc.magFilter == Filter::kLinear &&
	                    desc.mipFilter == Filter::kLinear;
	descriptor.maxAnisotropy =
	    static_cast<std::uint16_t>( linear ? std::clamp( desc.maxAnisotropy, 1u, 16u ) : 1u );
	// D24: compare the reference with each stored depth.
	descriptor.compare =
	    desc.comparison ? CompareOf( *desc.comparison ) : WGPUCompareFunction_Undefined;
	SamplerRecord record;
	record.sampler.Reset( wgpuDeviceCreateSampler( m_Device, &descriptor ) );
	if ( !record.sampler )
		return Fail( DeviceStatus::kInternal, op );
	record.comparison = desc.comparison.has_value();
	record.filtering = desc.minFilter == Filter::kLinear || desc.magFilter == Filter::kLinear ||
	                   desc.mipFilter == Filter::kLinear;
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupLayoutId> WebGpuDevice::CreateBindGroupLayout(
    const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindingDesc &binding : desc.bindings )
	{
		// WebGPU core has no binding arrays, and the draw group's binding
		// kDrawConstantsBinding is the draw constants'.
		if ( binding.count > 1 ||
		     ( desc.role == BindGroupRole::kDraw && binding.binding == kDrawConstantsBinding ) ||
		     binding.binding >= m_Limits.maxBindingsPerBindGroup )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupId> WebGpuDevice::CreateBindGroup( const BindGroupDesc &desc )
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
	// The WebGPU groups are built when a pipeline first uses this one: only
	// then is their layout known.
	BindGroupRecord record;
	record.layout = desc.layout;
	record.bindings.assign( layout->bindings.begin(), layout->bindings.end() );
	record.entries.assign( desc.entries.begin(), desc.entries.end() );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

WGPUBindGroup WebGpuDevice::BuiltGroup( BindGroupRecord &group, const GroupLayout &layout )
{
	if ( const auto found = group.built.find( layout.key ); found != group.built.end() )
		return found->second.Get();
	std::vector<WGPUBindGroupEntry> entries;
	for ( const BindingLine &line : layout.entries )
	{
		const auto entry = std::find_if( group.entries.begin(), group.entries.end(),
		    [&]( const BindGroupEntry &e )
		    {
			    return e.binding == line.source;
		    } );
		if ( entry == group.entries.end() )
			return nullptr;
		WGPUBindGroupEntry out = WGPU_BIND_GROUP_ENTRY_INIT;
		out.binding = line.binding;
		switch ( line.kind )
		{
		case BindingLine::Kind::kUniform:
		case BindingLine::Kind::kStorage:
		case BindingLine::Kind::kReadOnlyStorage:
		{
			BufferRecord *buffer = ExistingBuffer( entry->buffer.value );
			if ( !buffer )
				return nullptr;
			out.buffer = buffer->buffer.Get();
			out.offset = entry->offset;
			out.size = entry->size ? entry->size : buffer->desc.size - entry->offset;
			// WGSL has no empty runtime array: a binding smaller than the
			// stage's minimum (an empty list) is bound up to it, into the
			// allocation's zeros (CreateBuffer's floor).
			if ( out.size < line.minSize && entry->offset + line.minSize <= buffer->allocated )
				out.size = line.minSize;
			break;
		}
		case BindingLine::Kind::kTexture:
		case BindingLine::Kind::kStorageTexture:
		{
			TextureRecord *texture = ExistingTexture( entry->texture.value );
			if ( !texture )
				return nullptr;
			if ( texture->padded )
			{
				std::fprintf( stderr, "render.device.webgpu: refused: sampling a block-compressed "
				                      "texture whose first mip is not whole blocks\n" );
				return nullptr;
			}
			const bool depth =
			    IsDepthFormat( texture->desc.format ) && HasStencil( texture->desc.format );
			out.textureView = SampledView( *texture, line.dimension, depth );
			break;
		}
		case BindingLine::Kind::kSampler:
		{
			const SamplerRecord *sampler = ExistingSampler( entry->sampler.value );
			if ( !sampler )
				return nullptr;
			out.sampler = sampler->sampler.Get();
			break;
		}
		}
		entries.push_back( out );
	}
	WGPUBindGroupDescriptor descriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	descriptor.layout = layout.layout.Get();
	descriptor.entryCount = entries.size();
	descriptor.entries = entries.data();
	BindGroupHandle built( wgpuDeviceCreateBindGroup( m_Device, &descriptor ) );
	const WGPUBindGroup raw = built.Get();
	group.built.emplace( layout.key, std::move( built ) );
	return raw;
}

WGPUBindGroup WebGpuDevice::ConstantsGroup( BindGroupRecord *group, const GroupLayout &layout,
    WGPUBuffer constants, Submission &submission )
{
	std::vector<WGPUBindGroupEntry> entries;
	for ( const BindingLine &line : layout.entries )
	{
		if ( !group )
			return nullptr;
		// The draw group's own entries, built as BuiltGroup builds them.
		const auto entry = std::find_if( group->entries.begin(), group->entries.end(),
		    [&]( const BindGroupEntry &e )
		    {
			    return e.binding == line.source;
		    } );
		if ( entry == group->entries.end() )
			return nullptr;
		WGPUBindGroupEntry out = WGPU_BIND_GROUP_ENTRY_INIT;
		out.binding = line.binding;
		if ( entry->buffer.IsValid() )
		{
			BufferRecord *buffer = ExistingBuffer( entry->buffer.value );
			out.buffer = buffer->buffer.Get();
			out.offset = entry->offset;
			out.size = entry->size ? entry->size : buffer->desc.size - entry->offset;
			if ( out.size < line.minSize && entry->offset + line.minSize <= buffer->allocated )
				out.size = line.minSize; // as BuiltGroup binds an empty list
		}
		else if ( entry->texture.IsValid() )
		{
			TextureRecord *texture = ExistingTexture( entry->texture.value );
			out.textureView = SampledView( *texture, line.dimension,
			    IsDepthFormat( texture->desc.format ) && HasStencil( texture->desc.format ) );
		}
		else if ( entry->sampler.IsValid() )
		{
			out.sampler = ExistingSampler( entry->sampler.value )->sampler.Get();
		}
		entries.push_back( out );
	}
	WGPUBindGroupEntry block = WGPU_BIND_GROUP_ENTRY_INIT;
	block.binding = kDrawConstantsBinding;
	block.buffer = constants;
	block.offset = 0;
	block.size = kMaxDrawConstantBytes;
	entries.push_back( block );
	WGPUBindGroupDescriptor descriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	descriptor.layout = layout.layout.Get();
	descriptor.entryCount = entries.size();
	descriptor.entries = entries.data();
	submission.groups.emplace_back( wgpuDeviceCreateBindGroup( m_Device, &descriptor ) );
	return submission.groups.back().Get();
}

DeviceResult<void> WebGpuDevice::Release( ResourceId resource, CompletionToken releaseAfter )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	m_Releases.push_back( { resource, releaseAfter } );
	return {};
}

void WebGpuDevice::Erase( ResourceId resource )
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

void WebGpuDevice::ClearResources()
{
	m_Buffers.clear();
	m_Textures.clear();
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Pipelines.clear();
	m_Releases.clear();
	m_InFlight.clear();
	m_HeldSubmissions.clear();
}

std::size_t WebGpuDevice::LiveResourceCount() const
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

// Encoders, submission and completion -------------------------------------------

void WebGpuDevice::StageUpload(
    RecordingEncoder &, Command &command, std::span<const std::byte> bytes )
{
	command.fromRing = false;
	command.bytes.assign( bytes.begin(), bytes.end() );
}

DeviceResult<CommandEncoder> WebGpuDevice::BeginEncoder( QueueKind queue )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder(
	    queue, std::make_unique<RecordingEncoder>(
	               static_cast<recording::IUploadStager &>( *this ), this, queue ) );
}

namespace
{

// What WebGPU's copy rules cannot express (offsets and sizes in whole words,
// tightly packed rows of whole words), refused by name before anything runs.
const char *Inexpressible( const Command &command,
    const std::unordered_map<std::uint64_t, TextureRecord> &textures,
    const std::unordered_map<std::uint64_t, BufferRecord> &buffers )
{
	switch ( command.op )
	{
	case Op::kWriteBuffer:
	{
		// A write that ends at the buffer's end is padded into the buffer's
		// whole-word allocation (WriteEnd); any other partial word is refused.
		const auto buffer = buffers.find( command.a );
		const bool toEnd = buffer != buffers.end() &&
		                   command.copy.destinationOffset + command.bytes.size() ==
		                       buffer->second.desc.size;
		if ( command.copy.destinationOffset % 4 != 0 ||
		     ( command.bytes.size() % 4 != 0 && !toEnd ) )
			return "a buffer write of bytes that are not whole words";
		break;
	}
	case Op::kCopyBuffer:
		if ( command.copy.sourceOffset % 4 != 0 || command.copy.destinationOffset % 4 != 0 ||
		     command.copy.size % 4 != 0 )
			return "a buffer copy of bytes that are not whole words";
		break;
	case Op::kCopyTextureToBuffer:
	case Op::kCopyBufferToTexture:
	{
		const auto texture =
		    textures.find( command.op == Op::kCopyTextureToBuffer ? command.a : command.b );
		if ( texture == textures.end() )
			break;
		// Copies into depth are drawn (DepthUploadFor), from whole floats.
		if ( command.op == Op::kCopyBufferToTexture &&
		     IsDepthFormat( texture->second.desc.format ) &&
		     command.textureCopy.bufferOffset % 4 != 0 )
			return "a buffer copied into a depth texture from a partial word";
		const std::uint64_t row =
		    RegionBytes( texture->second.desc.format, command.textureCopy.width, 1 );
		if ( row % kCopyRowAlignment != 0 &&
		     ( row % 4 != 0 || command.textureCopy.bufferOffset % 4 != 0 ) )
			return "a texture copy whose rows are not whole words";
		break;
	}
	default:
		break;
	}
	return nullptr;
}

} // namespace

DeviceResult<CompletionToken> WebGpuDevice::Submit(
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
	for ( RecordingEncoder *encoder : recorded )
	{
		for ( const Command &command : encoder->Commands() )
		{
			if ( const char *why = Inexpressible( command, m_Textures, m_Buffers ) )
			{
				std::fprintf( stderr, "render.device.webgpu: refused: %s\n", why );
				return Fail( DeviceStatus::kUnsupported, op );
			}
		}
	}
	auto submission = std::make_unique<Submission>();
	CommandBuffer commands = Execute( recorded, *submission );
	if ( !commands )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const auto &[id, usage] : states )
	{
		if ( TextureRecord *t = LiveTexture( id ) )
			t->usage = usage;
		else if ( BufferRecord *b = LiveBuffer( id ) )
			b->usage = usage;
	}
	for ( RecordingEncoder *encoder : recorded )
		encoder->MarkSubmitted();
	const CompletionToken token{ queue, m_Epoch, ++m_Submitted };
	submission->value = token.value;
	if ( m_Options.sensitivity.completeOnSubmit )
		m_Completed.store( token.value );
	if ( m_Held )
		m_HeldSubmissions.emplace_back( std::move( submission ), std::move( commands ) );
	else
		SubmitNative( std::move( submission ), std::move( commands ) );
	return token;
}

void WebGpuDevice::SubmitNative( std::unique_ptr<Submission> submission, CommandBuffer commands )
{
	// A readback copy still mapping from an earlier submission is waited
	// for: the queue may not copy into a buffer being mapped.
	for ( std::uint64_t id : submission->readbacks )
	{
		BufferRecord *b = ExistingBuffer( id );
		if ( b && b->mapPending )
		{
			(void)WaitFor( b->mapFuture );
			ProcessEvents();
		}
	}
	const WGPUCommandBuffer raw = commands.Get();
	wgpuQueueSubmit( m_Queue, 1, &raw );
	Submission &s = *submission;
	m_InFlight.push_back( std::move( submission ) );
	WGPUQueueWorkDoneCallbackInfo done = WGPU_QUEUE_WORK_DONE_CALLBACK_INFO_INIT;
	done.mode = WGPUCallbackMode_AllowProcessEvents;
	done.callback = []( WGPUQueueWorkDoneStatus status, WGPUStringView, void *userdata1, void * )
	{
		std::unique_ptr<CallbackTarget> target( static_cast<CallbackTarget *>( userdata1 ) );
		target->device->OnWorkDone(
		    target->epoch, target->value, status != WGPUQueueWorkDoneStatus_Success );
	};
	done.userdata1 = new CallbackTarget{ this, m_Epoch, s.value, 0 };
	(void)wgpuQueueOnSubmittedWorkDone( m_Queue, done );
	for ( std::uint64_t id : s.readbacks )
	{
		BufferRecord *b = ExistingBuffer( id );
		b->mapPending = true;
		++s.pendingMaps;
		WGPUBufferMapCallbackInfo mapped = WGPU_BUFFER_MAP_CALLBACK_INFO_INIT;
		mapped.mode = WGPUCallbackMode_AllowProcessEvents;
		mapped.callback = []( WGPUMapAsyncStatus status, WGPUStringView, void *userdata1, void * )
		{
			std::unique_ptr<CallbackTarget> target( static_cast<CallbackTarget *>( userdata1 ) );
			target->device->OnMapped( target->epoch, target->value, target->buffer,
			    status == WGPUMapAsyncStatus_Success );
		};
		mapped.userdata1 = new CallbackTarget{ this, m_Epoch, s.value, id };
		b->mapFuture = wgpuBufferMapAsync( b->readback.Get(), WGPUMapMode_Read, 0,
		    static_cast<std::size_t>( b->allocated ), mapped );
	}
}

void WebGpuDevice::Hold( bool held )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_Held = held;
	if ( held )
		return;
	auto pending = std::move( m_HeldSubmissions );
	m_HeldSubmissions.clear();
	for ( auto &[submission, commands] : pending )
		SubmitNative( std::move( submission ), std::move( commands ) );
}

void WebGpuDevice::OnWorkDone( std::uint32_t epoch, std::uint64_t value, bool failed )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( epoch != m_Epoch )
		return;
	for ( std::unique_ptr<Submission> &s : m_InFlight )
	{
		if ( s->value == value )
		{
			s->workDone = true;
			s->failed |= failed;
		}
	}
	Advance();
}

void WebGpuDevice::OnMapped( std::uint32_t epoch, std::uint64_t value, std::uint64_t id, bool ok )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	BufferRecord *b = epoch == m_Epoch ? ExistingBuffer( id ) : nullptr;
	if ( b )
	{
		if ( ok )
		{
			const void *bytes = wgpuBufferGetConstMappedRange(
			    b->readback.Get(), 0, static_cast<std::size_t>( b->allocated ) );
			if ( bytes )
				std::memcpy( b->shadow.data(), bytes, b->shadow.size() );
			wgpuBufferUnmap( b->readback.Get() );
		}
		b->mapPending = false;
	}
	if ( epoch != m_Epoch )
		return;
	for ( std::unique_ptr<Submission> &s : m_InFlight )
	{
		if ( s->value == value && s->pendingMaps > 0 )
		{
			--s->pendingMaps;
			s->failed |= !ok;
		}
	}
	Advance();
}

void WebGpuDevice::Advance()
{
	// The queue completes in order (D6): a submission's token completes once
	// it and every earlier one is done.
	while ( !m_InFlight.empty() && m_InFlight.front()->workDone &&
	        m_InFlight.front()->pendingMaps == 0 )
	{
		if ( m_InFlight.front()->failed )
			CountError( "a submission failed" );
		m_Completed.store( std::max( m_Completed.load(), m_InFlight.front()->value ) );
		m_InFlight.pop_front();
	}
}

bool WebGpuDevice::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() )
		return true;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( token.epoch < m_Epoch )
		return true;
	if ( token.epoch > m_Epoch || token.queue != QueueKind::kGraphics )
		return false;
	if ( token.value > m_Completed.load() )
		ProcessEvents();
	return token.value <= m_Completed.load();
}

std::size_t WebGpuDevice::Collect()
{
	const std::uint64_t completed = m_Completed.load();
	std::size_t freed = 0;
	for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
	{
		const CompletionToken &token = it->token;
		const bool done = m_Options.sensitivity.skipReleaseWait || !token.NamesSubmission() ||
		                  token.epoch < m_Epoch ||
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
	return freed;
}

std::size_t WebGpuDevice::Poll()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	ProcessEvents();
	return Collect();
}

DeviceResult<void> WebGpuDevice::ReadBuffer(
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
	// The caller waited for the submission that wrote it, whose token
	// completed only after its copy landed here.
	std::memcpy( out.data(), buffer->shadow.data() + offset, out.size() );
	return {};
}

DeviceResult<void> WebGpuDevice::WaitIdle()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_Held )
		Hold( false );
	// Every submission's work done and its readback maps landed.
	while ( !m_InFlight.empty() )
	{
		const std::uint64_t before = m_InFlight.size();
		WGPUQueueWorkDoneCallbackInfo done = WGPU_QUEUE_WORK_DONE_CALLBACK_INFO_INIT;
		done.mode = WGPUCallbackMode_WaitAnyOnly;
		done.callback = []( WGPUQueueWorkDoneStatus, WGPUStringView, void *, void * ) {};
		if ( !WaitFor( wgpuQueueOnSubmittedWorkDone( m_Queue, done ) ) )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kSubmit );
		ProcessEvents();
		if ( m_InFlight.size() == before && State() != DeviceState::kAvailable )
			break; // a lost device completes nothing more
	}
	return {};
}

DeviceResult<void> WebGpuDevice::Recover()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( State() == DeviceState::kAvailable )
		return {};
	m_State.store( DeviceState::kRecovering );
	ClearResources();
	m_EmptyGroup.Reset();
	m_GroupLayouts.clear();
	m_DepthUploads.clear();
	m_EmptyLayout = nullptr;
	++m_Epoch;
	m_Completed.store( 0 );
	m_Submitted = 0;
	m_Held = false;
	// A new adapter and device: WebGPU's adapter makes one device.
	if ( m_Queue )
		wgpuQueueRelease( m_Queue );
	if ( m_Device )
		wgpuDeviceRelease( m_Device );
	if ( m_Adapter )
		wgpuAdapterRelease( m_Adapter );
	m_Queue = nullptr;
	m_Device = nullptr;
	m_Adapter = nullptr;
	ProcessEvents();
	if ( !OpenDevice() )
	{
		m_State.store( DeviceState::kFatal );
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateDevice );
	}
	m_EmptyLayout = CachedLayout( {}, false );
	WGPUBindGroupDescriptor empty = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	empty.layout = m_EmptyLayout->layout.Get();
	m_EmptyGroup.Reset( wgpuDeviceCreateBindGroup( m_Device, &empty ) );
	m_State.store( DeviceState::kAvailable );
	return {};
}

namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	WebGpuAdapterOptions options;
	options.validation = request.validation;
	auto created = Create( options );
	if ( !created )
		return created;
	if ( FirstMissing( created.Value()->Facts().capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return created;
}

WebGpuDevice *Of( const IRenderDevice2 &device )
{
	return dynamic_cast<WebGpuDevice *>( const_cast<IRenderDevice2 *>( &device ) );
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "webgpu", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const WebGpuAdapterOptions &options )
{
	auto device = std::make_unique<WebGpuDevice>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
}

std::uint64_t ValidationErrors( const IRenderDevice2 &device )
{
	const WebGpuDevice *webgpu = Of( device );
	return webgpu ? webgpu->ValidationErrors() : 0;
}

bool SimulateDeviceLoss( IRenderDevice2 &device )
{
	WebGpuDevice *webgpu = Of( device );
	if ( webgpu )
		webgpu->SimulateLoss();
	return webgpu != nullptr;
}

bool PresentToCanvas( IRenderDevice2 &device, TextureId color, std::uint32_t width,
    std::uint32_t height, const char *selector )
{
	WebGpuDevice *webgpu = Of( device );
	return webgpu && webgpu->PresentToCanvas( color.value, width, height, selector );
}

bool HoldSubmissions( IRenderDevice2 &device, bool held )
{
	WebGpuDevice *webgpu = Of( device );
	if ( webgpu )
		webgpu->Hold( held );
	return webgpu != nullptr;
}

} // namespace render::device::webgpu
