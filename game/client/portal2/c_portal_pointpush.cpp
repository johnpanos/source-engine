//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the portal point push; predicts pushing paint blobs
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_portal_pointpush.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Portal 2 port: the header aliases CPortal_PointPush to the client class for
// the shared code; the network name must stay the server class name.
#undef CPortal_PointPush
IMPLEMENT_CLIENTCLASS_DT( C_Portal_PointPush, DT_PortalPointPush, CPortal_PointPush )
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropFloat( RECVINFO( m_flMagnitude ) ),
	RecvPropFloat( RECVINFO( m_flRadius ) ),
	RecvPropFloat( RECVINFO( m_flInnerRadius ) ),
	RecvPropFloat( RECVINFO( m_flConeOfInfluence ) ),
END_RECV_TABLE()


C_Portal_PointPush::C_Portal_PointPush()
{
}


void C_Portal_PointPush::Spawn( void )
{
	BaseClass::Spawn();
}


void C_Portal_PointPush::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( gpGlobals->curtime + 0.05f );
	}
}


void C_Portal_PointPush::ClientThink( void )
{
	PushThink();
}
