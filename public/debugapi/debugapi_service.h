//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Protobuf-free entry point for hosting the debug API server. Products link
// debugapi_core and include only this header; the JSON-RPC/protobuf
// implementation stays private to debugapi/core.
//
//=============================================================================//

#ifndef PUBLIC_DEBUGAPI_DEBUGAPI_SERVICE_H
#define PUBLIC_DEBUGAPI_DEBUGAPI_SERVICE_H

#include <memory>
#include <string>
#include <string_view>

#include "debugapi/debugapi_framing.h"
#include "debugapi/debugapi_host.h"
#include "debugapi/debugapi_transport.h"
#include "foundation/expected.h"

namespace debugapi
{

// Runs on the host's main sequence. The host must outlive the service; the
// destructor implies Shutdown() (deferred calls are answered with CANCELLED).
class IDebugApiService
{
public:
	virtual ~IDebugApiService() = default;
	virtual void Pump() = 0;
	virtual void Shutdown() = 0;
};

using ServiceResult = foundation::Expected<std::unique_ptr<IDebugApiService>, std::string>;

// Creates the transport from `address` and starts serving. Fails without side
// effects when the transport or framing cannot be used.
ServiceResult StartDebugApiService( const TransportProvider &transport, std::string_view address,
    const FramingProvider &framing, IDebugApiHost &host );

} // namespace debugapi

#endif // PUBLIC_DEBUGAPI_DEBUGAPI_SERVICE_H
