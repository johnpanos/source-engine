//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Named, serializable editor commands (RFC 0002, hammer.app). The one
//			entry point that the GTK host, command scripts, UI-driven
//			conformance tests and an MCP server share. Each command maps to one
//			EditorController operation, so every front end drives the same
//			authority and the same undo history.
//
//			A command is a name plus string arguments. A script is one command
//			per line: `name key=value key="value with spaces"`, with '#' comments.
//			Vectors are "x y z". The catalog lists every command with its
//			arguments, for help text and tool listings.
//
//=============================================================================//

#ifndef HAMMER_APP_EDITOR_COMMANDS_H
#define HAMMER_APP_EDITOR_COMMANDS_H

#include "foundation/expected.h"
#include "hammer/app/editor_controller.h"
#include "hammer/ports/file_store.h"
#include "hammer/ports/map_builder.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::app
{

using CommandArgs = std::map<std::string, std::string>;

enum class CommandStatus
{
	UnknownCommand,
	MissingArgument,
	InvalidArgument, // unparsable value or an argument the command does not take
	Rejected,        // the controller refused the edit (degenerate, unknown id, no-op)
	IoFailure,       // the file store could not read or write
	SyntaxError,     // a script line could not be parsed
};

// A short lowercase name for reports ("unknown command", "i/o failure", ...).
const char *CommandStatusName( CommandStatus status );

// Readable context is part of the error because commands are the application
// boundary: `command` and `detail` name the request that failed.
struct CommandError
{
	CommandStatus status = CommandStatus::Rejected;
	std::string command;
	std::string detail;
	int line = 0; // script line, 1-based; 0 outside a script
};

struct CommandInfo
{
	std::string name;
	std::vector<std::string> required;
	std::vector<std::string> optional;
	std::string summary;
};

struct ScriptCommand
{
	std::string name;
	CommandArgs args;
	int line = 0;
};

class EditorCommands
{
public:
	// Borrows all three; they must outlive this object. Without a map builder,
	// build_map is rejected (a composition that cannot compile maps).
	EditorCommands( EditorController &controller, ports::IFileStore &store,
	    ports::IMapBuilder *builder = nullptr );

	// Runs one command. On success returns its output: the new object's id for
	// creating commands, a short summary for "info", otherwise empty.
	[[nodiscard]] foundation::Expected<std::string, CommandError> Execute(
	    std::string_view name, const CommandArgs &args );

	// Runs a parsed script in order, stopping at the first error. Returns the
	// outputs of the commands that ran.
	[[nodiscard]] foundation::Expected<std::vector<std::string>, CommandError> Run(
	    const std::vector<ScriptCommand> &script );

	static const std::vector<CommandInfo> &Catalog();

private:
	EditorController &m_controller;
	ports::IFileStore &m_store;
	ports::IMapBuilder *m_builder;
};

[[nodiscard]] foundation::Expected<std::vector<ScriptCommand>, CommandError> ParseCommandScript(
    std::string_view text );

} // namespace hammer::app

#endif // HAMMER_APP_EDITOR_COMMANDS_H
