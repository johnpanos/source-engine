//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX child-process provider composition seam.
//
//=============================================================================//

#ifndef PLATFORM_POSIX_TOOL_PROCESS_PROVIDER_H
#define PLATFORM_POSIX_TOOL_PROCESS_PROVIDER_H

#include "platform/contracts/tool_process.h"

#include <memory>

namespace platform
{

// Application roots explicitly select this backend on POSIX hosts. Native
// handles and process-management details remain private to platform/posix.
[[nodiscard]] std::unique_ptr<IToolProcessProvider> CreatePosixToolProcessProvider();

} // namespace platform

#endif // PLATFORM_POSIX_TOOL_PROCESS_PROVIDER_H
