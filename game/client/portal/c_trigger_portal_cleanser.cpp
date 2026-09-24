//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side fizzler trigger; predicts portal removal for the local gun
//			and drives the field's look (the FizzlerVortex material proxy and the
//			cleanser_scanline particle)
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled) and the retail
// Linux client.so. Not original Valve source; the repository's provenance and
// distribution warning applies.
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
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "imaterialproxydict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_portal_cleanser_default_intensity( "cl_portal_cleanser_default_intensity", "1.0f",
    FCVAR_CHEAT, "The default intensity of the cleanser field effect." );
ConVar cl_portal_cleanser_shot_pulse_time( "cl_portal_cleanser_shot_pulse_time", "0.1f",
    FCVAR_CHEAT, "The amount of time to pulse the cleanser field for when it is shot at." );
ConVar cl_portal_cleanser_shot_pulse_intensity( "cl_portal_cleanser_shot_pulse_intensity", "10.0f",
    FCVAR_CHEAT, "The intensity of the cleanser field when it gets shot at." );
ConVar cl_portal_cleanser_powerup_time( "cl_portal_cleanser_powerup_time", "1.0f", FCVAR_CHEAT,
    "The amount of time the power up sequence takes to complete." );
ConVar cl_portal_cleanser_scanline(
    "cl_portal_cleanser_scanline", "1", FCVAR_CHEAT, "Use particle scanline." );

IMPLEMENT_CLIENTCLASS_DT(
    C_TriggerPortalCleanser, DT_TriggerPortalCleanser, CTriggerPortalCleanser )
RecvPropBool( RECVINFO( m_bDisabled ) ), RecvPropBool( RECVINFO( m_bVisible ) ),
    RecvPropFloat( RECVINFO( m_flPortalShotTime ) ), RecvPropBool( RECVINFO( m_bObject1InRange ) ),
    RecvPropBool( RECVINFO( m_bObject2InRange ) ), RecvPropEHandle( RECVINFO( m_hObject1 ) ),
    RecvPropEHandle( RECVINFO( m_hObject2 ) ), RecvPropBool( RECVINFO( m_bUseScanline ) ),
    RecvPropBool( RECVINFO( m_bPlayersPassTriggerFilters ) ),
    END_RECV_TABLE()

        C_TriggerPortalCleanser::C_TriggerPortalCleanser()
{
	m_bDisabled = false;
	m_bVisible = false;
	m_bUseScanline = false;
	m_bPlayersPassTriggerFilters = false;
	m_bObject1InRange = false;
	m_bObject2InRange = false;
	m_flPortalShotTime = 0.0f;
	m_flPowerUpTimer = 0.0f;
	m_flLastShotTime = 0.0f;
	m_flShotPulseTimer = 0.0f;
	m_flLastUpdateTime = 0.0f;
}

C_TriggerPortalCleanser::~C_TriggerPortalCleanser()
{
	StopScanline();
}

//-----------------------------------------------------------------------------
// Purpose: Predict the cleanser fizzling the portals of a gun that passes through
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::Touch( C_BaseEntity *pOther )
{
	if ( m_bDisabled )
		return;

	// The server tells whether players pass this trigger's filters at all.
	if ( !m_bPlayersPassTriggerFilters )
		return;

	// A dropped portalgun can pass through by itself
	const bool bIsPortalGun = ( pOther && FClassnameIs( pOther, "weapon_portalgun" ) );

	if ( !pOther->IsPlayer() && !bIsPortalGun )
		return;

	{
		C_Portal_Player *pPlayer = ToPortalPlayer( pOther );

		// In multiplayer, players without a gun have nothing to fizzle
		if ( gpGlobals->maxClients > 1 && pPlayer &&
		     !pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
			return;

		if ( pPlayer || bIsPortalGun )
		{
			C_WeaponPortalgun *pPortalgun =
			    pPlayer ? dynamic_cast<C_WeaponPortalgun *>(
			                  pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
			            : dynamic_cast<C_WeaponPortalgun *>( pOther );

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
	partition->RemoveAndInsert( PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS |
	                                PARTITION_CLIENT_NON_STATIC_EDICTS, // remove
	    PARTITION_CLIENT_TRIGGER_ENTITIES,                              // add
	    CollisionProp()->GetPartitionHandle() );
}

void C_TriggerPortalCleanser::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flLastUpdateTime = gpGlobals->curtime;
	}
}

C_BaseEntity *C_TriggerPortalCleanser::GetVortexObject( int iObject )
{
	if ( iObject == 0 )
		return m_bObject1InRange ? m_hObject1.Get() : NULL;
	return m_bObject2InRange ? m_hObject2.Get() : NULL;
}

void C_TriggerPortalCleanser::GetCurrentState( float &flIntensity, float &flPowerUp )
{
	const float flLastUpdateTime = m_flLastUpdateTime;
	const float flFrameTime = gpGlobals->curtime - flLastUpdateTime;
	m_flLastUpdateTime = gpGlobals->curtime;

	// A portal shot hitting the field pulses its intensity up and back down.
	flIntensity = cl_portal_cleanser_default_intensity.GetFloat();
	if ( m_flPortalShotTime > m_flLastShotTime )
	{
		m_flLastShotTime = m_flPortalShotTime;
		m_flShotPulseTimer = 0.0f;
	}
	const float flPulseTime = cl_portal_cleanser_shot_pulse_time.GetFloat();
	if ( m_flPortalShotTime != 0.0f && m_flShotPulseTimer <= flPulseTime + flPulseTime )
	{
		m_flShotPulseTimer += flFrameTime;
		float flPulse = m_flShotPulseTimer < flPulseTime
		                    ? m_flShotPulseTimer / flPulseTime
		                    : ( flPulseTime - m_flShotPulseTimer ) / flPulseTime + 1.0f;
		flPulse = clamp( flPulse, 0.0f, 1.0f );
		flIntensity = Lerp( flPulse, cl_portal_cleanser_default_intensity.GetFloat(),
		    cl_portal_cleanser_shot_pulse_intensity.GetFloat() );
	}

	// The field powers up while enabled and back down while disabled.
	const float flPowerUpTime = cl_portal_cleanser_powerup_time.GetFloat();
	if ( !m_bDisabled )
	{
		if ( m_flPowerUpTimer < flPowerUpTime )
			m_flPowerUpTimer += flFrameTime;
	}
	else
	{
		if ( m_flPowerUpTimer > 0.0f )
			m_flPowerUpTimer = ( flLastUpdateTime + m_flPowerUpTimer ) - gpGlobals->curtime;
		StopScanline();
	}
	m_flPowerUpTimer = clamp( m_flPowerUpTimer, 0.0f, flPowerUpTime );
	flPowerUp = flPowerUpTime > 0.0f ? clamp( m_flPowerUpTimer / flPowerUpTime, 0.0f, 1.0f ) : 1.0f;

	if ( !m_bDisabled && cl_portal_cleanser_scanline.GetBool() && m_bUseScanline )
		UpdateScanline();
	else
		StopScanline();
}

//-----------------------------------------------------------------------------
// Purpose: The cleanser_scanline particle sweeps the field's bounds; control
//			points 4 and 5 follow the vortex objects (or sit far below)
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::UpdateScanline( void )
{
	Vector vMins, vMaxs;
	CollisionProp()->CollisionToWorldSpace( CollisionProp()->OBBMins(), &vMins );
	CollisionProp()->CollisionToWorldSpace( CollisionProp()->OBBMaxs(), &vMaxs );

	if ( !m_hScanlineEffect )
	{
		// Floor and ceiling fields get no scanline.
		if ( vMaxs.z - vMins.z < 32.0f )
			return;

		m_hScanlineEffect = ParticleProp()->Create( "cleanser_scanline", PATTACH_CUSTOMORIGIN, -1 );
		if ( !m_hScanlineEffect )
			return;

		m_hScanlineEffect->SetControlPoint( 0, vMaxs );
		m_hScanlineEffect->SetControlPoint( 1, Vector( vMins.x, vMins.y, vMaxs.z ) );
		m_hScanlineEffect->SetControlPoint( 2, Vector( vMaxs.x, vMaxs.y, vMins.z ) );
	}

	const Vector vHidden( vMins.x, vMins.y, vMins.z - 512.0f );
	C_BaseEntity *pObject1 = GetVortexObject( 0 );
	m_hScanlineEffect->SetControlPoint( 4, pObject1 ? pObject1->WorldSpaceCenter() : vHidden );
	C_BaseEntity *pObject2 = GetVortexObject( 1 );
	m_hScanlineEffect->SetControlPoint( 5, pObject2 ? pObject2->WorldSpaceCenter() : vHidden );
}

void C_TriggerPortalCleanser::StopScanline( void )
{
	if ( m_hScanlineEffect )
	{
		m_hScanlineEffect->StopEmission( false, false, true );
		m_hScanlineEffect = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: SolidEnergy's inputs from the cleanser it draws: the vortex objects,
//			the field intensity and the power-up amount
//-----------------------------------------------------------------------------
class CFizzlerVortexProxy : public CEntityMaterialProxy
{
public:
	CFizzlerVortexProxy()
	    : m_pMaterial( NULL ), m_pVortexPos1( NULL ), m_pVortex1( NULL ), m_pVortexPos2( NULL ),
	      m_pVortex2( NULL ), m_pIntensity( NULL ), m_pPowerUp( NULL )
	{
	}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		m_pMaterial = pMaterial;
		bool bFound;
		m_pVortexPos1 = pMaterial->FindVar( "$FLOW_VORTEX_POS1", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortex1 = pMaterial->FindVar( "$flow_vortex1", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortexPos2 = pMaterial->FindVar( "$FLOW_VORTEX_POS2", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortex2 = pMaterial->FindVar( "$flow_vortex2", &bFound, true );
		if ( !bFound )
			return false;
		m_pIntensity = pMaterial->FindVar( "$flow_color_intensity", &bFound, true );
		if ( !bFound )
			return false;
		m_pPowerUp = pMaterial->FindVar( "$powerup", &bFound, true );
		if ( !bFound )
			m_pPowerUp = NULL;
		return true;
	}

	virtual void OnBind( C_BaseEntity *pEntity )
	{
		C_TriggerPortalCleanser *pCleanser = dynamic_cast<C_TriggerPortalCleanser *>( pEntity );
		if ( !pCleanser )
			return;

		SetVortex( pCleanser->GetVortexObject( 0 ), m_pVortex1, m_pVortexPos1 );
		SetVortex( pCleanser->GetVortexObject( 1 ), m_pVortex2, m_pVortexPos2 );

		float flIntensity, flPowerUp;
		pCleanser->GetCurrentState( flIntensity, flPowerUp );
		m_pIntensity->SetFloatValue( flIntensity );
		if ( m_pPowerUp )
			m_pPowerUp->SetFloatValue( flPowerUp );
	}

	virtual IMaterial *GetMaterial() { return m_pMaterial; }

private:
	static void SetVortex( C_BaseEntity *pObject, IMaterialVar *pEnabled, IMaterialVar *pPosition )
	{
		if ( !pObject )
		{
			pEnabled->SetIntValue( 0 );
			return;
		}
		pEnabled->SetIntValue( 1 );
		const Vector &vCenter = pObject->WorldSpaceCenter();
		pPosition->SetVecValue( vCenter.x, vCenter.y, vCenter.z );
	}

	IMaterial *m_pMaterial;
	IMaterialVar *m_pVortexPos1;
	IMaterialVar *m_pVortex1;
	IMaterialVar *m_pVortexPos2;
	IMaterialVar *m_pVortex2;
	IMaterialVar *m_pIntensity;
	IMaterialVar *m_pPowerUp;
};

EXPOSE_MATERIAL_PROXY( CFizzlerVortexProxy, FizzlerVortex );
