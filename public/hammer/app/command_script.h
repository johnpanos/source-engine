//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared vocabulary of Hammer's named command layers (RFC 0002,
//			hammer.app): argument maps, structured errors, catalog entries, the
//			line-oriented script format, argument validation against a catalog
//			entry, and the value parsers commands share. SessionCommands (over
//			EditSession) routes through this one owner, so scripts, MCP tools
//			and UI actions parse and fail the same way.
//
//			Script format: one command per line, `name key=value key="a b"`,
//			'#' starts a comment. Vectors are "x y z".
//
//=============================================================================//

#ifndef HAMMER_APP_COMMAND_SCRIPT_H
#define HAMMER_APP_COMMAND_SCRIPT_H

#include "foundation/expected.h"
#include "mapgeometry/brush.h"

#include <functional>
#include <map>
#include <optional>
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
	Rejected,        // the editor refused the edit (degenerate, unknown id, no-op)
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

using CommandResult = foundation::Expected<std::string, CommandError>;

inline foundation::Unexpected<CommandError> CommandFailure(
    CommandStatus status, std::string_view command, std::string detail )
{
	return foundation::MakeUnexpected(
	    CommandError{ status, std::string( command ), std::move( detail ), 0 } );
}

[[nodiscard]] foundation::Expected<std::vector<ScriptCommand>, CommandError> ParseCommandScript(
    std::string_view text );

// Missing required arguments and arguments the entry does not declare.
[[nodiscard]] foundation::Expected<void, CommandError> ValidateCommandArgs(
    const CommandInfo &info, const CommandArgs &args );

// Runs 'script' through 'execute' in order, stopping at the first error, which
// carries its script line. Returns the outputs of the commands that ran.
[[nodiscard]] foundation::Expected<std::vector<std::string>, CommandError> RunCommandScript(
    const std::vector<ScriptCommand> &script,
    const std::function<CommandResult( std::string_view, const CommandArgs & )> &execute );

// Value parsers: the whole text must be consumed.
std::optional<double> ParseCommandNumber( const std::string &text );
std::optional<mapgeometry::Vec3d> ParseCommandVector( const std::string &text );

} // namespace hammer::app

#endif // HAMMER_APP_COMMAND_SCRIPT_H
