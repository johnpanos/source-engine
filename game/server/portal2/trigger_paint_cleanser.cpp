//========= Portal 2 reconstruction ============================================//
//
// Purpose: Trigger volume that removes paint from entities and paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "trigger_paint_cleanser.h"
#include "paint_cleanser_manager.h"
#include "paint_database.h"
#include "portal2/portal/portal_player.h"
#include "weapon_paintgun.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CTriggerPaintCleanser )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CTriggerPaintCleanser, DT_TriggerPaintCleanser )
SendPropBool( SENDINFO_NOCHECK( m_bDisabled ) ),
    END_SEND_TABLE()

        LINK_ENTITY_TO_CLASS( trigger_paint_cleanser, CTriggerPaintCleanser );

CTriggerPaintCleanser::CTriggerPaintCleanser()
{
}

void CTriggerPaintCleanser::Spawn()
{
	BaseClass::Spawn();

	InitTrigger();
}

void CTriggerPaintCleanser::Activate()
{
	BaseClass::Activate();

	PaintCleanserManager.AddPaintCleanser( this );
}

void CTriggerPaintCleanser::UpdateOnRemove()
{
	PaintCleanserManager.RemovePaintCleanser( this );

	BaseClass::UpdateOnRemove();
}

void CTriggerPaintCleanser::Touch( CBaseEntity *pOther )
{
	//Find the paint gun that should be cleansed, if any
	CWeaponPaintGun *pPaintGun = NULL;
	if ( pOther->IsPlayer() )
	{
		CPortal_Player *pPlayer = ToPortalPlayer( pOther );
		if ( pPlayer )
		{
			pPaintGun = dynamic_cast<CWeaponPaintGun *>(
			    pPlayer->Weapon_OwnsThisType( "weapon_paintgun" ) );
			pPlayer->CleansePaint();

			IGameEvent *event = gameeventmanager->CreateEvent( "player_touch_paint_cleanser" );
			if ( event )
			{
				event->SetInt( "userid", pPlayer->GetUserID() );
				event->SetInt( "subject", entindex() );
				gameeventmanager->FireEvent( event );
			}
		}
	}
	else
	{
		pPaintGun = dynamic_cast<CWeaponPaintGun *>( pOther );
	}

	if ( pPaintGun )
	{
		pPaintGun->CleansePaint();
	}
	else if ( FClassnameIs( pOther, "paintgun_blob" ) )
	{
		UTIL_Remove( pOther );
	}
	else if ( FClassnameIs( pOther, "prop_weighted_cube" ) )
	{
		PaintDatabase.RemovePaintedEntity( pOther );
	}
	else if ( FClassnameIs( pOther, "prop_paint_bomb" ) )
	{
		//Destroy the paint bomb
		CTakeDamageInfo info( this, this, pOther->GetHealth(), DMG_GENERIC );
		info.SetDamagePosition( pOther->GetAbsOrigin() );
		pOther->TakeDamage( info );
	}
}

void CTriggerPaintCleanser::Enable()
{
	BaseClass::Enable();
}

int CTriggerPaintCleanser::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
