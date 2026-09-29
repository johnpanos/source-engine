//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: IMapBuilder over the platform tool-process contract: runs
//			tools/quality/vmf_map_build.py for a saved map (RFC 0002 build_map;
//			the first product consumer of platform.tool_process, RFC 0001 R40).
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_PLATFORM_TOOL_PROCESS_MAP_BUILDER_H
#define HAMMER_ADAPTERS_PLATFORM_TOOL_PROCESS_MAP_BUILDER_H

#include "hammer/ports/map_builder.h"
#include "platform/contracts/tool_process.h"

#include <functional>
#include <string>
#include <vector>

namespace hammer::adapters::platform
{

class ToolProcessMapBuilder final : public ports::IMapBuilder
{
public:
	// `repoRoot` holds tools/quality/vmf_map_build.py; each build writes to
	// <outRoot>/<map>. `diskPath` turns a file-store path into a disk path.
	// `buildArgs` are passed on to every vmf_map_build.py run (e.g. --runtime
	// DIR for a product's staged content). Borrows the provider, which must
	// outlive this object.
	ToolProcessMapBuilder( ::platform::IToolProcessProvider &provider, std::string repoRoot,
	    std::string outRoot, std::function<std::string( const std::string & )> diskPath,
	    std::vector<std::string> buildArgs = {} );

	ports::MapBuildResult Build( const ports::MapBuildRequest &request ) override;

private:
	::platform::IToolProcessProvider &m_provider;
	std::string m_repoRoot;
	std::string m_outRoot;
	std::function<std::string( const std::string & )> m_diskPath;
	std::vector<std::string> m_buildArgs;
};

} // namespace hammer::adapters::platform

#endif // HAMMER_ADAPTERS_PLATFORM_TOOL_PROCESS_MAP_BUILDER_H
