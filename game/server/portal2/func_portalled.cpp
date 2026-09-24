//========= Portal 2 reconstruction ============================================//
//
// Purpose: Fires outputs when an entity teleports through a portal in its volume
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "func_portalled.h"
#include "portal/prop_portal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_portalled, CFunc_Portalled );

BEGIN_DATADESC( CFunc_Portalled )
	DEFINE_KEYFIELD( m_bFireOnPlayer, FIELD_BOOLEAN, "FireOnPlayer" ),
	DEFINE_KEYFIELD( m_bFireOnDeparture, FIELD_BOOLEAN, "FireOnDeparture" ),
	DEFINE_KEYFIELD( m_bFireOnArrival, FIELD_BOOLEAN, "FireOnArrival" ),

	DEFINE_OUTPUT( m_OnEntityPrePortalled, "OnEntityPrePortalled" ),
	DEFINE_OUTPUT( m_OnEntityPostPortalled, "OnEntityPostPortalled" ),
END_DATADESC()


void CFunc_Portalled::PortalPlacedInsideBounds( CProp_Portal *pPortal )
{
	pPortal->SetFuncPortalled( this );
}

void CFunc_Portalled::OnPrePortalled( CBaseEntity *pOther, bool bDeparting )
{
	if ( bDeparting )
	{
		if ( !m_bFireOnDeparture )
			return;
	}
	else
	{
		if ( !m_bFireOnArrival )
			return;
	}

	// Reconstruction note: both builds only fire when "FireOnPlayer" is set and the
	// teleported entity is a player; non-player entities never fire the outputs.
	if ( m_bFireOnPlayer && pOther->IsPlayer() )
	{
		m_OnEntityPrePortalled.FireOutput( pOther, this );
	}
}

void CFunc_Portalled::OnPostPortalled( CBaseEntity *pOther, bool bDeparting )
{
	if ( bDeparting )
	{
		if ( !m_bFireOnDeparture )
			return;
	}
	else
	{
		if ( !m_bFireOnArrival )
			return;
	}

	if ( m_bFireOnPlayer && pOther->IsPlayer() )
	{
		m_OnEntityPostPortalled.FireOutput( pOther, this );
	}
}
