//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the deterministic null render backend provider and
//			its injectable defects. See null_render_backend.h.
//
//=============================================================================//

#include "null_render_backend.h"

#include <cstdio>
#include <vector>

namespace rendertest
{

using namespace render;

namespace
{

// ---------------------------------------------------------------------------
// Completion token: FIFO GPU-completion model driven by PollCompletion.
// ---------------------------------------------------------------------------

class NullCompletionToken : public IRenderCompletionToken
{
public:
	explicit NullCompletionToken( uint64_t submissionId ) : m_SubmissionId( submissionId ) {}
	bool IsComplete() const override { return m_Complete; }

	uint64_t SubmissionId() const { return m_SubmissionId; }
	void MarkComplete() { m_Complete = true; }

private:
	uint64_t m_SubmissionId;
	bool m_Complete = false;
};

class NullCommandContext : public IRenderCommandContext
{
public:
	void RecordUse( RenderResourceHandle handle ) override { m_Used.push_back( handle ); }

private:
	std::vector<RenderResourceHandle> m_Used;
};

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------

struct ResourceSlot
{
	RenderResourceType type = RenderResourceType::kBuffer;
	bool live = false;
};

struct DeferredDestroy
{
	RenderResourceHandle handle = kInvalidResource;
	IRenderCompletionToken *token = nullptr;
};

class NullDevice : public IRenderDevice
{
public:
	NullDevice( const RenderDeviceCaps &caps, const NullBackendDefects &defects )
		: m_Caps( caps ), m_Defects( defects )
	{
	}

	~NullDevice() override
	{
		for ( NullCompletionToken *t : m_Tokens )
			delete t;
		for ( NullCommandContext *c : m_Contexts )
			delete c;
	}

	const RenderDeviceCaps &GetCapabilities() const override
	{
		if ( m_Defects.mutableCaps )
		{
			m_MutableCaps = m_Caps;
			m_MutableCaps.maxTextureDimension += static_cast<uint32_t>( ++m_CapsQueryCount );
			return m_MutableCaps;
		}
		return m_Caps;
	}

	RenderDeviceState GetState() const override { return m_State; }

	// -- Resources --
	RenderResourceHandle CreateResource( RenderResourceType type ) override
	{
		m_Slots.push_back( ResourceSlot{ type, true } );
		return static_cast<RenderResourceHandle>( m_Slots.size() ); // 1-based handle
	}

	bool IsResourceLive( RenderResourceHandle handle ) const override
	{
		const ResourceSlot *slot = Slot( handle );
		return slot != nullptr && slot->live;
	}

	size_t GetLiveResourceCount() const override
	{
		size_t n = 0;
		for ( const ResourceSlot &s : m_Slots )
		{
			if ( s.live )
				++n;
		}
		return n;
	}

	void DestroyResourceWhenComplete( RenderResourceHandle handle,
		IRenderCompletionToken &token ) override
	{
		m_Deferred.push_back( DeferredDestroy{ handle, &token } );
	}

	void CollectCompletedDestructions() override
	{
		std::vector<DeferredDestroy> remaining;
		for ( const DeferredDestroy &d : m_Deferred )
		{
			// Correct: recycle only when the token completes. Defect: recycle as
			// soon as the submit/frame counter advanced, ignoring the token (a
			// frame index is not proof of GPU completion, per RFC 0006).
			const bool ready = m_Defects.recycleOnFrameAdvance
				? ( m_FramesAdvanced > 0 )
				: d.token->IsComplete();
			if ( ready && !m_Defects.leakResources )
			{
				ResourceSlot *slot = Slot( d.handle );
				if ( slot )
					slot->live = false;
			}
			else
			{
				remaining.push_back( d );
			}
		}
		m_Deferred.swap( remaining );
	}

	// -- Submission --
	IRenderCommandContext *CreateCommandContext() override
	{
		NullCommandContext *ctx = new NullCommandContext();
		m_Contexts.push_back( ctx );
		return ctx;
	}

	IRenderCompletionToken *Submit( IRenderCommandContext & ) override
	{
		++m_FramesAdvanced;
		const uint64_t id = ++m_NextSubmissionId;
		NullCompletionToken *token = new NullCompletionToken( id );
		m_Tokens.push_back( token );
		m_Outstanding.push_back( token );
		return token;
	}

	uint32_t PollCompletion() override
	{
		if ( m_Outstanding.empty() )
			return 0;

		size_t index = 0; // oldest by default (FIFO)
		if ( m_Defects.unorderedCompletion && m_Outstanding.size() > 1 )
			index = m_Outstanding.size() - 1; // complete the newest first (wrong order)

		NullCompletionToken *token = m_Outstanding[index];
		token->MarkComplete();
		if ( token->SubmissionId() > m_LastCompleted )
			m_LastCompleted = token->SubmissionId();
		m_Outstanding.erase( m_Outstanding.begin() + static_cast<std::ptrdiff_t>( index ) );
		return 1;
	}

	uint64_t LastCompletedSubmission() const override { return m_LastCompleted; }

	// -- Device loss --
	bool SimulateDeviceLoss() override
	{
		if ( m_Defects.lossSkipsLostState )
		{
			m_State = RenderDeviceState::kAvailable; // never actually reports loss
			return true;
		}
		m_State = RenderDeviceState::kDeviceLost;
		return true;
	}

	bool RecoverDevice() override
	{
		if ( m_SupportsRecovery )
		{
			m_State = RenderDeviceState::kAvailable;
			return true;
		}
		m_State = RenderDeviceState::kFatal;
		return false;
	}

	void SetSupportsRecovery( bool v ) { m_SupportsRecovery = v; }

private:
	const ResourceSlot *Slot( RenderResourceHandle handle ) const
	{
		if ( handle == kInvalidResource || handle > m_Slots.size() )
			return nullptr;
		return &m_Slots[static_cast<size_t>( handle ) - 1];
	}
	ResourceSlot *Slot( RenderResourceHandle handle )
	{
		return const_cast<ResourceSlot *>(
			static_cast<const NullDevice *>( this )->Slot( handle ) );
	}

	RenderDeviceCaps m_Caps;
	mutable RenderDeviceCaps m_MutableCaps;
	mutable uint64_t m_CapsQueryCount = 0;
	NullBackendDefects m_Defects;
	RenderDeviceState m_State = RenderDeviceState::kAvailable;

	std::vector<ResourceSlot> m_Slots;
	std::vector<DeferredDestroy> m_Deferred;

	std::vector<NullCommandContext *> m_Contexts;
	std::vector<NullCompletionToken *> m_Tokens;
	std::vector<NullCompletionToken *> m_Outstanding;
	uint64_t m_NextSubmissionId = 0;
	uint64_t m_LastCompleted = 0;
	uint64_t m_FramesAdvanced = 0;

	bool m_SupportsRecovery = true;
};

// ---------------------------------------------------------------------------
// Provider
// ---------------------------------------------------------------------------

class NullProvider : public IRenderBackendProvider
{
public:
	explicit NullProvider( const NullBackendDefects &defects ) : m_Defects( defects )
	{
		// One deterministic offscreen adapter.
		std::snprintf( m_Adapter.id, sizeof( m_Adapter.id ), "%s", "null-0" );
		std::snprintf( m_Adapter.name, sizeof( m_Adapter.name ), "%s", "Null Recording Adapter" );
		m_Adapter.vendorId = 0;
		m_Adapter.deviceMemoryBytes = 0;
		m_Adapter.isSoftware = true;
		m_Adapter.supportedFeatures.Add( RenderFeature::kSampledSrgb );
		m_Adapter.supportedFeatures.Add( RenderFeature::kDepthColorPairing );
		m_Adapter.supportedFeatures.Add( RenderFeature::kOffscreenRender );
	}

	~NullProvider() override
	{
		for ( NullDevice *d : m_Devices )
			delete d;
	}

	RenderBackendId GetBackendId() const override
	{
		RenderBackendId id;
		id.id = "null";
		id.name = "Null Recording Backend";
		id.version = 1;
		return id;
	}

	RenderProviderCaps GetProviderCaps() const override
	{
		RenderProviderCaps caps;
		caps.supportsOffscreenDevice = true;
		caps.supportsDeviceLossRecovery = true;
		caps.supportsRuntimeShaderCompile = false;
		return caps;
	}

	int GetAdapterCount() const override { return 1; }

	bool GetAdapterInfo( int index, RenderAdapterInfo *out ) const override
	{
		if ( index != 0 || out == nullptr )
			return false;
		*out = m_Adapter;
		return true;
	}

	IRenderDevice *CreateDevice( const RenderDeviceRequest &request,
		RenderCreateError *error ) override
	{
		if ( request.adapterIndex != 0 )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ),
					"no adapter at index %d", request.adapterIndex );
			}
			return nullptr;
		}

		if ( !m_Defects.swallowRequiredFeature &&
			!m_Adapter.supportedFeatures.Contains( request.requiredFeatures ) )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kUnsupportedRequiredFeature;
				error->missingFeature = FirstMissing( request.requiredFeatures );
				std::snprintf( error->message, sizeof( error->message ),
					"adapter does not support a required feature" );
			}
			return nullptr;
		}

		if ( error )
			error->status = RenderCreateStatus::kOk;

		RenderDeviceCaps caps;
		caps.features = m_Adapter.supportedFeatures;
		caps.maxTextureDimension = 16384;
		caps.maxColorTargets = 8;
		caps.maxSampleCount = 4;

		NullDevice *device = new NullDevice( caps, m_Defects );
		const RenderProviderCaps pcaps = GetProviderCaps();
		device->SetSupportsRecovery( pcaps.supportsDeviceLossRecovery );
		m_Devices.push_back( device );
		return device;
	}

	void DestroyDevice( IRenderDevice *device ) override
	{
		for ( size_t i = 0; i < m_Devices.size(); ++i )
		{
			if ( m_Devices[i] == device )
			{
				delete m_Devices[i];
				m_Devices.erase( m_Devices.begin() + static_cast<std::ptrdiff_t>( i ) );
				return;
			}
		}
	}

	size_t GetLiveDeviceCount() const override { return m_Devices.size(); }

	bool OwnsDevice( const IRenderDevice &device ) const override
	{
		if ( m_Defects.disownDevices )
			return false;
		for ( const NullDevice *d : m_Devices )
		{
			if ( d == &device )
				return true;
		}
		return false;
	}

private:
	RenderFeature FirstMissing( const RenderFeatureSet &required ) const
	{
		for ( uint32_t bit = 0; bit < 32; ++bit )
		{
			const RenderFeature f = static_cast<RenderFeature>( bit );
			if ( required.Has( f ) && !m_Adapter.supportedFeatures.Has( f ) )
				return f;
		}
		return RenderFeature::kNeverSupported;
	}

	NullBackendDefects m_Defects;
	RenderAdapterInfo m_Adapter;
	std::vector<NullDevice *> m_Devices;
};

} // namespace

std::unique_ptr<IRenderBackendProvider> MakeNullRenderBackend( NullBackendDefects defects )
{
	return std::unique_ptr<IRenderBackendProvider>( new NullProvider( defects ) );
}

} // namespace rendertest
