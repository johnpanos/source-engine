//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Map build port (RFC 0002 hammer.ports): compile a saved map into a
//			playable one. The command layer's build_map (F9 in the editor, a
//			script line, an MCP call) asks for it; a composition root supplies
//			the implementation, e.g. one that runs tools/quality/vmf_map_build.py
//			through the platform tool-process contract.
//
//=============================================================================//

#ifndef HAMMER_PORTS_MAP_BUILDER_H
#define HAMMER_PORTS_MAP_BUILDER_H

#include <string>

namespace hammer::ports
{

struct MapBuildRequest
{
	std::string vmfPath;      // the saved map, as the file store names it
	bool fullQuality = false; // release lighting instead of the fast edit loop
	bool publish = false;     // make it playable (./play <map>)
};

struct MapBuildResult
{
	bool ok = false;
	std::string status; // "pass", "leak", "missing-materials", "fail", ...
	std::string detail; // one line a user can act on
};

class IMapBuilder
{
public:
	virtual ~IMapBuilder() = default;
	// Synchronous; returns only once the build has finished or failed.
	virtual MapBuildResult Build( const MapBuildRequest &request ) = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_MAP_BUILDER_H
