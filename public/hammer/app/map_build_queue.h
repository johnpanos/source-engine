//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One map build at a time, off the editing thread (RFC 0002
//			hammer.app; the first product consumer of platform.task-runner.v1).
//			The editor saves on its own thread, then starts the compile here: the
//			builder runs on the work runner, and its result is delivered on the
//			reply runner (the editor's thread), so a front end never blocks on a
//			compile.
//
//			Threading: Start, Busy and destruction happen on the reply runner's
//			sequence. IMapBuilder::Build runs on the work runner, one build at a
//			time. Destroying the queue with a build in flight is safe: the build
//			finishes on the work runner and its reply is dropped.
//
//=============================================================================//

#ifndef HAMMER_APP_MAP_BUILD_QUEUE_H
#define HAMMER_APP_MAP_BUILD_QUEUE_H

#include "hammer/ports/map_builder.h"
#include "platform/contracts/task_runner.h"

#include <functional>
#include <memory>

namespace hammer::app
{

enum class BuildStart
{
	kStarted,
	kBusy,        // a build is already running; nothing was started
	kUnavailable, // the work runner refused the build (shut down)
};

class MapBuildQueue
{
public:
	using Done = std::function<void( const ports::MapBuildResult & )>;

	// Borrows all three; the builder and both runners must outlive every build
	// this queue starts (the owner shuts the work runner down first).
	MapBuildQueue( ports::IMapBuilder &builder, platform::ITaskRunner &work,
	    platform::ISequencedTaskRunner &reply );
	~MapBuildQueue();

	MapBuildQueue( const MapBuildQueue & ) = delete;
	MapBuildQueue &operator=( const MapBuildQueue & ) = delete;

	// Starts compiling an already-saved map. `done` runs on the reply runner
	// with the result, unless the queue was destroyed first.
	[[nodiscard]] BuildStart Start( const ports::MapBuildRequest &request, Done done );
	bool Busy() const;

	struct State;

private:
	std::shared_ptr<State> m_state;
};

} // namespace hammer::app

#endif // HAMMER_APP_MAP_BUILD_QUEUE_H
