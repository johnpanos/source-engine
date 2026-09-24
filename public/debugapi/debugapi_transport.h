//========= Copyright Valve Corporation, All rights reserved. ============//
//
// debugapi.transport.v1: a byte-stream connection transport for the engine
// debug API (source.debug.v1). Transports know nothing about messages; a codec
// frames and encodes bytes above them. Providers are selected by the
// composition root from an address scheme ("unix:/path").
//
// Contract (checked by unittests/debugapitest/test_transport.cpp for every
// provider, including seeded bad ones):
//  - Single sequence: every method runs on the owner's sequence; callbacks run
//    only inside Poll(), never re-entrantly from Send/Close/Shutdown.
//  - Poll() never blocks. It accepts pending connections, delivers readable
//    bytes and flushes queued output.
//  - Connection ids are nonzero and never reused within a transport instance.
//  - Per connection: OnConnected, then any number of OnData in byte order with
//    no loss or duplication, then exactly one OnDisconnected. Bytes that arrived
//    before a peer close are delivered before its OnDisconnected.
//  - Send() takes a copy and preserves order; it returns false for an unknown,
//    closing or closed connection and never truncates a queued write.
//  - Close(Immediate) discards queued output; Close(AfterFlush) stops reading
//    and closes once queued output is written. Both deliver OnDisconnected on a
//    later Poll().
//  - Shutdown() is idempotent. It makes one non-blocking attempt to write
//    queued output, then releases every connection and the listening endpoint
//    without further callbacks; later Send() fails and Poll() is a no-op. The
//    destructor implies Shutdown().
//
//=============================================================================//

#ifndef PUBLIC_DEBUGAPI_DEBUGAPI_TRANSPORT_H
#define PUBLIC_DEBUGAPI_DEBUGAPI_TRANSPORT_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "foundation/expected.h"

namespace debugapi
{

struct ConnectionId
{
	uint32_t value = 0;

	friend bool operator==( ConnectionId, ConnectionId ) = default;
	friend auto operator<=>( ConnectionId, ConnectionId ) = default;
};

class ITransportSink
{
public:
	virtual void OnConnected( ConnectionId id ) = 0;
	virtual void OnData( ConnectionId id, std::string_view bytes ) = 0;
	virtual void OnDisconnected( ConnectionId id ) = 0;

protected:
	~ITransportSink() = default;
};

enum class CloseMode
{
	Immediate,
	AfterFlush,
};

class ITransport
{
public:
	virtual ~ITransport() = default;

	// Stable description for diagnostics and HelloResult, e.g. "unix:/run/x.sock".
	virtual std::string_view Describe() const = 0;
	virtual void Poll( ITransportSink &sink ) = 0;
	[[nodiscard]] virtual bool Send( ConnectionId id, std::string_view bytes ) = 0;
	// Bytes accepted by Send() and not yet written; 0 for unknown connections.
	virtual size_t PendingOutputBytes( ConnectionId id ) const = 0;
	virtual void Close( ConnectionId id, CloseMode mode ) = 0;
	virtual void Shutdown() = 0;
};

enum class TransportErrorCode
{
	InvalidAddress,
	AddressInUse,
	PermissionDenied,
	SystemError,
};

struct TransportError
{
	TransportErrorCode code;
	std::string message;
};

using TransportResult = foundation::Expected<std::unique_ptr<ITransport>, TransportError>;

// Linked, root-selected provider descriptor. The address excludes "scheme:".
struct TransportProvider
{
	const char *scheme;
	TransportResult ( *create )( std::string_view address );
};

} // namespace debugapi

#endif // PUBLIC_DEBUGAPI_DEBUGAPI_TRANSPORT_H
