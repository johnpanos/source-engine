//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the platform.achievement-service.v1 suite:
//			the SAME shared predicate must reject each broken service (one
//			defect each) while a defect-free build of the same service passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.achievement_service.sensitivity
//
//=============================================================================//

#include "achievement_service_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <mutex>
#include <set>

namespace
{

enum class Defect
{
	kNone,
	kAcceptsInvalidIds,
	kSubmitsWhileSignedOut,
	kDropsWhileSignedOut,
	kNeverRetries,
	kResubmitsAccepted,
	kResubmitsInFlight,
	kShowsWhileSignedOut,
	kAnnouncesWhileSignedOut,
	kIgnoresPlatformBanner,
};

// A small service with the contract's rules and one switchable defect. It
// never submits under its lock, since the platform may complete inside Submit.
class BrokenService : public platform::IAchievementService
{
public:
	BrokenService( platform::IAchievementPlatform &platform, Defect defect )
	    : m_platform( platform ), m_defect( defect ), m_state( std::make_shared<State>() )
	{
	}

	void SetSignedIn( bool signedIn )
	{
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			m_state->signedIn = signedIn;
		}
		Deliver();
	}

	bool ReportCompleted( std::string_view id ) override
	{
		if ( m_defect != Defect::kAcceptsInvalidIds && !platform::IsValidAchievementId( id ) )
			return false;
		bool signedIn;
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			signedIn = m_state->signedIn;
			const bool known = ( m_defect != Defect::kResubmitsAccepted && m_state->accepted.count( std::string( id ) ) ) ||
			                   ( m_defect != Defect::kResubmitsInFlight && m_state->inFlight.count( std::string( id ) ) );
			const bool drop = m_defect == Defect::kDropsWhileSignedOut && !signedIn;
			if ( !known && !drop )
				m_state->waiting.insert( std::string( id ) );
		}
		Deliver( m_defect == Defect::kSubmitsWhileSignedOut );
		return true;
	}

	bool ShowAchievements() override
	{
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			if ( !m_state->signedIn && m_defect != Defect::kShowsWhileSignedOut )
				return false;
		}
		return m_platform.Present();
	}

	bool AnnouncesCompletions() const override
	{
		std::lock_guard<std::mutex> lock( m_state->mutex );
		const bool signedIn = m_state->signedIn || m_defect == Defect::kAnnouncesWhileSignedOut;
		const bool banner = m_defect == Defect::kIgnoresPlatformBanner || m_platform.AnnouncesCompletions();
		return signedIn && banner;
	}

private:
	struct State
	{
		std::mutex mutex;
		bool signedIn = false;
		std::set<std::string> waiting, inFlight, accepted;
	};

	void Deliver( bool evenSignedOut = false )
	{
		std::vector<std::string> batch;
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			if ( ( !m_state->signedIn && !evenSignedOut ) || m_state->waiting.empty() )
				return;
			batch.assign( m_state->waiting.begin(), m_state->waiting.end() );
			m_state->inFlight.insert( batch.begin(), batch.end() );
			m_state->waiting.clear();
		}
		std::vector<std::string> ids = batch;
		const bool retry = m_defect != Defect::kNeverRetries;
		m_platform.Submit( std::move( batch ), [state = m_state, ids, retry]( bool accepted ) {
			std::lock_guard<std::mutex> lock( state->mutex );
			for ( const std::string &id : ids )
			{
				state->inFlight.erase( id );
				if ( accepted )
					state->accepted.insert( id );
				else if ( retry )
					state->waiting.insert( id );
			}
		} );
	}

	platform::IAchievementPlatform &m_platform;
	const Defect m_defect;
	const std::shared_ptr<State> m_state;
};

int g_checks = 0;
int g_failures = 0;

void Expect( const char *name, Defect defect, bool shouldPass )
{
	platformtest::AchievementSubjectFactory make = [defect] {
		platformtest::AchievementSubject s;
		s.platform = std::make_unique<platformtest::FakeAchievementPlatform>();
		auto service = std::make_unique<BrokenService>( *s.platform, defect );
		BrokenService *raw = service.get();
		s.service = std::move( service );
		s.setSignedIn = [raw]( bool signedIn ) { raw->SetSignedIn( signedIn ); };
		return s;
	};
	const platformtest::AchievementReport r = platformtest::RunAchievementServiceConformance( make );
	const bool passed = r.failures == 0;
	++g_checks;
	if ( passed != shouldPass )
	{
		++g_failures;
		std::printf( "FAIL %s: expected the suite to %s it (%d/%d checks failed)\n", name,
		    shouldPass ? "pass" : "reject", r.failures, r.checks );
		return;
	}
	if ( shouldPass )
		std::printf( "ok %s: defect-free service passes (%d checks)\n", name, r.checks );
	else
		std::printf( "ok %s: rejected; first failure: %s (line %d)\n", name, r.firstFailure,
		    r.firstFailureLine );
}

} // namespace

int main()
{
	Expect( "defect-free", Defect::kNone, true );
	Expect( "accepts-invalid-ids", Defect::kAcceptsInvalidIds, false );
	Expect( "submits-while-signed-out", Defect::kSubmitsWhileSignedOut, false );
	Expect( "drops-while-signed-out", Defect::kDropsWhileSignedOut, false );
	Expect( "never-retries", Defect::kNeverRetries, false );
	Expect( "resubmits-accepted", Defect::kResubmitsAccepted, false );
	Expect( "resubmits-in-flight", Defect::kResubmitsInFlight, false );
	Expect( "shows-while-signed-out", Defect::kShowsWhileSignedOut, false );
	Expect( "announces-while-signed-out", Defect::kAnnouncesWhileSignedOut, false );
	Expect( "ignores-platform-banner", Defect::kIgnoresPlatformBanner, false );
	return testing::ReportConformance( g_checks, g_failures );
}
