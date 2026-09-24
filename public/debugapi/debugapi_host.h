//========= Copyright Valve Corporation, All rights reserved. ============//
//
// debugapi.host.v1: what the debug API server needs from the product that
// hosts it. The engine implements it (engine/debugapi_host.cpp); tests use a
// fake. Plain C++ types only: no protobuf and no Source headers cross here.
//
// Contract:
//  - Every method except ILogSink::Append runs on the host's main sequence,
//    from the server's Pump(); the host never calls the server re-entrantly.
//  - The server may be created before the host's game systems initialize; it
//    queries the host only from Pump(), and SetLogSink() at any time.
//  - Exec() runs the command to completion before returning.
//  - Screenshot tickets complete, fail or are cancelled; PollScreenshot()
//    returns nullopt while pending and the final outcome exactly once.
//  - SetLogSink(nullptr) (or replacing the sink) guarantees the previous sink
//    receives no Append() once it returns, from any thread.
//
//=============================================================================//

#ifndef PUBLIC_DEBUGAPI_DEBUGAPI_HOST_H
#define PUBLIC_DEBUGAPI_DEBUGAPI_HOST_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "foundation/expected.h"

namespace debugapi
{

enum class HostProduct
{
	Client,
	Dedicated,
};

enum class HostStateKind
{
	Unknown,
	NewGame,
	LoadGame,
	ChangeLevelSinglePlayer,
	ChangeLevelMultiPlayer,
	Run,
	GameShutdown,
	Shutdown,
	Restart,
};

struct HostProviderInfo
{
	std::string role;
	std::string id;
};

struct HostInfo
{
	HostProduct product = HostProduct::Client;
	std::string buildRevision;
	std::string gameDir;
	std::vector<HostProviderInfo> providers;
};

struct HostStatus
{
	HostStateKind hostState = HostStateKind::Unknown;
	std::string map;
	bool serverActive = false;
	// SIGNONSTATE_* value of the local client; 0 when there is none.
	int clientSignon = 0;
	uint32_t playerCount = 0;
	uint32_t maxPlayers = 0;
	uint32_t hostFrame = 0;
	double hostTime = 0.0;
};

enum class HostErrorCode
{
	InvalidArgument,
	NotFound,
	FailedPrecondition,
	Unavailable,
	Internal,
};

struct HostError
{
	HostErrorCode code;
	std::string message;
};

template <typename T> using HostResult = foundation::Expected<T, HostError>;

struct ExecOutput
{
	std::string text;
	bool truncated = false;
};

struct CvarInfo
{
	std::string name;
	std::string value;
	std::string defaultValue;
	double floatValue = 0.0;
	int intValue = 0;
	std::vector<std::string> flags;
	std::string help;
};

enum class ScreenshotKind
{
	TgaPath,
	InlineJpeg,
};

struct ScreenshotSpec
{
	ScreenshotKind kind = ScreenshotKind::TgaPath;
	int jpegQuality = 90;
};

struct ScreenshotCapture
{
	uint32_t width = 0;
	uint32_t height = 0;
	std::string path;
	std::string jpeg;
	uint32_t hostFrame = 0;
};

using ScreenshotTicket = uint64_t;

// May be called from any thread, concurrently; must not block for long.
class ILogSink
{
public:
	virtual void Append( std::string_view text, bool developer, uint32_t rgba ) noexcept = 0;

protected:
	~ILogSink() = default;
};

class IDebugApiHost
{
public:
	virtual HostInfo Describe() = 0;
	virtual HostStatus Status() = 0;
	// Monotonic seconds.
	virtual double NowSeconds() = 0;
	virtual ExecOutput Exec( std::string_view commandLine, size_t maxOutputBytes ) = 0;
	virtual HostResult<CvarInfo> GetCvar( std::string_view name ) = 0;
	virtual HostResult<CvarInfo> SetCvar( std::string_view name, std::string_view value ) = 0;
	// Unavailable on products that do not render.
	virtual HostResult<ScreenshotTicket> RequestScreenshot( const ScreenshotSpec &spec ) = 0;
	virtual std::optional<HostResult<ScreenshotCapture>> PollScreenshot( ScreenshotTicket ticket ) = 0;
	virtual void CancelScreenshot( ScreenshotTicket ticket ) = 0;
	virtual void RequestQuit() = 0;
	virtual void SetLogSink( ILogSink *sink ) = 0;

protected:
	~IDebugApiHost() = default;
};

} // namespace debugapi

#endif // PUBLIC_DEBUGAPI_DEBUGAPI_HOST_H
