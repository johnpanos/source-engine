//========= F-Stop port =======================================================//
//
// Purpose: Client side of env_dof_controller (F-Stop's weapon_camera focus).
//
// Ported from CS:GO game/client/c_env_dof_controller.cpp so single player has
// the client class the server's DT_EnvDOFController needs. CS:GO applied the
// values in its depth-of-field post-process; this client has no such pass, so
// the controller keeps the networked parameters and says once, when a map or
// the camera turns depth of field on, that it is not drawn.
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_EnvDOFController : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvDOFController, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	C_EnvDOFController();

	virtual void OnDataChanged( DataUpdateType_t updateType );

private:
	C_EnvDOFController( const C_EnvDOFController & );

	bool  m_bDOFEnabled;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;
};

IMPLEMENT_CLIENTCLASS_DT( C_EnvDOFController, DT_EnvDOFController, CEnvDOFController )
	RecvPropInt( RECVINFO( m_bDOFEnabled ) ),
	RecvPropFloat( RECVINFO( m_flNearBlurDepth ) ),
	RecvPropFloat( RECVINFO( m_flNearFocusDepth ) ),
	RecvPropFloat( RECVINFO( m_flFarFocusDepth ) ),
	RecvPropFloat( RECVINFO( m_flFarBlurDepth ) ),
	RecvPropFloat( RECVINFO( m_flNearBlurRadius ) ),
	RecvPropFloat( RECVINFO( m_flFarBlurRadius ) )
END_RECV_TABLE()

C_EnvDOFController::C_EnvDOFController()
	: m_bDOFEnabled( true ),
	m_flNearBlurDepth( 50.0f ),
	m_flNearFocusDepth( 100.0f ),
	m_flFarFocusDepth( 250.0f ),
	m_flFarBlurDepth( 1000.0f ),
	m_flNearBlurRadius( 0.0f ),		// no near blur by default
	m_flFarBlurRadius( 5.0f )
{
}

void C_EnvDOFController::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	bool bBlurs = m_bDOFEnabled && ( m_flNearBlurRadius > 0.0f || m_flFarBlurRadius > 0.0f );
	static bool s_bWarned = false;
	if ( bBlurs && !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "env_dof_controller: depth of field is enabled, but this renderer has no depth-of-field pass; it is not drawn.\n" );
	}
}
