//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: platform.process-spawn.v1: start long-running programs that keep
//			their own output (games, compositors), for run providers (RFC
//			0027 L1). The synchronous tool-process contract is for tools whose
//			output the caller collects; this one is for programs it waits on.
//
//			Every spawned process is the leader of its own process group, so
//			Terminate stops what it started too. A spawner keeps no global
//			state; one spawner may be used from one thread at a time.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_PROCESS_SPAWN_H
#define PLATFORM_CONTRACTS_PROCESS_SPAWN_H

#include "platform/contracts/tool_process.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace platform
{

struct SpawnRequest
{
	// argv[0] is the program: a path (relative to workingDirectory when it
	// contains a '/') or a name found on PATH.
	std::vector<std::string> argv;
	std::string workingDirectory;
	// Changes to the inherited environment; "{inherit}" in a value is the
	// variable's inherited value.
	std::vector<ToolProcessEnvironmentOverride> environment;
};

struct SpawnedProcess
{
	std::int64_t id = -1; // the provider's handle; -1 when nothing started
};

class IProcessSpawner
{
public:
	virtual ~IProcessSpawner() = default;
	// Starts the program; on failure returns an invalid handle and says why.
	virtual SpawnedProcess Spawn( const SpawnRequest &request, std::string &error ) = 0;
	// The exit status once it has ended (a signal is 128 + its number).
	virtual std::optional<int> Poll( SpawnedProcess process ) = 0;
	virtual int Wait( SpawnedProcess process ) = 0;
	// Asks the process group to stop, then forces it after `graceMs`; reaps it.
	virtual void Terminate( SpawnedProcess process, int graceMs ) = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_PROCESS_SPAWN_H
