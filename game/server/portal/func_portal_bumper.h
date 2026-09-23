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

	CFuncPortalBumper();

	virtual void Spawn( void );

	void InputActivate( inputdata_t &inputdata );
	void InputDeactivate( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	bool IsActive() { return m_bActive; }

	DECLARE_DATADESC();

private:
	bool m_bActive;
};

#endif // FUNC_PORTAL_BUMPER_H
