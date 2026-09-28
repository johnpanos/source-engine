//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A trigger that calls a member function of its parent when touched.
//			F-Stop's prop_mousetrap uses it for its cheese and arm volumes.
//			Declared as in the CS:GO-era triggers.h; F-Stop's registration of
//			trigger_callback is not in the F-Stop drop, so it lives here.
//
//=============================================================================//

#ifndef TRIGGER_CALLBACK_H
#define TRIGGER_CALLBACK_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

class CTriggerCallback : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerCallback, CBaseTrigger );
	DECLARE_DATADESC();

	CTriggerCallback() : m_pfnCallback( NULL ) {}

	virtual void Spawn( void );
	virtual void StartTouch( CBaseEntity *pOther );

	static CTriggerCallback *Create( const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecMins, const Vector &vecMaxs, CBaseEntity *pOwner, void (CBaseEntity::*pfnCallback)(CBaseEntity *) )
	{
		CTriggerCallback *pTrigger = (CTriggerCallback *) CreateEntityByName( "trigger_callback" );
		if ( pTrigger == NULL )
			return NULL;

		UTIL_SetOrigin( pTrigger, vecOrigin );
		pTrigger->SetAbsAngles( vecAngles );
		UTIL_SetSize( pTrigger, vecMins, vecMaxs );

		DispatchSpawn( pTrigger );

		pTrigger->SetParent( (CBaseEntity *) pOwner );

		// Save our callback function
		pTrigger->m_pfnCallback = pfnCallback;

		return pTrigger;
	}

private:
	// Not saved: owners destroy and recreate their callback triggers on restore.
	void (CBaseEntity::*m_pfnCallback)(CBaseEntity *);
};

#endif // TRIGGER_CALLBACK_H
