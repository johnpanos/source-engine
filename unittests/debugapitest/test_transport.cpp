//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: debugapi.transport.v1 — the shared transport suite against the
//          memory and Unix socket providers. Sensitivity builds instead run it
//          against one deliberately bad transport and pass only if the suite
//          detects the defect:
//            DEBUGAPI_SEED_TRANSPORT_TRUNCATE          Send drops a byte
//            DEBUGAPI_SEED_TRANSPORT_DOUBLE_DISCONNECT OnDisconnected twice
//            DEBUGAPI_SEED_TRANSPORT_REENTRANT_CLOSE   Close calls back inline
//
//===========================================================================//

#include "conformance/transport_conformance.h"
#include "conformance/transport_fixtures.h"
#include "debugapi/core/memory_transport.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

using namespace debugapitest;

#if defined( DEBUGAPI_SEED_TRANSPORT_TRUNCATE ) ||                                                 \
    defined( DEBUGAPI_SEED_TRANSPORT_DOUBLE_DISCONNECT ) ||                                        \
    defined( DEBUGAPI_SEED_TRANSPORT_REENTRANT_CLOSE )
#define DEBUGAPI_SEEDED 1

// Wraps the correct memory transport and breaks exactly one obligation.
class BadTransport final : public debugapi::ITransport, private debugapi::ITransportSink
{
public:
	debugapi::MemoryTransport inner;

	std::string_view Describe() const override { return inner.Describe(); }

	void Poll( debugapi::ITransportSink &sink ) override
	{
		m_Sink = &sink;
		inner.Poll( *this );
	}

	bool Send( debugapi::ConnectionId id, std::string_view bytes ) override
	{
#ifdef DEBUGAPI_SEED_TRANSPORT_TRUNCATE
		if ( bytes.size() > 1 )
			bytes.remove_suffix( 1 );
#endif
		return inner.Send( id, bytes );
	}

	size_t PendingOutputBytes( debugapi::ConnectionId id ) const override
	{
		return inner.PendingOutputBytes( id );
	}

	void Close( debugapi::ConnectionId id, debugapi::CloseMode mode ) override
	{
		inner.Close( id, mode );
#ifdef DEBUGAPI_SEED_TRANSPORT_REENTRANT_CLOSE
		if ( m_Sink )
			m_Sink->OnDisconnected( id );
#endif
	}

	void Shutdown() override { inner.Shutdown(); }

private:
	void OnConnected( debugapi::ConnectionId id ) override { m_Sink->OnConnected( id ); }
	void OnData( debugapi::ConnectionId id, std::string_view bytes ) override
	{
		m_Sink->OnData( id, bytes );
	}
	void OnDisconnected( debugapi::ConnectionId id ) override
	{
		m_Sink->OnDisconnected( id );
#ifdef DEBUGAPI_SEED_TRANSPORT_DOUBLE_DISCONNECT
		m_Sink->OnDisconnected( id );
#endif
	}

	debugapi::ITransportSink *m_Sink = nullptr;
};

class BadFixture final : public ITransportFixture
{
public:
	const char *Name() const override { return "seeded-bad"; }
	std::unique_ptr<debugapi::ITransport> Create() override
	{
		return std::make_unique<BadTransport>();
	}
	std::unique_ptr<ITestPeer> Connect( debugapi::ITransport &transport ) override
	{
		return m_Memory.Connect( static_cast<BadTransport &>( transport ).inner );
	}

private:
	MemoryFixture m_Memory;
};
#endif

} // namespace

int main()
{
	Results results;
#ifdef DEBUGAPI_SEEDED
	BadFixture bad;
	RunTransportConformance( bad, results );
	std::printf( "seeded transport: %lu of %lu checks failed\n", results.failures, results.checks );
	return testing::ReportConformance( 1, results.failures > 0 ? 0 : 1 );
#else
	MemoryFixture memory;
	RunTransportConformance( memory, results );

	UnixSocketFixture unixSocket;
	results.scope = "unix-socket";
	results.Check( !unixSocket.Directory().empty(), "a private socket directory was created" );
	if ( !unixSocket.Directory().empty() )
		RunTransportConformance( unixSocket, results );
	return testing::ReportConformance( results.checks, results.failures );
#endif
}
