//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless Hammer host (RFC 0002 map-building loop). A composition
//			root that runs command scripts through the domain command layer
//			(hammer::app::SessionCommands over an EditSession) against the
//			disk, with no UI, composing the full-fidelity VMF codec, the tool
//			process map builder and, with --fgd, the FGD entity catalog. UI
//			hosts, UI-driven tests and the MCP server drive the same commands.
//
//			hammer_cli --script room.hcmd --root <dir>   run a script
//			hammer_cli --mcp --root <dir>                 serve MCP on stdin/stdout
//			hammer_cli --commands                         list the command catalog
//			  [--fgd FILE]  entity schema (its @includes resolve beside it)
//
//			Script paths are relative to --root (default: the working
//			directory). Absolute paths and ".." components are rejected.
//
//=============================================================================//

#include "../../platform/posix/tool_process_provider.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/adapters/mcp/mcp_server.h"
#include "hammer/adapters/platform/tool_process_map_builder.h"
#include "hammer/app/session_commands.h"
#include "hammer/formats/fgd_entity_catalog.h"
#include "hammer/formats/vmf_map_codec.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
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

int Usage()
{
	std::fprintf( stderr,
	    "usage: hammer_cli (--script FILE | --mcp) [--root DIR] [--repo DIR] [--builds DIR] "
	    "[--fgd FILE] | --commands\n" );
	return 2;
}

int ListCommands()
{
	for ( const hammer::app::CommandInfo &info : hammer::app::SessionCommands::Catalog() )
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
	std::string repo = ".";
	std::string builds;
	std::string fgd;
	bool mcp = false;
	for ( int i = 1; i < argc; ++i )
	{
		const std::string_view arg = argv[i];
		if ( arg == "--commands" )
			return ListCommands();
		if ( arg == "--script" && i + 1 < argc )
			script = argv[++i];
		else if ( arg == "--mcp" )
			mcp = true;
		else if ( arg == "--root" && i + 1 < argc )
			root = argv[++i];
		else if ( arg == "--repo" && i + 1 < argc )
			repo = argv[++i];
		else if ( arg == "--builds" && i + 1 < argc )
			builds = argv[++i];
		else if ( arg == "--fgd" && i + 1 < argc )
			fgd = argv[++i];
		else
			return Usage();
	}
	if ( script.empty() == !mcp )
		return Usage();

	// An entity schema is optional; without one any class name is accepted.
	std::optional<hammer::formats::FgdEntityCatalog> catalog;
	if ( !fgd.empty() )
	{
		const std::string dir =
		    fgd.find( '/' ) == std::string::npos ? "." : fgd.substr( 0, fgd.rfind( '/' ) );
		auto loaded = hammer::formats::FgdEntityCatalog::Load( fgd,
		    [&]( const std::string &name ) -> std::optional<std::string>
		    {
			    std::ifstream in( name == fgd ? name : dir + "/" + name, std::ios::binary );
			    if ( !in )
				    return std::nullopt;
			    return std::string(
			        ( std::istreambuf_iterator<char>( in ) ), std::istreambuf_iterator<char>() );
		    } );
		if ( !loaded )
		{
			std::fprintf( stderr, "hammer_cli: %s:%zu: %s\n", loaded.Error().file.c_str(),
			    loaded.Error().line, loaded.Error().message.c_str() );
			return 1;
		}
		catalog.emplace( std::move( loaded ).Value() );
	}

	hammer::app::EditSession session;
	hammer::app::EditorSettings settings;
	hammer::formats::VmfMapCodec codec;
	hammer::app::MapFragment clipboard;
	RootedFileStore store( root );
	// build_map runs tools/quality/vmf_map_build.py through the platform
	// tool-process provider, with file-store paths resolved under --root.
	const std::unique_ptr<platform::IToolProcessProvider> processes =
	    platform::CreatePosixToolProcessProvider();
	hammer::adapters::platform::ToolProcessMapBuilder builder( *processes, repo,
	    builds.empty() ? repo + "/quality-results/hammer-builds" : builds,
	    [&root]( const std::string &path )
	    {
		    return root.empty() ? path : root + "/" + path;
	    } );
	hammer::app::SessionServices services;
	services.codec = &codec;
	services.store = &store;
	services.builder = &builder;
	services.catalog = catalog ? &*catalog : nullptr;
	services.clipboard = &clipboard;
	hammer::app::SessionCommands commands( session, settings, services );

	if ( mcp )
	{
		// MCP stdio transport: one JSON-RPC message per line in and out. Nothing
		// else may reach stdout; diagnostics go to stderr.
		hammer::adapters::mcp::McpServer server( hammer::app::SessionCommands::Catalog(),
		    [&commands]( std::string_view name, const hammer::app::CommandArgs &args )
		    {
			    return commands.Execute( name, args );
		    } );
		std::string line;
		while ( std::getline( std::cin, line ) )
		{
			if ( !line.empty() && line.back() == '\r' )
				line.pop_back();
			if ( line.empty() )
				continue;
			if ( const auto response = server.HandleLine( line ) )
			{
				std::fwrite( response->data(), 1, response->size(), stdout );
				std::fputc( '\n', stdout );
				std::fflush( stdout );
			}
		}
		return 0;
	}

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
		    hammer::app::CommandStatusName( parsed.Error().status ),
		    parsed.Error().detail.c_str() );
		return 1;
	}

	auto outputs = commands.Run( parsed.Value() );
	if ( !outputs )
	{
		const hammer::app::CommandError &error = outputs.Error();
		std::fprintf( stderr, "%s:%d: %s: %s: %s\n", script.c_str(), error.line,
		    error.command.c_str(), hammer::app::CommandStatusName( error.status ),
		    error.detail.c_str() );
		return 1;
	}
	for ( const std::string &output : outputs.Value() )
	{
		if ( !output.empty() )
			std::printf( "%s\n", output.c_str() );
	}
	return 0;
}
