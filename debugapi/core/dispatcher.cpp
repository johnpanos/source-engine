//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Executes source.debug.v1 methods against an IDebugApiHost.
//
//=============================================================================//

#include "dispatcher.h"

#include <cmath>
#include <utility>

namespace debugapi
{
namespace
{

namespace gp = ::google::protobuf;

constexpr size_t kMaxCommandBytes = 510;
constexpr size_t kMaxCvarBytes = 255;
constexpr uint32_t kMaxWaitMs = 600000;
constexpr int kSignonFull = 6;

CallOutcome Fail( pb::ErrorCode code, std::string message )
{
	CallOutcome outcome;
	outcome.error = RpcError{ code, std::move( message ) };
	return outcome;
}

template <typename T> CallOutcome Succeed( std::unique_ptr<T> result )
{
	CallOutcome outcome;
	outcome.result = std::move( result );
	return outcome;
}

bool ValidCvarName( const std::string &name )
{
	if ( name.empty() || name.size() > kMaxCvarBytes )
		return false;
	for ( char c : name )
	{
		const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
		                ( c >= '0' && c <= '9' ) || c == '_' || c == '.' || c == '+' || c == '-';
		if ( !ok )
			return false;
	}
	return true;
}

pb::HostState ToProto( HostStateKind state )
{
	switch ( state )
	{
	case HostStateKind::NewGame:
		return pb::HOST_STATE_NEW_GAME;
	case HostStateKind::LoadGame:
		return pb::HOST_STATE_LOAD_GAME;
	case HostStateKind::ChangeLevelSinglePlayer:
		return pb::HOST_STATE_CHANGE_LEVEL_SP;
	case HostStateKind::ChangeLevelMultiPlayer:
		return pb::HOST_STATE_CHANGE_LEVEL_MP;
	case HostStateKind::Run:
		return pb::HOST_STATE_RUN;
	case HostStateKind::GameShutdown:
		return pb::HOST_STATE_GAME_SHUTDOWN;
	case HostStateKind::Shutdown:
		return pb::HOST_STATE_SHUTDOWN;
	case HostStateKind::Restart:
		return pb::HOST_STATE_RESTART;
	case HostStateKind::Unknown:
		break;
	}
	return pb::HOST_STATE_UNSPECIFIED;
}

void FillCvar( const CvarInfo &info, pb::CvarResult &out )
{
	out.set_name( SanitizeUtf8( info.name ) );
	out.set_value( SanitizeUtf8( info.value ) );
	out.set_default_value( SanitizeUtf8( info.defaultValue ) );
	out.set_float_value( std::isfinite( info.floatValue ) ? info.floatValue : 0.0 );
	out.set_int_value( info.intValue );
	for ( const std::string &flag : info.flags )
		out.add_flags( SanitizeUtf8( flag ) );
	out.set_help( SanitizeUtf8( info.help ) );
}

uint32_t ElapsedMs( double started, double now )
{
	const double ms = ( now - started ) * 1000.0;
	return ms <= 0.0 ? 0u : ms >= 4294967295.0 ? 4294967295u : static_cast<uint32_t>( ms );
}

} // namespace

void FillStatus( const HostStatus &status, pb::StatusResult &out )
{
	out.set_host_state( ToProto( status.hostState ) );
	out.set_map( SanitizeUtf8( status.map ) );
	out.set_server_active( status.serverActive );
	out.set_client_signon( pb::SignonState_IsValid( status.clientSignon )
	                           ? static_cast<pb::SignonState>( status.clientSignon )
	                           : pb::SIGNON_STATE_NONE );
	out.set_player_count( status.playerCount );
	out.set_max_players( status.maxPlayers );
	out.set_host_frame( status.hostFrame );
	out.set_host_time( std::isfinite( status.hostTime ) ? status.hostTime : 0.0 );
}

RpcError FromHostError( const HostError &error )
{
	pb::ErrorCode code = pb::ERROR_CODE_INTERNAL_ERROR;
	switch ( error.code )
	{
	case HostErrorCode::InvalidArgument:
		code = pb::ERROR_CODE_INVALID_PARAMS;
		break;
	case HostErrorCode::NotFound:
		code = pb::ERROR_CODE_NOT_FOUND;
		break;
	case HostErrorCode::FailedPrecondition:
		code = pb::ERROR_CODE_FAILED_PRECONDITION;
		break;
	case HostErrorCode::Unavailable:
		code = pb::ERROR_CODE_UNAVAILABLE_IN_PRODUCT;
		break;
	case HostErrorCode::Internal:
		break;
	}
	return RpcError{ code, SanitizeUtf8( error.message ) };
}

Dispatcher::Dispatcher( IDebugApiHost &host, ServerIdentity identity, DispatcherLimits limits )
    : m_Host( host ), m_Identity( std::move( identity ) ), m_Limits( limits )
{
}

Dispatcher::~Dispatcher()
{
	for ( auto &[tag, deferred] : m_Deferred )
	{
		if ( deferred.method == Method::Screenshot )
			m_Host.CancelScreenshot( deferred.ticket );
	}
}

std::optional<CallOutcome> Dispatcher::Handle(
    CallTag tag, Method method, const gp::Message &params )
{
	switch ( method )
	{
	case Method::Hello:
		return Hello( static_cast<const pb::HelloRequest &>( params ) );
	case Method::Exec:
		return Exec( static_cast<const pb::ExecRequest &>( params ) );
	case Method::CvarGet:
		return GetCvar( static_cast<const pb::CvarGetRequest &>( params ) );
	case Method::CvarSet:
		return SetCvar( static_cast<const pb::CvarSetRequest &>( params ) );
	case Method::Status:
		return Status();
	case Method::WaitFor:
		return WaitFor( tag, static_cast<const pb::WaitForRequest &>( params ) );
	case Method::Screenshot:
		return Screenshot( tag, static_cast<const pb::ScreenshotRequest &>( params ) );
	case Method::Quit:
		return Quit();
	case Method::Subscribe:
		break;
	}
	return Fail( pb::ERROR_CODE_INTERNAL_ERROR, "method is not dispatched here" );
}

CallOutcome Dispatcher::Hello( const pb::HelloRequest &request )
{
	if ( request.protocol_version() != pb::PROTOCOL_VERSION_UNSPECIFIED &&
	     request.protocol_version() != pb::PROTOCOL_VERSION_1 )
		return Fail( pb::ERROR_CODE_FAILED_PRECONDITION, "server speaks protocol version 1" );

	const HostInfo info = m_Host.Describe();
	auto result = std::make_unique<pb::HelloResult>();
	result->set_protocol_version( pb::PROTOCOL_VERSION_1 );
	result->set_framing( m_Identity.framing );
	result->set_transport( SanitizeUtf8( m_Identity.transport ) );
	result->set_product(
	    info.product == HostProduct::Dedicated ? pb::PRODUCT_DEDICATED : pb::PRODUCT_CLIENT );
	result->set_build_revision( SanitizeUtf8( info.buildRevision ) );
	result->set_host_state( ToProto( m_Host.Status().hostState ) );
	for ( const HostProviderInfo &provider : info.providers )
	{
		pb::Provider *out = result->add_providers();
		out->set_role( SanitizeUtf8( provider.role ) );
		out->set_id( SanitizeUtf8( provider.id ) );
	}
	result->set_game_dir( SanitizeUtf8( info.gameDir ) );
	return Succeed( std::move( result ) );
}

CallOutcome Dispatcher::Exec( const pb::ExecRequest &request )
{
	const std::string &command = request.command();
	if ( command.empty() || command.size() > kMaxCommandBytes )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "command must be 1..510 bytes" );
	if ( command.find_first_of( std::string_view( "\n\r\0", 3 ) ) != std::string::npos )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "command must be one line; chain with ';'" );

	ExecOutput output = m_Host.Exec( command, m_Limits.maxExecOutputBytes );
	auto result = std::make_unique<pb::ExecResult>();
	result->set_output( SanitizeUtf8( output.text ) );
	result->set_truncated( output.truncated );
	return Succeed( std::move( result ) );
}

CallOutcome Dispatcher::GetCvar( const pb::CvarGetRequest &request )
{
	if ( !ValidCvarName( request.name() ) )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "invalid cvar name" );
	HostResult<CvarInfo> info = m_Host.GetCvar( request.name() );
	if ( !info )
		return Fail( FromHostError( info.Error() ).code, FromHostError( info.Error() ).message );
	auto result = std::make_unique<pb::CvarResult>();
	FillCvar( info.Value(), *result );
	return Succeed( std::move( result ) );
}

CallOutcome Dispatcher::SetCvar( const pb::CvarSetRequest &request )
{
	if ( !ValidCvarName( request.name() ) )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "invalid cvar name" );
	const std::string &value = request.value();
	if ( value.size() > kMaxCvarBytes ||
	     value.find_first_of( std::string_view( "\"\n\r\0", 4 ) ) != std::string::npos )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS,
		    "cvar value must be at most 255 bytes without quotes or newlines" );
	HostResult<CvarInfo> info = m_Host.SetCvar( request.name(), value );
	if ( !info )
		return Fail( FromHostError( info.Error() ).code, FromHostError( info.Error() ).message );
	auto result = std::make_unique<pb::CvarResult>();
	FillCvar( info.Value(), *result );
	return Succeed( std::move( result ) );
}

CallOutcome Dispatcher::Status()
{
	auto result = std::make_unique<pb::StatusResult>();
	FillStatus( m_Host.Status(), *result );
	return Succeed( std::move( result ) );
}

std::optional<CallOutcome> Dispatcher::WaitFor( CallTag tag, const pb::WaitForRequest &request )
{
	if ( request.condition() == pb::WAIT_CONDITION_UNSPECIFIED )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "condition is required" );
	if ( request.timeout_ms() == 0 || request.timeout_ms() > kMaxWaitMs )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "timeoutMs must be 1..600000" );
	if ( request.condition() == pb::WAIT_CONDITION_CLIENT_ACTIVE &&
	     m_Host.Describe().product == HostProduct::Dedicated )
		return Fail( pb::ERROR_CODE_UNAVAILABLE_IN_PRODUCT, "a dedicated server has no client" );

	Deferred deferred;
	deferred.method = Method::WaitFor;
	deferred.started = m_Host.NowSeconds();
	deferred.timeoutSeconds = request.timeout_ms() / 1000.0;
	deferred.condition = request.condition();
	deferred.frames = request.frames();
	if ( std::optional<CallOutcome> done = Advance( deferred, false ) )
		return done;
	m_Deferred.emplace( tag, deferred );
	return std::nullopt;
}

std::optional<CallOutcome> Dispatcher::Screenshot(
    CallTag tag, const pb::ScreenshotRequest &request )
{
	ScreenshotSpec spec;
	switch ( request.mode() )
	{
	case pb::SCREENSHOT_MODE_PATH:
		spec.kind = ScreenshotKind::TgaPath;
		break;
	case pb::SCREENSHOT_MODE_INLINE_JPEG:
		spec.kind = ScreenshotKind::InlineJpeg;
		break;
	default:
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "mode is required" );
	}
	if ( request.jpeg_quality() > 100 )
		return Fail( pb::ERROR_CODE_INVALID_PARAMS, "jpegQuality must be 0..100" );
	spec.jpegQuality = request.jpeg_quality() == 0 ? 90 : static_cast<int>( request.jpeg_quality() );

	HostResult<ScreenshotTicket> ticket = m_Host.RequestScreenshot( spec );
	if ( !ticket )
		return Fail( FromHostError( ticket.Error() ).code, FromHostError( ticket.Error() ).message );

	Deferred deferred;
	deferred.method = Method::Screenshot;
	deferred.started = m_Host.NowSeconds();
	deferred.timeoutSeconds = m_Limits.screenshotTimeoutSeconds;
	deferred.ticket = ticket.Value();
	m_Deferred.emplace( tag, deferred );
	return std::nullopt;
}

CallOutcome Dispatcher::Quit()
{
	m_Host.RequestQuit();
	return Succeed( std::make_unique<pb::QuitResult>() );
}

bool Dispatcher::ConditionHolds( const Deferred &deferred, const HostStatus &status ) const
{
	switch ( deferred.condition )
	{
	case pb::WAIT_CONDITION_MAP_LOADED:
		return status.serverActive;
	case pb::WAIT_CONDITION_CLIENT_ACTIVE:
		return status.clientSignon == kSignonFull;
	case pb::WAIT_CONDITION_FRAMES:
		return deferred.framesWaited >= deferred.frames;
	default:
		return false;
	}
}

CallOutcome Dispatcher::WaitResult( const Deferred &deferred, const HostStatus &status ) const
{
	auto result = std::make_unique<pb::WaitForResult>();
	result->set_frames_waited( deferred.framesWaited );
	result->set_elapsed_ms( ElapsedMs( deferred.started, m_Host.NowSeconds() ) );
	FillStatus( status, *result->mutable_status() );
	return Succeed( std::move( result ) );
}

std::optional<CallOutcome> Dispatcher::Advance( Deferred &deferred, bool newFrame )
{
	if ( deferred.method == Method::WaitFor )
	{
		if ( newFrame )
			++deferred.framesWaited;
		const HostStatus status = m_Host.Status();
		if ( ConditionHolds( deferred, status ) )
			return WaitResult( deferred, status );
	}
	else
	{
		std::optional<HostResult<ScreenshotCapture>> capture =
		    m_Host.PollScreenshot( deferred.ticket );
		if ( capture )
		{
			if ( !*capture )
				return Fail(
				    FromHostError( capture->Error() ).code, FromHostError( capture->Error() ).message );
			const ScreenshotCapture &value = capture->Value();
			auto result = std::make_unique<pb::ScreenshotResult>();
			result->set_width( value.width );
			result->set_height( value.height );
			result->set_path( SanitizeUtf8( value.path ) );
			result->set_jpeg( value.jpeg );
			result->set_host_frame( value.hostFrame );
			return Succeed( std::move( result ) );
		}
	}

	if ( m_Host.NowSeconds() - deferred.started >= deferred.timeoutSeconds )
	{
		if ( deferred.method == Method::Screenshot )
			m_Host.CancelScreenshot( deferred.ticket );
		return Fail( pb::ERROR_CODE_TIMEOUT, deferred.method == Method::Screenshot
		                                         ? "no frame was captured before the timeout"
		                                         : "condition did not hold before the timeout" );
	}
	return std::nullopt;
}

void Dispatcher::Tick( std::vector<Completion> &completed )
{
	for ( auto it = m_Deferred.begin(); it != m_Deferred.end(); )
	{
		std::optional<CallOutcome> outcome = Advance( it->second, true );
		if ( !outcome )
		{
			++it;
			continue;
		}
		completed.push_back( Completion{ it->first, std::move( *outcome ) } );
		it = m_Deferred.erase( it );
	}
}

void Dispatcher::Cancel( CallTag tag )
{
	auto it = m_Deferred.find( tag );
	if ( it == m_Deferred.end() )
		return;
	if ( it->second.method == Method::Screenshot )
		m_Host.CancelScreenshot( it->second.ticket );
	m_Deferred.erase( it );
}

void Dispatcher::CancelAll( std::vector<Completion> &completed )
{
	for ( auto &[tag, deferred] : m_Deferred )
	{
		if ( deferred.method == Method::Screenshot )
			m_Host.CancelScreenshot( deferred.ticket );
		completed.push_back(
		    Completion{ tag, Fail( pb::ERROR_CODE_CANCELLED, "the debug API server is shutting down" ) } );
	}
	m_Deferred.clear();
}

} // namespace debugapi
