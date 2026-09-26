//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX dynamic-library loader provider composition seam
//          (RFC 0001 rank 4, reference migration step 4).
//
//=============================================================================//

#ifndef PLATFORM_POSIX_DYNAMIC_LIBRARY_PROVIDER_H
#define PLATFORM_POSIX_DYNAMIC_LIBRARY_PROVIDER_H

#include "platform/contracts/dynamic_library.h"

#include <memory>

namespace platform
{

// Application roots and the legacy Sys_* bridge explicitly select this backend
// on POSIX hosts; portable code sees only IDynamicLibraryLoader. The dlopen
// handle stays private to platform/posix.
//
// Load opens exactly the path it is given: a path without '/' is opened
// relative to the working directory, never through the dynamic linker's
// library search, because path resolution belongs to the module resolver.
// Resolve-without-load is not supported (RTLD_NOLOAD only finds libraries that
// are already loaded) and is reported as kUnsupportedNoLoad. Destroying the
// loader while it still owns libraries aborts the process.
//
// Every load request, symbol lookup and unload is reported to `observer`
// (load-site telemetry), which must outlive the loader. There is no default:
// a root that wants no telemetry passes an explicit no-op observer.
[[nodiscard]] std::unique_ptr<IDynamicLibraryLoader> CreatePosixDynamicLibraryLoader(
    IDynamicLibraryObserver &observer );

} // namespace platform

#endif // PLATFORM_POSIX_DYNAMIC_LIBRARY_PROVIDER_H
