//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server side of the projected entity base: networking, save/restore
//			and placement helper ownership.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "baseprojectedentity.h"
#include "info_placement_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CBaseProjectedEntity )

	DEFINE_FIELD( m_hHitPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSourcePortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecSourcePortalCenter, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecSourcePortalRemoteCenter, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecSourcePortalAngle, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecSourcePortalRemoteAngle, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecStartPoint, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecEndPoint, FIELD_VECTOR ),
	DEFINE_FIELD( m_hChildSegment, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iMaxRemainingRecursions, FIELD_INTEGER ),
	DEFINE_FIELD( m_bCreatePlacementHelper, FIELD_BOOLEAN ),

	// The placement helper is not saved; OnRestore() recreates it.

END_DATADESC()

extern void SendProxy_Angles( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

IMPLEMENT_SERVERCLASS_ST( CBaseProjectedEntity, DT_BaseProjectedEntity )

	// Projected entities are positioned by projection, send them at full precision
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropVector( SENDINFO( m_vecOrigin ), -1, SPROP_NOSCALE, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropVector( SENDINFO( m_angRotation ), -1, SPROP_NOSCALE, 0.0f, HIGH_DEFAULT, SendProxy_Angles ),

	SendPropEHandle( SENDINFO( m_hHitPortal ) ),
	SendPropEHandle( SENDINFO( m_hSourcePortal ) ),
	SendPropVector( SENDINFO( m_vecSourcePortalCenter ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecSourcePortalRemoteCenter ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecSourcePortalAngle ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecSourcePortalRemoteAngle ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecStartPoint ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecEndPoint ), -1, SPROP_NOSCALE ),
	SendPropEHandle( SENDINFO( m_hChildSegment ) ),
	SendPropEHandle( SENDINFO( m_hPlacementHelper ) ),
	SendPropInt( SENDINFO( m_iMaxRemainingRecursions ) ),

END_SEND_TABLE()


//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
CBaseProjectedEntity::CBaseProjectedEntity()
{
}

void CBaseProjectedEntity::Spawn( void )
{
	BaseClass::Spawn();

	FindProjectedEndpoints();
}

void CBaseProjectedEntity::OnRestore( void )
{
	BaseClass::OnRestore();

	// Placement helpers aren't saved, put ours back
	if ( m_bCreatePlacementHelper )
	{
		bool bCreatePlacement = ( m_hPlacementHelper.Get() == NULL );
		if ( bCreatePlacement )
		{
			m_hPlacementHelper = (CInfoPlacementHelper *) CreateEntityByName( "info_placement_helper" );
		}

		PlacePlacementHelper( m_hPlacementHelper );

		if ( bCreatePlacement )
		{
			DispatchSpawn( m_hPlacementHelper );
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Take the rest of the projection chain and our helper with us
//--------------------------------------------------------------------------------------------------
void CBaseProjectedEntity::UpdateOnRemove( void )
{
	if ( m_hChildSegment.Get() )
	{
		UTIL_Remove( m_hChildSegment.Get() );
	}

	if ( m_hPlacementHelper.Get() )
	{
		UTIL_Remove( m_hPlacementHelper.Get() );
	}

	BaseClass::UpdateOnRemove();
}

//--------------------------------------------------------------------------------------------------
// Put the portal placement helper at the far end of the projection
//--------------------------------------------------------------------------------------------------
void CBaseProjectedEntity::PlacePlacementHelper( CInfoPlacementHelper *pHelper )
{
	Vector vecPosition = GetEndPoint();

	if ( pHelper )
	{
		// Back off the surface we hit along our projection direction
		Vector vForward;
		GetVectors( &vForward, NULL, NULL );
		// Reconstruction note: the 2010 builds fold this to a subtraction of the unscaled
		// forward vector, so the scale factor is one unit.
		pHelper->SetAbsOrigin( vecPosition - ( vForward * 1.0f ) );
		pHelper->KeyValue( "radius", "52" );
		pHelper->KeyValue( "force_placement", "1" );
		pHelper->SetOwnerEntity( this );
	}
}
