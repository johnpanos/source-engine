//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy shader-API modules as render.contracts providers, the legacy
//			quirk table, and legacy profile selection. See the header.
//
//=============================================================================//

#include "legacy_render_backend_provider.h"

#include "materialsystem/imaterialsystem.h"
#include "shaderapi/IShaderDevice.h"

// <cstring> is unavailable here: material-system targets define strncpy away.
#include <cstdio>

namespace render
{

namespace
{

void CopyString( char *dest, size_t size, const char *source )
{
	std::snprintf( dest, size, "%s", source ? source : "" );
}

const RenderQuirk s_LegacyQuirks[] = {
    { "gl.float-normalization-cubemaps",
        "The D3D9-to-OpenGL translation layer (togl) has poor 8-bit and signed "
        "UVWQ8888 cubemap format support, so the texture manager has always built "
        "normalization cubemaps as RGBA16F whenever it ran over OpenGL.",
        nullptr, "opengl", 0, 0, kRenderAnyDeviceIdMax, 0, kRenderAnyDriverVersionMax, 0,
        RenderWorkaroundSet::Bit( RenderWorkaround::kFloatNormalizationCubemaps ) },
};

struct FeatureName
{
	RenderFeature feature;
	const char *name;
};

const FeatureName s_FeatureNames[] = {
    { RenderFeature::kSampledSrgb, "sampled-srgb" },
    { RenderFeature::kDepthColorPairing, "depth-color-pairing" },
    { RenderFeature::kComputeShaders, "compute-shaders" },
    { RenderFeature::kMultiSample4x, "msaa-4x" },
    { RenderFeature::kRuntimeShaderCompile, "runtime-shader-compile" },
    { RenderFeature::kOffscreenRender, "offscreen-render" },
};

// Views a bound device manager as an adapter source.
class CDeviceMgrAdapterSource : public ILegacyAdapterSource
{
public:
	explicit CDeviceMgrAdapterSource( const IShaderDeviceMgr &manager ) : m_Manager( manager ) {}
	int GetAdapterCount() const { return m_Manager.GetAdapterCount(); }
	void GetAdapterInfo( int adapter, MaterialAdapterInfo_t &info ) const
	{
		m_Manager.GetAdapterInfo( adapter, info );
	}

private:
	const IShaderDeviceMgr &m_Manager;
};

void Append( char *buffer, size_t size, const char *text )
{
	size_t used = 0;
	while ( used < size && buffer[used] )
		++used;
	if ( used + 1 < size )
		std::snprintf( buffer + used, size - used, "%s", text );
}

} // namespace

LegacyRenderBackendProvider::LegacyRenderBackendProvider(
    const LegacyShaderProvider &provider, const LegacyShaderServices &services )
    : m_pId( provider.id ), m_pName( provider.legacyModuleName ), m_bValid( true ), m_nAdapters( 0 )
{
	m_InvalidReason[0] = '\0';
	if ( !services.manager )
	{
		Capture( NULL, services.describeAdapter );
		return;
	}
	const CDeviceMgrAdapterSource adapters( *services.manager );
	Capture( &adapters, services.describeAdapter );
}

LegacyRenderBackendProvider::LegacyRenderBackendProvider( const LegacyShaderProvider &provider,
    const ILegacyAdapterSource *adapters, LegacyDescribeAdapterFn describe )
    : m_pId( provider.id ), m_pName( provider.legacyModuleName ), m_bValid( true ), m_nAdapters( 0 )
{
	m_InvalidReason[0] = '\0';
	Capture( adapters, describe );
}

void LegacyRenderBackendProvider::Capture(
    const ILegacyAdapterSource *adapters, LegacyDescribeAdapterFn describe )
{
	if ( !m_pId || !m_pId[0] || !adapters )
	{
		Invalidate( "the provider has no id or no connected device manager" );
		return;
	}

	const int count = adapters->GetAdapterCount();
	if ( count < 0 || count > kMaxAdapters )
	{
		Invalidate( "the device manager reports an unsupported adapter count" );
		return;
	}

	for ( int i = 0; i < count; ++i )
	{
		MaterialAdapterInfo_t legacy = MaterialAdapterInfo_t();
		adapters->GetAdapterInfo( i, legacy );

		RenderAdapterInfo &info = m_Adapters[i];
		info = RenderAdapterInfo();
		std::snprintf( info.id, sizeof( info.id ), "%s:%d", m_pId, i );
		legacy.m_pDriverName[sizeof( legacy.m_pDriverName ) - 1] = '\0';
		CopyString( info.name, sizeof( info.name ), legacy.m_pDriverName );
		info.vendorId = legacy.m_VendorID;
		info.deviceId = legacy.m_DeviceID;
		info.driverVersion =
		    ( uint64_t( legacy.m_nDriverVersionHigh ) << 32 ) | legacy.m_nDriverVersionLow;

		// The manager owns adapter identity; the backend's describe hook owns
		// only the semantic facts the legacy structure cannot express.
		if ( describe )
		{
			RenderAdapterInfo facts;
			if ( !describe( i, &facts ) )
			{
				Invalidate( "the backend cannot describe an adapter its manager enumerates" );
				return;
			}
			if ( facts.supportedFeatures.Has( RenderFeature::kNeverSupported ) )
			{
				Invalidate( "the backend claims the reserved never-supported feature" );
				return;
			}
			info.supportedFeatures = facts.supportedFeatures;
			facts.driverApi[sizeof( facts.driverApi ) - 1] = '\0';
			CopyString( info.driverApi, sizeof( info.driverApi ), facts.driverApi );
			info.deviceMemoryBytes = facts.deviceMemoryBytes;
			info.isSoftware = facts.isSoftware;
		}
		++m_nAdapters;
	}
}

void LegacyRenderBackendProvider::Invalidate( const char *reason )
{
	m_bValid = false;
	m_nAdapters = 0;
	CopyString( m_InvalidReason, sizeof( m_InvalidReason ), reason );
}

RenderBackendId LegacyRenderBackendProvider::GetBackendId() const
{
	RenderBackendId id;
	id.id = m_pId;
	id.name = m_pName;
	id.version = 1;
	return id;
}

RenderProviderCaps LegacyRenderBackendProvider::GetProviderCaps() const
{
	// Devices and presentation remain on the legacy SetMode path; see the header.
	return RenderProviderCaps();
}

int LegacyRenderBackendProvider::GetAdapterCount() const
{
	return m_nAdapters;
}

bool LegacyRenderBackendProvider::GetAdapterInfo( int index, RenderAdapterInfo *out ) const
{
	if ( !out || index < 0 || index >= m_nAdapters )
		return false;
	*out = m_Adapters[index];
	return true;
}

IRenderDevice *LegacyRenderBackendProvider::CreateDevice(
    const RenderDeviceRequest &request, RenderCreateError *error )
{
	RenderCreateError result;
	if ( request.adapterIndex < 0 || request.adapterIndex >= m_nAdapters )
	{
		result.status = RenderCreateStatus::kInvalidAdapter;
		CopyString( result.message, sizeof( result.message ), "no such adapter" );
	}
	else if ( !m_Adapters[request.adapterIndex].supportedFeatures.Contains(
	              request.requiredFeatures ) )
	{
		const uint32_t missing = request.requiredFeatures.bits &
		                         ~m_Adapters[request.adapterIndex].supportedFeatures.bits;
		result.status = RenderCreateStatus::kUnsupportedRequiredFeature;
		for ( uint32_t bit = 0; bit < 32; ++bit )
		{
			if ( missing & ( uint32_t( 1 ) << bit ) )
			{
				result.missingFeature = static_cast<RenderFeature>( bit );
				break;
			}
		}
		CopyString( result.message, sizeof( result.message ), "adapter lacks a required feature" );
	}
	else
	{
		result.status = RenderCreateStatus::kNotAdvertised;
		CopyString( result.message, sizeof( result.message ),
		    "legacy devices are created by the material system's SetMode path" );
	}
	if ( error )
		*error = result;
	return nullptr;
}

void LegacyRenderBackendProvider::DestroyDevice( IRenderDevice * )
{
	// CreateDevice never returns a device, so there is nothing to release.
}

size_t LegacyRenderBackendProvider::GetLiveDeviceCount() const
{
	return 0;
}

const RenderQuirk *LegacyRenderQuirks( size_t *count )
{
	if ( count )
		*count = sizeof( s_LegacyQuirks ) / sizeof( s_LegacyQuirks[0] );
	return s_LegacyQuirks;
}

bool SelectLegacyRenderProfile( const LegacyShaderProvider &provider,
    const LegacyShaderServices &services, int adapter, const RenderProfileRequest &request,
    RenderFeatureProfile *profile, RenderProfileError *error )
{
	const LegacyRenderBackendProvider backend( provider, services );
	return SelectLegacyRenderProfile( backend, adapter, request, profile, error );
}

bool SelectLegacyRenderProfile( const LegacyRenderBackendProvider &backend, int adapter,
    const RenderProfileRequest &request, RenderFeatureProfile *profile, RenderProfileError *error )
{
	const char *id = backend.GetBackendId().id;
	if ( !backend.IsValid() )
	{
		if ( error )
		{
			*error = RenderProfileError();
			error->status = RenderProfileStatus::kInvalidProvider;
			CopyString( error->message, sizeof( error->message ), backend.InvalidReason() );
		}
		return false;
	}

	RenderAdapterInfo info;
	if ( !backend.GetAdapterInfo( adapter, &info ) )
	{
		if ( error )
		{
			*error = RenderProfileError();
			error->status = RenderProfileStatus::kInvalidAdapter;
			std::snprintf( error->message, sizeof( error->message ),
			    "provider '%s' has no adapter %d", id, adapter );
		}
		return false;
	}

	size_t quirkCount = 0;
	const RenderQuirk *quirks = LegacyRenderQuirks( &quirkCount );
	return SelectRenderFeatureProfile( id, info, request, quirks, quirkCount, profile, error );
}

void DescribeRenderProfile( const RenderFeatureProfile &profile, char *buffer, size_t size )
{
	if ( !buffer || !size )
		return;
	buffer[0] = '\0';
	Append( buffer, size, "features=" );
	bool any = false;
	for ( size_t i = 0; i < sizeof( s_FeatureNames ) / sizeof( s_FeatureNames[0] ); ++i )
	{
		if ( !profile.enabled.Has( s_FeatureNames[i].feature ) )
			continue;
		Append( buffer, size, any ? "," : "" );
		Append( buffer, size, s_FeatureNames[i].name );
		any = true;
	}
	Append( buffer, size, any ? "" : "none" );
	Append( buffer, size, " quirks=" );
	for ( uint32_t i = 0; i < profile.appliedQuirkCount; ++i )
	{
		Append( buffer, size, i ? "," : "" );
		Append( buffer, size, profile.appliedQuirks[i]->id );
	}
	Append( buffer, size, profile.appliedQuirkCount ? "" : "none" );
}

} // namespace render
