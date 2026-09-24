//========= Portal 2 reconstruction ============================================//
//
// Purpose: One looping fizzler sound follows the closest active field.
//
// The retained retail Portal 2 client names the class and empty receive table.
// The server creates one sound player for all fizzlers. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_fizzler_multiorigin_sound_player.h"
#include "cliententitylist.h"
#include "portal/c_trigger_portal_cleanser.h"
#include "soundinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( fizzler_multiorigin_sound_player, ClientFizzlerMultiOriginSoundPlayer );

IMPLEMENT_CLIENTCLASS_DT( ClientFizzlerMultiOriginSoundPlayer, DT_FizzlerMultiOriginSoundPlayer,
    FizzlerMultiOriginSoundPlayer )
END_RECV_TABLE()

bool ClientFizzlerMultiOriginSoundPlayer::GetSoundSpatialization( SpatializationInfo_t &info )
{
	if ( IsDormant() )
	{
		return false;
	}

	float flClosestDistSqr = FLT_MAX;
	Vector vecClosest;
	bool bFound = false;
	for ( int i = 0; i <= ClientEntityList().GetHighestEntityIndex(); ++i )
	{
		C_BaseEntity *pEntity = ClientEntityList().GetBaseEntity( i );
		if ( !pEntity || pEntity->IsDormant() ||
		     !FClassnameIs( pEntity, "trigger_portal_cleanser" ) )
		{
			continue;
		}

		C_TriggerPortalCleanser *pFizzler = dynamic_cast<C_TriggerPortalCleanser *>( pEntity );
		if ( !pFizzler || !pFizzler->IsEnabled() )
		{
			continue;
		}

		Vector vecNearest;
		pFizzler->CollisionProp()->CalcNearestPoint( info.info.vListenerOrigin, &vecNearest );
		float flDistSqr = ( vecNearest - info.info.vListenerOrigin ).LengthSqr();
		if ( flDistSqr < flClosestDistSqr )
		{
			flClosestDistSqr = flDistSqr;
			vecClosest = vecNearest;
			bFound = true;
		}
	}

	if ( !bFound )
	{
		return false;
	}
	if ( info.pOrigin )
	{
		*info.pOrigin = vecClosest;
	}
	if ( info.pflRadius )
	{
		*info.pflRadius = 0.0f;
	}
	return true;
}
