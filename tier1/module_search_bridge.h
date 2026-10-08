//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sys_LoadModule's module search on POSIX, answered by
//			platform.module-resolver.v1 and the POSIX file probe (R11). The
//			frozen Sys_* bridge keeps its control flow; only the search moved.
//
//=============================================================================//

#ifndef TIER1_MODULE_SEARCH_BRIDGE_H
#define TIER1_MODULE_SEARCH_BRIDGE_H

#include <cstddef>

// The legacy foundLibraryWithPrefix contract: searches `root` for `moduleName`
// with its extension replaced by `extension`, trying
// <root>/<libDir>lib<name>, <root>/<libDir><name>, <root>/lib<name> and
// <root>/<name> in that order (libDir is "bin/" or ""), accepting anything that
// exists. Writes the found path, or the last candidate tried, into `out` and
// returns whether one exists.
bool Tier1_FindModuleWithPrefix( char *out, std::size_t outSize, const char *root,
    const char *moduleName, const char *libDir, const char *extension );

// Whether an absolute module path exists (the legacy stat() check).
bool Tier1_ModulePathExists( const char *path );

#endif // TIER1_MODULE_SEARCH_BRIDGE_H
