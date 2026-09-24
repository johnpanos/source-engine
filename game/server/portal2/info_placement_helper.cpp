//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that snaps nearby portal placement to a designer spot
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "info_placement_helper.h"
#include "portal_base2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_show_placement_help_in_preview( "sv_show_placement_help_in_preview", "0", 0, "Forces the placement preview to show any help in placement given from info_placement_helper entities.\n" );

extern ConVar sv_portal_placement_debug;

LINK_ENTITY_TO_CLASS( info_placement_helper, CInfoPlacementHelper );

BEGIN_DATADESC( CInfoPlacementHelper )

	DEFINE_KEYFIELD( m_strTargetProxy, FIELD_STRING, "proxy_name" ),
	DEFINE_KEYFIELD( m_strTargetEntity, FIELD_STRING, "attach_target_name" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "radius" ),
	DEFINE_KEYFIELD( m_bSnapToHelperAngles, FIELD_BOOLEAN, "snap_to_helper_angles" ),
	DEFINE_KEYFIELD( m_bForcePlacement, FIELD_BOOLEAN, "force_placement" ),
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_FIELD( m_flDisableTime, FIELD_TIME ),
	DEFINE_FIELD( m_bDeferringToPortal, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	DEFINE_OUTPUT( m_OnObjectPlaced, "OnObjectPlaced" ),
	DEFINE_OUTPUT( m_ObjectPlacedSize, "OnObjectPlacedSize" ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CInfoPlacementHelper, DT_InfoPlacementHelper )
	SendPropStringT( SENDINFO( m_strTargetProxy ) ),
	SendPropStringT( SENDINFO( m_strTargetEntity ) ),
	SendPropFloat( SENDINFO( m_flRadius ) ),
	SendPropBool( SENDINFO( m_bSnapToHelperAngles ) ),
	SendPropBool( SENDINFO( m_bForcePlacement ) ),
	SendPropBool( SENDINFO( m_bDisabled ) ),
	SendPropFloat( SENDINFO( m_flDisableTime ) ),
	SendPropBool( SENDINFO( m_bDeferringToPortal ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Tracks every live placement helper so placement queries don't walk the entity list
//-----------------------------------------------------------------------------
class CInfoPlacementManager : public CAutoGameSystem
{
public:
	virtual void LevelShutdownPostEntity( void )
	{
		Purge();
	}

	void Purge( void )
	{
		m_PlacementHelpers.Purge();
	}

	void AddPlacementHelper( CInfoPlacementHelper *pHelper )
	{
		CHandle<CInfoPlacementHelper> hHelper = pHelper;

		// Don't add a helper twice (restore can re-register a spawned helper)
		if ( m_PlacementHelpers.Find( hHelper ) != m_PlacementHelpers.InvalidIndex() )
			return;

		m_PlacementHelpers.AddToTail( hHelper );
	}

	void RemovePlacementHelper( CInfoPlacementHelper *pHelper )
	{
		CHandle<CInfoPlacementHelper> hHelper = pHelper;
		m_PlacementHelpers.FindAndRemove( hHelper );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Find the enabled helper closest to the player that lies within the
	//			cone its radius subtends around the player's view direction
	//-----------------------------------------------------------------------------
	CInfoPlacementHelper *FindPlacementHelper( const Vector &vecEndPoint, CBasePlayer *pPlayer )
	{
		if ( pPlayer == NULL )
			return NULL;

		Vector vecEyeDir = pPlayer->EyeDirection3D();
		Vector vecEyePos = pPlayer->EyePosition();

		CInfoPlacementHelper *pBestHelper = NULL;
		float flBestDist = 1e18f;

		for ( int i = 0; i < m_PlacementHelpers.Count(); i++ )
		{
			CInfoPlacementHelper *pHelper = m_PlacementHelpers[i];
			if ( pHelper == NULL )
				continue;

			if ( pHelper->IsEnabled() == false )
				continue;

			Vector vecTargetDir = pHelper->GetAbsOrigin() - vecEyePos;
			float flTargetDist = VectorNormalize( vecTargetDir );

			float flToleranceAngle = atan2( pHelper->GetTargetRadius(), flTargetDist );
			float flTargetAngle = acosf( DotProduct( vecEyeDir, vecTargetDir ) );

			// Must be looking within the radius of the helper
			if ( flTargetAngle > flToleranceAngle )
				continue;

			// Take the closest helper
			if ( flTargetDist > flBestDist )
				continue;

			pBestHelper = pHelper;
			flBestDist = flTargetDist;

			if ( sv_portal_placement_debug.GetBool() )
			{
				Msg( "PortalPlacement: Considering placement helper (%s) at %f, %f, %f. Radius %f\n", pHelper->GetEntityName().ToCStr(), XYZ( pHelper->GetAbsOrigin() ), pHelper->GetTargetRadius() );
			}
		}

		return pBestHelper;
	}

private:

	CUtlVector< CHandle<CInfoPlacementHelper> >	m_PlacementHelpers;
};

static CInfoPlacementManager g_PlacementManager;

//-----------------------------------------------------------------------------
// Purpose: Find the placement helper the player is aiming at
//-----------------------------------------------------------------------------
CInfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, CBasePlayer *pPlayer )
{
	return g_PlacementManager.FindPlacementHelper( vecEndPoint, pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CInfoPlacementHelper::CInfoPlacementHelper( void )
{
	m_bSnapToHelperAngles = false;
	m_bForcePlacement = false;
	m_flDisableTime = 0.0f;
	m_bDeferringToPortal = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::Spawn( void )
{
	BaseClass::Spawn();

	g_PlacementManager.AddPlacementHelper( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::OnRestore( void )
{
	g_PlacementManager.AddPlacementHelper( this );

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();

	g_PlacementManager.RemovePlacementHelper( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CInfoPlacementHelper::ShouldUseHelperAngles( void )
{
	return m_bSnapToHelperAngles;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::InputEnable( inputdata_t &inputdata )
{
	m_bDisabled = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::InputDisable( inputdata_t &inputdata )
{
	m_bDisabled = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::DrawDebugGeometryOverlays( void )
{
	NDebugOverlay::Axis( GetAbsOrigin(), GetAbsAngles(), 8.0f, false, 0.1f );

	// Yellow when enabled, red when not
	int g = IsEnabled() ? 255 : 0;
	NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), m_flRadius, 255, g, 0, 0, false, 0.1f );

	BaseClass::DrawDebugGeometryOverlays();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CInfoPlacementHelper::IsEnabled( void )
{
	// Turned off by the map
	if ( m_bDisabled )
		return false;

	// A portal is currently resting on this helper
	if ( m_bDeferringToPortal )
		return false;

	// Forced helpers ignore the reset delay
	if ( m_bForcePlacement )
		return true;

	return ( m_flDisableTime < gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: A portal we're deferring to has changed state
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	// We don't care about linkage
	if ( nEventType == PORTALEVENT_LINKED )
		return;

	// If the portal only moved slightly, it's still resting on us
	if ( nEventType == PORTALEVENT_MOVED )
	{
		if ( ( pNotifier->GetAbsOrigin() - GetTargetOrigin() ).LengthSqr() < Square( 2.0f ) )
			return;
	}

	// Stop listening to this portal
	pNotifier->RemovePortalEventListener( this );
	m_bDeferringToPortal = false;

	// Don't become available again immediately unless the portal fizzled
	float flResetDelay = ( nEventType == PORTALEVENT_FIZZLE ) ? 0.0f : 3.0f;
	m_flDisableTime = gpGlobals->curtime + flResetDelay;
}

//-----------------------------------------------------------------------------
// Purpose: A portal was placed using this helper; disable until it leaves
//-----------------------------------------------------------------------------
void CInfoPlacementHelper::BindToPortal( CPortal_Base2D *pPortal )
{
	if ( pPortal == NULL )
		return;

	// Forced helpers can always be used
	if ( m_bForcePlacement )
		return;

	pPortal->AddPortalEventListener( this );

	m_bDeferringToPortal = true;
}
