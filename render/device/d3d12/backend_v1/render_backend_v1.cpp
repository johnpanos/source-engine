//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.backend.v1 over Direct3D 12 (RFC 0024 X4); see
//			render_backend_v1.h.
//
//=============================================================================//

#include "render_backend_v1.h"

#include "render/device/d3d12/host_device.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>

namespace render_d3d12
{

using namespace render;

namespace
{

template <typename T> void Release( T *&object )
{
	if ( object )
		object->Release();
	object = nullptr;
}

class D3d12CompletionToken final : public IRenderCompletionToken
{
public:
	D3d12CompletionToken( std::uint64_t id, std::uint64_t value ) : m_Id( id ), m_Value( value ) {}
	bool IsComplete() const override { return m_Complete; }
	void MarkComplete() { m_Complete = true; }
	std::uint64_t Id() const { return m_Id; }
	std::uint64_t Value() const { return m_Value; }

private:
	std::uint64_t m_Id;
	std::uint64_t m_Value;
	bool m_Complete = false;
};

class D3d12CommandContext final : public IRenderCommandContext
{
public:
	void RecordUse( RenderResourceHandle handle ) override { uses.push_back( handle ); }
	std::vector<RenderResourceHandle> uses;
	std::vector<std::function<void( ID3D12GraphicsCommandList & )>> native;
};

struct Resource
{
	RenderResourceType type = RenderResourceType::kBuffer;
	ID3D12Resource *texture = nullptr;
	bool live = false;
	std::uint64_t lastUse = 0; // fence value of the last submission that used it
};

class D3d12BackendDevice final : public IRenderDevice, public D3d12DeviceEndpoint
{
public:
	D3d12BackendDevice( IDXGIFactory4 *factory, ID3D12Device *device ) : m_Factory( factory ), m_Device( device )
	{
		m_Factory->AddRef();
	}

	bool Initialize()
	{
		D3D12_COMMAND_QUEUE_DESC queue{};
		queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		if ( FAILED( m_Device->CreateCommandQueue(
		         &queue, IID_ID3D12CommandQueue, reinterpret_cast<void **>( &m_Queue ) ) ) ||
		     FAILED( m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence,
		         reinterpret_cast<void **>( &m_Fence ) ) ) ||
		     FAILED( m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence,
		         reinterpret_cast<void **>( &m_Gate ) ) ) ||
		     FAILED( m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT,
		         IID_ID3D12CommandAllocator, reinterpret_cast<void **>( &m_Allocator ) ) ) )
			return false;
		m_Event = CreateEventW( nullptr, FALSE, FALSE, nullptr );
		m_Caps.maxTextureDimension = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		m_Caps.maxColorTargets = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
		m_Caps.maxSampleCount = 4;
		m_Caps.features.Add( RenderFeature::kSampledSrgb );
		m_Caps.features.Add( RenderFeature::kDepthColorPairing );
		m_Caps.features.Add( RenderFeature::kComputeShaders );
		m_Caps.features.Add( RenderFeature::kMultiSample4x );
		m_Caps.features.Add( RenderFeature::kOffscreenRender );
		return m_Event != nullptr;
	}

	~D3d12BackendDevice() override
	{
		if ( m_Held )
			ReleaseCompletion();
		if ( m_Port )
			(void)m_Port->WaitIdle();
		m_Port.reset();
		(void)WaitFor( m_Submitted );
		for ( Resource &r : m_Resources )
			Release( r.texture );
		for ( Outstanding &o : m_Outstanding )
		{
			Release( o.list );
			Release( o.allocator );
		}
		for ( D3d12CompletionToken *token : m_Tokens )
			delete token;
		for ( D3d12CommandContext *context : m_Contexts )
			delete context;
		Release( m_Allocator );
		Release( m_Gate );
		Release( m_Fence );
		Release( m_Queue );
		Release( m_Device );
		Release( m_Factory );
		if ( m_Event )
			CloseHandle( m_Event );
	}

	// IRenderDevice ----------------------------------------------------------------

	const RenderDeviceCaps &GetCapabilities() const override { return m_Caps; }
	RenderDeviceState GetState() const override { return m_State; }

	RenderResourceHandle CreateResource( RenderResourceType type ) override
	{
		Resource r;
		r.type = type;
		r.live = true;
		m_Resources.push_back( r );
		return static_cast<RenderResourceHandle>( m_Resources.size() );
	}

	bool IsResourceLive( RenderResourceHandle handle ) const override
	{
		const Resource *r = Slot( handle );
		return r && r->live;
	}

	std::size_t GetLiveResourceCount() const override
	{
		std::size_t live = 0;
		for ( const Resource &r : m_Resources )
			live += r.live ? 1 : 0;
		return live;
	}

	void DestroyResourceWhenComplete(
	    RenderResourceHandle handle, IRenderCompletionToken &token ) override
	{
		if ( IsResourceLive( handle ) )
			m_Pending.push_back( { handle, &token } );
	}

	void CollectCompletedDestructions() override
	{
		for ( auto it = m_Pending.begin(); it != m_Pending.end(); )
		{
			if ( !it->token->IsComplete() )
			{
				++it;
				continue;
			}
			if ( Resource *r = Slot( it->handle ) )
			{
				Release( r->texture );
				r->live = false;
			}
			it = m_Pending.erase( it );
		}
	}

	IRenderCommandContext *CreateCommandContext() override
	{
		D3d12CommandContext *context = new D3d12CommandContext();
		m_Contexts.push_back( context );
		return context;
	}

	IRenderCompletionToken *Submit( IRenderCommandContext &context ) override
	{
		D3d12CommandContext &recorded = static_cast<D3d12CommandContext &>( context );
		std::function<void( ID3D12GraphicsCommandList & )> record;
		if ( !recorded.native.empty() )
		{
			record = [&recorded]( ID3D12GraphicsCommandList &list )
			{
				for ( const auto &step : recorded.native )
					step( list );
			};
		}
		IRenderCompletionToken *token =
		    SubmitRecorded( record, recorded.uses.data(), recorded.uses.size() );
		recorded.native.clear();
		return token;
	}

	void RecordNative( IRenderCommandContext &context,
	    std::function<void( ID3D12GraphicsCommandList & )> record ) override
	{
		static_cast<D3d12CommandContext &>( context ).native.push_back( std::move( record ) );
	}

	std::uint32_t PollCompletion() override
	{
		if ( m_Outstanding.empty() )
			return 0;
		// The oldest submission (submission order), observed on the GPU's
		// fence: completion reflects the GPU, not a CPU event.
		Outstanding o = m_Outstanding.front();
		m_Outstanding.pop_front();
		(void)WaitFor( o.token->Value() );
		o.token->MarkComplete();
		if ( o.token->Id() > m_LastCompleted )
			m_LastCompleted = o.token->Id();
		if ( o.allocator )
		{
			o.list->Release();
			m_FreeAllocators.push_back( o.allocator );
		}
		return 1;
	}

	std::uint64_t LastCompletedSubmission() const override { return m_LastCompleted; }

	bool SimulateDeviceLoss() override
	{
		// Everything submitted completes before the modeled loss.
		(void)WaitFor( m_Submitted );
		m_State = RenderDeviceState::kDeviceLost;
		return true;
	}

	bool RecoverDevice() override
	{
		m_State = RenderDeviceState::kAvailable;
		return true;
	}

	// D3d12DeviceEndpoint ----------------------------------------------------------

	ID3D12Device *NativeDevice() const override { return m_Device; }
	ID3D12CommandQueue *Queue() const override { return m_Queue; }
	IDXGIFactory4 *Factory() const override { return m_Factory; }

	RenderResourceHandle CreateTexture(
	    std::uint32_t width, std::uint32_t height, DXGI_FORMAT format ) override
	{
		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ID3D12Resource *texture = nullptr;
		if ( FAILED( m_Device->CreateCommittedResource( &heap, D3D12_HEAP_FLAG_NONE, &desc,
		         D3D12_RESOURCE_STATE_COMMON, nullptr, IID_ID3D12Resource,
		         reinterpret_cast<void **>( &texture ) ) ) )
			return kInvalidResource;
		const RenderResourceHandle handle = CreateResource( RenderResourceType::kTexture );
		m_Resources[handle - 1].texture = texture;
		return handle;
	}

	ID3D12Resource *Texture( RenderResourceHandle handle ) const override
	{
		const Resource *r = Slot( handle );
		return r && r->live ? r->texture : nullptr;
	}

	IRenderCompletionToken *SubmitRecorded(
	    const std::function<void( ID3D12GraphicsCommandList & )> &record,
	    const RenderResourceHandle *uses, std::size_t useCount ) override
	{
		ID3D12CommandAllocator *allocator = nullptr;
		ID3D12GraphicsCommandList *list = nullptr;
		if ( record )
		{
			if ( !m_FreeAllocators.empty() )
			{
				allocator = m_FreeAllocators.back();
				m_FreeAllocators.pop_back();
				allocator->Reset();
			}
			else if ( FAILED( m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT,
			              IID_ID3D12CommandAllocator, reinterpret_cast<void **>( &allocator ) ) ) )
				return nullptr;
			if ( FAILED( m_Device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator,
			         nullptr, IID_ID3D12GraphicsCommandList, reinterpret_cast<void **>( &list ) ) ) )
			{
				allocator->Release();
				return nullptr;
			}
			record( *list );
			list->Close();
		}
		if ( m_Held )
			m_Queue->Wait( m_Gate, m_GateValue + 1 );
		if ( list )
		{
			ID3D12CommandList *lists[] = { list };
			m_Queue->ExecuteCommandLists( 1, lists );
		}
		const std::uint64_t value = ++m_Submitted;
		m_Queue->Signal( m_Fence, value );
		for ( std::size_t i = 0; i < useCount; ++i )
		{
			if ( Resource *r = Slot( uses[i] ) )
				r->lastUse = value;
		}
		D3d12CompletionToken *token = new D3d12CompletionToken( ++m_NextSubmissionId, value );
		m_Tokens.push_back( token );
		m_Outstanding.push_back( { token, allocator, list } );
		return token;
	}

	std::uint64_t TokenValue( const IRenderCompletionToken &token ) const override
	{
		for ( const D3d12CompletionToken *mine : m_Tokens )
		{
			if ( mine == &token )
				return mine->Value();
		}
		return 0;
	}

	bool WaitFor( std::uint64_t value ) override
	{
		if ( !m_Fence )
			return false;
		if ( m_Fence->GetCompletedValue() >= value )
			return true;
		if ( m_Held )
			return false; // held work cannot complete until the release
		if ( FAILED( m_Fence->SetEventOnCompletion( value, m_Event ) ) )
			return false;
		WaitForSingleObject( m_Event, INFINITE );
		return true;
	}

	bool Completed( std::uint64_t value ) const override
	{
		return m_Fence && m_Fence->GetCompletedValue() >= value;
	}

	std::uint64_t LastSubmitted() const override { return m_Submitted; }

	render::device::IRenderDevice2 *Port() override
	{
		if ( !m_Port )
		{
			auto hosted = render::device::d3d12::CreateHosted( {}, m_Factory, m_Device, m_Queue );
			if ( hosted )
				m_Port = std::move( hosted ).Value();
		}
		return m_Port.get();
	}

	bool HoldCompletion() override
	{
		m_Held = true;
		return true;
	}

	void ReleaseCompletion() override
	{
		if ( !m_Held )
			return;
		m_Held = false;
		m_Gate->Signal( ++m_GateValue );
	}

	bool HasIncompleteGpuWork() const override
	{
		return m_Fence->GetCompletedValue() < m_Submitted;
	}

private:
	struct Outstanding
	{
		D3d12CompletionToken *token;
		ID3D12CommandAllocator *allocator;
		ID3D12GraphicsCommandList *list;
	};
	struct PendingDestruction
	{
		RenderResourceHandle handle;
		IRenderCompletionToken *token;
	};

	Resource *Slot( RenderResourceHandle handle )
	{
		return handle == kInvalidResource || handle > m_Resources.size() ? nullptr
		                                                                 : &m_Resources[handle - 1];
	}
	const Resource *Slot( RenderResourceHandle handle ) const
	{
		return handle == kInvalidResource || handle > m_Resources.size() ? nullptr
		                                                                 : &m_Resources[handle - 1];
	}

	IDXGIFactory4 *m_Factory;
	ID3D12Device *m_Device;
	ID3D12CommandQueue *m_Queue = nullptr;
	ID3D12Fence *m_Fence = nullptr;
	ID3D12Fence *m_Gate = nullptr;
	ID3D12CommandAllocator *m_Allocator = nullptr;
	HANDLE m_Event = nullptr;
	RenderDeviceCaps m_Caps;
	RenderDeviceState m_State = RenderDeviceState::kAvailable;
	std::vector<Resource> m_Resources;
	std::vector<PendingDestruction> m_Pending;
	std::deque<Outstanding> m_Outstanding;
	std::vector<ID3D12CommandAllocator *> m_FreeAllocators;
	std::vector<D3d12CompletionToken *> m_Tokens;
	std::vector<D3d12CommandContext *> m_Contexts;
	std::uint64_t m_Submitted = 0;
	std::uint64_t m_NextSubmissionId = 0;
	std::uint64_t m_LastCompleted = 0;
	std::uint64_t m_GateValue = 0;
	bool m_Held = false;
	std::unique_ptr<render::device::IRenderDevice2> m_Port;
};

class D3d12Backend final : public D3d12RenderBackend
{
public:
	bool Initialize( std::string *outError )
	{
		if ( FAILED( CreateDXGIFactory2( 0, IID_IDXGIFactory4, reinterpret_cast<void **>( &m_Factory ) ) ) )
		{
			if ( outError )
				*outError = "CreateDXGIFactory2 failed";
			return false;
		}
		for ( UINT i = 0;; ++i )
		{
			IDXGIAdapter1 *adapter = nullptr;
			if ( m_Factory->EnumAdapters1( i, &adapter ) == DXGI_ERROR_NOT_FOUND )
				break;
			DXGI_ADAPTER_DESC1 desc{};
			adapter->GetDesc1( &desc );
			if ( !( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE ) &&
			     SUCCEEDED( D3D12CreateDevice(
			         adapter, D3D_FEATURE_LEVEL_11_0, IID_ID3D12Device, nullptr ) ) )
			{
				RenderAdapterInfo info;
				std::snprintf( info.id, sizeof( info.id ), "d3d12:%u", i );
				for ( std::size_t c = 0; c + 1 < sizeof( info.name ) && desc.Description[c]; ++c )
					info.name[c] = desc.Description[c] < 128 ? char( desc.Description[c] ) : '?';
				info.vendorId = desc.VendorId;
				info.deviceId = desc.DeviceId;
				std::snprintf( info.driverApi, sizeof( info.driverApi ), "d3d12" );
				info.deviceMemoryBytes = desc.DedicatedVideoMemory;
				info.supportedFeatures.Add( RenderFeature::kSampledSrgb );
				info.supportedFeatures.Add( RenderFeature::kDepthColorPairing );
				info.supportedFeatures.Add( RenderFeature::kComputeShaders );
				info.supportedFeatures.Add( RenderFeature::kMultiSample4x );
				info.supportedFeatures.Add( RenderFeature::kOffscreenRender );
				m_Adapters.push_back( info );
				m_Native.push_back( adapter );
				continue;
			}
			adapter->Release();
		}
		if ( m_Adapters.empty() && outError )
			*outError = "no D3D12 hardware adapter";
		return !m_Adapters.empty();
	}

	~D3d12Backend() override
	{
		for ( D3d12BackendDevice *device : m_Devices )
			delete device;
		for ( IDXGIAdapter1 *adapter : m_Native )
			adapter->Release();
		Release( m_Factory );
	}

	RenderBackendId GetBackendId() const override
	{
		RenderBackendId id;
		id.id = "d3d12";
		id.name = "Direct3D 12 Backend";
		id.version = 1;
		return id;
	}

	RenderProviderCaps GetProviderCaps() const override
	{
		RenderProviderCaps caps;
		caps.supportsOffscreenDevice = true;
		caps.supportsDeviceLossRecovery = true;
		return caps;
	}

	int GetAdapterCount() const override { return static_cast<int>( m_Adapters.size() ); }

	bool GetAdapterInfo( int index, RenderAdapterInfo *out ) const override
	{
		if ( index < 0 || index >= GetAdapterCount() || !out )
			return false;
		*out = m_Adapters[std::size_t( index )];
		return true;
	}

	IRenderDevice *CreateDevice( const RenderDeviceRequest &request, RenderCreateError *error ) override
	{
		if ( request.adapterIndex < 0 || request.adapterIndex >= GetAdapterCount() )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ), "no adapter at index %d",
				    request.adapterIndex );
			}
			return nullptr;
		}
		const RenderAdapterInfo &adapter = m_Adapters[std::size_t( request.adapterIndex )];
		if ( !adapter.supportedFeatures.Contains( request.requiredFeatures ) )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kUnsupportedRequiredFeature;
				for ( std::uint32_t bit = 0; bit < 32; ++bit )
				{
					const auto feature = static_cast<RenderFeature>( bit );
					if ( request.requiredFeatures.Has( feature ) &&
					     !adapter.supportedFeatures.Has( feature ) )
					{
						error->missingFeature = feature;
						break;
					}
				}
				std::snprintf( error->message, sizeof( error->message ),
				    "adapter does not support a required feature" );
			}
			return nullptr;
		}
		ID3D12Device *native = nullptr;
		if ( FAILED( D3D12CreateDevice( m_Native[std::size_t( request.adapterIndex )],
		         D3D_FEATURE_LEVEL_11_0, IID_ID3D12Device, reinterpret_cast<void **>( &native ) ) ) )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ), "D3D12CreateDevice failed" );
			}
			return nullptr;
		}
		auto *device = new D3d12BackendDevice( m_Factory, native );
		if ( !device->Initialize() )
		{
			delete device;
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ),
				    "queue or fence creation failed" );
			}
			return nullptr;
		}
		m_Devices.push_back( device );
		return device;
	}

	void DestroyDevice( IRenderDevice *device ) override
	{
		for ( auto it = m_Devices.begin(); it != m_Devices.end(); ++it )
		{
			if ( *it == device )
			{
				delete *it;
				m_Devices.erase( it );
				return;
			}
		}
	}

	std::size_t GetLiveDeviceCount() const override { return m_Devices.size(); }

	bool OwnsDevice( const IRenderDevice &device ) const override
	{
		for ( const D3d12BackendDevice *mine : m_Devices )
		{
			if ( static_cast<const IRenderDevice *>( mine ) == &device )
				return true;
		}
		return false;
	}

	D3d12DeviceEndpoint *FindDevice( IRenderDevice &device ) override
	{
		for ( D3d12BackendDevice *mine : m_Devices )
		{
			if ( static_cast<IRenderDevice *>( mine ) == &device )
				return mine;
		}
		return nullptr;
	}

private:
	IDXGIFactory4 *m_Factory = nullptr;
	std::vector<RenderAdapterInfo> m_Adapters;
	std::vector<IDXGIAdapter1 *> m_Native;
	std::vector<D3d12BackendDevice *> m_Devices;
};

} // namespace

std::unique_ptr<D3d12RenderBackend> MakeD3d12RenderBackend( std::string *outError )
{
	auto backend = std::make_unique<D3d12Backend>();
	if ( !backend->Initialize( outError ) )
		return nullptr;
	return backend;
}

} // namespace render_d3d12
