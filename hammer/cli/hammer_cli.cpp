//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless Hammer host (RFC 0002 map-building loop). A composition
//			root that runs command scripts through the shared command layer
//			(hammer::app::EditorCommands) against the disk, with no UI. The GTK
//			editor, UI-driven tests and an MCP server drive the same commands.
//
//			hammer_cli --script room.hcmd --root <dir>   run a script
//			hammer_cli --commands                         list the command catalog
//
//			Script paths are relative to --root (default: the working
//			directory). Absolute paths and ".." components are rejected.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/app/editor_commands.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

namespace
{

// Confines every file-store path to one root directory.
class RootedFileStore final : public hammer::ports::IFileStore
{
public:
	explicit RootedFileStore( std::string root ) : m_Root( std::move( root ) ) {}

	bool Exists( const std::string &path ) const override
	{
		std::string full;
		return Resolve( path, full ) && m_Disk.Exists( full );
	}
	bool Read( const std::string &path, std::string &out ) const override
	{
		std::string full;
		return Resolve( path, full ) && m_Disk.Read( full, out );
	}
	bool Write( const std::string &path, const std::string &content ) override
	{
		std::string full;
		return Resolve( path, full ) && m_Disk.Write( full, content );
	}
	bool Rename( const std::string &from, const std::string &to ) override
	{
		std::string fullFrom;
		std::string fullTo;
		return Resolve( from, fullFrom ) && Resolve( to, fullTo ) &&
		       m_Disk.Rename( fullFrom, fullTo );
	}
	bool Remove( const std::string &path ) override
	{
		std::string full;
		return Resolve( path, full ) && m_Disk.Remove( full );
	}

private:
	bool Resolve( const std::string &path, std::string &full ) const
	{
		if ( path.empty() || path.front() == '/' || path.find( '\\' ) != std::string::npos )
			return false;
		std::stringstream parts( path );
		std::string part;
		while ( std::getline( parts, part, '/' ) )
		{
			if ( part == ".." )
				return false;
		}
		full = m_Root.empty() ? path : m_Root + "/" + path;
		return true;
	}

	std::string m_Root;
	hammer::adapters::platform::DiskFileStore m_Disk;
};

const char *StatusName( hammer::app::CommandStatus status )
{
	using hammer::app::CommandStatus;
	switch ( status )
	{
	case CommandStatus::UnknownCommand:
		return "unknown command";
	case CommandStatus::MissingArgument:
		return "missing argument";
	case CommandStatus::InvalidArgument:
		return "invalid argument";
	case CommandStatus::Rejected:
		return "rejected";
	case CommandStatus::IoFailure:
		return "i/o failure";
	case CommandStatus::SyntaxError:
		return "syntax error";
	}
	return "error";
}

int Usage()
{
	std::fprintf( stderr, "usage: hammer_cli --script FILE [--root DIR] | --commands\n" );
	return 2;
}

int ListCommands()
{
	for ( const hammer::app::CommandInfo &info : hammer::app::EditorCommands::Catalog() )
	{
		std::printf( "%s", info.name.c_str() );
		for ( const std::string &arg : info.required )
			std::printf( " %s=...", arg.c_str() );
		for ( const std::string &arg : info.optional )
			std::printf( " [%s=...]", arg.c_str() );
		std::printf( "\n    %s\n", info.summary.c_str() );
	}
	return 0;
}

} // namespace

int main( int argc, char **argv )
{
	std::string script;
	std::string root;
	for ( int i = 1; i < argc; ++i )
	{
		const std::string_view arg = argv[i];
		if ( arg == "--commands" )
			return ListCommands();
		if ( arg == "--script" && i + 1 < argc )
			script = argv[++i];
		else if ( arg == "--root" && i + 1 < argc )
			root = argv[++i];
		else
			return Usage();
	}
	if ( script.empty() )
		return Usage();

	std::ifstream stream( script, std::ios::binary );
	if ( !stream )
	{
		std::fprintf( stderr, "hammer_cli: cannot read %s\n", script.c_str() );
		return 1;
	}
	const std::string text(
	    ( std::istreambuf_iterator<char>( stream ) ), std::istreambuf_iterator<char>() );

	auto parsed = hammer::app::ParseCommandScript( text );
	if ( !parsed )
	{
		std::fprintf( stderr, "%s:%d: %s: %s\n", script.c_str(), parsed.Error().line,
		    StatusName( parsed.Error().status ), parsed.Error().detail.c_str() );
		return 1;
	}

	hammer::app::EditorController controller;
	RootedFileStore store( root );
	hammer::app::EditorCommands commands( controller, store );
	auto outputs = commands.Run( parsed.Value() );
	if ( !outputs )
	{
		const hammer::app::CommandError &error = outputs.Error();
		std::fprintf( stderr, "%s:%d: %s: %s: %s\n", script.c_str(), error.line,
		    error.command.c_str(), StatusName( error.status ), error.detail.c_str() );
		return 1;
	}
	for ( const std::string &output : outputs.Value() )
	{
		if ( !output.empty() )
			std::printf( "%s\n", output.c_str() );
	}
	return 0;
}
