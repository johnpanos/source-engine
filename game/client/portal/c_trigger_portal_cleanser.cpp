//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side fizzler trigger; predicts portal removal for the local gun
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_trigger_portal_cleanser.h"
#include "c_portal_player.h"
#include "weapon_portalgun_shared.h"
#include "prop_portal_shared.h"
#ifdef PORTAL2
// Portal 2 port: the Portal 1 c_prop_portal.h in this directory would shadow it.
#include "portal2/portal/c_prop_portal.h"
#include "portal2/portal/c_weapon_portalgun.h"
#endif
#include "ispatialpartition.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Predict the cleanser fizzling the portals of a gun that passes through
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::Touch( C_BaseEntity *pOther )
{
	if ( m_bDisabled )
		return;

	// A dropped portalgun can pass through by itself
	const bool bIsPortalGun = ( pOther && FClassnameIs( pOther, "weapon_portalgun" ) );

	if ( !pOther->IsPlayer() && !bIsPortalGun )
		return;

	{
		C_Portal_Player *pPlayer = ToPortalPlayer( pOther );

		// In multiplayer, players without a gun have nothing to fizzle
		if ( gpGlobals->maxClients > 1 && pPlayer && !pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
			return;

		if ( pPlayer || bIsPortalGun )
		{
			C_WeaponPortalgun *pPortalgun = pPlayer ? dynamic_cast<C_WeaponPortalgun*>( pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
													: dynamic_cast<C_WeaponPortalgun*>( pOther );

			if ( pPortalgun )
			{
				bool bPortal1Active = false;
				bool bPortal2Active = false;

				// Deactivate any portals this gun owns
				bool bFizzledPortal = false;

				if ( pPortalgun->CanFirePortal1() )
				{
					CProp_Portal *pPortal = pPortalgun->GetAssociatedPortal( false );

					if ( pPortal && pPortal->IsActive() )
					{
						pPortal->SetActive( false );
						bPortal1Active = true;
						bFizzledPortal = true;
					}
				}

				if ( pPortalgun->CanFirePortal2() )
				{
					CProp_Portal *pPortal = pPortalgun->GetAssociatedPortal( true );

					if ( pPortal && pPortal->IsActive() )
					{
						pPortal->SetActive( false );
						bPortal2Active = true;
						bFizzledPortal = true;
					}
				}

				if ( bFizzledPortal )
				{
					pPortalgun->SendWeaponAnim( ACT_VM_FIZZLE );
					pPortalgun->DoCleanseEffect( bPortal1Active, bPortal2Active );
					pPortalgun->SetLastFiredPortal( 0 );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cleansers live in the client trigger list so predicted touches reach them
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::UpdatePartitionListEntry( void )
{
	partition->RemoveAndInsert( PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS | PARTITION_CLIENT_NON_STATIC_EDICTS,	// remove
								PARTITION_CLIENT_TRIGGER_ENTITIES,	// add
								CollisionProp()->GetPartitionHandle() );
}

IMPLEMENT_CLIENTCLASS_DT( C_TriggerPortalCleanser, DT_TriggerPortalCleanser, CTriggerPortalCleanser )
	RecvPropBool( RECVINFO( m_bDisabled ) ),
END_RECV_TABLE()
