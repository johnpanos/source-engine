//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game Center achievement platform (platform.apple) for
//			QueuedAchievementService on iOS and tvOS.
//
//			Authenticate installs GameKit's authentication handler. When Game
//			Center hands back a sign-in screen, the bridge presents it on the
//			app's key window as soon as one exists. Each achievement is
//			reported complete (100%) with Game Center's completion banner, so
//			AnnouncesCompletions is true. Present opens Game Center's
//			achievements screen through the access point.
//
//			GameKit needs the com.apple.developer.game-center entitlement and
//			the achievements defined for the app in App Store Connect under
//			the game's achievement names. Without the entitlement,
//			authentication fails and the player stays signed out, so the game
//			keeps its own screen and notices.
//
//			Construct, Authenticate and Present on the main thread (UIKit).
//
//=============================================================================//

#ifndef PLATFORM_APPLE_GAME_CENTER_ACHIEVEMENTS_H
#define PLATFORM_APPLE_GAME_CENTER_ACHIEVEMENTS_H

#include "../achievements/queued_achievement_service.h"

#include <functional>

namespace platform
{

class GameCenterAchievements final : public IAchievementPlatform
{
public:
	using SignInChanged = std::function<void( bool signedIn )>;

	// Starts Game Center authentication. `signInChanged` is called on the
	// main thread each time the sign-in state changes; it must stay callable
	// for the life of the app. Call once.
	void Authenticate( SignInChanged signInChanged );

	void Submit( std::vector<std::string> ids, Completion done ) override;
	bool Present() override;
	bool AnnouncesCompletions() const override;
};

// Diagnostics for tools/quality/game_center_e2e.py, independent of the
// reporting path: waits up to `timeoutSeconds` for a signed-in player, then
// logs what Game Center itself holds for this app (its achievement
// descriptions, and the player's achievements with percentComplete) as
// "Source: game-center-audit:" lines. Main thread; pumps the run loop.
void AuditGameCenter( double timeoutSeconds );

} // namespace platform

#endif // PLATFORM_APPLE_GAME_CENTER_ACHIEVEMENTS_H
