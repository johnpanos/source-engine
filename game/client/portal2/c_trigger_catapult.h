//========= Portal 2 reconstruction ============================================//
//
// Purpose: Predicted client representation of trigger_catapult. The receive
//          fields mirror the retail server's DT_TriggerCatapult table.
//
// Reconstructed from the retained Portal 2 binary tables and the TF2 copy of
// Portal 2 catapult launch logic. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_TRIGGER_CATAPULT_H
#define C_TRIGGER_CATAPULT_H

#include "c_triggers.h"
#include "networkvar.h"

DECLARE_AUTO_LIST( ITriggerCatapultAutoList );

class C_TriggerCatapult : public C_BaseTrigger, public ITriggerCatapultAutoList
{
public:
	DECLARE_CLASS( C_TriggerCatapult, C_BaseTrigger );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual C_BaseEntity *GetEntity() { return this; }
	virtual C_BasePlayer *GetPredictionOwner();
	virtual bool ShouldPredict();
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void UpdatePartitionListEntry();
	virtual void StartTouch( C_BaseEntity *pOther );
	virtual void EndTouch( C_BaseEntity *pOther );

	void LaunchThink();

private:
	void LaunchByTarget( C_BaseEntity *pVictim, C_BaseEntity *pTarget );
	Vector CalculateLaunchVector( C_BaseEntity *pVictim, C_BaseEntity *pTarget );
	Vector CalculateLaunchVectorPreserve( Vector vecInitialVelocity, C_BaseEntity *pVictim,
	    C_BaseEntity *pTarget, bool bForcePlayer = false );
	void LaunchByDirection( C_BaseEntity *pVictim );
	void OnLaunchedVictim( C_BaseEntity *pVictim );

	CNetworkArray( float, m_flRefireDelay, MAX_PLAYERS + 1 );
	float m_flPlayerVelocity;
	float m_flPhysicsVelocity;
	QAngle m_vecLaunchAngles;
	int m_ExactVelocityChoice;
	bool m_bUseExactVelocity;
	bool m_bUseThresholdCheck;
	bool m_bOnlyVelocityCheck;
	float m_flLowerThreshold;
	float m_flUpperThreshold;
	float m_flAirControlSupressionTime;
	bool m_bApplyAngularImpulse;
	float m_flEntryAngleTolerance;
	CHandle<C_BaseEntity> m_hLaunchTarget;
	bool m_bPlayersPassTriggerFilters;
	bool m_bDirectionSuppressAirControl;
	CUtlVector<CHandle<C_BaseEntity>> m_hAbortedLaunchees;
};

#endif // C_TRIGGER_CATAPULT_H
