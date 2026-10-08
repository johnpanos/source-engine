//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX platform.process-spawn.v1 provider (application roots only).
//
//=============================================================================//

#ifndef PLATFORM_POSIX_PROCESS_SPAWNER_H
#define PLATFORM_POSIX_PROCESS_SPAWNER_H

#include "platform/contracts/process_spawn.h"

#include <memory>

namespace platform
{

[[nodiscard]] std::unique_ptr<IProcessSpawner> CreatePosixProcessSpawner();

} // namespace platform

#endif // PLATFORM_POSIX_PROCESS_SPAWNER_H
