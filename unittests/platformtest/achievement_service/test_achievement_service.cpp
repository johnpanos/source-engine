//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for platform.achievement-service.v1:
//			the shared suite against QueuedAchievementService
//			(platform/achievements), the provider the Game Center bridge
//			completes, over a scripted platform.
//
//			On Linux and, through tools/quality/ios_conformance.py, on iOS and
//			tvOS devices.
//
//			Build/run: tools/quality/conformance.py check --suite platform.achievement_service
//
//=============================================================================//

#include "achievement_service_conformance.h"
#include "testing/conformance_result.h"

int main()
{
	int checks = 0;
	int failures = 0;
	platformtest::AchievementSubjectFactory make = [] {
		platformtest::AchievementSubject s;
		s.platform = std::make_unique<platformtest::FakeAchievementPlatform>();
		auto service = std::make_unique<platform::QueuedAchievementService>( *s.platform );
		platform::QueuedAchievementService *raw = service.get();
		s.service = std::move( service );
		s.setSignedIn = [raw]( bool signedIn ) { raw->SetSignedIn( signedIn ); };
		return s;
	};
	platformtest::ReportAchievementVariant( "achievement_service[queued]",
	    platformtest::RunAchievementServiceConformance( make ), checks, failures );
	return testing::ReportConformance( checks, failures );
}
