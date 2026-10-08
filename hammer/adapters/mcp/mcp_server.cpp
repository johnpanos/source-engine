//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: MCP server over the Hammer command layer (hammer.adapters.mcp).
//
//=============================================================================//

#include "hammer/adapters/mcp/mcp_server.h"

#include "foundation/json.h"

#include <algorithm>
#include <array>

namespace hammer::adapters::mcp
{

using JsonValue = foundation::json::Value;
using JsonError = foundation::json::ParseError;
constexpr auto ParseJson = &foundation::json::Parse;

namespace
{

constexpr int kParseError = -32700;
constexpr int kInvalidRequest = -32600;
constexpr int kMethodNotFound = -32601;
constexpr int kInvalidParams = -32602;

// Versions whose tool semantics this server implements; the newest is offered
// when a client asks for another.
constexpr std::array<const char *, 3> kSupportedVersions = {
    kProtocolVersion, "2025-03-26", "2024-11-05" };

std::string Respond( const JsonValue &id, JsonValue result )
{
	JsonValue message = JsonValue::Object();
	message.Set( "jsonrpc", JsonValue::String( "2.0" ) );
	message.Set( "id", id );
	message.Set( "result", std::move( result ) );
	return message.Write();
}

std::string RespondError( const JsonValue &id, int code, std::string text )
{
	JsonValue error = JsonValue::Object();
	error.Set( "code", JsonValue::Number( code ) );
	error.Set( "message", JsonValue::String( std::move( text ) ) );
	JsonValue message = JsonValue::Object();
	message.Set( "jsonrpc", JsonValue::String( "2.0" ) );
	message.Set( "id", id );
	message.Set( "error", std::move( error ) );
	return message.Write();
}

JsonValue TextResult( std::string text, bool isError )
{
	JsonValue item = JsonValue::Object();
	item.Set( "type", JsonValue::String( "text" ) );
	item.Set( "text", JsonValue::String( std::move( text ) ) );
	JsonValue content = JsonValue::Array();
	content.Push( std::move( item ) );
	JsonValue result = JsonValue::Object();
	result.Set( "content", std::move( content ) );
	result.Set( "isError", JsonValue::Bool( isError ) );
	return result;
}

JsonValue Initialize( const JsonValue *params )
{
	std::string version = kProtocolVersion;
	if ( const JsonValue *requested = params ? params->Find( "protocolVersion" ) : nullptr )
	{
		for ( const char *supported : kSupportedVersions )
		{
			if ( requested->GetKind() == JsonValue::Kind::String && requested->Text() == supported )
				version = supported;
		}
	}
	JsonValue tools = JsonValue::Object();
	tools.Set( "listChanged", JsonValue::Bool( false ) );
	JsonValue capabilities = JsonValue::Object();
	capabilities.Set( "tools", std::move( tools ) );
	JsonValue info = JsonValue::Object();
	info.Set( "name", JsonValue::String( "hammer" ) );
	info.Set( "title", JsonValue::String( "Hammer map editor" ) );
	info.Set( "version", JsonValue::String( "0.1.0" ) );
	JsonValue result = JsonValue::Object();
	result.Set( "protocolVersion", JsonValue::String( version ) );
	result.Set( "capabilities", std::move( capabilities ) );
	result.Set( "serverInfo", std::move( info ) );
	result.Set( "instructions",
	    JsonValue::String( "Edits one Source map document. Start with new_map or open; build "
	                       "rooms with create_block and hollow; place entities with "
	                       "place_entity or place_on_surface; save or build_map to write it. "
	                       "Coordinates are Hammer units, z up; vectors are \"x y z\". Every "
	                       "edit is undoable with undo." ) );
	return result;
}

JsonValue ToolList( const std::vector<app::CommandInfo> &catalog )
{
	JsonValue tools = JsonValue::Array();
	for ( const app::CommandInfo &info : catalog )
	{
		JsonValue properties = JsonValue::Object();
		JsonValue required = JsonValue::Array();
		auto add = [&]( const std::string &arg, bool isRequired )
		{
			JsonValue property = JsonValue::Object();
			property.Set( "type", JsonValue::String( "string" ) );
			properties.Set( arg, std::move( property ) );
			if ( isRequired )
				required.Push( JsonValue::String( arg ) );
		};
		for ( const std::string &arg : info.required )
			add( arg, true );
		for ( const std::string &arg : info.optional )
			add( arg, false );
		JsonValue schema = JsonValue::Object();
		schema.Set( "type", JsonValue::String( "object" ) );
		schema.Set( "properties", std::move( properties ) );
		schema.Set( "required", std::move( required ) );
		schema.Set( "additionalProperties", JsonValue::Bool( false ) );
		JsonValue tool = JsonValue::Object();
		tool.Set( "name", JsonValue::String( info.name ) );
		tool.Set( "description", JsonValue::String( info.summary ) );
		tool.Set( "inputSchema", std::move( schema ) );
		tools.Push( std::move( tool ) );
	}
	JsonValue result = JsonValue::Object();
	result.Set( "tools", std::move( tools ) );
	return result;
}

// Command arguments are strings. Clients may also send a number, a boolean
// (1/0) or an array of scalars (a vector, "x y z"); anything else is rejected.
bool ArgumentText( const JsonValue &value, std::string &out )
{
	switch ( value.GetKind() )
	{
	case JsonValue::Kind::String:
	case JsonValue::Kind::Number:
		out = value.Text();
		return true;
	case JsonValue::Kind::Bool:
		out = value.AsBool() ? "1" : "0";
		return true;
	case JsonValue::Kind::Array:
		out.clear();
		for ( const JsonValue &item : value.Items() )
		{
			if ( item.GetKind() != JsonValue::Kind::Number &&
			     item.GetKind() != JsonValue::Kind::String )
				return false;
			if ( !out.empty() )
				out += ' ';
			out += item.Text();
		}
		return !value.Items().empty();
	default:
		return false;
	}
}

} // namespace

McpServer::McpServer( const std::vector<app::CommandInfo> &catalog, Execute execute )
    : m_catalog( catalog ), m_execute( std::move( execute ) )
{
}

std::optional<std::string> McpServer::HandleLine( std::string_view line )
{
	const JsonValue noId;
	auto parsed = ParseJson( line );
	if ( !parsed )
	{
		return RespondError( noId, kParseError,
		    "parse error at byte " + std::to_string( parsed.Error().offset ) + ": " +
		        parsed.Error().detail );
	}
	const JsonValue &message = parsed.Value();
	if ( message.GetKind() != JsonValue::Kind::Object )
		return RespondError( noId, kInvalidRequest, "a message is one JSON object (no batches)" );

	const JsonValue *id = message.Find( "id" );
	const JsonValue *method = message.Find( "method" );
	const JsonValue *version = message.Find( "jsonrpc" );
	if ( id && id->GetKind() != JsonValue::Kind::String &&
	     id->GetKind() != JsonValue::Kind::Number )
		return RespondError( noId, kInvalidRequest, "id must be a string or a number" );
	if ( !method )
	{
		// A response to a server request: this server sends none, so ignore it.
		if ( id && ( message.Find( "result" ) || message.Find( "error" ) ) )
			return std::nullopt;
		return RespondError( id ? *id : noId, kInvalidRequest, "missing method" );
	}
	if ( !version || version->GetKind() != JsonValue::Kind::String || version->Text() != "2.0" ||
	     method->GetKind() != JsonValue::Kind::String )
		return RespondError( id ? *id : noId, kInvalidRequest, "not a JSON-RPC 2.0 request" );

	const std::string &name = method->Text();
	const JsonValue *params = message.Find( "params" );
	if ( params && params->GetKind() != JsonValue::Kind::Object )
		return RespondError( id ? *id : noId, kInvalidParams, "params must be an object" );

	if ( !id )
	{
		// Notifications (initialized, cancelled, ...) take no response.
		return std::nullopt;
	}
	if ( name == "initialize" )
	{
		m_initialized = true;
		return Respond( *id, Initialize( params ) );
	}
	if ( name == "ping" )
		return Respond( *id, JsonValue::Object() );
	if ( name != "tools/list" && name != "tools/call" )
		return RespondError( *id, kMethodNotFound, "unknown method " + name );
	if ( !m_initialized )
		return RespondError( *id, kInvalidRequest, name + " before initialize" );
	if ( name == "tools/list" )
		return Respond( *id, ToolList( m_catalog ) );

	const JsonValue *tool = params ? params->Find( "name" ) : nullptr;
	if ( !tool || tool->GetKind() != JsonValue::Kind::String )
		return RespondError( *id, kInvalidParams, "tools/call needs a tool name" );
	const auto &catalog = m_catalog;
	const auto info = std::find_if( catalog.begin(), catalog.end(),
	    [&]( const app::CommandInfo &entry )
	    {
		    return entry.name == tool->Text();
	    } );
	if ( info == catalog.end() )
		return RespondError( *id, kInvalidParams, "unknown tool " + tool->Text() );

	app::CommandArgs args;
	if ( const JsonValue *arguments = params->Find( "arguments" ) )
	{
		if ( arguments->GetKind() != JsonValue::Kind::Object )
			return RespondError( *id, kInvalidParams, "arguments must be an object" );
		for ( const auto &member : arguments->Members() )
		{
			std::string text;
			if ( !ArgumentText( member.second, text ) )
			{
				return RespondError( *id, kInvalidParams,
				    "argument " + member.first + " must be a string, number, boolean or vector" );
			}
			args[member.first] = std::move( text );
		}
	}

	auto result = m_execute( tool->Text(), args );
	if ( !result )
	{
		const app::CommandError &error = result.Error();
		return Respond( *id,
		    TextResult(
		        error.command + ": " + app::CommandStatusName( error.status ) + ": " + error.detail,
		        true ) );
	}
	return Respond( *id, TextResult( result.Value().empty() ? "ok" : result.Value(), false ) );
}

} // namespace hammer::adapters::mcp
