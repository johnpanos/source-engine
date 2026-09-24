//========= Copyright Valve Corporation, All rights reserved. ============//
//
// POSIX stream-socket debugapi.transport.v1 provider ("unix:/path"). The
// socket is non-blocking and polled on the owner's sequence; no threads.
//
// Endpoint policy: the socket file is created with mode 0600. An existing path
// is replaced only when it is a stale socket (nothing accepts on it); a live
// socket or any other file type fails with AddressInUse/InvalidAddress. The
// file is unlinked at Shutdown only if it is still the one this instance made.
// Liveness is probed by connecting, so a refused second server appears to the
// live one as a connection that closes immediately.
//
//=============================================================================//

#ifndef PUBLIC_DEBUGAPI_DEBUGAPI_UNIX_SOCKET_H
#define PUBLIC_DEBUGAPI_DEBUGAPI_UNIX_SOCKET_H

#include "debugapi/debugapi_transport.h"

namespace debugapi
{

struct UnixSocketLimits
{
	size_t maxConnections = 16;
	// Per connection per Poll(), so one chatty peer cannot starve a frame.
	size_t maxReadBytesPerPoll = 1024 * 1024;
};

TransportResult CreateUnixSocketTransport( std::string_view path, UnixSocketLimits limits = {} );

const TransportProvider &UnixSocketTransportProvider();

} // namespace debugapi

#endif // PUBLIC_DEBUGAPI_DEBUGAPI_UNIX_SOCKET_H
