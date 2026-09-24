//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client receive table and predicted touch lifecycle for the catapult.
//
// The receive fields mirror the retail Portal 2 DT_TriggerCatapult server table.
// Launch behavior lives in trigger_catapult_shared.cpp. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_trigger_catapult.h"
#include "c_baseplayer.h"
#include "ispatialpartition.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_TriggerCatapult, DT_TriggerCatapult, CTriggerCatapult )
RecvPropArray3(
    RECVINFO_ARRAY( m_flRefireDelay ), RecvPropFloat( RECVINFO( m_flRefireDelay[0] ) ) ),
    RecvPropFloat( RECVINFO( m_flPlayerVelocity ) ),
    RecvPropFloat( RECVINFO( m_flPhysicsVelocity ) ),
    RecvPropQAngles( RECVINFO( m_vecLaunchAngles ) ),
    RecvPropInt( RECVINFO( m_ExactVelocityChoice ) ),
    RecvPropBool( RECVINFO( m_bUseExactVelocity ) ),
    RecvPropBool( RECVINFO( m_bUseThresholdCheck ) ),
    RecvPropBool( RECVINFO( m_bOnlyVelocityCheck ) ),
    RecvPropFloat( RECVINFO( m_flLowerThreshold ) ),
    RecvPropFloat( RECVINFO( m_flUpperThreshold ) ),
    RecvPropFloat( RECVINFO( m_flAirControlSupressionTime ) ),
    RecvPropBool( RECVINFO( m_bApplyAngularImpulse ) ),
    RecvPropFloat( RECVINFO( m_flEntryAngleTolerance ) ),
    RecvPropEHandle( RECVINFO( m_hLaunchTarget ) ),
    RecvPropBool( RECVINFO( m_bPlayersPassTriggerFilters ) ),
    RecvPropBool( RECVINFO( m_bDirectionSuppressAirControl ) ),
    END_RECV_TABLE()

        BEGIN_PREDICTION_DATA( C_TriggerCatapult ) DEFINE_PRED_ARRAY(
            m_flRefireDelay, FIELD_FLOAT, MAX_PLAYERS + 1, FTYPEDESC_INSENDTABLE ),
    END_PREDICTION_DATA()

        LINK_ENTITY_TO_CLASS( trigger_catapult, C_TriggerCatapult );

C_BasePlayer *C_TriggerCatapult::GetPredictionOwner()
{
	return m_bClientSidePredicted ? C_BasePlayer::GetLocalPlayer() : NULL;
}

bool C_TriggerCatapult::ShouldPredict()
{
	C_BasePlayer *pPlayer = GetPredictionOwner();
	return pPlayer && pPlayer->IsLocalPlayer();
}

void C_TriggerCatapult::OnDataChanged( DataUpdateType_t updateType )
{
	SetPredictionEligible( m_bClientSidePredicted );
	BaseClass::OnDataChanged( updateType );
}

void C_TriggerCatapult::UpdatePartitionListEntry()
{
	if ( !m_bClientSidePredicted )
	{
		BaseClass::UpdatePartitionListEntry();
		return;
	}

	::partition->RemoveAndInsert( PARTITION_CLIENT_SOLID_EDICTS |
	                                  PARTITION_CLIENT_RESPONSIVE_EDICTS |
	                                  PARTITION_CLIENT_NON_STATIC_EDICTS,
	    PARTITION_CLIENT_TRIGGER_ENTITIES, CollisionProp()->GetPartitionHandle() );
}

void C_TriggerCatapult::EndTouch( C_BaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );
	m_hAbortedLaunchees.FindAndFastRemove( pOther );
}

void C_TriggerCatapult::LaunchThink()
{
	for ( int i = 0; i < m_hAbortedLaunchees.Count(); ++i )
	{
		C_BaseEntity *pOther = m_hAbortedLaunchees[i].Get();
		if ( !pOther || !pOther->IsPlayer() )
		{
			m_hAbortedLaunchees.Remove( i-- );
			continue;
		}
		StartTouch( pOther );
	}

	if ( m_hAbortedLaunchees.Count() )
	{
		SetThink( &C_TriggerCatapult::LaunchThink );
		SetNextThink( gpGlobals->curtime + 0.05f );
	}
	else
	{
		SetThink( NULL );
	}
}
