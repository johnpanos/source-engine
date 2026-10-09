//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX process entry: replace the calling process with a program
//			(RFC 0027 `kiln play` and `kiln run`). Application roots only.
//
//=============================================================================//

#ifndef PLATFORM_POSIX_PROCESS_EXEC_H
#define PLATFORM_POSIX_PROCESS_EXEC_H

#include "platform/contracts/tool_process.h"

#include <string>
#include <vector>

namespace platform
{

// Applies the environment changes (a value's "{inherit}" is replaced by the
// variable's current value; a missing value unsets it), enters
// `workingDirectory` and execs argv[0] (a path, relative to that directory
// when relative). Returns only on failure, with a description in `error`.
void ExecReplacingProcess( const std::vector<std::string> &argv,
    const std::vector<ToolProcessEnvironmentOverride> &environment,
    const std::string &workingDirectory, std::string &error );

// Calls `onInterrupt` on SIGINT and SIGTERM. The function runs in signal
// context, so it must be async-signal-safe (a lock-free atomic store).
void InstallInterruptHandler( void ( *onInterrupt )() );

} // namespace platform

#endif // PLATFORM_POSIX_PROCESS_EXEC_H
