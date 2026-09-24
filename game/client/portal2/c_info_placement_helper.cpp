//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side placement helper used by predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_info_placement_helper.h"
#include "c_portal_base2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Reconstruction note: both builds receive the server's string_t fields with RecvPropString
// into the string_t storage itself. The client only tests them against NULL_STRING and never
// dereferences them.
IMPLEMENT_CLIENTCLASS_DT( C_InfoPlacementHelper, DT_InfoPlacementHelper, CInfoPlacementHelper )
	RecvPropString( RECVINFO( m_strTargetProxy ) ),
	RecvPropString( RECVINFO( m_strTargetEntity ) ),
	RecvPropFloat( RECVINFO( m_flRadius ) ),
	RecvPropBool( RECVINFO( m_bSnapToHelperAngles ) ),
	RecvPropBool( RECVINFO( m_bForcePlacement ) ),
	RecvPropBool( RECVINFO( m_bDisabled ) ),
	RecvPropFloat( RECVINFO( m_flDisableTime ) ),
	RecvPropBool( RECVINFO( m_bDeferringToPortal ) ),
END_RECV_TABLE()

//
// Every live client placement helper
//

static CUtlVector<C_InfoPlacementHelper *> s_PlacementHelpers;

//-----------------------------------------------------------------------------
// Purpose: Find the enabled helper closest to the player that lies within the
//			cone its radius subtends around the player's view direction
//-----------------------------------------------------------------------------
C_InfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, C_BasePlayer *pPlayer )
{
	if ( pPlayer == NULL )
		return NULL;

	Vector vecEyeDir, vecEyePos;
	pPlayer->EyePositionAndVectors( &vecEyePos, &vecEyeDir, NULL, NULL );

	C_InfoPlacementHelper *pBestHelper = NULL;
	float flBestDist = 1e18f;

	for ( int i = 0; i < s_PlacementHelpers.Count(); i++ )
	{
		C_InfoPlacementHelper *pHelper = s_PlacementHelpers[i];
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
	}

	return pBestHelper;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_InfoPlacementHelper::C_InfoPlacementHelper( void )
{
	m_bSnapToHelperAngles = false;
	m_bForcePlacement = false;
	m_flDisableTime = 0.0f;
	m_bDeferringToPortal = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_InfoPlacementHelper::Spawn( void )
{
	BaseClass::Spawn();

	s_PlacementHelpers.AddToTail( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_InfoPlacementHelper::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();

	s_PlacementHelpers.FindAndFastRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_InfoPlacementHelper::ShouldUseHelperAngles( void )
{
	return m_bSnapToHelperAngles;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_InfoPlacementHelper::IsEnabled( void )
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
// Purpose: Predict the server deferring this helper to a newly placed portal
//-----------------------------------------------------------------------------
void C_InfoPlacementHelper::BindToPortal( C_Portal_Base2D *pPortal )
{
	if ( pPortal == NULL )
		return;

	// Forced helpers can always be used
	if ( m_bForcePlacement )
		return;

	m_bDeferringToPortal = true;
}
