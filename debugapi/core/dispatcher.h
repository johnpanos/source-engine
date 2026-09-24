//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Executes source.debug.v1 methods against an IDebugApiHost. Connection
// state (subscriptions, framing, ids) belongs to the server; the dispatcher
// only knows calls, identified by the server's CallTag. Deferred calls
// (WaitFor, Screenshot) advance once per server pump in Tick().
//
//=============================================================================//

#ifndef DEBUGAPI_CORE_DISPATCHER_H
#define DEBUGAPI_CORE_DISPATCHER_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "debugapi/debugapi_host.h"
#include "jsonrpc.h"

namespace debugapi
{

using CallTag = uint64_t;

// Exactly one of `result` and `error` is set.
struct CallOutcome
{
	std::unique_ptr<google::protobuf::Message> result;
	std::optional<RpcError> error;
};

struct Completion
{
	CallTag tag = 0;
	CallOutcome outcome;
};

struct ServerIdentity
{
	std::string framing;
	std::string transport;
};

struct DispatcherLimits
{
	size_t maxExecOutputBytes = 1024 * 1024;
	double screenshotTimeoutSeconds = 30.0;
};

class Dispatcher
{
public:
	Dispatcher( IDebugApiHost &host, ServerIdentity identity, DispatcherLimits limits = {} );
	~Dispatcher();

	Dispatcher( const Dispatcher & ) = delete;
	Dispatcher &operator=( const Dispatcher & ) = delete;

	// Returns the outcome, or nullopt when the call was deferred under `tag`.
	// Subscribe is connection state and is rejected here (the server owns it).
	std::optional<CallOutcome> Handle( CallTag tag, Method method, const google::protobuf::Message &params );
	// One server pump: advances every deferred call and appends completions.
	void Tick( std::vector<Completion> &completed );
	// Forgets a deferred call without an outcome (its connection is gone).
	void Cancel( CallTag tag );
	// Completes every deferred call with CANCELLED.
	void CancelAll( std::vector<Completion> &completed );
	size_t DeferredCount() const { return m_Deferred.size(); }

private:
	struct Deferred
	{
		Method method = Method::WaitFor;
		double started = 0.0;
		double timeoutSeconds = 0.0;
		pb::WaitCondition condition = pb::WAIT_CONDITION_UNSPECIFIED;
		uint32_t frames = 0;
		uint32_t framesWaited = 0;
		ScreenshotTicket ticket = 0;
	};

	CallOutcome Hello( const pb::HelloRequest &request );
	CallOutcome Exec( const pb::ExecRequest &request );
	CallOutcome GetCvar( const pb::CvarGetRequest &request );
	CallOutcome SetCvar( const pb::CvarSetRequest &request );
	CallOutcome Status();
	std::optional<CallOutcome> WaitFor( CallTag tag, const pb::WaitForRequest &request );
	std::optional<CallOutcome> Screenshot( CallTag tag, const pb::ScreenshotRequest &request );
	CallOutcome Quit();

	std::optional<CallOutcome> Advance( Deferred &deferred, bool newFrame );
	bool ConditionHolds( const Deferred &deferred, const HostStatus &status ) const;
	CallOutcome WaitResult( const Deferred &deferred, const HostStatus &status ) const;

	IDebugApiHost &m_Host;
	ServerIdentity m_Identity;
	DispatcherLimits m_Limits;
	std::map<CallTag, Deferred> m_Deferred;
};

// Converts host values into protocol messages (shared with tests).
void FillStatus( const HostStatus &status, pb::StatusResult &out );
RpcError FromHostError( const HostError &error );

} // namespace debugapi

#endif // DEBUGAPI_CORE_DISPATCHER_H
