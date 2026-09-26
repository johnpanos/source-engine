# Contract: `platform.achievement-service.v1`

Module: `platform.contracts` · Types: `platform::IAchievementService`,
`platform::IsValidAchievementId`
Header: `public/platform/contracts/achievement_service.h`
Shared suite: `unittests/platformtest/achievement_service/achievement_service_conformance.h`
Conformance: `unittests/platformtest/achievement_service/test_achievement_service.cpp`
(+ `_negative`), `test_achievement_service_game_center.mm` (Apple devices)
Test backend: `unittests/platformtest/achievement_service/fake_achievement_platform.h`
RFC: 0001 (platform service) · Migration: `PLAT-RECORDS-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, providers

A platform achievement service mirrors the achievements the player completes
to the platform (Game Center on iOS and tvOS), shows the platform's
achievements screen, and says whether the platform announces completions
itself. The game's achievement manager stays the authority for what was
earned. Products without a service bind none; every clause below is then
moot and the game shows its own screen and notices.

- Provider: `QueuedAchievementService` (`platform.achievements`), which owns
  the delivery rules, over an `IAchievementPlatform`. The Game Center bridge
  `GameCenterAchievements` (`platform.apple`) implements the platform: it
  authenticates, reports each achievement 100% complete with Game Center's
  completion banner, and opens the achievements screen through `GKAccessPoint`.
- Consumers (through `public/game/game_platform_services.h`):
  - `CAchievementMgr` reports each award and, after each load, every earned
    achievement;
  - `CAchievementNotificationPanel` hides the game's "achievement awarded"
    notice while `AnnouncesCompletions()`;
  - GameUI's `OpenAchievementsDialog` shows the platform screen when
    `ShowAchievements()` succeeds and the game's dialog otherwise.
- Game Center IDs are the game's achievement names unchanged
  (`PORTAL_GET_PORTALGUNS`); App Store Connect must define them.

## 2. Accepted inputs

IDs: 1 to 100 ASCII letters, digits, `_`, `-`, `.` (`IsValidAchievementId`).

## 3. Results and guarantees

1. `ReportCompleted` returns false and records nothing for an invalid ID.
2. While no player is signed in, reports wait: nothing reaches the platform,
   `ShowAchievements()` is false and presents nothing, and
   `AnnouncesCompletions()` is false.
3. Signing in delivers what is waiting; a report while signed in is delivered
   at once.
4. Once the platform accepts an ID, reporting it again delivers nothing.
5. A refused delivery is retried at the next report and the next sign-in, and
   not in a loop.
6. An ID in flight is not submitted again; a completion arriving after the
   service is destroyed is harmless.
7. Signed in, `ShowAchievements()` returns the platform's result, and
   `AnnouncesCompletions()` is the platform's banner setting.
8. Signing out makes reports wait again.
9. Concurrent reports from several threads are each delivered once, including
   when the platform completes inside `Submit`.

`ReportCompleted` never waits for the platform. `ShowAchievements` is called
on the thread that owns the app's UI (the main thread on iOS and tvOS).

## 4. Evidence

- `platform.achievement_service`: the shared suite against
  `QueuedAchievementService` over the scripted platform.
- `platform.achievement_service.sensitivity`: nine broken services, each rejected.
- `platform.achievement_service.game_center` (profile `apple-uikit-device`):
  the bridge on a real device. The conformance app has no Game Center
  entitlement, so it covers the signed-out path; signed-in behavior needs the
  entitled product app, a paid developer team and the achievements defined in
  App Store Connect.
