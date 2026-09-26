//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for a platform achievement service
//			(platform.achievement-service.v1), such as Game Center on iOS and
//			tvOS: it receives the achievements the player completes and can
//			show the platform's own achievements screen.
//
//			The game's achievement manager stays the authority for what the
//			player has earned; the service mirrors it. The application root
//			selects the provider; products without one bind none.
//
//			Contract doc:
//			unittests/platformtest/contracts/platform.achievement-service.v1.md
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_ACHIEVEMENT_SERVICE_H
#define PLATFORM_CONTRACTS_ACHIEVEMENT_SERVICE_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros.
#include <cstddef>
#include <string_view>

namespace platform
{

// The longest achievement ID a service accepts.
inline constexpr std::size_t kMaxAchievementIdLength = 100;

// IDs are 1 to kMaxAchievementIdLength ASCII letters, digits, '_', '-' and
// '.': the game's achievement names (PORTAL_GET_PORTALGUNS), which the
// platform's achievement definitions use unchanged. This is the one
// definition of a valid ID.
constexpr bool IsValidAchievementId( std::string_view id )
{
	if ( id.empty() || id.size() > kMaxAchievementIdLength )
		return false;
	for ( const char c : id )
	{
		const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
		                ( c >= '0' && c <= '9' ) || c == '_' || c == '-' || c == '.';
		if ( !ok )
			return false;
	}
	return true;
}

class IAchievementService
{
public:
	virtual ~IAchievementService() = default;

	// Records that the player completed achievement `id`. Returns false, and
	// records nothing, for an invalid ID. Otherwise the service delivers it to
	// the platform: at once when the player is signed in, else when they sign
	// in. A delivery the platform refuses is retried at the next sign-in or
	// report; once the platform accepts an ID, reporting it again delivers
	// nothing. Any thread; never waits for the platform.
	virtual bool ReportCompleted( std::string_view id ) = 0;

	// Shows the platform's achievements screen over the game. Returns false,
	// showing nothing, when it cannot now (no signed-in player); the caller
	// then shows its own screen. Call from the thread that owns the app's UI.
	virtual bool ShowAchievements() = 0;

	// Whether an achievement reported now is announced to the player by the
	// platform itself (Game Center's completion banner), so the game should
	// not show its own notice. False while no player is signed in. Any thread.
	virtual bool AnnouncesCompletions() const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_ACHIEVEMENT_SERVICE_H
