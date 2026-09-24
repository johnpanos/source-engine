//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the shared debugapi.transport.v1 suite. Every transport provider,
//          including deliberately bad ones, runs through the same cases.
//
//===========================================================================//

#ifndef DEBUGAPITEST_TRANSPORT_CONFORMANCE_H
#define DEBUGAPITEST_TRANSPORT_CONFORMANCE_H

#include <memory>
#include <string>
#include <string_view>

#include "checks.h"
#include "debugapi/debugapi_transport.h"

namespace debugapitest
{

// The client end of one connection, driven by the test.
class ITestPeer
{
public:
	virtual ~ITestPeer() = default;
	// Queues bytes for the server (a peer may buffer what the OS cannot take
	// yet and flush it on later calls); false once the connection is closed.
	virtual bool Write( std::string_view bytes ) = 0;
	// Everything received so far (non-blocking); also flushes queued writes.
	virtual std::string Read() = 0;
	// True once the server side closed and every byte it sent was read.
	virtual bool SawClose() = 0;
	virtual void Close() = 0;
};

class ITransportFixture
{
public:
	virtual ~ITransportFixture() = default;
	virtual const char *Name() const = 0;
	virtual std::unique_ptr<debugapi::ITransport> Create() = 0;
	virtual std::unique_ptr<ITestPeer> Connect( debugapi::ITransport &transport ) = 0;
};

void RunTransportConformance( ITransportFixture &fixture, Results &results );

} // namespace debugapitest

#endif // DEBUGAPITEST_TRANSPORT_CONFORMANCE_H
