//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portable provider of platform.achievement-service.v1
//			(platform.achievements) over a native achievement platform such as
//			Game Center. It owns the contract's delivery rules: reports made
//			while signed out wait for sign-in, each ID is delivered until the
//			platform accepts it once, and a failed delivery is retried at the
//			next sign-in or report. The native bridge only submits, presents
//			and tells it the sign-in state.
//
//			Platform completions may arrive on any thread, after the service is
//			gone; they hold the queue state by shared ownership for that reason
//			and touch nothing else.
//
//=============================================================================//

#ifndef PLATFORM_ACHIEVEMENTS_QUEUED_ACHIEVEMENT_SERVICE_H
#define PLATFORM_ACHIEVEMENTS_QUEUED_ACHIEVEMENT_SERVICE_H

#include "platform/contracts/achievement_service.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace platform
{

// The native half of QueuedAchievementService.
class IAchievementPlatform
{
public:
	using Completion = std::function<void( bool accepted )>;

	virtual ~IAchievementPlatform() = default;

	// Delivers completed achievements, each a valid ID, never an empty batch.
	// Must not wait for the platform. The platform calls `done` exactly once,
	// from any thread, possibly before Submit returns; `accepted` says whether
	// it recorded the whole batch.
	virtual void Submit( std::vector<std::string> ids, Completion done ) = 0;

	// Presents the platform's achievements screen; false if it cannot.
	virtual bool Present() = 0;

	// Whether the platform announces each achievement it accepts (Game
	// Center's completion banner). Constant.
	virtual bool AnnouncesCompletions() const = 0;
};

class QueuedAchievementService final : public IAchievementService
{
public:
	// `platform` is borrowed and must outlive the service.
	explicit QueuedAchievementService( IAchievementPlatform &platform );
	~QueuedAchievementService() override;

	QueuedAchievementService( const QueuedAchievementService & ) = delete;
	QueuedAchievementService &operator=( const QueuedAchievementService & ) = delete;

	// The platform's sign-in state; the bridge calls it whenever it changes,
	// from any thread. Signing in delivers what is waiting.
	void SetSignedIn( bool signedIn );

	bool ReportCompleted( std::string_view id ) override;
	bool ShowAchievements() override;
	bool AnnouncesCompletions() const override;

private:
	struct State;

	// Submits what is waiting if the player is signed in.
	void Deliver();

	IAchievementPlatform &m_platform;
	const std::shared_ptr<State> m_state;
};

} // namespace platform

#endif // PLATFORM_ACHIEVEMENTS_QUEUED_ACHIEVEMENT_SERVICE_H
