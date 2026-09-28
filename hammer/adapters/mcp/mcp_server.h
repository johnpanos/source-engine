//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Model Context Protocol server over a Hammer command layer
//			(RFC 0002, hammer.adapters.mcp). Each catalog entry is one MCP tool
//			and each tool call is one Execute of the same command layer, so an
//			agent edits the same document, history and files as the UI host and
//			command scripts. The composition root chooses the command layer
//			(app::SessionCommands in hammer_cli).
//
//			Transport-free: HandleLine takes one newline-delimited JSON-RPC 2.0
//			message (MCP stdio framing) and returns the response line, or
//			nothing for a notification. The composition root owns the stream.
//
//			Command failures are tool results with isError; protocol faults are
//			JSON-RPC errors (-32700 parse, -32600 invalid request, -32601
//			unknown method, -32602 unknown tool or invalid arguments).
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_MCP_MCP_SERVER_H
#define HAMMER_ADAPTERS_MCP_MCP_SERVER_H

#include "hammer/app/command_script.h"

#include <functional>
#include <vector>

#include <optional>
#include <string>
#include <string_view>

namespace hammer::adapters::mcp
{

inline constexpr const char *kProtocolVersion = "2025-06-18";

class McpServer
{
public:
	using Execute = std::function<app::CommandResult( std::string_view, const app::CommandArgs & )>;

	// A command layer: its catalog (which must outlive this object) and its
	// execute function (app::SessionCommands in hammer_cli).
	McpServer( const std::vector<app::CommandInfo> &catalog, Execute execute );

	// One message in; the response line (no trailing newline) or nothing.
	[[nodiscard]] std::optional<std::string> HandleLine( std::string_view line );

	bool Initialized() const { return m_initialized; }

private:
	const std::vector<app::CommandInfo> &m_catalog;
	Execute m_execute;
	bool m_initialized = false;
};

} // namespace hammer::adapters::mcp

#endif // HAMMER_ADAPTERS_MCP_MCP_SERVER_H
