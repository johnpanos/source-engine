//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A game module's platform-service binding; see
//			public/game/game_platform_services.h. Compiled into the client,
//			server and GameUI modules, each with its own binding.
//
//=============================================================================//

#include "game/game_platform_services.h"

#include "platform/contracts/achievement_service.h"

#include "tier1/interface.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

namespace
{

// The root binds before the engine starts the modules, and the modules read
// the services on the main thread, so no locking is needed.
GamePlatformServices g_Services;
bool g_bBound = false;
bool g_bUsed = false;

class CGamePlatformServicesBinding : public IGamePlatformServicesBinding
{
public:
	bool Bind( const GamePlatformServices &services ) override
	{
		if ( g_bBound || g_bUsed )
			return false;
		g_Services = services;
		g_bBound = true;
		return true;
	}
};

CGamePlatformServicesBinding g_Binding;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CGamePlatformServicesBinding, IGamePlatformServicesBinding,
    GAME_PLATFORM_SERVICES_BINDING_VERSION, g_Binding );

platform::IRecordStore *GamePlatformServices_PlayerRecords()
{
	g_bUsed = true;
	return g_Services.playerRecords;
}

platform::IAchievementService *GamePlatformServices_Achievements()
{
	g_bUsed = true;
	return g_Services.achievements;
}

bool GamePlatformServices_ShowAchievements()
{
	platform::IAchievementService *pService = GamePlatformServices_Achievements();
	return pService && pService->ShowAchievements();
}
