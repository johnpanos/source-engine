//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Platform services an application root hands to the game modules
//			(client, server and GameUI): where player records are kept and the
//			platform's achievement service.
//
//			Each module exposes GAME_PLATFORM_SERVICES_BINDING_VERSION through
//			its CreateInterface, the game module ABI. The root binds every
//			module once, before the engine initializes them; the iOS and tvOS
//			roots do (launcher_main/static_composition.cpp). A module that is
//			never bound keeps its defaults: the achievement manager keeps its
//			records in files under the game's write path, and there is no
//			achievement service.
//
//			The bound services are borrowed and must outlive the modules.
//
//=============================================================================//

#ifndef GAME_PLATFORM_SERVICES_H
#define GAME_PLATFORM_SERVICES_H

namespace platform
{
class IAchievementService;
class IRecordStore;
} // namespace platform

struct GamePlatformServices
{
	// Null keeps the consumer's file store.
	platform::IRecordStore *playerRecords = nullptr;
	// Null: the product has no platform achievement service.
	platform::IAchievementService *achievements = nullptr;
};

class IGamePlatformServicesBinding
{
public:
	// Fails if the module was already bound or has already used its services.
	virtual bool Bind( const GamePlatformServices &services ) = 0;
};

#define GAME_PLATFORM_SERVICES_BINDING_VERSION "GamePlatformServicesBinding001"

// The player record that holds the achievement manager's state. Lowercase:
// the engine's POSIX file system has always written it as gamestate.txt in
// the game's write directory, so the file store finds existing state.
#define GAME_STATE_RECORD_KEY "gamestate.txt"

// Inside a game module: the services it was bound to (null where unbound).
// The first call to either fixes them.
platform::IRecordStore *GamePlatformServices_PlayerRecords();
platform::IAchievementService *GamePlatformServices_Achievements();

#endif // GAME_PLATFORM_SERVICES_H
