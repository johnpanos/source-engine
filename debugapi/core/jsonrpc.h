//========= Copyright Valve Corporation, All rights reserved. ============//
//
// JSON-RPC 2.0 encoding of source.debug.v1. Method names, parameter/result
// types and error codes all come from the generated descriptors of
// debug_api.proto; nothing here restates the schema except the Method enum,
// which SchemaMatchesMethods() verifies against the service.
//
//=============================================================================//

#ifndef DEBUGAPI_CORE_JSONRPC_H
#define DEBUGAPI_CORE_JSONRPC_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "source/debug/v1/debug_api.pb.h"

namespace debugapi
{

namespace pb = ::source::debug::v1;

// Methods of `service DebugApi`, in declaration order.
enum class Method
{
	Hello,
	Exec,
	CvarGet,
	CvarSet,
	Status,
	WaitFor,
	Screenshot,
	Subscribe,
	Quit,
};

inline constexpr size_t kMethodCount = 9;

struct RpcError
{
	pb::ErrorCode code = pb::ERROR_CODE_INTERNAL_ERROR;
	std::string message;
};

// A JSON-RPC id as canonical JSON text ("7", "\"a\"", "null"); empty when the
// request is a notification (no id member).
struct RequestId
{
	std::string json;

	bool IsNotification() const { return json.empty(); }
};

struct DecodedCall
{
	RequestId id;
	Method method = Method::Hello;
	// Set exactly when `error` is empty.
	std::unique_ptr<google::protobuf::Message> params;
	std::optional<RpcError> error;
};

struct DecodedFrame
{
	bool batch = false;
	std::vector<DecodedCall> calls;
	// The frame as a whole is invalid: answer once with a null id.
	std::optional<RpcError> error;
};

struct JsonRpcLimits
{
	size_t maxBatch = 64;
};

// True when `service DebugApi` declares exactly the Method enum, in order.
bool SchemaMatchesMethods();

DecodedFrame DecodeFrame( std::string_view frame, const JsonRpcLimits &limits );

std::string EncodeResult( const RequestId &id, const google::protobuf::Message &result );
// A notification id is encoded as null.
std::string EncodeError( const RequestId &id, const RpcError &error );
std::string EncodeLogNotification( const pb::LogNotification &params );
std::string EncodeBatch( const std::vector<std::string> &members );

// The JSON-RPC method name, e.g. "waitFor".
std::string MethodName( Method method );
const google::protobuf::Descriptor *ResultType( Method method );

// Replaces invalid UTF-8 with U+FFFD; proto3 strings must be valid UTF-8 and
// engine text is not guaranteed to be.
std::string SanitizeUtf8( std::string_view text );

} // namespace debugapi

#endif // DEBUGAPI_CORE_JSONRPC_H
