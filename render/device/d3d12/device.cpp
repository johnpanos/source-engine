//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12: the device, its facts, resources, completion
//			and the provider's factory (public/render/device/d3d12/provider.h).
//
//=============================================================================//

#include "d3d12_device.h"

#include <windows.h>

#include <cstdio>
#include <cstring>

namespace render::device::d3d12
{

namespace
{

constexpr std::uint64_t kMinimumRingBytes = 4096;
constexpr std::uint32_t kTimestampSlots = 4096;
constexpr std::uint32_t kViewDescriptors = 262144;
constexpr std::uint32_t kSamplerDescriptors = 2048;

D3D12_HEAP_PROPERTIES Heap( D3D12_HEAP_TYPE type )
{
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = type;
	return heap;
}

D3D12_RESOURCE_DESC BufferResource( std::uint64_t size )
{
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	return desc;
}

DeviceStatus StatusOf( HRESULT result )
{
	if ( result == E_OUTOFMEMORY )
		return DeviceStatus::kOutOfMemory;
	if ( result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET ||
	     result == DXGI_ERROR_DEVICE_HUNG )
		return DeviceStatus::kDeviceLost;
	return DeviceStatus::kInternal;
}

} // namespace

void SetName( ID3D12Object *object, std::string_view name )
{
	if ( name.empty() )
		return;
	std::wstring wide( name.begin(), name.end() );
	object->SetName( wide.c_str() );
}

D3d12Device::D3d12Device( const D3d12AdapterOptions &options ) : m_Options( options )
{
}

D3d12Device::D3d12Device( const D3d12AdapterOptions &options, IDXGIFactory4 *factory,
    ID3D12Device *device, ID3D12CommandQueue *queue )
    : m_Options( options ), m_Hosted( true )
{
	factory->AddRef();
	device->AddRef();
	queue->AddRef();
	*m_Factory.Put() = factory;
	*m_Device.Put() = device;
	*m_Queue.Put() = queue;
}

D3d12Device::~D3d12Device()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State == DeviceState::kAvailable && m_Device )
		(void)WaitIdle();
	DestroyDeviceObjects();
	if ( m_Event )
		CloseHandle( m_Event );
}

DeviceResult<void> D3d12Device::Initialize()
{
	if ( m_Options.uploadRingBytes < kMinimumRingBytes )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateDevice );
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_Event = CreateEventW( nullptr, FALSE, FALSE, nullptr );
	if ( !m_Event )
		return Fail( DeviceStatus::kUnavailable, DeviceOperation::kCreateDevice );
	return CreateDeviceObjects();
}

DeviceResult<void> D3d12Device::OpenDeviceAndQueue()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	HRESULT result = S_OK;
	if ( m_Options.validation )
	{
		Com<ID3D12Debug> debug;
		if ( SUCCEEDED( D3D12GetDebugInterface( IID_ID3D12Debug, debug.PutVoid() ) ) )
			debug->EnableDebugLayer();
	}
	result = CreateDXGIFactory2( 0, IID_IDXGIFactory4, m_Factory.PutVoid() );
	if ( FAILED( result ) )
		return Fail( DeviceStatus::kUnavailable, op, result );
	// The first hardware adapter that opens a device.
	for ( UINT i = 0;; ++i )
	{
		Com<IDXGIAdapter1> adapter;
		if ( m_Factory->EnumAdapters1( i, adapter.Put() ) == DXGI_ERROR_NOT_FOUND )
			break;
		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1( &desc );
		if ( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
			continue;
		if ( SUCCEEDED( D3D12CreateDevice(
		         adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_ID3D12Device, m_Device.PutVoid() ) ) )
		{
			m_AdapterName.clear();
			for ( const WCHAR *c = desc.Description; *c; ++c )
				m_AdapterName.push_back( *c < 128 ? static_cast<char>( *c ) : '?' );
			m_Adapter = std::move( adapter );
			break;
		}
	}
	if ( !m_Device )
		return Fail( DeviceStatus::kUnavailable, op );
	if ( m_Options.validation )
		(void)m_Device->QueryInterface( IID_ID3D12InfoQueue, m_InfoQueue.PutVoid() );

	D3D12_COMMAND_QUEUE_DESC queue{};
	queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = m_Device->CreateCommandQueue( &queue, IID_ID3D12CommandQueue, m_Queue.PutVoid() );
	if ( FAILED( result ) )
		return Fail( StatusOf( result ), op, result );
	return {};
}

DeviceResult<void> D3d12Device::CreateDeviceObjects()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	HRESULT result = S_OK;
	if ( m_Hosted )
	{
		// The host's device and queue (constructor); only the port's own
		// objects are made below.
		m_AdapterName = "hosted";
		if ( m_Options.validation )
			(void)m_Device->QueryInterface( IID_ID3D12InfoQueue, m_InfoQueue.PutVoid() );
	}
	else
	{
		if ( auto opened = OpenDeviceAndQueue(); !opened )
			return opened;
	}
	result = m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, m_Fence.PutVoid() );
	if ( FAILED( result ) )
		return Fail( StatusOf( result ), op, result );

	D3D12_DESCRIPTOR_HEAP_DESC rtv{};
	rtv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv.NumDescriptors = 8;
	D3D12_DESCRIPTOR_HEAP_DESC dsv{};
	dsv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv.NumDescriptors = 1;
	if ( FAILED( m_Device->CreateDescriptorHeap(
	         &rtv, IID_ID3D12DescriptorHeap, m_RtvHeap.PutVoid() ) ) ||
	     FAILED( m_Device->CreateDescriptorHeap(
	         &dsv, IID_ID3D12DescriptorHeap, m_DsvHeap.PutVoid() ) ) )
		return Fail( DeviceStatus::kInternal, op );
	m_RtvStride = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	// Bind groups' tables live in one shader-visible heap of each kind.
	D3D12_DESCRIPTOR_HEAP_DESC views{};
	views.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	views.NumDescriptors = kViewDescriptors;
	views.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	D3D12_DESCRIPTOR_HEAP_DESC samplers{};
	samplers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplers.NumDescriptors = kSamplerDescriptors;
	samplers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if ( FAILED( m_Device->CreateDescriptorHeap(
	         &views, IID_ID3D12DescriptorHeap, m_ViewHeap.PutVoid() ) ) ||
	     FAILED( m_Device->CreateDescriptorHeap(
	         &samplers, IID_ID3D12DescriptorHeap, m_SamplerHeap.PutVoid() ) ) )
		return Fail( DeviceStatus::kInternal, op );
	m_ViewStride =
	    m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	m_SamplerStride = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
	m_ViewRanges.Reset( kViewDescriptors );
	m_SamplerRanges.Reset( kSamplerDescriptors );
	if ( auto loaded = LoadCompiler(); !loaded )
		return loaded;

	D3D12_QUERY_HEAP_DESC queries{};
	queries.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	queries.Count = kTimestampSlots;
	if ( SUCCEEDED( m_Device->CreateQueryHeap(
	         &queries, IID_ID3D12QueryHeap, m_Timestamps.PutVoid() ) ) )
		m_TimestampSlots = kTimestampSlots;

	auto ring = CommittedBuffer(
	    m_Options.uploadRingBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ );
	if ( !ring )
		return foundation::MakeUnexpected( ring.Error() );
	m_Ring = std::move( ring ).Value();
	D3D12_RANGE none{ 0, 0 };
	void *mapped = nullptr;
	if ( FAILED( m_Ring->Map( 0, &none, &mapped ) ) )
		return Fail( DeviceStatus::kInternal, op );
	m_RingData = static_cast<std::byte *>( mapped );
	{
		std::lock_guard<std::mutex> ringLock( m_RingLock );
		m_RingState.Reset( m_Options.uploadRingBytes );
	}
	QueryFacts();
	return {};
}

void D3d12Device::DestroyDeviceObjects()
{
	m_InFlight.clear();
	m_FreeAllocators.clear();
	m_Held.clear();
	m_Buffers.clear();
	m_Textures.clear();
	if ( m_Ring && m_RingData )
		m_Ring->Unmap( 0, nullptr );
	m_RingData = nullptr;
	m_Ring.Reset();
	m_Timestamps.Reset();
	m_TimestampSlots = 0;
	m_IndirectSignatures.clear();
	m_ViewHeap.Reset();
	m_SamplerHeap.Reset();
	m_RtvHeap.Reset();
	m_DsvHeap.Reset();
	m_InfoQueue.Reset();
	m_Fence.Reset();
	m_Queue.Reset();
	m_Device.Reset();
	m_Adapter.Reset();
	m_Factory.Reset();
}

void D3d12Device::QueryFacts()
{
	DeviceFacts facts;
	facts.diagnosticBackend = "d3d12";
	facts.adapterName = m_AdapterName;
	facts.artifactFormat = ArtifactFormat::kHlsl;
	facts.limits.maxBindGroups = kMaxBindGroups;
	facts.limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	facts.limits.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
	facts.limits.maxVertexBuffers = 16;
	facts.limits.uniformBufferAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	facts.limits.sampleCounts = 1;
	for ( std::uint32_t log2 = 1; log2 <= 3; ++log2 )
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{};
		levels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		levels.SampleCount = 1u << log2;
		if ( SUCCEEDED( m_Device->CheckFeatureSupport(
		         D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof( levels ) ) ) &&
		     levels.NumQualityLevels > 0 )
			facts.limits.sampleCounts |= 1u << log2;
	}
	// Feature level 11 guarantees BC formats, cube arrays, compute and UAVs;
	// indexed indirect draws (with a count buffer) are ExecuteIndirect. An
	// indirect record's firstInstance is not claimed: SV_InstanceID leaves it
	// out, and the base-instance root constant is per call.
	CapabilitySet have{ Capability::kTextureCompressionBC, Capability::kCubeArrays,
		Capability::kCompute, Capability::kStorageBuffers, Capability::kMultiDrawIndirect,
		Capability::kDrawIndirectCount, Capability::kFillModeLines };
	UINT64 frequency = 0;
	if ( m_TimestampSlots && SUCCEEDED( m_Queue->GetTimestampFrequency( &frequency ) ) &&
	     frequency )
	{
		have.Add( Capability::kTimestamps );
		facts.timestampPeriodNs = 1e9 / static_cast<double>( frequency );
	}
	for ( std::uint32_t i = 0; i < static_cast<std::uint32_t>( Capability::kCount ); ++i )
	{
		const auto capability = static_cast<Capability>( i );
		if ( have.Has( capability ) && m_Options.allowed.Has( capability ) )
			facts.capabilities.Add( capability );
	}
	if ( !facts.capabilities.Has( Capability::kTimestamps ) )
		facts.timestampPeriodNs = 0.0;
	m_Facts = facts;
}

void D3d12Device::SimulateLoss()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_State = DeviceState::kLost;
}

void D3d12Device::CheckRemoved()
{
	if ( m_State == DeviceState::kAvailable && m_Device &&
	     m_Device->GetDeviceRemovedReason() != S_OK )
		m_State = DeviceState::kLost;
}

void D3d12Device::CountDebugMessages()
{
	if ( !m_InfoQueue )
		return;
	const UINT64 stored = m_InfoQueue->GetNumStoredMessages();
	for ( UINT64 i = m_MessagesSeen; i < stored; ++i )
	{
		SIZE_T size = 0;
		if ( FAILED( m_InfoQueue->GetMessage( i, nullptr, &size ) ) || size == 0 )
			continue;
		std::vector<std::byte> storage( size );
		auto *message = reinterpret_cast<D3D12_MESSAGE *>( storage.data() );
		if ( FAILED( m_InfoQueue->GetMessage( i, message, &size ) ) )
			continue;
		if ( message->Severity != D3D12_MESSAGE_SEVERITY_ERROR &&
		     message->Severity != D3D12_MESSAGE_SEVERITY_CORRUPTION )
			continue;
		std::fprintf( stderr, "render.device.d3d12: debug layer: %s\n", message->pDescription );
		m_Messages.fetch_add( 1 );
		if ( m_Options.validationCounter )
			m_Options.validationCounter->fetch_add( 1 );
	}
	m_MessagesSeen = stored;
}

DeviceResult<Com<ID3D12Resource>> D3d12Device::CommittedBuffer(
    std::uint64_t size, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_STATES state, bool unordered )
{
	const D3D12_HEAP_PROPERTIES properties = Heap( heap );
	D3D12_RESOURCE_DESC desc = BufferResource( size );
	if ( unordered )
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	Com<ID3D12Resource> resource;
	const HRESULT result = m_Device->CreateCommittedResource( &properties, D3D12_HEAP_FLAG_NONE,
	    &desc, state, nullptr, IID_ID3D12Resource, resource.PutVoid() );
	if ( FAILED( result ) )
		return Fail( StatusOf( result ), DeviceOperation::kCreateBuffer, result );
	return resource;
}

// Resources ---------------------------------------------------------------------

BufferRecord *D3d12Device::LiveBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
}

TextureRecord *D3d12Device::LiveTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
}

BufferRecord *D3d12Device::ExistingBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() ? &found->second : nullptr;
}

TextureRecord *D3d12Device::ExistingTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() ? &found->second : nullptr;
}

std::optional<LayoutView> D3d12Device::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

bool *D3d12Device::ReleasedFlag( ResourceId resource )
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

// Recorded resources (recording::IRecordedResources) --------------------------------

std::optional<recording::TextureView> D3d12Device::Texture( std::uint64_t id ) const
{
	const auto found = m_Textures.find( id );
	if ( found == m_Textures.end() || found->second.released )
		return std::nullopt;
	return recording::TextureView{ found->second.desc, found->second.layers, found->second.usage };
}

std::optional<recording::BufferView> D3d12Device::Buffer( std::uint64_t id ) const
{
	const auto found = m_Buffers.find( id );
	if ( found == m_Buffers.end() || found->second.released )
		return std::nullopt;
	return recording::BufferView{ found->second.desc, found->second.usage };
}

std::optional<recording::PipelineView> D3d12Device::Pipeline( std::uint64_t id ) const
{
	const auto found = m_Pipelines.find( id );
	if ( found == m_Pipelines.end() || found->second.released )
		return std::nullopt;
	const PipelineRecord &p = found->second;
	return recording::PipelineView{ p.kind, p.colorFormats, p.depthFormat, p.sampleCount,
	    p.vertexBuffers, p.drawConstantBytes, p.layouts, p.layoutHasBindings };
}

std::optional<BindGroupLayoutId> D3d12Device::BindGroup( std::uint64_t id ) const
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

bool D3d12Device::CanCopyWithBuffer( const recording::TextureView &texture ) const
{
	// D24's plane-0 footprint (R24X8) is not the port's 32-bit depth transfer.
	return texture.desc.format != Format::kD24UnormS8;
}

DeviceResult<BufferId> D3d12Device::CreateBuffer( const BufferDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBuffer( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Readback buffers live in the readback heap, fixed in COPY_DEST; every
	// other buffer is default-heap memory the GPU writes through copies (an
	// upload-heap buffer could not be a copy destination).
	BufferRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	const bool readback = desc.memory == MemoryKind::kReadback;
	record.fixedState = readback;
	record.state = readback ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON;
	// Sized to whole 256-byte units, so a constant-buffer view of the
	// buffer's end stays inside it; storage buffers take UAVs.
	const std::uint64_t size = ( desc.size + 255u ) & ~std::uint64_t( 255u );
	const bool storage = desc.usages.Has( ResourceUsage::kStorageRead ) ||
	                     desc.usages.Has( ResourceUsage::kStorageWrite );
	auto resource = CommittedBuffer( size,
	    readback ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_DEFAULT, record.state,
	    storage && !readback );
	if ( !resource )
		return foundation::MakeUnexpected( resource.Error() );
	record.resource = std::move( resource ).Value();
	SetName( record.resource.Get(), desc.debugName );
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BufferId> D3d12Device::CreateUploadBuffer( std::span<const std::byte> bytes )
{
	const DeviceOperation op = DeviceOperation::kCreateBuffer;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( bytes.empty() )
		return Fail( DeviceStatus::kInvalidDescription, op );
	// D25: an upload-heap buffer, written before this returns and only ever
	// read by copies (GENERIC_READ for its whole life).
	auto resource =
	    CommittedBuffer( bytes.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ );
	if ( !resource )
		return foundation::MakeUnexpected( resource.Error() );
	BufferRecord record;
	record.desc.size = bytes.size();
	record.desc.memory = MemoryKind::kUpload;
	record.desc.usages = { ResourceUsage::kCopySource };
	record.resource = std::move( resource ).Value();
	record.state = D3D12_RESOURCE_STATE_GENERIC_READ;
	record.fixedState = true;
	D3D12_RANGE none{ 0, 0 };
	void *mapped = nullptr;
	const HRESULT result = record.resource->Map( 0, &none, &mapped );
	if ( FAILED( result ) )
		return Fail( StatusOf( result ), op, result );
	std::memcpy( mapped, bytes.data(), bytes.size() );
	record.resource->Unmap( 0, nullptr );
	record.usage = ResourceUsage::kCopySource;
	const BufferId id{ ++m_NextId };
	m_Buffers.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<TextureId> D3d12Device::CreateTexture( const TextureDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateTexture( desc, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	// Presentation belongs to a render.presentation.v1 bridge (RFC 0024 X4).
	if ( desc.usages.Has( ResourceUsage::kPresent ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( IsBlockCompressed( desc.format ) &&
	     !m_Facts.capabilities.Has( Capability::kTextureCompressionBC ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.dimension == TextureDimension::kCube && desc.depthOrLayers > 6 &&
	     !m_Facts.capabilities.Has( Capability::kCubeArrays ) )
		return Fail( DeviceStatus::kUnsupported, op );
	const bool depth = IsDepthFormat( desc.format );
	const bool attachment = desc.usages.Has( ResourceUsage::kColorAttachment ) ||
	                        desc.usages.Has( ResourceUsage::kDepthWrite ) ||
	                        desc.usages.Has( ResourceUsage::kDepthRead ) ||
	                        desc.usages.Has( ResourceUsage::kResolveDestination );
	const bool storage = desc.usages.Has( ResourceUsage::kStorageRead ) ||
	                     desc.usages.Has( ResourceUsage::kStorageWrite );
	const DxFormat format = FormatOf( desc.format );
	if ( format.resource == DXGI_FORMAT_UNKNOWN ||
	     ( attachment && desc.dimension == TextureDimension::k3D ) ||
	     ( storage && !m_Facts.capabilities.Has( Capability::kStorageBuffers ) ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( desc.sampleCount > 1 && !( m_Facts.limits.sampleCounts & desc.sampleCount ) )
		return Fail( DeviceStatus::kUnsupported, op );

	D3D12_RESOURCE_DESC resource{};
	resource.Dimension = desc.dimension == TextureDimension::k3D
	                         ? D3D12_RESOURCE_DIMENSION_TEXTURE3D
	                         : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resource.Width = desc.width;
	resource.Height = desc.height;
	resource.DepthOrArraySize = static_cast<UINT16>( desc.depthOrLayers );
	resource.MipLevels = static_cast<UINT16>( desc.mipLevels );
	resource.Format = format.resource;
	resource.SampleDesc.Count = desc.sampleCount;
	resource.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	// Every uncompressed color texture can be a render target, because
	// ClearTexture clears through a render-target view; depth textures are
	// depth-stencil resources.
	if ( depth )
		resource.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	else if ( !IsBlockCompressed( desc.format ) )
		resource.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	if ( storage )
		resource.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_FEATURE_DATA_FORMAT_SUPPORT support{};
	support.Format = depth ? format.attachment : format.resource;
	if ( FAILED( m_Device->CheckFeatureSupport(
	         D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof( support ) ) ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( !depth && !IsBlockCompressed( desc.format ) &&
	     !( support.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET ) )
		resource.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const D3D12_HEAP_PROPERTIES heap = Heap( D3D12_HEAP_TYPE_DEFAULT );
	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	record.layers = desc.dimension == TextureDimension::k3D ? 1u : desc.depthOrLayers;
	record.state = D3D12_RESOURCE_STATE_COMMON;
	const HRESULT result = m_Device->CreateCommittedResource( &heap, D3D12_HEAP_FLAG_NONE,
	    &resource, record.state, nullptr, IID_ID3D12Resource, record.resource.PutVoid() );
	if ( FAILED( result ) )
		return Fail( result == E_OUTOFMEMORY ? DeviceStatus::kOutOfMemory
		                                     : DeviceStatus::kUnsupported,
		    op, result );
	SetName( record.resource.Get(), desc.debugName );
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<SamplerId> D3d12Device::CreateSampler( const SamplerDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateSampler;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateSampler( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	if ( desc.maxAnisotropy > D3D12_MAX_MAXANISOTROPY )
		return Fail( DeviceStatus::kUnsupported, op );
	// The descriptor is written into a group's sampler table (X2); the record
	// keeps the description.
	SamplerRecord record;
	record.desc = desc;
	const SamplerId id{ ++m_NextId };
	m_Samplers.emplace( id.value, record );
	return id;
}

DeviceResult<BindGroupLayoutId> D3d12Device::CreateBindGroupLayout(
    const BindGroupLayoutDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroupLayout;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( auto valid = ValidateBindGroupLayout( desc ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	for ( const BindingDesc &binding : desc.bindings )
	{
		if ( ( binding.kind == BindingKind::kStorageBuffer ||
		         binding.kind == BindingKind::kStorageTexture ) &&
		     !m_Facts.capabilities.Has( Capability::kStorageBuffers ) )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	LayoutRecord record;
	record.role = desc.role;
	record.bindings.assign( desc.bindings.begin(), desc.bindings.end() );
	const BindGroupLayoutId id{ ++m_NextId };
	m_Layouts.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<BindGroupId> D3d12Device::CreateBindGroup( const BindGroupDesc &desc )
{
	const DeviceOperation op = DeviceOperation::kCreateBindGroup;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
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
	record.bindings.assign( layout->bindings.begin(), layout->bindings.end() );
	record.entries.assign( desc.entries.begin(), desc.entries.end() );
	if ( auto written = WriteGroupDescriptors( record ); !written )
		return foundation::MakeUnexpected( written.Error() );
	const BindGroupId id{ ++m_NextId };
	m_BindGroups.emplace( id.value, std::move( record ) );
	return id;
}

DeviceResult<void> D3d12Device::Release( ResourceId resource, CompletionToken releaseAfter )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	if ( m_Options.sensitivity.skipReleaseWait )
		releaseAfter = {};
	m_Releases.push_back( { resource, releaseAfter } );
	return {};
}

void D3d12Device::Erase( ResourceId resource )
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
		if ( const auto found = m_BindGroups.find( resource.value ); found != m_BindGroups.end() )
		{
			m_ViewRanges.Free( found->second.viewBase, found->second.viewCount );
			m_SamplerRanges.Free( found->second.samplerBase, found->second.samplerCount );
			m_BindGroups.erase( found );
		}
		break;
	case ResourceKind::kNone:
		break;
	}
}

ID3D12CommandSignature *D3d12Device::IndirectSignature( std::uint32_t stride )
{
	Com<ID3D12CommandSignature> &signature = m_IndirectSignatures[stride];
	if ( !signature )
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		D3D12_COMMAND_SIGNATURE_DESC desc{};
		desc.ByteStride = stride;
		desc.NumArgumentDescs = 1;
		desc.pArgumentDescs = &argument;
		if ( FAILED( m_Device->CreateCommandSignature(
		         &desc, nullptr, IID_ID3D12CommandSignature, signature.PutVoid() ) ) )
			return nullptr;
	}
	return signature.Get();
}

std::size_t D3d12Device::LiveResourceCount() const
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

MemoryBudgetSnapshot D3d12Device::ReadMemoryBudget() const
{
	MemoryBudgetSnapshot result;
	if ( m_State != DeviceState::kAvailable || !m_Adapter )
		return result;
	Com<IDXGIAdapter3> adapter;
	if ( FAILED( m_Adapter->QueryInterface( IID_IDXGIAdapter3, adapter.PutVoid() ) ) )
		return result;
	result.supported = true;
	result.epoch = m_Epoch;
	const DXGI_MEMORY_SEGMENT_GROUP groups[] = { DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
		DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL };
	for ( std::uint32_t i = 0; i < 2; ++i )
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO info{};
		if ( FAILED( adapter->QueryVideoMemoryInfo( 0, groups[i], &info ) ) )
			continue;
		HeapMemoryBudget heap;
		heap.heap = i;
		heap.deviceLocal = i == 0;
		heap.usageKnown = true;
		heap.budgetKnown = true;
		heap.usageBytes = info.CurrentUsage;
		heap.budgetBytes = info.Budget;
		result.heaps.push_back( heap );
	}
	return result;
}

// Encoders, submission and completion -------------------------------------------

DeviceResult<CommandEncoder> D3d12Device::BeginEncoder( QueueKind queue )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	// One direct queue until async compute lands with X2's compute pipelines.
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder( queue, std::make_unique<RecordingEncoder>(
	                                  static_cast<recording::IUploadStager &>( *this ), this, queue ) );
}

DeviceResult<CompletionToken> D3d12Device::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );

	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	CheckRemoved();
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, op );
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		// One queue: a wait on an earlier submission holds by queue order.
		if ( wait.queue != QueueKind::kGraphics || wait.epoch > m_Epoch ||
		     wait.value > m_Submitted )
			return Fail( DeviceStatus::kInvalidDescription, op );
	}
	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<RecordingEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		auto *encoder = dynamic_cast<RecordingEncoder *>( backend.get() );
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
			{
				if ( m_Options.sensitivity.unsafeUploadReuse )
					m_RingState.Abandon( allocation );
				else
					m_RingState.Submit( allocation, token );
			}
			encoder->MarkSubmitted();
		}
	}
	if ( m_Holding )
	{
		m_Held.push_back( { std::move( backends ), std::move( recorded ), token } );
		return token;
	}
	if ( auto issued = Issue( recorded, token ); !issued )
	{
		m_State = DeviceState::kLost;
		return foundation::MakeUnexpected( issued.Error() );
	}
	return token;
}

void D3d12Device::IssueHeld()
{
	for ( HeldSubmission &held : m_Held )
	{
		if ( !Issue( held.encoders, held.token ) )
			m_State = DeviceState::kLost;
	}
	m_Held.clear();
}

void D3d12Device::Hold( bool held )
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	m_Holding = held;
	if ( !held && m_State == DeviceState::kAvailable )
		IssueHeld();
}

void D3d12Device::UpdateCompletion() const
{
	if ( !m_Fence )
		return;
	const UINT64 value = m_Fence->GetCompletedValue();
	// UINT64_MAX: the device was removed (CheckRemoved reports it).
	if ( value != UINT64_MAX && value > m_Completed.load( std::memory_order_relaxed ) )
		m_Completed.store( value, std::memory_order_release );
}

bool D3d12Device::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() )
		return true;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( token.epoch < m_Epoch )
		return true;
	if ( token.epoch > m_Epoch || token.queue != QueueKind::kGraphics )
		return false;
	if ( m_State == DeviceState::kAvailable )
		UpdateCompletion();
	return token.value <= m_Completed.load( std::memory_order_acquire );
}

std::size_t D3d12Device::Collect()
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
	while ( !m_InFlight.empty() && m_InFlight.front().value <= completed )
	{
		InFlight &done = m_InFlight.front();
		if ( SUCCEEDED( done.allocator->Reset() ) )
			m_FreeAllocators.push_back( std::move( done.allocator ) );
		m_InFlight.pop_front();
	}
	std::lock_guard<std::mutex> ring( m_RingLock );
	m_RingState.Retire( m_Epoch, completed );
	return freed;
}

std::size_t D3d12Device::Poll()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	CheckRemoved();
	if ( m_State == DeviceState::kAvailable )
		UpdateCompletion();
	CountDebugMessages();
	return Collect();
}

DeviceResult<void> D3d12Device::ReadBuffer(
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
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( out.empty() )
		return {};
	const D3D12_RANGE range{ static_cast<SIZE_T>( offset ),
		static_cast<SIZE_T>( offset + out.size() ) };
	void *mapped = nullptr;
	const HRESULT result = buffer->resource->Map( 0, &range, &mapped );
	if ( FAILED( result ) )
		return Fail( StatusOf( result ), op, result );
	std::memcpy( out.data(), static_cast<const std::byte *>( mapped ) + offset, out.size() );
	const D3D12_RANGE written{ 0, 0 };
	buffer->resource->Unmap( 0, &written );
	return {};
}

DeviceResult<void> D3d12Device::WaitIdle()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable || !m_Fence )
		return {};
	IssueHeld();
	if ( m_Fence->GetCompletedValue() < m_Submitted )
	{
		if ( FAILED( m_Fence->SetEventOnCompletion( m_Submitted, m_Event ) ) )
			return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kSubmit );
		WaitForSingleObject( m_Event, INFINITE );
	}
	UpdateCompletion();
	CountDebugMessages();
	return {};
}

DeviceResult<void> D3d12Device::Recover()
{
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State == DeviceState::kAvailable )
		return {};
	if ( m_Hosted )
	{
		// The host owns the device and queue: it recreates both, and the port
		// device with them.
		m_State = DeviceState::kFatal;
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateDevice );
	}
	m_State = DeviceState::kRecovering;
	// The lost device and everything on it go; a new device starts the new
	// epoch with nothing live. Tokens of the old epoch are complete.
	m_Samplers.clear();
	m_Layouts.clear();
	m_BindGroups.clear();
	m_Pipelines.clear();
	m_Releases.clear();
	DestroyDeviceObjects();
	++m_Epoch;
	m_Submitted = 0;
	m_Completed.store( 0 );
	m_MessagesSeen = 0;
	if ( !CreateDeviceObjects() )
	{
		m_State = DeviceState::kFatal;
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kCreateDevice );
	}
	m_State = DeviceState::kAvailable;
	return {};
}

namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	D3d12AdapterOptions options;
	options.validation = request.validation;
	auto created = Create( options );
	if ( !created )
		return created;
	if ( FirstMissing( created.Value()->Facts().capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return created;
}

D3d12Device *Of( const IRenderDevice2 &device )
{
	return dynamic_cast<D3d12Device *>( const_cast<IRenderDevice2 *>( &device ) );
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "d3d12", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const D3d12AdapterOptions &options )
{
	auto device = std::make_unique<D3d12Device>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
}

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateHosted( const D3d12AdapterOptions &options,
    IDXGIFactory4 *factory, ID3D12Device *device, ID3D12CommandQueue *queue )
{
	if ( !factory || !device || !queue )
		return Fail( DeviceStatus::kInvalidDescription, DeviceOperation::kCreateDevice );
	auto hosted = std::make_unique<D3d12Device>( options, factory, device, queue );
	if ( auto initialized = hosted->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( hosted ) );
}

DeviceResult<TextureId> ImportTexture(
    IRenderDevice2 &device, ID3D12Resource *texture, const TextureDesc &desc, ResourceUsage home )
{
	D3d12Device *d = Of( device );
	if ( !d )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kCreateTexture );
	return d->Import( texture, desc, home );
}

DeviceResult<TextureId> D3d12Device::Import(
    ID3D12Resource *texture, const TextureDesc &desc, ResourceUsage home )
{
	const DeviceOperation op = DeviceOperation::kCreateTexture;
	std::lock_guard<std::recursive_mutex> lock( m_Lock );
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	if ( !texture || desc.mipLevels != 1 || desc.sampleCount != 1 || !desc.usages.Has( home ) )
		return Fail( DeviceStatus::kInvalidDescription, op );
	// kExternal names a texture another owner's work (the presentation
	// bridge's copy) uses in its home state; the rest of the description is
	// validated as a created texture's (the Vulkan adapter's ImportImage rule).
	TextureDesc plain = desc;
	plain.usages = UsageSet();
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( ResourceUsage::kCount ); ++bit )
	{
		const auto usage = static_cast<ResourceUsage>( bit );
		if ( usage != ResourceUsage::kExternal && desc.usages.Has( usage ) )
			plain.usages.Add( usage );
	}
	if ( auto valid = ValidateTexture( plain, m_Facts.limits ); !valid )
		return foundation::MakeUnexpected( valid.Error() );
	const D3D12_RESOURCE_DESC native = texture->GetDesc();
	if ( native.Width != desc.width || native.Height != desc.height ||
	     native.Format != FormatOf( desc.format ).resource )
		return Fail( DeviceStatus::kInvalidDescription, op );
	TextureRecord record;
	record.desc = desc;
	record.desc.debugName = {};
	texture->AddRef();
	*record.resource.Put() = texture;
	record.layers = desc.depthOrLayers;
	record.state = StateOf( home, IsDepthFormat( desc.format ) );
	record.usage = home;
	const TextureId id{ ++m_NextId };
	m_Textures.emplace( id.value, std::move( record ) );
	return id;
}

std::uint64_t ValidationMessages( const IRenderDevice2 &device )
{
	const D3d12Device *d = Of( device );
	return d ? d->ValidationMessages() : 0;
}

std::uint64_t DeferredUploads( const IRenderDevice2 &device )
{
	const D3d12Device *d = Of( device );
	return d ? d->DeferredUploads() : 0;
}

bool HoldSubmissions( IRenderDevice2 &device, bool held )
{
	D3d12Device *d = Of( device );
	if ( d )
		d->Hold( held );
	return d != nullptr;
}

bool SimulateDeviceLoss( IRenderDevice2 &device )
{
	D3d12Device *d = Of( device );
	if ( d )
		d->SimulateLoss();
	return d != nullptr;
}

} // namespace render::device::d3d12
