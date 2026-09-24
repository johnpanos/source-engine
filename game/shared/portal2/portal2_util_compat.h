//========= Portal 2 port ======================================================//
//
// Purpose: Later (CS:GO-era) base-game helpers the imported Portal 2 sources
//          use, which need the complete base game declarations. Both cbase.h
//          files include this header last when PORTAL2 is defined.
//
//=============================================================================//

#ifndef PORTAL2_UTIL_COMPAT_H
#define PORTAL2_UTIL_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "portal2_shared_compat.h"
#include "tier3/tier3.h"
#ifdef CLIENT_DLL
// Client adapters for IVEngineClient calls this engine lacks (Portal2Engine::).
#include "portal2_engine_compat.h"
#endif

//-----------------------------------------------------------------------------
// Later base-game helpers (CS:GO mathlib.h and util_shared.h)
//-----------------------------------------------------------------------------

// Inclusive point-in-box test.
inline bool IsPointInBounds( const Vector &vPoint, const Vector &vMin, const Vector &vMax )
{
	return vPoint.x >= vMin.x && vPoint.x <= vMax.x &&
		   vPoint.y >= vMin.y && vPoint.y <= vMax.y &&
		   vPoint.z >= vMin.z && vPoint.z <= vMax.z;
}

// CTraceFilterSimple that also skips every player.
class CTraceFilterNoPlayers : public CTraceFilterSimple
{
public:
	CTraceFilterNoPlayers( const IHandleEntity *passentity = NULL, int collisionGroup = COLLISION_GROUP_NONE )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );
};

// CS:GO vmatrix.h: the rotation taking the unit vector vFromAxis onto the unit
// vector vToAxis (identity when they are parallel, as in CS:GO).
VMatrix SetupMatrixAxisToAxisRot( const Vector &vFromAxis, const Vector &vToAxis );

// CS:GO collisionutils.h: intersects a ray with a z-aligned cylinder centered
// at center. Fills pTrace (endpos, fraction, contents, normal) on a hit.
bool IntersectRayWithAACylinder( const Ray_t &ray, const Vector &center, float radius, float height, CBaseTrace *pTrace );

// CS:GO tier0/basetypes.h integer helpers.
FORCEINLINE int imax( int x, int y ) { return x > y ? x : y; }
FORCEINLINE int imin( int x, int y ) { return x < y ? x : y; }
FORCEINLINE int isel( int a, int x, int y ) { return a >= 0 ? x : y; }

class IRecipientFilter;

// CS:GO particle_parse.h overload: dispatch a named particle system to the
// given recipients (all PAS recipients when filter is NULL). This engine has
// one local player per client, so nSplitScreenPlayerSlot is ignored.
void DispatchParticleEffect( const char *pszParticleName, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pEntity,
							 int nSplitScreenPlayerSlot, IRecipientFilter *filter );

// UTIL_CalcFrustumThroughConvexPolygon (CS:GO util_shared.h).
#include "portal2_frustum_util.h"

#ifdef CLIENT_DLL
// UTIL_Remove( C_BaseEntity * ) is in portal2_engine_compat.h.

// CS:GO util_shared.h: the height of the water surface in the column at
// position between minz and maxz (the server has had this in util.h).
float UTIL_FindWaterSurface( const Vector &position, float minz, float maxz );

// Split screen: this engine has one local player, so there is no split-screen
// VGui and no remote player views to register (C_BasePlayer statics in CS:GO).
inline bool VGui_IsSplitScreen() { return false; }
inline void AddRemoteSplitScreenViewPlayer( C_BasePlayer *pPlayer ) {}
inline void RemoveRemoteSplitScreenViewPlayer( C_BasePlayer *pPlayer ) {}
#endif // CLIENT_DLL

#ifdef GAME_DLL
// CS:GO util.h: the single-player local player, or the listen server host in
// multiplayer (NULL on a dedicated server).
inline CBasePlayer *UTIL_GetLocalPlayerOrListenServerHost( void )
{
	if ( gpGlobals->maxClients > 1 )
	{
		if ( engine->IsDedicatedServer() )
			return NULL;
		return UTIL_GetListenServerHost();
	}
	return UTIL_GetLocalPlayer();
}

// CS:GO util.h: sends a Steam achievement or stat event to the player's
// client. This build has no Steam achievement backend for Portal 2's stat
// names, so the event is reported once as unsupported and otherwise dropped.
void UTIL_RecordAchievementEvent( const char *pszAchievementname, CBasePlayer *pPlayer = NULL );
#endif

#endif // PORTAL2_UTIL_COMPAT_H
