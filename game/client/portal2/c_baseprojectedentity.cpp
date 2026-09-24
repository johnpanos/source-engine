//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of entities projected from a projector (light bridge and
//			tractor beam segments). The projection itself lives in
//			baseprojectedentity_shared.cpp.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_baseprojectedentity.h"
#include "c_portal_base2d.h"
#include "c_prop_portal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT( C_BaseProjectedEntity, DT_BaseProjectedEntity, CBaseProjectedEntity )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropQAngles( RECVINFO_NAME( m_angNetworkAngles, m_angRotation ) ),
	RecvPropEHandle( RECVINFO( m_hHitPortal ) ),
	RecvPropEHandle( RECVINFO( m_hSourcePortal ) ),
	RecvPropVector( RECVINFO( m_vecSourcePortalCenter ) ),
	RecvPropVector( RECVINFO( m_vecSourcePortalRemoteCenter ) ),
	RecvPropQAngles( RECVINFO( m_vecSourcePortalAngle ) ),
	RecvPropQAngles( RECVINFO( m_vecSourcePortalRemoteAngle ) ),
	RecvPropVector( RECVINFO( m_vecStartPoint ) ),
	RecvPropVector( RECVINFO( m_vecEndPoint ) ),
	RecvPropEHandle( RECVINFO( m_hChildSegment ) ),
	RecvPropEHandle( RECVINFO( m_hPlacementHelper ) ),
	RecvPropInt( RECVINFO( m_iMaxRemainingRecursions ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseProjectedEntity )
	DEFINE_PRED_FIELD( m_hHitPortal, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hSourcePortal, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecSourcePortalCenter, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecSourcePortalRemoteCenter, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecSourcePortalAngle, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecSourcePortalRemoteAngle, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecStartPoint, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecEndPoint, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hPlacementHelper, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()


// Every projected entity on the client, so portal changes can retest all projections
static CUtlVector<C_BaseProjectedEntity *> s_AllProjectedEntities;

C_BaseProjectedEntity::C_BaseProjectedEntity()
{
	s_AllProjectedEntities.AddToTail( this );
}

C_BaseProjectedEntity::~C_BaseProjectedEntity()
{
	s_AllProjectedEntities.FindAndFastRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose: Projections through a portal are predicted by the player who fired it
//-----------------------------------------------------------------------------
C_BasePlayer *C_BaseProjectedEntity::GetPredictionOwner( void )
{
	C_Portal_Base2D *pSourcePortal = m_hSourcePortal;
	if ( pSourcePortal )
	{
		C_Prop_Portal *pSourcePropPortal = dynamic_cast< C_Prop_Portal * >( pSourcePortal );
		if ( pSourcePropPortal )
		{
			C_BaseEntity *pFiredBy = pSourcePropPortal->m_hFiredByPlayer.Get();
			if ( pFiredBy && pFiredBy->IsPlayer() )
			{
				return (C_BasePlayer *)pFiredBy;
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_BaseProjectedEntity::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		OnProjected();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Only segments coming out of a portal can be predicted
//-----------------------------------------------------------------------------
void C_BaseProjectedEntity::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	C_Portal_Base2D *pSourcePortal = m_hSourcePortal.Get();
	SetPredictionEligible( pSourcePortal != NULL );

	if ( pSourcePortal && pSourcePortal->IsPlayerSimulated() )
	{
		SetPlayerSimulated( pSourcePortal->GetSimulatingPlayer() );
	}
	else
	{
		UnsetPlayerSimulated();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_BaseProjectedEntity::TestAllForProjectionChanges( void )
{
	for ( int i = 0; i < s_AllProjectedEntities.Count(); ++i )
	{
		s_AllProjectedEntities[i]->TestForProjectionChanges();
	}
}
