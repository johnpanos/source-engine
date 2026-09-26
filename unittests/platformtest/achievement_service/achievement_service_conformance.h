//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for platform.achievement-service.v1
//			(platform::IAchievementService). A provider is driven through a
//			scripted platform (FakeAchievementPlatform) and its sign-in hook,
//			and judged by what reaches the platform. The sensitivity suite
//			feeds it broken providers.
//
//			Contract doc:
//			unittests/platformtest/contracts/platform.achievement-service.v1.md
//
//=============================================================================//

#ifndef PLATFORMTEST_ACHIEVEMENT_SERVICE_CONFORMANCE_H
#define PLATFORMTEST_ACHIEVEMENT_SERVICE_CONFORMANCE_H

#include "fake_achievement_platform.h"
#include "platform/contracts/achievement_service.h"

#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace platformtest
{

struct AchievementReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define AS_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// One provider instance over one scripted platform.
struct AchievementSubject
{
	std::unique_ptr<FakeAchievementPlatform> platform;
	std::unique_ptr<platform::IAchievementService> service;
	std::function<void( bool signedIn )> setSignedIn;
};

using AchievementSubjectFactory = std::function<AchievementSubject()>;

inline AchievementReport RunAchievementServiceConformance( const AchievementSubjectFactory &make )
{
	using Mode = FakeAchievementPlatform::Mode;
	AchievementReport r;

	// Clauses 1-4: invalid IDs, signed-out waiting, sign-in delivery, no
	// resubmission once accepted.
	{
		AchievementSubject s = make();
		FakeAchievementPlatform &p = *s.platform;
		platform::IAchievementService &service = *s.service;

		const std::string invalid[] = { "", "has space", "slash/id", "\xC3\xA9",
			std::string( platform::kMaxAchievementIdLength + 1, 'A' ) };
		for ( const std::string &id : invalid )
			AS_CHECK( r, !service.ReportCompleted( id ) );
		AS_CHECK( r, platform::IsValidAchievementId( "PORTAL_GET_PORTALGUNS" ) );

		AS_CHECK( r, service.ReportCompleted( "PORTAL_GET_PORTALGUNS" ) );
		AS_CHECK( r, p.TotalSubmitted() == 0 );
		AS_CHECK( r, !service.ShowAchievements() );
		AS_CHECK( r, p.Presented() == 0 );
		AS_CHECK( r, !service.AnnouncesCompletions() );

		s.setSignedIn( true );
		AS_CHECK( r, p.Submitted( "PORTAL_GET_PORTALGUNS" ) == 1 );
		AS_CHECK( r, p.TotalSubmitted() == 1 );
		AS_CHECK( r, service.AnnouncesCompletions() );

		AS_CHECK( r, service.ReportCompleted( "PORTAL_GET_PORTALGUNS" ) );
		s.setSignedIn( true );
		AS_CHECK( r, p.Submitted( "PORTAL_GET_PORTALGUNS" ) == 1 );

		// A new report while signed in goes at once.
		AS_CHECK( r, service.ReportCompleted( "PORTAL_BEAT_GAME" ) );
		AS_CHECK( r, p.Submitted( "PORTAL_BEAT_GAME" ) == 1 );
		AS_CHECK( r, p.TotalSubmitted() == 2 );
	}

	// Clause 5: a refused delivery is retried at the next report and the next
	// sign-in, and not before.
	{
		AchievementSubject s = make();
		FakeAchievementPlatform &p = *s.platform;
		s.setSignedIn( true );
		p.SetMode( Mode::kRefuse );
		AS_CHECK( r, s.service->ReportCompleted( "A" ) );
		AS_CHECK( r, p.Submitted( "A" ) == 1 );
		p.SetMode( Mode::kAccept );
		AS_CHECK( r, s.service->ReportCompleted( "B" ) );
		AS_CHECK( r, p.Submitted( "A" ) == 2 );
		AS_CHECK( r, p.Submitted( "B" ) == 1 );

		p.SetMode( Mode::kRefuse );
		AS_CHECK( r, s.service->ReportCompleted( "C" ) );
		p.SetMode( Mode::kAccept );
		s.setSignedIn( true );
		AS_CHECK( r, p.Submitted( "C" ) == 2 );
		AS_CHECK( r, p.Submitted( "A" ) == 2 );
	}

	// Clause 6: an ID in flight is not submitted again; its completion
	// decides; a completion after the service is gone is harmless.
	{
		AchievementSubject s = make();
		FakeAchievementPlatform &p = *s.platform;
		s.setSignedIn( true );
		p.SetMode( Mode::kHold );
		AS_CHECK( r, s.service->ReportCompleted( "HELD" ) );
		AS_CHECK( r, s.service->ReportCompleted( "HELD" ) );
		AS_CHECK( r, p.Submitted( "HELD" ) == 1 );
		p.SetMode( Mode::kAccept );
		p.CompleteHeld( true );
		AS_CHECK( r, s.service->ReportCompleted( "HELD" ) );
		AS_CHECK( r, p.Submitted( "HELD" ) == 1 );

		p.SetMode( Mode::kHold );
		AS_CHECK( r, s.service->ReportCompleted( "LATE" ) );
		s.service.reset();
		p.CompleteHeld( true );
		AS_CHECK( r, p.Submitted( "LATE" ) == 1 );
	}

	// Clause 7: the achievements screen and completion notices follow sign-in
	// and the platform.
	{
		AchievementSubject s = make();
		FakeAchievementPlatform &p = *s.platform;
		s.setSignedIn( true );
		AS_CHECK( r, s.service->ShowAchievements() );
		AS_CHECK( r, p.Presented() == 1 );
		p.SetPresentResult( false );
		AS_CHECK( r, !s.service->ShowAchievements() );
		AS_CHECK( r, p.Presented() == 2 );
		p.SetAnnounces( false );
		AS_CHECK( r, !s.service->AnnouncesCompletions() );

		// Clause 8: signing out holds reports until the next sign-in.
		p.SetAnnounces( true );
		s.setSignedIn( false );
		AS_CHECK( r, !s.service->AnnouncesCompletions() );
		AS_CHECK( r, !s.service->ShowAchievements() );
		AS_CHECK( r, p.Presented() == 2 );
		AS_CHECK( r, s.service->ReportCompleted( "WHILE_OUT" ) );
		AS_CHECK( r, p.Submitted( "WHILE_OUT" ) == 0 );
		s.setSignedIn( true );
		AS_CHECK( r, p.Submitted( "WHILE_OUT" ) == 1 );
	}

	// Clause 9: concurrent reports, each delivered exactly once, with the
	// platform completing inside Submit.
	{
		AchievementSubject s = make();
		FakeAchievementPlatform &p = *s.platform;
		s.setSignedIn( true );
		const int threads = 4;
		const int perThread = 50;
		std::vector<std::thread> workers;
		for ( int t = 0; t < threads; ++t )
		{
			workers.emplace_back( [&, t] {
				for ( int i = 0; i < perThread; ++i )
				{
					// Each ID twice, and every thread reports a shared one.
					const std::string id = "T" + std::to_string( t ) + "_" + std::to_string( i );
					s.service->ReportCompleted( id );
					s.service->ReportCompleted( id );
					s.service->ReportCompleted( "SHARED" );
				}
			} );
		}
		for ( std::thread &w : workers )
			w.join();
		bool eachOnce = true;
		for ( int t = 0; t < threads; ++t )
		{
			for ( int i = 0; i < perThread; ++i )
				eachOnce &= p.Submitted( "T" + std::to_string( t ) + "_" + std::to_string( i ) ) == 1;
		}
		AS_CHECK( r, eachOnce );
		AS_CHECK( r, p.Submitted( "SHARED" ) == 1 );
		AS_CHECK( r, p.TotalSubmitted() == threads * perThread + 1 );
	}
	return r;
}

inline int ReportAchievementVariant( const char *name, const AchievementReport &r, int &checks,
    int &failures )
{
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_ACHIEVEMENT_SERVICE_CONFORMANCE_H
