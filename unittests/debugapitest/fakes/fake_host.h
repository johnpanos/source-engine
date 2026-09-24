//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a deterministic IDebugApiHost for the server suites and the shared
//          JSON-RPC fixtures (debugapi/fixtures/jsonrpc_cases.json). Every
//          value the fixtures depend on is documented here.
//
//          Exec: "echo <text>" -> "<text>\n"; "log <text>" appends "<text>\n"
//          to the log sink and prints nothing; "latin1" -> "caf\xE9" (invalid
//          UTF-8); "spam" -> 4096 bytes of 's'; "pump" calls the reentry hook;
//          anything else -> "Unknown command: <cmd>\n".
//          Cvars: fps_max (300, default 300), cheatvar (flag "cheat"; setting
//          it fails with FailedPrecondition).
//          Screenshots complete on the second poll after the request.
//          Time is virtual: `now` only changes when the test advances it.
//
//===========================================================================//

#ifndef DEBUGAPITEST_FAKE_HOST_H
#define DEBUGAPITEST_FAKE_HOST_H

#include <functional>
#include <map>
#include <set>
#include <string>

#include "debugapi/debugapi_host.h"

namespace debugapitest
{

class FakeHost final : public debugapi::IDebugApiHost
{
public:
	FakeHost();

	debugapi::HostInfo Describe() override { return info; }
	debugapi::HostStatus Status() override { return status; }
	double NowSeconds() override { return now; }
	debugapi::ExecOutput Exec( std::string_view commandLine, size_t maxOutputBytes ) override;
	debugapi::HostResult<debugapi::CvarInfo> GetCvar( std::string_view name ) override;
	debugapi::HostResult<debugapi::CvarInfo> SetCvar(
	    std::string_view name, std::string_view value ) override;
	debugapi::HostResult<debugapi::ScreenshotTicket> RequestScreenshot(
	    const debugapi::ScreenshotSpec &spec ) override;
	std::optional<debugapi::HostResult<debugapi::ScreenshotCapture>> PollScreenshot(
	    debugapi::ScreenshotTicket ticket ) override;
	void CancelScreenshot( debugapi::ScreenshotTicket ticket ) override;
	void RequestQuit() override { quitRequested = true; }
	void SetLogSink( debugapi::ILogSink *sink ) override
	{
		logSink = sink;
		++logSinkChanges;
	}

	debugapi::HostInfo info;
	debugapi::HostStatus status;
	double now = 100.0;
	bool quitRequested = false;
	debugapi::ILogSink *logSink = nullptr;
	int logSinkChanges = 0;
	std::map<std::string, debugapi::CvarInfo> cvars;
	std::set<debugapi::ScreenshotTicket> cancelled;
	std::function<void()> reentry;

private:
	struct Capture
	{
		debugapi::ScreenshotSpec spec;
		int polls = 0;
	};

	std::map<debugapi::ScreenshotTicket, Capture> m_Captures;
	debugapi::ScreenshotTicket m_NextTicket = 1;
};

} // namespace debugapitest

#endif // DEBUGAPITEST_FAKE_HOST_H
