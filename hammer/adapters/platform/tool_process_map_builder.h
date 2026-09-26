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

namespace hammer::adapters::platform
{

class ToolProcessMapBuilder final : public ports::IMapBuilder
{
public:
	// `repoRoot` holds tools/quality/vmf_map_build.py; each build writes to
	// <outRoot>/<map>. `diskPath` turns a file-store path into a disk path.
	// Borrows the provider, which must outlive this object.
	ToolProcessMapBuilder( ::platform::IToolProcessProvider &provider, std::string repoRoot,
	    std::string outRoot, std::function<std::string( const std::string & )> diskPath );

	ports::MapBuildResult Build( const ports::MapBuildRequest &request ) override;

private:
	::platform::IToolProcessProvider &m_provider;
	std::string m_repoRoot;
	std::string m_outRoot;
	std::function<std::string( const std::string & )> m_diskPath;
};

} // namespace hammer::adapters::platform

#endif // HAMMER_ADAPTERS_PLATFORM_TOOL_PROCESS_MAP_BUILDER_H
