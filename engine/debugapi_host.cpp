//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the engine's IDebugApiHost and the lifecycle of the root-bound
//          debug API server (public/engine/debugapi_binding.h). Built only
//          with --debug-api=enabled (SOURCE_DEBUGAPI).
//
//=============================================================================//

#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "debugapi/debugapi_unix_socket.h"
#include "debugapi/debugapi_service.h"
#include "engine/debugapi_binding.h"

#include "quakedef.h"
#include "cmd.h"
#include "console.h"
#include "debugapi_engine.h"
#include "filesystem_engine.h"
#include "host.h"
#include "host_state.h"
#include "icvar.h"
#include "server.h"
#include "tier1/convar.h"
#include "tier1/utlbuffer.h"
#ifndef SWDS
#include "bitmap/tgawriter.h"
#include "cl_main.h"
#include "client.h"
#include "ivideomode.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

using namespace debugapi;

struct BoundSelection
{
	const TransportProvider *transport = nullptr;
	std::string address;
	const FramingProvider *framing = nullptr;
	std::vector<HostProviderInfo> providers;
};

HostStateKind ToHostState( int state )
{
	switch ( state )
	{
	case 0:
		return HostStateKind::NewGame;
	case 1:
		return HostStateKind::LoadGame;
	case 2:
		return HostStateKind::ChangeLevelSinglePlayer;
	case 3:
		return HostStateKind::ChangeLevelMultiPlayer;
	case 4:
		return HostStateKind::Run;
	case 5:
		return HostStateKind::GameShutdown;
	case 6:
		return HostStateKind::Shutdown;
	case 7:
		return HostStateKind::Restart;
	default:
		return HostStateKind::Unknown;
	}
}

struct FlagName
{
	int flag;
	const char *name;
};

const FlagName kFlagNames[] = {
	{ FCVAR_CHEAT, "cheat" },
	{ FCVAR_ARCHIVE, "archive" },
	{ FCVAR_REPLICATED, "replicated" },
	{ FCVAR_NOTIFY, "notify" },
	{ FCVAR_PROTECTED, "protected" },
	{ FCVAR_SPONLY, "sponly" },
	{ FCVAR_USERINFO, "userinfo" },
	{ FCVAR_GAMEDLL, "gamedll" },
	{ FCVAR_CLIENTDLL, "clientdll" },
	{ FCVAR_DEVELOPMENTONLY, "devonly" },
	{ FCVAR_HIDDEN, "hidden" },
	{ FCVAR_PRINTABLEONLY, "printableonly" },
	{ FCVAR_UNLOGGED, "unlogged" },
};

HostError Error( HostErrorCode code, std::string message )
{
	return HostError{ code, std::move( message ) };
}

class EngineDebugApiHost final : public IDebugApiHost, public IConsoleDisplayFunc
{
public:
	explicit EngineDebugApiHost( const BoundSelection &selection ) : m_Selection( selection ) {}

	// IDebugApiHost
	HostInfo Describe() override
	{
		HostInfo info;
#ifdef SWDS
		info.product = HostProduct::Dedicated;
#else
		info.product = sv.IsDedicated() ? HostProduct::Dedicated : HostProduct::Client;
#endif
		info.buildRevision = "build " + std::to_string( build_number() );
		info.gameDir = com_gamedir;
		info.providers = m_Selection.providers;
		return info;
	}

	HostStatus Status() override
	{
		HostStatus status;
		status.hostState = ToHostState( HostState_GetCurrentState() );
		status.serverActive = sv.IsActive();
		if ( status.serverActive )
		{
			status.map = sv.GetMapName();
			status.playerCount = static_cast<uint32_t>( sv.GetNumClients() );
		}
		status.maxPlayers = static_cast<uint32_t>( sv.GetMaxClients() );
#ifndef SWDS
		if ( !sv.IsDedicated() )
		{
			status.clientSignon = cl.m_nSignonState;
			if ( status.map.empty() )
				status.map = cl.m_szLevelBaseName;
		}
#endif
		status.hostFrame = static_cast<uint32_t>( host_framecount );
		status.hostTime = host_time;
		return status;
	}

	double NowSeconds() override { return Plat_FloatTime(); }

	ExecOutput Exec( std::string_view commandLine, size_t maxOutputBytes ) override
	{
		m_Capture.clear();
		m_CaptureLimit = maxOutputBytes;
		m_CaptureTruncated = false;
		const std::string command( commandLine );
		Con_SetOutputCapture( &EngineDebugApiHost::CaptureThunk, this );
		Cbuf_AddText( command.c_str() );
		Cbuf_AddText( "\n" );
		Cbuf_Execute();
		Con_SetOutputCapture( NULL, NULL );
		ExecOutput output;
		output.text = std::move( m_Capture );
		output.truncated = m_CaptureTruncated;
		m_Capture.clear();
		return output;
	}

	HostResult<CvarInfo> GetCvar( std::string_view name ) override
	{
		const std::string key( name );
		ConVar *var = g_pCVar->FindVar( key.c_str() );
		if ( !var )
			return foundation::MakeUnexpected(
			    Error( HostErrorCode::NotFound, "no console variable named " + key ) );
		return DescribeCvar( *var );
	}

	HostResult<CvarInfo> SetCvar( std::string_view name, std::string_view value ) override
	{
		const std::string key( name );
		ConVar *var = g_pCVar->FindVar( key.c_str() );
		if ( !var )
			return foundation::MakeUnexpected(
			    Error( HostErrorCode::NotFound, "no console variable named " + key ) );
		// The console path applies cheat, replication and protection rules;
		// they are not restated here.
		const std::string before = var->GetString();
		const ExecOutput output = Exec( key + " \"" + std::string( value ) + "\"", 4096 );
		const std::string after = var->GetString();
		if ( after == before && before != value && !output.text.empty() )
			return foundation::MakeUnexpected(
			    Error( HostErrorCode::FailedPrecondition, output.text ) );
		return DescribeCvar( *var );
	}

	HostResult<ScreenshotTicket> RequestScreenshot( const ScreenshotSpec &spec ) override
	{
#ifdef SWDS
		(void)spec;
		return foundation::MakeUnexpected(
		    Error( HostErrorCode::Unavailable, "a dedicated server does not render" ) );
#else
		if ( sv.IsDedicated() || !videomode )
			return foundation::MakeUnexpected(
			    Error( HostErrorCode::Unavailable, "this product does not render" ) );
		PendingCapture pending;
		pending.ticket = m_NextTicket++;
		pending.spec = spec;
		m_Captures.push_back( std::move( pending ) );
		// Hooked only while a capture is pending: the hook pauses material
		// threading for the frame, like the engine's own screenshot path.
		CL_SetFrameCaptureHook( &EngineDebugApiHost::FrameCaptureThunk, this );
		return m_Captures.back().ticket;
#endif
	}

	std::optional<HostResult<ScreenshotCapture>> PollScreenshot( ScreenshotTicket ticket ) override
	{
		for ( auto it = m_Captures.begin(); it != m_Captures.end(); ++it )
		{
			if ( it->ticket != ticket )
				continue;
			if ( !it->done )
				return std::nullopt;
			HostResult<ScreenshotCapture> result = std::move( it->result );
			m_Captures.erase( it );
			return result;
		}
		return HostResult<ScreenshotCapture>( foundation::MakeUnexpected(
		    Error( HostErrorCode::Internal, "unknown screenshot ticket" ) ) );
	}

	void CancelScreenshot( ScreenshotTicket ticket ) override
	{
		for ( auto it = m_Captures.begin(); it != m_Captures.end(); ++it )
		{
			if ( it->ticket == ticket )
			{
				m_Captures.erase( it );
				break;
			}
		}
		UnhookWhenIdle();
	}

	void RequestQuit() override { Cbuf_AddText( "quit\n" ); }

	void SetLogSink( ILogSink *sink ) override
	{
		std::lock_guard<std::mutex> lock( m_LogMutex );
		m_LogSink = sink;
	}

	// IConsoleDisplayFunc: may run on any thread.
	void ColorPrint( const Color &clr, const char *message ) override
	{
		const uint32_t rgba = ( uint32_t( clr.r() ) << 24 ) | ( uint32_t( clr.g() ) << 16 ) |
		                      ( uint32_t( clr.b() ) << 8 ) | uint32_t( clr.a() );
		Forward( message, false, rgba );
	}
	void Print( const char *message ) override { Forward( message, false, 0 ); }
	void DPrint( const char *message ) override { Forward( message, true, 0 ); }

#ifndef SWDS
	static void FrameCaptureThunk( void *context )
	{
		static_cast<EngineDebugApiHost *>( context )->CaptureFrame();
	}
#endif

private:
	struct PendingCapture
	{
		ScreenshotTicket ticket = 0;
		ScreenshotSpec spec;
		bool done = false;
		HostResult<ScreenshotCapture> result = ScreenshotCapture{};
	};

	static void CaptureThunk( void *context, const char *text )
	{
		auto *self = static_cast<EngineDebugApiHost *>( context );
		const size_t room = self->m_CaptureLimit - self->m_Capture.size();
		const size_t length = Q_strlen( text );
		if ( length > room )
		{
			self->m_Capture.append( text, room );
			self->m_CaptureTruncated = true;
			return;
		}
		self->m_Capture.append( text, length );
	}

	void Forward( const char *message, bool developer, uint32_t rgba )
	{
		if ( !message || !message[0] )
			return;
		std::lock_guard<std::mutex> lock( m_LogMutex );
		if ( m_LogSink )
			m_LogSink->Append( message, developer, rgba );
	}

	CvarInfo DescribeCvar( ConVar &var )
	{
		CvarInfo info;
		info.name = var.GetName();
		for ( const FlagName &flag : kFlagNames )
		{
			if ( var.IsFlagSet( flag.flag ) )
				info.flags.push_back( flag.name );
		}
		if ( !var.IsFlagSet( FCVAR_PROTECTED ) )
		{
			info.value = var.GetString();
			info.defaultValue = var.GetDefault();
			info.floatValue = var.GetFloat();
			info.intValue = var.GetInt();
		}
		if ( var.GetHelpText() )
			info.help = var.GetHelpText();
		return info;
	}

#ifndef SWDS
	void CaptureFrame()
	{
		for ( PendingCapture &pending : m_Captures )
		{
			if ( pending.done )
				continue;
			pending.result = Capture( pending );
			pending.done = true;
		}
		UnhookWhenIdle();
	}

	HostResult<ScreenshotCapture> Capture( const PendingCapture &pending )
	{
		ScreenshotCapture capture;
		capture.width = static_cast<uint32_t>( videomode->GetModeStereoWidth() );
		capture.height = static_cast<uint32_t>( videomode->GetModeStereoHeight() );
		capture.hostFrame = static_cast<uint32_t>( host_framecount );
		if ( pending.spec.kind == ScreenshotKind::InlineTga )
		{
			// The same read and encoder as TakeSnapshotTGA, kept in memory.
			std::vector<uint8> pixels( size_t( capture.width ) * capture.height * 3 );
			videomode->ReadScreenPixels( 0, 0, capture.width, capture.height, pixels.data(),
			    IMAGE_FORMAT_RGB888 );
			CUtlBuffer buffer;
			if ( !TGAWriter::WriteToBuffer( pixels.data(), buffer, capture.width, capture.height,
			         IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888 ) )
				return foundation::MakeUnexpected(
				    Error( HostErrorCode::Internal, "TGA encoding failed" ) );
			capture.tga.assign( static_cast<const char *>( buffer.Base() ), buffer.TellPut() );
			return capture;
		}
		char name[MAX_PATH];
		Q_snprintf( name, sizeof( name ), "screenshots/debugapi_%06d_%llu.tga", host_framecount,
		    static_cast<unsigned long long>( pending.ticket ) );
		g_pFileSystem->CreateDirHierarchy( "screenshots", "DEFAULT_WRITE_PATH" );
		videomode->TakeSnapshotTGA( name );
		char path[MAX_PATH];
		if ( !g_pFileSystem->GetLocalPath( name, path, sizeof( path ) ) )
			return foundation::MakeUnexpected(
			    Error( HostErrorCode::Internal, "the screenshot was not written" ) );
		capture.path = path;
		return capture;
	}
#endif

	void UnhookWhenIdle()
	{
#ifndef SWDS
		for ( const PendingCapture &pending : m_Captures )
		{
			if ( !pending.done )
				return;
		}
		CL_SetFrameCaptureHook( NULL, NULL );
#endif
	}

	const BoundSelection &m_Selection;
	std::string m_Capture;
	size_t m_CaptureLimit = 0;
	bool m_CaptureTruncated = false;
	std::mutex m_LogMutex;
	ILogSink *m_LogSink = nullptr;
	std::vector<PendingCapture> m_Captures;
	ScreenshotTicket m_NextTicket = 1;
};

// Owned by the engine between Start and Stop; the service is destroyed first.
BoundSelection s_Selection;
bool s_Bound = false;
std::unique_ptr<EngineDebugApiHost> s_Host;
std::unique_ptr<IDebugApiService> s_Service;

void Uninstall()
{
#ifndef SWDS
	CL_SetFrameCaptureHook( NULL, NULL );
#endif
	if ( s_Host && g_pCVar )
		g_pCVar->RemoveConsoleDisplayFunc( s_Host.get() );
	s_Host.reset();
}

} // namespace

DLL_EXPORT const debugapi::TransportProvider *DebugApi_UnixSocketTransport()
{
	return &debugapi::UnixSocketTransportProvider();
}

DLL_EXPORT const debugapi::FramingProvider *DebugApi_NewlineFraming()
{
	return &debugapi::NewlineFraming();
}

DLL_EXPORT const debugapi::FramingProvider *DebugApi_ContentLengthFraming()
{
	return &debugapi::ContentLengthFraming();
}

DLL_EXPORT bool Engine_BindDebugApi( const DebugApiSelection *selection )
{
	if ( s_Bound || s_Service || !selection || !selection->transport ||
	     !selection->transport->create || !selection->address || !selection->address[0] ||
	     !selection->framing || !selection->framing->createDecoder || !selection->framing->encode ||
	     ( selection->providerCount != 0 && !selection->providers ) )
		return false;
	BoundSelection bound;
	bound.transport = selection->transport;
	bound.address = selection->address;
	bound.framing = selection->framing;
	for ( size_t i = 0; i < selection->providerCount; ++i )
	{
		const DebugApiComposedProvider &provider = selection->providers[i];
		if ( !provider.role || !provider.id )
			return false;
		bound.providers.push_back( HostProviderInfo{ provider.role, provider.id } );
	}
	s_Selection = std::move( bound );
	s_Bound = true;
	return true;
}

bool DebugApi_EngineStart()
{
	if ( !s_Bound || s_Service )
		return true;
	s_Host = std::make_unique<EngineDebugApiHost>( s_Selection );
	g_pCVar->InstallConsoleDisplayFunc( s_Host.get() );
	ServiceResult service = StartDebugApiService(
	    *s_Selection.transport, s_Selection.address, *s_Selection.framing, *s_Host );
	if ( !service )
	{
		Warning( "Debug API failed to start: %s\n", service.Error().c_str() );
		Uninstall();
		return false;
	}
	s_Service = std::move( service ).Value();
	Msg( "Debug API: JSON-RPC 2.0 on %s:%s (%s framing)\n", s_Selection.transport->scheme,
	    s_Selection.address.c_str(), s_Selection.framing->name );
	return true;
}

void DebugApi_EngineFrame()
{
	if ( s_Service )
		s_Service->Pump();
}

void DebugApi_EngineStop()
{
	s_Service.reset();
	Uninstall();
}
