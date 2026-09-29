//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A volume which bumps portal placement.
//
//======================================================================================//

#ifndef FUNC_PORTAL_BUMPER_H
#define FUNC_PORTAL_BUMPER_H

#include "cbase.h"

class CFuncPortalBumper : public CBaseEntity
{
public:
	DECLARE_CLASS( CFuncPortalBumper, CBaseEntity );
#ifdef PORTAL2
	// Portal 2 networks the bumper, so the client's predicted portal
	// placement bumps off it too (DT_FuncPortalBumper in the 2010 dSYMs and
	// the retail client and server)
	DECLARE_SERVERCLASS();
#endif

	CFuncPortalBumper();

	virtual void Spawn( void );

	void InputActivate( inputdata_t &inputdata );
	void InputDeactivate( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	bool IsActive() { return m_bActive; }

	DECLARE_DATADESC();

private:
#ifdef PORTAL2
	CNetworkVar( bool, m_bActive );
#else
	bool m_bActive;
#endif
};

#endif // FUNC_PORTAL_BUMPER_H
