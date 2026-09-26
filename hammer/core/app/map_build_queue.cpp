//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One map build at a time, off the editing thread.
//
//=============================================================================//

#include "hammer/app/map_build_queue.h"

#include "platform/contracts/sequence_checker.h"

#include <atomic>
#include <cstdlib>

namespace hammer::app
{

// Shared by the queue and its in-flight build tasks, so the queue can be
// destroyed while a build runs (the destruction context of the state is the
// last of the queue and its tasks). `busy` belongs to the reply sequence;
// `alive` is cleared by the destructor, which may run on the owner's thread.
struct MapBuildQueue::State
{
	State( ports::IMapBuilder &b, platform::ITaskRunner &w, platform::ISequencedTaskRunner &r )
	    : builder( b ), work( w ), reply( r )
	{
	}

	ports::IMapBuilder &builder;
	platform::ITaskRunner &work;
	platform::ISequencedTaskRunner &reply;
	bool busy = false;
	std::atomic<bool> alive{ true };
	platform::SequenceChecker replySequence;
};

MapBuildQueue::MapBuildQueue( ports::IMapBuilder &builder, platform::ITaskRunner &work,
    platform::ISequencedTaskRunner &reply )
    : m_state( std::make_shared<State>( builder, work, reply ) )
{
}

MapBuildQueue::~MapBuildQueue()
{
	m_state->alive = false;
}

bool MapBuildQueue::Busy() const
{
	PLATFORM_CHECK_SEQUENCE( m_state->replySequence );
	return m_state->busy;
}

BuildStart MapBuildQueue::Start( const ports::MapBuildRequest &request, Done done )
{
	if ( !m_state->reply.RunsTasksInCurrentSequence() )
		std::abort(); // a front end starts builds from its own (reply) sequence
	PLATFORM_CHECK_SEQUENCE( m_state->replySequence );
	if ( m_state->busy )
		return BuildStart::kBusy;
	std::shared_ptr<State> state = m_state;
	const platform::PostResult posted = state->work.PostTask(
	    [state, request, done = std::move( done )]() mutable
	    {
		    ports::MapBuildResult result = state->builder.Build( request );
		    // If the reply runner is gone, so is the front end: nothing to tell.
		    (void)state->reply.PostTask(
		        [state, result = std::move( result ), done = std::move( done )]
		        {
			        if ( !state->alive )
				        return;
			        PLATFORM_CHECK_SEQUENCE( state->replySequence );
			        state->busy = false;
			        if ( done )
				        done( result );
		        } );
	    } );
	if ( posted != platform::PostResult::kAccepted )
		return BuildStart::kUnavailable;
	m_state->busy = true;
	return BuildStart::kStarted;
}

} // namespace hammer::app
