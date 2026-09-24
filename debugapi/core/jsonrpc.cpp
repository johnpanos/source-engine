//========= Copyright Valve Corporation, All rights reserved. ============//
//
// JSON-RPC 2.0 encoding of source.debug.v1.
//
//=============================================================================//

#include "jsonrpc.h"

#include <array>
#include <cmath>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/json_util.h>

namespace debugapi
{
namespace
{

namespace gp = ::google::protobuf;

constexpr std::array<std::string_view, kMethodCount> kRpcNames = {
    "Hello", "Exec", "CvarGet", "CvarSet", "Status", "WaitFor", "Screenshot", "Subscribe", "Quit" };

constexpr double kMaxExactInteger = 9007199254740992.0; // 2^53

const gp::ServiceDescriptor *ApiService()
{
	return pb::HelloRequest::descriptor()->file()->FindServiceByName( "DebugApi" );
}

const gp::ServiceDescriptor *NotificationService()
{
	return pb::HelloRequest::descriptor()->file()->FindServiceByName( "DebugApiNotifications" );
}

std::string LowerCamel( std::string_view rpcName )
{
	std::string name( rpcName );
	if ( !name.empty() && name[0] >= 'A' && name[0] <= 'Z' )
		name[0] = char( name[0] - 'A' + 'a' );
	return name;
}

gp::util::JsonPrintOptions PrintOptions()
{
	gp::util::JsonPrintOptions options;
	options.always_print_fields_with_no_presence = true;
	return options;
}

gp::util::JsonParseOptions StrictParse()
{
	gp::util::JsonParseOptions options;
	options.ignore_unknown_fields = false;
	return options;
}

std::string JsonString( std::string_view text )
{
	gp::Value value;
	value.set_string_value( SanitizeUtf8( text ) );
	std::string out;
	if ( !gp::util::MessageToJsonString( value, &out ).ok() )
		return "\"\"";
	return out;
}

RpcError Error( pb::ErrorCode code, std::string message )
{
	return RpcError{ code, std::move( message ) };
}

// Resolves an id member; false when the id has an invalid type or value.
bool DecodeId( const gp::Value &value, RequestId &id )
{
	switch ( value.kind_case() )
	{
	case gp::Value::kNullValue:
		id.json = "null";
		return true;
	case gp::Value::kStringValue:
		id.json = JsonString( value.string_value() );
		return true;
	case gp::Value::kNumberValue:
	{
		const double number = value.number_value();
		if ( !std::isfinite( number ) || std::floor( number ) != number ||
		     std::fabs( number ) > kMaxExactInteger )
			return false;
		id.json = std::to_string( static_cast<long long>( number ) );
		return true;
	}
	default:
		return false;
	}
}

DecodedCall DecodeCall( const gp::Value &value )
{
	DecodedCall call;
	auto invalid = [&call]( std::string message ) {
		// An invalid request object is answered even without an id.
		if ( call.id.IsNotification() )
			call.id.json = "null";
		call.error = Error( pb::ERROR_CODE_INVALID_REQUEST, std::move( message ) );
		return std::move( call );
	};

	if ( value.kind_case() != gp::Value::kStructValue )
		return invalid( "request must be a JSON object" );
	const auto &fields = value.struct_value().fields();

	if ( auto it = fields.find( "id" ); it != fields.end() && !DecodeId( it->second, call.id ) )
	{
		call.id.json = "null";
		return invalid( "id must be a string, an integer or null" );
	}
	for ( const auto &[key, member] : fields )
	{
		if ( key != "jsonrpc" && key != "id" && key != "method" && key != "params" )
			return invalid( "unexpected member \"" + std::string( key ) + "\"" );
	}
	auto version = fields.find( "jsonrpc" );
	if ( version == fields.end() || version->second.kind_case() != gp::Value::kStringValue ||
	     version->second.string_value() != "2.0" )
		return invalid( "jsonrpc must be \"2.0\"" );
	auto method = fields.find( "method" );
	if ( method == fields.end() || method->second.kind_case() != gp::Value::kStringValue )
		return invalid( "method must be a string" );

	const gp::ServiceDescriptor *service = ApiService();
	const gp::MethodDescriptor *descriptor = nullptr;
	for ( size_t i = 0; i < kMethodCount; ++i )
	{
		if ( LowerCamel( kRpcNames[i] ) == method->second.string_value() )
		{
			call.method = static_cast<Method>( i );
			descriptor = service->FindMethodByName( kRpcNames[i] );
		}
	}
	if ( descriptor == nullptr )
	{
		call.error = Error( pb::ERROR_CODE_METHOD_NOT_FOUND,
		    "unknown method \"" + SanitizeUtf8( method->second.string_value() ) + "\"" );
		return call;
	}

	const gp::Message *prototype =
	    gp::MessageFactory::generated_factory()->GetPrototype( descriptor->input_type() );
	std::unique_ptr<gp::Message> params( prototype->New() );
	if ( auto it = fields.find( "params" ); it != fields.end() )
	{
		if ( it->second.kind_case() != gp::Value::kStructValue )
		{
			call.error = Error( pb::ERROR_CODE_INVALID_PARAMS, "params must be an object" );
			return call;
		}
		std::string json;
		const auto printed = gp::util::MessageToJsonString( it->second.struct_value(), &json );
		const auto parsed = printed.ok()
		                        ? gp::util::JsonStringToMessage( json, params.get(), StrictParse() )
		                        : printed;
		if ( !parsed.ok() )
		{
			call.error = Error( pb::ERROR_CODE_INVALID_PARAMS,
			    SanitizeUtf8( std::string_view( parsed.message().data(), parsed.message().size() ) ) );
			return call;
		}
	}
	call.params = std::move( params );
	return call;
}

std::string_view ErrorCodeName( pb::ErrorCode code )
{
	return pb::ErrorCode_Name( code );
}

} // namespace

bool SchemaMatchesMethods()
{
	const gp::ServiceDescriptor *service = ApiService();
	const gp::ServiceDescriptor *notifications = NotificationService();
	if ( service == nullptr || notifications == nullptr ||
	     static_cast<size_t>( service->method_count() ) != kMethodCount ||
	     notifications->FindMethodByName( "Log" ) == nullptr ||
	     notifications->FindMethodByName( "Log" )->input_type() !=
	         pb::LogNotification::descriptor() )
		return false;
	for ( size_t i = 0; i < kMethodCount; ++i )
	{
		if ( service->method( static_cast<int>( i ) )->name() != kRpcNames[i] )
			return false;
	}
	return true;
}

DecodedFrame DecodeFrame( std::string_view frame, const JsonRpcLimits &limits )
{
	DecodedFrame decoded;
	gp::Value root;
	const auto status = gp::util::JsonStringToMessage( frame, &root, StrictParse() );
	if ( !status.ok() )
	{
		decoded.error = Error( pb::ERROR_CODE_PARSE_ERROR,
		    SanitizeUtf8( std::string_view( status.message().data(), status.message().size() ) ) );
		return decoded;
	}
	if ( root.kind_case() != gp::Value::kListValue )
	{
		decoded.calls.push_back( DecodeCall( root ) );
		return decoded;
	}
	decoded.batch = true;
	const auto &items = root.list_value().values();
	if ( items.empty() )
		decoded.error = Error( pb::ERROR_CODE_INVALID_REQUEST, "empty batch" );
	else if ( static_cast<size_t>( items.size() ) > limits.maxBatch )
		decoded.error = Error( pb::ERROR_CODE_RESOURCE_EXHAUSTED,
		    "batch exceeds " + std::to_string( limits.maxBatch ) + " requests" );
	else
	{
		for ( const gp::Value &item : items )
			decoded.calls.push_back( DecodeCall( item ) );
	}
	return decoded;
}

std::string EncodeResult( const RequestId &id, const gp::Message &result )
{
	std::string body;
	if ( !gp::util::MessageToJsonString( result, &body, PrintOptions() ).ok() )
		return EncodeError( id, Error( pb::ERROR_CODE_INTERNAL_ERROR, "result encoding failed" ) );
	std::string out = "{\"jsonrpc\":\"2.0\",\"id\":";
	out += id.IsNotification() ? "null" : id.json;
	out += ",\"result\":";
	out += body;
	out += '}';
	return out;
}

std::string EncodeError( const RequestId &id, const RpcError &error )
{
	std::string out = "{\"jsonrpc\":\"2.0\",\"id\":";
	out += id.IsNotification() ? "null" : id.json;
	out += ",\"error\":{\"code\":";
	out += std::to_string( static_cast<int>( error.code ) );
	out += ",\"message\":";
	out += JsonString( error.message );
	out += ",\"data\":{\"errorCode\":\"";
	out += ErrorCodeName( error.code );
	out += "\"}}}";
	return out;
}

std::string EncodeLogNotification( const pb::LogNotification &params )
{
	std::string body;
	if ( !gp::util::MessageToJsonString( params, &body, PrintOptions() ).ok() )
		return std::string();
	return "{\"jsonrpc\":\"2.0\",\"method\":\"log\",\"params\":" + body + "}";
}

std::string EncodeBatch( const std::vector<std::string> &members )
{
	std::string out = "[";
	for ( size_t i = 0; i < members.size(); ++i )
	{
		if ( i != 0 )
			out += ',';
		out += members[i];
	}
	out += ']';
	return out;
}

std::string MethodName( Method method )
{
	return LowerCamel( kRpcNames[static_cast<size_t>( method )] );
}

const gp::Descriptor *ResultType( Method method )
{
	return ApiService()->method( static_cast<int>( method ) )->output_type();
}

std::string SanitizeUtf8( std::string_view text )
{
	std::string out;
	out.reserve( text.size() );
	const auto *bytes = reinterpret_cast<const unsigned char *>( text.data() );
	const size_t size = text.size();
	size_t i = 0;
	while ( i < size )
	{
		const unsigned char lead = bytes[i];
		size_t length = 0;
		uint32_t codepoint = 0;
		if ( lead < 0x80 )
		{
			length = 1;
			codepoint = lead;
		}
		else if ( lead >= 0xC2 && lead <= 0xDF )
		{
			length = 2;
			codepoint = lead & 0x1F;
		}
		else if ( lead >= 0xE0 && lead <= 0xEF )
		{
			length = 3;
			codepoint = lead & 0x0F;
		}
		else if ( lead >= 0xF0 && lead <= 0xF4 )
		{
			length = 4;
			codepoint = lead & 0x07;
		}
		bool valid = length != 0 && i + length <= size;
		for ( size_t k = 1; valid && k < length; ++k )
		{
			if ( ( bytes[i + k] & 0xC0 ) != 0x80 )
				valid = false;
			else
				codepoint = ( codepoint << 6 ) | ( bytes[i + k] & 0x3F );
		}
		if ( valid && ( ( length == 3 && ( codepoint < 0x800 ||
		                                     ( codepoint >= 0xD800 && codepoint <= 0xDFFF ) ) ) ||
		                  ( length == 4 && ( codepoint < 0x10000 || codepoint > 0x10FFFF ) ) ) )
			valid = false;
		if ( !valid )
		{
			out += "\xEF\xBF\xBD";
			++i;
			continue;
		}
		out.append( text.substr( i, length ) );
		i += length;
	}
	return out;
}

} // namespace debugapi
