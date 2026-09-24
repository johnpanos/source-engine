//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a deterministic IDebugApiHost (see fake_host.h).
//
//===========================================================================//

#include "fake_host.h"

#include <cstdlib>

namespace debugapitest
{

using debugapi::HostError;
using debugapi::HostErrorCode;

FakeHost::FakeHost()
{
	info.product = debugapi::HostProduct::Client;
	info.buildRevision = "fixture-revision";
	info.gameDir = "/game/portal";
	info.providers = { { "render", "null" }, { "physics", "fixture" } };

	status.hostState = debugapi::HostStateKind::Run;
	status.map = "testchmb_a_00";
	status.serverActive = true;
	status.clientSignon = 6;
	status.playerCount = 1;
	status.maxPlayers = 1;
	status.hostFrame = 42;
	status.hostTime = 12.5;

	debugapi::CvarInfo fps;
	fps.name = "fps_max";
	fps.value = "300";
	fps.defaultValue = "300";
	fps.floatValue = 300.0;
	fps.intValue = 300;
	fps.flags = { "archive" };
	fps.help = "Frame rate limiter";
	cvars[fps.name] = fps;

	debugapi::CvarInfo cheat;
	cheat.name = "cheatvar";
	cheat.value = "0";
	cheat.defaultValue = "0";
	cheat.flags = { "cheat" };
	cvars[cheat.name] = cheat;
}

debugapi::ExecOutput FakeHost::Exec( std::string_view commandLine, size_t maxOutputBytes )
{
	std::string text;
	if ( commandLine.rfind( "echo ", 0 ) == 0 )
		text = std::string( commandLine.substr( 5 ) ) + "\n";
	else if ( commandLine.rfind( "log ", 0 ) == 0 )
	{
		if ( logSink )
			logSink->Append( std::string( commandLine.substr( 4 ) ) + "\n", false, 0 );
	}
	else if ( commandLine == "latin1" )
		text = "caf\xE9";
	else if ( commandLine == "spam" )
		text = std::string( 4096, 's' );
	else if ( commandLine == "pump" )
	{
		if ( reentry )
			reentry();
	}
	else
		text = "Unknown command: " + std::string( commandLine ) + "\n";

	debugapi::ExecOutput output;
	output.truncated = text.size() > maxOutputBytes;
	if ( output.truncated )
		text.resize( maxOutputBytes );
	output.text = std::move( text );
	return output;
}

debugapi::HostResult<debugapi::CvarInfo> FakeHost::GetCvar( std::string_view name )
{
	auto it = cvars.find( std::string( name ) );
	if ( it == cvars.end() )
		return foundation::MakeUnexpected(
		    HostError{ HostErrorCode::NotFound, "no cvar named " + std::string( name ) } );
	return it->second;
}

debugapi::HostResult<debugapi::CvarInfo> FakeHost::SetCvar(
    std::string_view name, std::string_view value )
{
	auto it = cvars.find( std::string( name ) );
	if ( it == cvars.end() )
		return foundation::MakeUnexpected(
		    HostError{ HostErrorCode::NotFound, "no cvar named " + std::string( name ) } );
	for ( const std::string &flag : it->second.flags )
	{
		if ( flag == "cheat" )
			return foundation::MakeUnexpected( HostError{ HostErrorCode::FailedPrecondition,
			    "cheat cvars require sv_cheats 1" } );
	}
	it->second.value = std::string( value );
	it->second.floatValue = std::atof( it->second.value.c_str() );
	it->second.intValue = std::atoi( it->second.value.c_str() );
	return it->second;
}

debugapi::HostResult<debugapi::ScreenshotTicket> FakeHost::RequestScreenshot(
    const debugapi::ScreenshotSpec &spec )
{
	if ( info.product == debugapi::HostProduct::Dedicated )
		return foundation::MakeUnexpected(
		    HostError{ HostErrorCode::Unavailable, "a dedicated server does not render" } );
	const debugapi::ScreenshotTicket ticket = m_NextTicket++;
	m_Captures[ticket] = Capture{ spec, 0 };
	return ticket;
}

std::optional<debugapi::HostResult<debugapi::ScreenshotCapture>> FakeHost::PollScreenshot(
    debugapi::ScreenshotTicket ticket )
{
	auto it = m_Captures.find( ticket );
	if ( it == m_Captures.end() )
		return debugapi::HostResult<debugapi::ScreenshotCapture>(
		    foundation::MakeUnexpected( HostError{ HostErrorCode::Internal, "unknown ticket" } ) );
	if ( ++it->second.polls < 2 )
		return std::nullopt;
	debugapi::ScreenshotCapture capture;
	capture.width = 640;
	capture.height = 480;
	capture.hostFrame = status.hostFrame;
	if ( it->second.spec.kind == debugapi::ScreenshotKind::InlineJpeg )
		capture.jpeg = std::string( "\xFF\xD8" "fixture" "\xFF\xD9", 11 );
	else
		capture.path = "/game/portal/screenshots/debugapi_0001.tga";
	m_Captures.erase( it );
	return debugapi::HostResult<debugapi::ScreenshotCapture>( capture );
}

void FakeHost::CancelScreenshot( debugapi::ScreenshotTicket ticket )
{
	cancelled.insert( ticket );
	m_Captures.erase( ticket );
}

} // namespace debugapitest
