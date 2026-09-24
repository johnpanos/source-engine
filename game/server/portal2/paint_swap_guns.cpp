//========= Portal 2 reconstruction ============================================//
//
// Purpose: Swapping the paint gun and the portal gun between two players.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "paint_swap_guns.h"
#include "portal2/portal/portal_player.h"
#include "portal2/portal/prop_portal.h"
#include "weapon_paintgun.h"
#include "weapon_portalgun.h"
#include "portal_util_shared.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_max_swap_distance( "sv_max_swap_distance", "100", FCVAR_CHEAT );
ConVar sv_max_swap_look_angle( "sv_max_swap_look_angle", "50", FCVAR_CHEAT );
ConVar sv_max_swap_portal_look_angle( "sv_max_swap_portal_look_angle", "50", FCVAR_CHEAT );
ConVar g_debug_swap_guns_trace( "g_debug_swap_guns_trace", "0", FCVAR_CHEAT );

extern void PaintPowerPickup( int colorIndex, CBasePlayer *pPlayer );

enum SwapGunTraceResult
{
	SWAP_TRACE_HIT_NOTHING = 0,
	SWAP_TRACE_HIT_SOMETHING
};


void CheckSwapPaintGun( CBaseCombatWeapon *pWeapon )
{
	if( !pWeapon )
		return;

	// The new owner of a paint gun gets the power it has selected
	CWeaponPaintGun *pPaintGun = dynamic_cast< CWeaponPaintGun* >( pWeapon );
	if( pPaintGun )
	{
		PaintPowerPickup( pPaintGun->GetCurrentPaint(), ToBasePlayer( pPaintGun->GetOwner() ) );
	}
}


void SwapPaintAndPortalGuns( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 )
{
	CBaseCombatWeapon *pWeapon1 = pPlayer1->GetActiveWeapon();
	CBaseCombatWeapon *pWeapon2 = pPlayer2->GetActiveWeapon();

	CWeaponPortalgun *pPortalGun = NULL;

	if( pWeapon1 && FClassnameIs( pWeapon1, "weapon_portalgun" ) )
	{
		pPortalGun = dynamic_cast< CWeaponPortalgun* >( pWeapon1 );
	}
	else if( pWeapon2 && FClassnameIs( pWeapon2, "weapon_portalgun" ) )
	{
		pPortalGun = dynamic_cast< CWeaponPortalgun* >( pWeapon2 );
	}

	// Equipping the portal gun resets its linkage, so remember it
	char linkageID = pPortalGun->GetLinkageGroupID();

	pPlayer1->Weapon_Drop( pWeapon1, NULL, NULL );
	pPlayer2->Weapon_Drop( pWeapon2, NULL, NULL );

	if( pWeapon1 )
	{
		pPlayer2->Weapon_Equip( pWeapon1 );
		CheckSwapPaintGun( pWeapon1 );
	}

	if( pWeapon2 )
	{
		pPlayer1->Weapon_Equip( pWeapon2 );
		CheckSwapPaintGun( pWeapon2 );
	}

	pPortalGun->SetLinkageGroupID( linkageID );
}


SwapGunTraceResult SwapGunsTraceLine( const Vector &vecStartPos, const Vector &vecEndPos, CBaseEntity *pStartEnt, trace_t *trace )
{
	CTraceFilterSimpleList filter( COLLISION_GROUP_PROJECTILE );
	filter.AddEntityToIgnore( pStartEnt );

	UTIL_TraceLine( vecStartPos, vecEndPos, MASK_SOLID, &filter, trace );

	if( g_debug_swap_guns_trace.GetBool() )
	{
		NDebugOverlay::Line( trace->endpos, vecEndPos, 0, 255, 0, true, 0.1f );
		NDebugOverlay::Line( trace->startpos, trace->endpos, 255, 0, 0, true, 0.1f );
		NDebugOverlay::Sphere( vecEndPos, 2.0f, 0, 255, 0, true, 0.1f );
		NDebugOverlay::Sphere( trace->endpos, 2.0f, 0, 0, 255, true, 0.1f );
	}

	// Cleansers block the swap
	CBaseEntity *list[1024];

	Ray_t ray;
	ray.Init( vecStartPos, vecEndPos );

	int nCount = UTIL_EntitiesAlongRay( list, 1024, ray, 0 );
	for( int i = 0; i < nCount; ++i )
	{
		if( FClassnameIs( list[i], "trigger_paint_cleanser" ) ||
			FClassnameIs( list[i], "trigger_portal_cleanser" ) )
		{
			return SWAP_TRACE_HIT_SOMETHING;
		}
	}

	// Hitting the other player is fine
	if( trace->DidHit() && trace->m_pEnt->IsPlayer() )
	{
		return SWAP_TRACE_HIT_NOTHING;
	}

	if( trace->fraction != 1.0f )
	{
		return SWAP_TRACE_HIT_SOMETHING;
	}

	return SWAP_TRACE_HIT_NOTHING;
}


bool CheckSwapThroughPortals( CPortal_Player *pPlayer1, CProp_Portal *pPlayer1Portal, CPortal_Player *pPlayer2, CProp_Portal *pPlayer2Portal )
{
	// Trace from the first player to the first portal
	trace_t tr;
	SwapGunTraceResult result = SwapGunsTraceLine( pPlayer1->EyePosition(), pPlayer1Portal->GetAbsOrigin(), pPlayer1, &tr );
	if( result == SWAP_TRACE_HIT_NOTHING && tr.fraction == 1.0f )
	{
		// Trace from the second portal to the second player
		trace_t tr2;
		result = SwapGunsTraceLine( pPlayer2Portal->GetAbsOrigin(), pPlayer2->EyePosition(), pPlayer2Portal, &tr2 );
		if( result == SWAP_TRACE_HIT_NOTHING )
		{
			// Reconstruction note: both builds test the first trace here, not tr2.
			return ( tr.fraction == 1.0f );
		}
	}

	return false;
}


bool CanSeePositionThroughPortal( const Vector &vecStartPos, const Vector &vecForwardDir, const Vector &vecEndPos, CProp_Portal *pPortal )
{
	// Where the end position appears on this side of the portal
	Vector vecNewEndPos;
	UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vecEndPos, vecNewEndPos );

	Vector vecNewEndDir = vecNewEndPos - vecStartPos;
	VectorNormalize( vecNewEndDir );

	float flAngleDiff = RAD2DEG( acos( DotProduct( vecNewEndDir, vecForwardDir ) ) );

	return flAngleDiff <= sv_max_swap_look_angle.GetFloat();
}


bool CheckSwapProximity( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 )
{
	// pPlayer1 holds the paint gun, pPlayer2 the portal gun
	Vector vecPaintGunPlayerPos = pPlayer1->EyePosition();
	Vector vecPortalGunPlayerPos = pPlayer2->EyePosition();

	// Get the directions the players are looking
	Vector vecPaintPlayerForward, vecPortalPlayerForward;
	AngleVectors( pPlayer1->EyeAngles(), &vecPaintPlayerForward );
	VectorNormalize( vecPaintPlayerForward );
	AngleVectors( pPlayer2->EyeAngles(), &vecPortalPlayerForward );
	VectorNormalize( vecPortalPlayerForward );

	// Check if the players can see each other directly
	Vector vecStraightLineDir = vecPortalGunPlayerPos - vecPaintGunPlayerPos;
	float flStraightLineDistance = vecStraightLineDir.LengthSqr();
	VectorNormalize( vecStraightLineDir );

	float flMaxSwapDistance = sv_max_swap_distance.GetFloat() * sv_max_swap_distance.GetFloat();
	if( flStraightLineDistance <= flMaxSwapDistance )
	{
		float flPaintPlayerAngleDiff = RAD2DEG( acos( DotProduct( vecPaintPlayerForward, vecStraightLineDir ) ) );
		float flPortalPlayerAngleDiff = RAD2DEG( acos( DotProduct( vecPortalPlayerForward, -vecStraightLineDir ) ) );

		if( flPaintPlayerAngleDiff <= sv_max_swap_look_angle.GetFloat() &&
			flPortalPlayerAngleDiff <= sv_max_swap_look_angle.GetFloat() )
		{
			trace_t trace;
			SwapGunTraceResult result = SwapGunsTraceLine( vecPaintGunPlayerPos, vecPortalGunPlayerPos, pPlayer2, &trace );
			if( result == SWAP_TRACE_HIT_NOTHING )
			{
				return true;
			}
		}
	}

	// Check if the players can see each other through the portals
	CWeaponPortalgun *pPortalGun = dynamic_cast< CWeaponPortalgun* >( gEntList.FindEntityByClassname( NULL, "weapon_portalgun" ) );
	if( !pPortalGun )
		return false;

	CProp_Portal *pPortal1 = CProp_Portal::FindPortal( pPortalGun->GetLinkageGroupID(), false );
	CProp_Portal *pPortal2 = CProp_Portal::FindPortal( pPortalGun->GetLinkageGroupID(), true );

	if( pPortal1 && pPortal1->IsActive() && pPortal2 && pPortal2->IsActive() )
	{
		Vector vecPortal1PaintGunPlayerDir = pPortal1->GetAbsOrigin() - vecPaintGunPlayerPos;
		VectorNormalize( vecPortal1PaintGunPlayerDir );
		Vector vecPortal2PaintGunPlayerDir = pPortal2->GetAbsOrigin() - vecPaintGunPlayerPos;
		VectorNormalize( vecPortal2PaintGunPlayerDir );

		float flPaintPlayerPortal1Angle = RAD2DEG( acos( DotProduct( vecPaintPlayerForward, vecPortal1PaintGunPlayerDir ) ) );
		float flPaintPlayerPortal2Angle = RAD2DEG( acos( DotProduct( vecPaintPlayerForward, vecPortal2PaintGunPlayerDir ) ) );

		Vector vecPortal1PortalGunPlayerDir = pPortal1->GetAbsOrigin() - vecPortalGunPlayerPos;
		VectorNormalize( vecPortal1PortalGunPlayerDir );
		Vector vecPortal2PortalGunPlayerDir = pPortal2->GetAbsOrigin() - vecPortalGunPlayerPos;
		VectorNormalize( vecPortal2PortalGunPlayerDir );

		float flPortalPlayerPortal1Angle = RAD2DEG( acos( DotProduct( vecPortalPlayerForward, vecPortal1PortalGunPlayerDir ) ) );
		float flPortalPlayerPortal2Angle = RAD2DEG( acos( DotProduct( vecPortalPlayerForward, vecPortal2PortalGunPlayerDir ) ) );

		float flMaxPortalLookAngle = sv_max_swap_portal_look_angle.GetFloat();

		// Paint player looking into portal 1, portal player looking into portal 2
		if( flPaintPlayerPortal1Angle <= flMaxPortalLookAngle && flPortalPlayerPortal2Angle <= flMaxPortalLookAngle )
		{
			bool bCanSeeOtherPlayer = CanSeePositionThroughPortal( vecPaintGunPlayerPos, vecPaintPlayerForward, vecPortalGunPlayerPos, pPortal1 );

			float flThroughPortal1Distance = UTIL_Portal_DistanceThroughPortalSqr( pPortal1, vecPaintGunPlayerPos, vecPortalGunPlayerPos );
			if( !bCanSeeOtherPlayer || flThroughPortal1Distance == -1.0f || flThroughPortal1Distance > flMaxSwapDistance )
			{
				return false;
			}

			return CheckSwapThroughPortals( pPlayer1, pPortal1, pPlayer2, pPortal2 );
		}

		// Paint player looking into portal 2, portal player looking into portal 1
		if( flPaintPlayerPortal2Angle <= flMaxPortalLookAngle && flPortalPlayerPortal1Angle <= flMaxPortalLookAngle )
		{
			bool bCanSeeOtherPlayer = CanSeePositionThroughPortal( vecPaintGunPlayerPos, vecPaintPlayerForward, vecPortalGunPlayerPos, pPortal2 );

			float flThroughPortal2Distance = UTIL_Portal_DistanceThroughPortalSqr( pPortal2, vecPaintGunPlayerPos, vecPortalGunPlayerPos );
			if( bCanSeeOtherPlayer && flThroughPortal2Distance != -1.0f && flThroughPortal2Distance <= flMaxSwapDistance )
			{
				return CheckSwapThroughPortals( pPlayer1, pPortal2, pPlayer2, pPortal1 );
			}
		}
	}

	return false;
}


// Reconstruction note: 2010 read ConVar sv_can_swap_guns; the retail
// portal_player.cpp replaced it with this constant.
#ifndef sv_can_swap_guns
#define sv_can_swap_guns	1
#endif

static void swap_paint_and_portal_guns( const CCommand &args )
{
	if( sv_can_swap_guns && gpGlobals->maxClients > 1 )
	{
		CPortal_Player *pPlayer1 = GetPortalPlayer( 1 );
		CPortal_Player *pPlayer2 = GetPortalPlayer( 2 );

		if( pPlayer1 && pPlayer1->IsConnected() && pPlayer2 && pPlayer2->IsConnected() && pPlayer1 != pPlayer2 )
		{
			SwapPaintAndPortalGuns( pPlayer1, pPlayer2 );
		}
	}
}

static ConCommand swap_paint_and_portal_guns_command( "swap_paint_and_portal_guns", swap_paint_and_portal_guns, "Swaps the paint and portal guns if allowed" );
