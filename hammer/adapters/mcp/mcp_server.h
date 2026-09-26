//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Model Context Protocol server over the Hammer command layer
//			(RFC 0002, hammer.adapters.mcp). Each EditorCommands catalog entry
//			is one MCP tool, and each tool call is one EditorCommands::Execute,
//			so an agent edits the same document, history and files as the GTK
//			host and command scripts.
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

#include "hammer/app/editor_commands.h"

#include <optional>
#include <string>
#include <string_view>

namespace hammer::adapters::mcp
{

inline constexpr const char *kProtocolVersion = "2025-06-18";

class McpServer
{
public:
	// Borrows the command layer, which must outlive this object.
	explicit McpServer( app::EditorCommands &commands );

	// One message in; the response line (no trailing newline) or nothing.
	[[nodiscard]] std::optional<std::string> HandleLine( std::string_view line );

	bool Initialized() const { return m_initialized; }

private:
	app::EditorCommands &m_commands;
	bool m_initialized = false;
};

} // namespace hammer::adapters::mcp

#endif // HAMMER_ADAPTERS_MCP_MCP_SERVER_H
