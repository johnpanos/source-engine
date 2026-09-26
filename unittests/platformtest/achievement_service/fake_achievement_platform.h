//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A scripted achievement platform (the native half of
//			QueuedAchievementService) for the platform.achievement-service.v1
//			suite: it records every submission and completes each one by
//			accepting, refusing or holding it for the test to finish later.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_ACHIEVEMENT_PLATFORM_H
#define PLATFORMTEST_FAKE_ACHIEVEMENT_PLATFORM_H

#include "../../../platform/achievements/queued_achievement_service.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace platformtest
{

class FakeAchievementPlatform : public platform::IAchievementPlatform
{
public:
	enum class Mode
	{
		kAccept,
		kRefuse,
		kHold,
	};

	void SetMode( Mode mode )
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		m_mode = mode;
	}

	void SetPresentResult( bool result ) { m_presentResult = result; }
	void SetAnnounces( bool announces ) { m_announces = announces; }

	void Submit( std::vector<std::string> ids, Completion done ) override
	{
		Mode mode;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			for ( const std::string &id : ids )
				++m_submitted[id];
			++m_batches;
			mode = m_mode;
			if ( mode == Mode::kHold )
			{
				m_held.push_back( std::move( done ) );
				return;
			}
		}
		// Completes before Submit returns, which the provider must allow.
		done( mode == Mode::kAccept );
	}

	bool Present() override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		++m_presented;
		return m_presentResult;
	}

	bool AnnouncesCompletions() const override { return m_announces; }

	// Completes every held submission.
	void CompleteHeld( bool accepted )
	{
		std::vector<Completion> held;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			held.swap( m_held );
		}
		for ( Completion &done : held )
			done( accepted );
	}

	int Submitted( const std::string &id ) const
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		auto it = m_submitted.find( id );
		return it == m_submitted.end() ? 0 : it->second;
	}

	int TotalSubmitted() const
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		int total = 0;
		for ( const auto &entry : m_submitted )
			total += entry.second;
		return total;
	}

	int Presented() const
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		return m_presented;
	}

private:
	mutable std::mutex m_mutex;
	Mode m_mode = Mode::kAccept;
	bool m_presentResult = true;
	bool m_announces = true;
	std::map<std::string, int> m_submitted;
	std::vector<Completion> m_held;
	int m_batches = 0;
	int m_presented = 0;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_ACHIEVEMENT_PLATFORM_H
