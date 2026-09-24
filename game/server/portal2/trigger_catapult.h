//========= Portal 2 reconstruction ============================================//
//
// Purpose: trigger_catapult: flings players and physics objects either in a
//          fixed direction or along a ballistic arc onto a launch target.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). The TF2 copy of this
// entity (hl2_src game/server/tf/trigger_catapult.*, "copied from portal2
// code") and the retail Portal 2 server's datadesc and DT_TriggerCatapult
// tables supply the members, key names and send order. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
// Networked (DT_TriggerCatapult, baseclass DT_BaseTrigger), in send order:
//   m_flRefireDelay[ MAX_PLAYERS + 1 ]  float array; [0] physics objects,
//                                       [entindex] each player
//   m_flPlayerVelocity                  float
//   m_flPhysicsVelocity                 float
//   m_vecLaunchAngles                   QAngle
//   m_ExactVelocityChoice               int
//   m_bUseExactVelocity                 bool
//   m_bUseThresholdCheck                bool
//   m_bOnlyVelocityCheck                bool
//   m_flLowerThreshold                  float
//   m_flUpperThreshold                  float
//   m_flAirControlSupressionTime        float
//   m_bApplyAngularImpulse              bool
//   m_flEntryAngleTolerance             float
//   m_hLaunchTarget                     EHANDLE
//   m_bPlayersPassTriggerFilters        bool
//   m_bDirectionSuppressAirControl      bool
//
// game/shared/portal2/trigger_catapult_shared.cpp implements the launch code
// for both sides and owns IMPLEMENT_AUTO_LIST( ITriggerCatapultAutoList ) and
// the catapult_physics_drag_boost ConVar. The client class must declare the
// same members and the shared methods listed below.
//
//=============================================================================//

#ifndef TRIGGER_CATAPULT_H
#define TRIGGER_CATAPULT_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

DECLARE_AUTO_LIST( ITriggerCatapultAutoList );

class CTriggerCatapult : public CBaseTrigger, public ITriggerCatapultAutoList
{
	DECLARE_CLASS( CTriggerCatapult, CBaseTrigger );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:

	CTriggerCatapult( void );

	virtual void	Spawn( void );
	virtual void	StartTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );
	virtual int		DrawDebugTextOverlays( void );
	virtual void	DrawDebugGeometryOverlays( void );

	void			LaunchThink( void );

	void			PlayerPassesTriggerFiltersThink( void );
	static const char *s_szPlayerPassesTriggerFiltersThinkContext;

private:
	void			InputSetPlayerSpeed( inputdata_t &in );
	void			InputSetPhysicsSpeed( inputdata_t &in );
	void			InputSetLaunchTarget( inputdata_t &in );
	void			InputSetExactVelocityChoiceType( inputdata_t &in );

	// Shared with the client (trigger_catapult_shared.cpp)
	void			LaunchByTarget( CBaseEntity *pVictim, CBaseEntity *pTarget );
	Vector			CalculateLaunchVector( CBaseEntity *pVictim, CBaseEntity *pTarget );
	Vector			CalculateLaunchVectorPreserve( Vector vecInitialVelocity, CBaseEntity *pVictim, CBaseEntity *pTarget, bool bForcePlayer = false );

	void			LaunchByDirection( CBaseEntity *pVictim );
	void			OnLaunchedVictim( CBaseEntity *pVictim );

	CNetworkArray( float, m_flRefireDelay, MAX_PLAYERS + 1 ); // 0 for physics objects, the rest for each player entindex
	CNetworkVar( float, m_flPlayerVelocity );
	CNetworkVar( float, m_flPhysicsVelocity );
	CNetworkQAngle( m_vecLaunchAngles );
	string_t m_strLaunchTarget;

	CNetworkVar( int, m_ExactVelocityChoice );
	CNetworkVar( bool, m_bUseExactVelocity );

	CNetworkVar( bool, m_bUseThresholdCheck );
	CNetworkVar( float, m_flLowerThreshold );
	CNetworkVar( float, m_flUpperThreshold );
	CNetworkVar( float, m_flEntryAngleTolerance );

	CNetworkHandle( CBaseEntity, m_hLaunchTarget );

	CNetworkVar( bool, m_bOnlyVelocityCheck );
	CNetworkVar( bool, m_bApplyAngularImpulse );
	CNetworkVar( bool, m_bPlayersPassTriggerFilters );

	CNetworkVar( float, m_flAirControlSupressionTime );	// After a catapult, stop air control for this long (or a quarter second if this value is negative)
	CNetworkVar( bool, m_bDirectionSuppressAirControl );	// Use air control suppression for directional catapults too

	COutputEvent	m_OnCatapulted;

	CUtlVector< EHANDLE > m_hAbortedLaunchees;
};

#endif // TRIGGER_CATAPULT_H
