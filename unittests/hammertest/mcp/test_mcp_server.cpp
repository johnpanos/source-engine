//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.mcp conformance (RFC 0002 / roadmap R08, R08-MCP):
//			the MCP server over the command layer. The JSON reader and writer
//			(escapes, surrogates, depth bound, malformed input), every JSON-RPC
//			error class, the initialize handshake, the tool catalog, and a room
//			authored through tool calls that must save byte-identically to the
//			same room authored by command script. Protocol faults must leave the
//			document unchanged.
//
//=============================================================================//

#include "foundation/json.h"
#include "hammer/adapters/mcp/mcp_server.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"
#include "hammer/formats/vmf_map_codec.h"
#include "testing/checks.h"

#include "../app/fake_file_store.h"

#include <string>
#include <vector>

namespace
{

using JsonValue = foundation::json::Value;
using hammer::adapters::mcp::McpServer;
constexpr auto ParseJson = &foundation::json::Parse;
using hammer::app::SessionCommands;
using hammertest::InMemoryFileStore;

// The command layer hammer_cli --mcp serves: SessionCommands over one
// EditSession, with the strict VMF codec over an in-memory store.
struct Fixture
{
	hammer::app::EditSession session;
	hammer::app::EditorSettings settings;
	hammer::formats::VmfMapCodec codec;
	InMemoryFileStore store;
	SessionCommands commands{ session, settings,
	    hammer::app::SessionServices{ &codec, &store, nullptr, nullptr, nullptr, nullptr } };
	McpServer server{ SessionCommands::Catalog(),
	    [this]( std::string_view name, const hammer::app::CommandArgs &args )
	    {
		    return commands.Execute( name, args );
	    } };
};

std::string Call( int id, const std::string &tool, const std::string &arguments = "{}" )
{
	return R"({"jsonrpc":"2.0","id":)" + std::to_string( id ) +
	       R"(,"method":"tools/call","params":{"name":")" + tool + R"(","arguments":)" + arguments +
	       "}}";
}

const char kInitialize[] = R"({"jsonrpc":"2.0","id":0,"method":"initialize","params":)"
                           R"({"protocolVersion":"2025-06-18","capabilities":{},)"
                           R"("clientInfo":{"name":"test","version":"1"}}})";

// A sealed room with a player start and a light, authored by script.
const char kRoomScript[] = R"(new_map
create_block mins="-144 -144 -16" maxs="144 144 0"
create_block mins="-144 -144 128" maxs="144 144 144"
create_block mins="-144 -144 0" maxs="-128 144 128"
create_block mins="128 -144 0" maxs="144 144 128"
create_block mins="-128 -144 0" maxs="128 -128 128"
create_block mins="-128 128 0" maxs="128 144 128"
place_entity classname=info_player_start origin="0 0 1"
place_entity classname=light origin="0 0 96"
save path=maps/room.vmf
)";

} // namespace

int main()
{
	testing::Checks checks;

	// ---- JSON reader and writer -------------------------------------------------
	{
		auto doc =
		    ParseJson( R"( {"a":[1,-2.5e3,true,false,null],"b":"x\"\\\/\n\u00e9\ud83d\ude00"} )" );
		checks.That( doc.HasValue(), "json.parse-document" );
		if ( doc )
		{
			checks.Equal( doc.Value().Write(),
			    std::string( "{\"a\":[1,-2.5e3,true,false,null],\"b\":\"x\\\"\\\\/\\n"
			                 "\xC3\xA9\xF0\x9F\x98\x80\"}" ),
			    "json.round-trip-utf8" );
			const JsonValue *b = doc.Value().Find( "b" );
			checks.That( b && b->Text().size() == 11, "json.escapes-decoded" );
		}
		checks.Equal( JsonValue::String( std::string( "\x01\t" ) ).Write(),
		    std::string( "\"\\u0001\\t\"" ), "json.control-escaped" );

		const std::vector<std::pair<const char *, const char *>> bad = {
		    { "", "json.reject-empty" },
		    { "{\"a\":1} x", "json.reject-trailing" },
		    { "\"abc", "json.reject-unterminated" },
		    { "\"\\x\"", "json.reject-bad-escape" },
		    { "\"\\ud83d\"", "json.reject-lone-high" },
		    { "\"\\ude00\"", "json.reject-lone-low" },
		    { "\"a\nb\"", "json.reject-raw-control" },
		    { "01", "json.reject-leading-zero" },
		    { "1.", "json.reject-bad-fraction" },
		    { "-", "json.reject-bare-minus" },
		    { "{\"a\":1,\"a\":2}", "json.reject-duplicate" },
		    { "[1,]", "json.reject-trailing-comma" },
		    { "tru", "json.reject-literal" },
		};
		for ( const auto &[text, name] : bad )
			checks.That( !ParseJson( text ).HasValue(), name );

		const std::string deepOk = std::string( 64, '[' ) + std::string( 64, ']' );
		const std::string tooDeep = std::string( 65, '[' ) + std::string( 65, ']' );
		checks.That( ParseJson( deepOk ).HasValue(), "json.depth-64-accepted" );
		checks.That( !ParseJson( tooDeep ).HasValue(), "json.depth-65-rejected" );
		const std::string hostile( 100000, '[' );
		checks.That( !ParseJson( hostile ).HasValue(), "json.hostile-nesting-rejected" );
	}

	// ---- Protocol ---------------------------------------------------------------
	auto response = [&]( McpServer &server, const std::string &line ) -> JsonValue
	{
		const auto out = server.HandleLine( line );
		if ( !out )
			return JsonValue();
		auto parsed = ParseJson( *out );
		checks.That( parsed.HasValue() && out->find( '\n' ) == std::string::npos,
		    "protocol.response-is-one-json-line" );
		return parsed ? parsed.Value() : JsonValue();
	};
	auto errorCode = []( const JsonValue &message ) -> std::string
	{
		const JsonValue *error = message.Find( "error" );
		const JsonValue *code = error ? error->Find( "code" ) : nullptr;
		return code ? code->Text() : std::string( "none" );
	};
	auto idText = []( const JsonValue &message ) -> std::string
	{
		const JsonValue *id = message.Find( "id" );
		return id ? id->Write() : std::string( "absent" );
	};
	auto toolResult = []( const JsonValue &message, bool &isError ) -> std::string
	{
		const JsonValue *result = message.Find( "result" );
		const JsonValue *flag = result ? result->Find( "isError" ) : nullptr;
		const JsonValue *content = result ? result->Find( "content" ) : nullptr;
		isError = flag && flag->AsBool();
		if ( !content || content->Items().empty() )
			return std::string();
		const JsonValue *text = content->Items()[0].Find( "text" );
		return text ? text->Text() : std::string();
	};

	{
		Fixture f;
		const JsonValue early =
		    response( f.server, R"({"jsonrpc":"2.0","id":1,"method":"tools/list"})" );
		checks.Equal(
		    errorCode( early ), std::string( "-32600" ), "protocol.tools-before-initialize" );
		const JsonValue ping =
		    response( f.server, R"({"jsonrpc":"2.0","id":"p","method":"ping"})" );
		checks.Equal( ping.Find( "result" ) ? ping.Find( "result" )->Write() : std::string(),
		    std::string( "{}" ), "protocol.ping-before-initialize" );

		const JsonValue init = response( f.server, kInitialize );
		const JsonValue *result = init.Find( "result" );
		checks.That( result && result->Find( "protocolVersion" ) &&
		                 result->Find( "protocolVersion" )->Text() == "2025-06-18",
		    "protocol.initialize-version" );
		checks.That( result && result->Find( "capabilities" ) &&
		                 result->Find( "capabilities" )->Find( "tools" ),
		    "protocol.initialize-tools-capability" );
		checks.That( f.server.Initialized(), "protocol.initialized" );

		Fixture g;
		const JsonValue other = response( g.server,
		    R"({"jsonrpc":"2.0","id":0,"method":"initialize","params":{"protocolVersion":"1999-01-01"}})" );
		checks.That( other.Find( "result" ) &&
		                 other.Find( "result" )->Find( "protocolVersion" )->Text() == "2025-06-18",
		    "protocol.initialize-unknown-version-offers-latest" );
		const JsonValue older = response( g.server,
		    R"({"jsonrpc":"2.0","id":0,"method":"initialize","params":{"protocolVersion":"2025-03-26"}})" );
		checks.That( older.Find( "result" ) &&
		                 older.Find( "result" )->Find( "protocolVersion" )->Text() == "2025-03-26",
		    "protocol.initialize-supported-version-echoed" );

		checks.That(
		    !f.server.HandleLine( R"({"jsonrpc":"2.0","method":"notifications/initialized"})" ),
		    "protocol.notification-no-response" );
		checks.That( !f.server.HandleLine( R"({"jsonrpc":"2.0","id":7,"result":{}})" ),
		    "protocol.client-response-ignored" );

		const JsonValue parse = response( f.server, "{\"jsonrpc\":" );
		checks.Equal( errorCode( parse ), std::string( "-32700" ), "protocol.parse-error" );
		checks.Equal( idText( parse ), std::string( "null" ), "protocol.parse-error-null-id" );
		checks.Equal( errorCode( response( f.server, "[]" ) ), std::string( "-32600" ),
		    "protocol.batch-rejected" );
		checks.Equal(
		    errorCode( response( f.server, R"({"jsonrpc":"2.0","id":{},"method":"ping"})" ) ),
		    std::string( "-32600" ), "protocol.object-id-rejected" );
		checks.Equal( errorCode( response( f.server, R"({"id":3,"method":"ping"})" ) ),
		    std::string( "-32600" ), "protocol.missing-jsonrpc" );
		checks.Equal( errorCode( response( f.server, R"({"jsonrpc":"2.0","id":3})" ) ),
		    std::string( "-32600" ), "protocol.missing-method" );
		const JsonValue unknown =
		    response( f.server, R"({"jsonrpc":"2.0","id":1.50e3,"method":"resources/list"})" );
		checks.Equal( errorCode( unknown ), std::string( "-32601" ), "protocol.unknown-method" );
		checks.Equal( idText( unknown ), std::string( "1.50e3" ), "protocol.id-echoed-verbatim" );
		checks.Equal(
		    idText( response( f.server, R"({"jsonrpc":"2.0","id":"abc","method":"x"})" ) ),
		    std::string( "\"abc\"" ), "protocol.string-id-echoed" );
		checks.Equal( errorCode( response( f.server,
		                  R"({"jsonrpc":"2.0","id":4,"method":"tools/list","params":[]})" ) ),
		    std::string( "-32602" ), "protocol.array-params-rejected" );

		// Tool catalog: one tool per command, with the command's arguments.
		const JsonValue list =
		    response( f.server, R"({"jsonrpc":"2.0","id":5,"method":"tools/list"})" );
		const JsonValue *tools =
		    list.Find( "result" ) ? list.Find( "result" )->Find( "tools" ) : nullptr;
		const auto &catalog = SessionCommands::Catalog();
		checks.That( tools && tools->Items().size() == catalog.size(), "tools.one-per-command" );
		bool schemasMatch = tools != nullptr;
		for ( size_t i = 0; tools && i < tools->Items().size() && i < catalog.size(); ++i )
		{
			const JsonValue &tool = tools->Items()[i];
			const JsonValue *schema = tool.Find( "inputSchema" );
			const JsonValue *required = schema ? schema->Find( "required" ) : nullptr;
			const JsonValue *properties = schema ? schema->Find( "properties" ) : nullptr;
			schemasMatch = schemasMatch && tool.Find( "name" )->Text() == catalog[i].name &&
			               required && properties &&
			               required->Items().size() == catalog[i].required.size() &&
			               properties->Members().size() ==
			                   catalog[i].required.size() + catalog[i].optional.size();
		}
		checks.That( schemasMatch, "tools.schemas-match-catalog" );

		// Tool calls: errors of the protocol vs. of the command.
		checks.Equal( errorCode( response( f.server, Call( 10, "no_such_tool" ) ) ),
		    std::string( "-32602" ), "call.unknown-tool" );
		checks.Equal( errorCode( response( f.server, Call( 11, "new_map", "[]" ) ) ),
		    std::string( "-32602" ), "call.arguments-not-object" );
		checks.Equal( errorCode( response( f.server, Call( 12, "set_grid", R"({"size":null})" ) ) ),
		    std::string( "-32602" ), "call.null-argument" );
		checks.Equal( errorCode( response( f.server, Call( 13, "set_grid", R"({"size":[]})" ) ) ),
		    std::string( "-32602" ), "call.empty-vector-argument" );
		checks.Equal(
		    errorCode( response( f.server, R"({"jsonrpc":"2.0","id":14,"method":"tools/call"})" ) ),
		    std::string( "-32602" ), "call.missing-name" );

		bool isError = false;
		const std::string missing =
		    toolResult( response( f.server, Call( 15, "create_block" ) ), isError );
		checks.That( isError && missing.find( "missing argument" ) != std::string::npos,
		    "call.command-error-is-tool-result" );
		const std::string ok = toolResult( response( f.server, Call( 16, "new_map" ) ), isError );
		checks.That( !isError && ok == "ok", "call.success-text" );
	}

	// ---- Same room, two front ends ------------------------------------------------
	{
		Fixture byScript;
		auto script = hammer::app::ParseCommandScript( kRoomScript );
		checks.That( script.HasValue() && byScript.commands.Run( script.Value() ).HasValue(),
		    "room.script-authored" );

		Fixture byMcp;
		std::vector<std::string> calls = {
		    kInitialize,
		    Call( 1, "new_map" ),
		    Call( 2, "create_block", R"({"mins":[-144,-144,-16],"maxs":[144,144,0]})" ),
		    Call( 3, "create_block", R"({"mins":"-144 -144 128","maxs":"144 144 144"})" ),
		    Call( 4, "create_block", R"({"mins":[-144,-144,0],"maxs":[-128,144,128]})" ),
		    Call( 5, "create_block", R"({"mins":[128,-144,0],"maxs":[144,144,128]})" ),
		    Call( 6, "create_block", R"({"mins":[-128,-144,0],"maxs":[128,-128,128]})" ),
		    Call( 7, "create_block", R"({"mins":[-128,128,0],"maxs":[128,144,128]})" ),
		    Call( 8, "place_entity", R"({"classname":"info_player_start","origin":[0,0,1]})" ),
		    Call( 9, "place_entity", R"({"classname":"light","origin":"0 0 96"})" ),
		};
		bool allOk = true;
		for ( const std::string &call : calls )
		{
			const JsonValue message = response( byMcp.server, call );
			bool isError = false;
			toolResult( message, isError );
			allOk = allOk && message.Find( "result" ) && !isError;
		}
		checks.That( allOk, "room.mcp-authored" );

		// Protocol faults and failed commands change nothing.
		bool isError = false;
		const std::string before =
		    toolResult( response( byMcp.server, Call( 20, "info" ) ), isError );
		(void)byMcp.server.HandleLine(
		    "{\"jsonrpc\":\"2.0\",\"id\":21,\"method\":\"tools/call\"," );
		(void)byMcp.server.HandleLine(
		    Call( 22, "create_block", R"({"mins":{},"maxs":[1,1,1]})" ) );
		(void)byMcp.server.HandleLine(
		    Call( 23, "create_block", R"({"mins":"0 0 0","maxs":"0 0 0"})" ) );
		(void)byMcp.server.HandleLine( Call( 24, "no_such_tool" ) );
		const std::string after =
		    toolResult( response( byMcp.server, Call( 25, "info" ) ), isError );
		checks.That( !before.empty() && before == after, "room.faults-change-nothing" );

		const JsonValue saved =
		    response( byMcp.server, Call( 30, "save", R"({"path":"maps/room.vmf"})" ) );
		toolResult( saved, isError );
		checks.That( !isError, "room.mcp-saved" );
		const auto a = byScript.store.files.find( "maps/room.vmf" );
		const auto b = byMcp.store.files.find( "maps/room.vmf" );
		checks.That( a != byScript.store.files.end() && b != byMcp.store.files.end() &&
		                 a->second == b->second,
		    "room.mcp-equals-script-bytes" );

		// Undo is the same history: one undo removes the light.
		toolResult( response( byMcp.server, Call( 31, "undo" ) ), isError );
		const std::string undone =
		    toolResult( response( byMcp.server, Call( 32, "info" ) ), isError );
		checks.That( !isError && undone != after, "room.undo-through-mcp" );
	}

	return checks.Report();
}
