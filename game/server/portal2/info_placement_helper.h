//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that snaps nearby portal placement to a designer spot
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef INFO_PLACEMENT_HELPER_H
#define INFO_PLACEMENT_HELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "portal_shareddefs.h"

class CPortal_Base2D;

class CInfoPlacementHelper : public CPointEntity
{
	DECLARE_CLASS( CInfoPlacementHelper, CPointEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CInfoPlacementHelper( void );
	virtual void	Spawn( void );
	virtual void	UpdateOnRemove( void );
	virtual void	DrawDebugGeometryOverlays( void );
	virtual void	NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier );
	virtual void	OnRestore( void );

	virtual int		UpdateTransmitState( void )
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	virtual int		ObjectCaps( void )
	{
		// Helpers created by other entities (projected bridges, lasers) are owned by them and never saved
		int flags = ( GetOwnerEntity() != NULL ) ? FCAP_DONT_SAVE : 0;
		return ( BaseClass::ObjectCaps() | flags );
	}

	float			GetTargetRadius( void ) { return m_flRadius; }

	void			OnObjectPlaced( CBaseEntity *pActivator )
	{
		m_OnObjectPlaced.FireOutput( pActivator, this );
	}

	CBaseEntity		*GetTargetOverride( void )
	{
		if ( m_strTargetEntity.Get() == NULL_STRING )
			return NULL;

		return gEntList.FindEntityByName( NULL, m_strTargetEntity );
	}

	const Vector	&GetTargetOrigin( void )
	{
		// Use the proxy's position when one was specified
		if ( m_strTargetProxy.Get() != NULL_STRING )
		{
			CBaseEntity *pProxy = gEntList.FindEntityByName( NULL, m_strTargetProxy );
			if ( pProxy != NULL )
				return pProxy->GetAbsOrigin();

			Warning( "Placement proxy entity %s not found!\n", STRING( m_strTargetProxy ) );
		}

		return GetAbsOrigin();
	}

	const QAngle	&GetTargetAngles( void )
	{
		// Use the proxy's orientation when one was specified
		if ( m_strTargetProxy.Get() != NULL_STRING )
		{
			CBaseEntity *pProxy = gEntList.FindEntityByName( NULL, m_strTargetProxy );
			if ( pProxy != NULL )
				return pProxy->GetAbsAngles();

			Warning( "Placement proxy entity %s not found!\n", STRING( m_strTargetProxy ) );
		}

		return GetAbsAngles();
	}

	bool			ShouldUseHelperAngles( void );
	bool			ShouldForcePlacement( void ) { return m_bForcePlacement; }

	void			Enable( void ) { m_bDisabled = false; }
	void			Disable( void ) { m_bDisabled = true; }
	void			Disable( float flDuration ) { m_flDisableTime = gpGlobals->curtime + flDuration; }
	bool			IsEnabled( void );
	void			BindToPortal( CPortal_Base2D *pPortal );

	void			InputEnable( inputdata_t &inputdata );
	void			InputDisable( inputdata_t &inputdata );

	COutputEvent	m_OnObjectPlaced;
	COutputFloat	m_ObjectPlacedSize;

private:
	CNetworkVar( string_t, m_strTargetProxy );		// Entity whose position/angles stand in for ours
	CNetworkVar( string_t, m_strTargetEntity );		// Entity that placed objects are attached to
	CNetworkVar( float, m_flRadius );
	CNetworkVar( bool, m_bSnapToHelperAngles );
	CNetworkVar( bool, m_bForcePlacement );
	CNetworkVar( bool, m_bDisabled );
	CNetworkVar( float, m_flDisableTime );			// Helper is ignored until this time
	CNetworkVar( bool, m_bDeferringToPortal );		// A portal is resting on us; wait for it to go away
};

CInfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, CBasePlayer *pPlayer );

#endif // INFO_PLACEMENT_HELPER_H
