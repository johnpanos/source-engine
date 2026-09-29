//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: IMapBuilder over the platform tool-process contract.
//
//=============================================================================//

#include "hammer/adapters/platform/tool_process_map_builder.h"

#include <chrono>
#include <sstream>
#include <utility>

namespace hammer::adapters::platform
{

ToolProcessMapBuilder::ToolProcessMapBuilder( ::platform::IToolProcessProvider &provider,
    std::string repoRoot, std::string outRoot,
    std::function<std::string( const std::string & )> diskPath, std::vector<std::string> buildArgs )
    : m_provider( provider ), m_repoRoot( std::move( repoRoot ) ),
      m_outRoot( std::move( outRoot ) ), m_diskPath( std::move( diskPath ) ),
      m_buildArgs( std::move( buildArgs ) )
{
}

ports::MapBuildResult ToolProcessMapBuilder::Build( const ports::MapBuildRequest &request )
{
	const std::string vmf = m_diskPath( request.vmfPath );
	std::string stem = vmf.substr( vmf.find_last_of( '/' ) + 1 );
	stem = stem.substr( 0, stem.rfind( '.' ) );

	::platform::ToolProcessRequest run;
	run.argv = { "python3", m_repoRoot + "/tools/quality/vmf_map_build.py", "build", "--vmf", vmf,
	    "--out", m_outRoot + "/" + stem, "--quality", request.fullQuality ? "full" : "fast" };
	run.argv.insert( run.argv.end(), m_buildArgs.begin(), m_buildArgs.end() );
	if ( request.publish )
	{
		run.argv.push_back( "--publish" );
	}
	if ( !request.lighting.empty() )
	{
		run.argv.push_back( "--lighting" );
		run.argv.push_back( request.lighting );
	}
	run.workingDirectory = m_repoRoot;
	run.executionTimeout = std::chrono::minutes( 30 );
	run.cancellationTimeout = std::chrono::seconds( 5 );
	const ::platform::ToolProcessResult result = m_provider.Run( run );

	// The tool prints "vmf_map_build: <map> <status> (<build.json>)".
	ports::MapBuildResult out;
	std::istringstream lines( result.stdoutData );
	for ( std::string line; std::getline( lines, line ); )
	{
		const std::string prefix = "vmf_map_build: " + stem + " ";
		if ( line.rfind( prefix, 0 ) == 0 )
		{
			out.status =
			    line.substr( prefix.size(), line.find( ' ', prefix.size() ) - prefix.size() );
			out.detail = line;
		}
	}
	out.ok = result.Succeeded() && out.status == "pass";
	if ( out.status.empty() )
	{
		out.status = "fail";
		out.detail = result.error.IsOk() ? "vmf_map_build reported no result"
		                                 : "vmf_map_build did not run: " + result.error.detail;
	}
	return out;
}

} // namespace hammer::adapters::platform
