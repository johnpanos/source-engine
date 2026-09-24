//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Root-selected debug API server for the engine (source.debug.v1, JSON-RPC
// 2.0). Present only in builds configured with --debug-api=enabled; store and
// mobile profiles never enable it. The composition root picks a transport and
// framing from the engine's linked catalog, names the providers it composed,
// and binds before the engine loads. The engine starts the server after its
// game systems initialize, pumps it once per engine frame on the main thread
// and stops it before they shut down.
//
//=============================================================================//

#ifndef ENGINE_DEBUGAPI_BINDING_H
#define ENGINE_DEBUGAPI_BINDING_H

#include <cstddef>

namespace debugapi
{
struct TransportProvider;
struct FramingProvider;
} // namespace debugapi

// A provider the root composed, reported by the Hello method.
struct DebugApiComposedProvider
{
	const char *role;
	const char *id;
};

struct DebugApiSelection
{
	const debugapi::TransportProvider *transport = nullptr;
	// Transport address without the scheme, e.g. "/run/user/1000/source.sock".
	const char *address = nullptr;
	const debugapi::FramingProvider *framing = nullptr;
	const DebugApiComposedProvider *providers = nullptr;
	size_t providerCount = 0;
};

extern "C" const debugapi::TransportProvider *DebugApi_UnixSocketTransport();
extern "C" const debugapi::FramingProvider *DebugApi_NewlineFraming();
extern "C" const debugapi::FramingProvider *DebugApi_ContentLengthFraming();

// Copies the selection (strings included). Fails without changing anything
// when it is incomplete, already bound, or the engine has started the server.
// A bound server that cannot start fails engine Load.
extern "C" bool Engine_BindDebugApi( const DebugApiSelection *selection );

#endif // ENGINE_DEBUGAPI_BINDING_H
