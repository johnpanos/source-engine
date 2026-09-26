//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portable provider of platform.achievement-service.v1; see
//			queued_achievement_service.h.
//
//=============================================================================//

#include "queued_achievement_service.h"

#include <mutex>
#include <set>
#include <utility>

namespace platform
{

// Every ID is in at most one of the three sets.
struct QueuedAchievementService::State
{
	std::mutex mutex;
	bool signedIn = false;
	// Reported, not yet submitted (or submitted and refused).
	std::set<std::string, std::less<>> waiting;
	// Submitted, completion not yet received.
	std::set<std::string, std::less<>> inFlight;
	// Accepted by the platform; later reports of these are no-ops.
	std::set<std::string, std::less<>> accepted;
};

QueuedAchievementService::QueuedAchievementService( IAchievementPlatform &platform )
    : m_platform( platform ), m_state( std::make_shared<State>() )
{
}

QueuedAchievementService::~QueuedAchievementService() = default;

void QueuedAchievementService::SetSignedIn( bool signedIn )
{
	{
		std::lock_guard<std::mutex> lock( m_state->mutex );
		m_state->signedIn = signedIn;
	}
	Deliver();
}

bool QueuedAchievementService::ReportCompleted( std::string_view id )
{
	if ( !IsValidAchievementId( id ) )
		return false;
	{
		std::lock_guard<std::mutex> lock( m_state->mutex );
		if ( !m_state->accepted.contains( id ) && !m_state->inFlight.contains( id ) )
			m_state->waiting.emplace( id );
	}
	// Also retries earlier refused deliveries.
	Deliver();
	return true;
}

bool QueuedAchievementService::ShowAchievements()
{
	{
		std::lock_guard<std::mutex> lock( m_state->mutex );
		if ( !m_state->signedIn )
			return false;
	}
	return m_platform.Present();
}

bool QueuedAchievementService::AnnouncesCompletions() const
{
	std::lock_guard<std::mutex> lock( m_state->mutex );
	return m_state->signedIn && m_platform.AnnouncesCompletions();
}

void QueuedAchievementService::Deliver()
{
	std::vector<std::string> batch;
	{
		std::lock_guard<std::mutex> lock( m_state->mutex );
		if ( !m_state->signedIn || m_state->waiting.empty() )
			return;
		for ( const std::string &id : m_state->waiting )
		{
			batch.push_back( id );
			m_state->inFlight.insert( id );
		}
		m_state->waiting.clear();
	}

	// Outside the lock: the platform may complete before Submit returns.
	std::vector<std::string> submitted = batch;
	m_platform.Submit( std::move( batch ),
	    [state = m_state, ids = std::move( submitted )]( bool accepted ) {
		    std::lock_guard<std::mutex> lock( state->mutex );
		    for ( const std::string &id : ids )
		    {
			    state->inFlight.erase( id );
			    if ( accepted )
				    state->accepted.insert( id );
			    else
				    state->waiting.insert( id );
		    }
	    } );
}

} // namespace platform
