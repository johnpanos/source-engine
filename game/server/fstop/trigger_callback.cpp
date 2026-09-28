//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A trigger that calls a member function of its parent when touched.
//			The implementation is CS:GO's triggers.cpp CTriggerCallback.
//
//=============================================================================//

#include "cbase.h"
#include "trigger_callback.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( trigger_callback, CTriggerCallback );

BEGIN_DATADESC( CTriggerCallback )
END_DATADESC()

//----------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------
void CTriggerCallback::Spawn( void )
{
	// Setup our basic attributes
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_OBB );
	SetSolidFlags( FSOLID_NOT_SOLID|FSOLID_TRIGGER );

	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS|SF_TRIGGER_ALLOW_NPCS|SF_TRIGGER_ALLOW_PHYSICS );
}

//----------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------
void CTriggerCallback::StartTouch( CBaseEntity *pOther )
{
	// Don't touch things under certain circumstances
	if( pOther->VPhysicsGetObject() )
	{
		if( pOther->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
			return;
	}

	if ( PassesTriggerFilters( pOther ) == false )
		return;

	Assert( m_pfnCallback );

	if ( GetParent() && m_pfnCallback )
	{
		(GetParent()->*m_pfnCallback)( pOther );
	}

	BaseClass::StartTouch( pOther );
}
