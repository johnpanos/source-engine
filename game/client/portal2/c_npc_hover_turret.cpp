//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of npc_hover_turret: draws the aiming/firing laser beam.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "beam_shared.h"
#include "portal_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HOVER_TURRET_EYE_ATTACHMENT		1
#define HOVER_TURRET_LASER_ATTACHMENT	2
#define HOVER_TURRET_LASER_RANGE		8192

#define HOVER_TURRET_END_POINT_PULSE_SCALE	4.0f

class C_NPC_Hover_Turret : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Hover_Turret, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	virtual ~C_NPC_Hover_Turret( void );

	virtual void	Spawn( void );
	virtual void	ClientThink( void );

	bool	IsLaserOn( void ) { return m_pBeam != NULL; }
	void	LaserOff( void );
	void	LaserOn( void );
	float	LaserEndPointSize( void );

private:
	C_Beam	*m_pBeam;

	// Laser state as of the last beam update, and the networked value
	int		m_iLastLaserState;
	int		m_iLaserState;
	int		m_sLaserHaloSprite;
	float	m_fPulseOffset;
	float	m_flLastUpdateTime;

	float	m_bBeamFlickerOff;
	float	m_fBeamFlickerTime;
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Hover_Turret, DT_NPC_HoverTurret, CNPC_HoverTurret )
	RecvPropInt( RECVINFO( m_iLaserState ) ),
	RecvPropInt( RECVINFO( m_sLaserHaloSprite ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_NPC_Hover_Turret::~C_NPC_Hover_Turret( void )
{
	LaserOff();

	if ( m_pBeam )
	{
		m_pBeam->Remove();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_NPC_Hover_Turret::Spawn( void )
{
	SetThink( &C_NPC_Hover_Turret::ClientThink );
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	m_pBeam = NULL;
	m_fPulseOffset = RandomFloat( 0.0f, 2.0f * M_PI );

	m_bBeamFlickerOff = false;
	m_fBeamFlickerTime = 0.0f;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_NPC_Hover_Turret::ClientThink( void )
{
	if ( m_iLaserState > 0 )
	{
		LaserOn();
	}
	else
	{
		LaserOff();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_NPC_Hover_Turret::LaserOff( void )
{
	if ( m_pBeam )
	{
		m_pBeam->AddEffects( EF_NODRAW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create or refresh the beam for the current laser state and aim it.
//-----------------------------------------------------------------------------
void C_NPC_Hover_Turret::LaserOn( void )
{
	if ( !IsBoneAccessAllowed() )
	{
		LaserOff();
		return;
	}

	Vector vecMuzzle;
	QAngle angMuzzleDir;
	GetAttachment( HOVER_TURRET_LASER_ATTACHMENT, vecMuzzle, angMuzzleDir );

	Vector vecEye;
	QAngle angEyeDir;
	GetAttachment( HOVER_TURRET_EYE_ATTACHMENT, vecEye, angEyeDir );

	Vector vecMuzzleDir;
	AngleVectors( angEyeDir, &vecMuzzleDir );

	if ( m_iLastLaserState != m_iLaserState || !m_pBeam )
	{
		// The laser changed mode: rebuild the beam
		if ( m_pBeam )
		{
			delete m_pBeam;
			m_pBeam = NULL;
		}

		if ( m_iLaserState == 1 )
		{
			// Thin aiming laser
			m_pBeam = C_Beam::BeamCreate( "effects/bluelaser1.vmt", 1.0f );
			m_pBeam->SetColor( 90, 255, 255 );
			m_pBeam->SetHaloTexture( m_sLaserHaloSprite );
			m_pBeam->SetHaloScale( 1.5f );
		}
		else if ( m_iLaserState == 2 )
		{
			// Firing beam
			m_pBeam = C_Beam::BeamCreate( "sprites/bluelaser1.vmt", 12.0f );
			m_pBeam->Activate();
			m_pBeam->SetWidth( 16.0f );
			m_pBeam->SetEndWidth( 16.0f );
		}
	}
	else
	{
		m_pBeam->RemoveEffects( EF_NODRAW );
	}

	m_iLastLaserState = m_iLaserState;

	// Trace to find an endpoint
	Vector vEndPoint;
	float fEndFraction;
	Ray_t rayPath;
	rayPath.Init( vecMuzzle, vecMuzzle + vecMuzzleDir * HOVER_TURRET_LASER_RANGE );

	CTraceFilterSkipClassname traceFilter( this, "prop_energy_ball", COLLISION_GROUP_NONE );

	if ( UTIL_Portal_TraceRay_Beam( rayPath, MASK_SHOT, &traceFilter, &fEndFraction ) )
		vEndPoint = vecMuzzle + vecMuzzleDir * HOVER_TURRET_LASER_RANGE;	// Trace went through portal and endpoint is unknown
	else
		vEndPoint = vecMuzzle + vecMuzzleDir * HOVER_TURRET_LASER_RANGE * fEndFraction;	// Trace hit a wall

	// The beam is backwards, sort of. The endpoint is the turret. This is
	// so that the beam can be tapered to very thin where it emits from the turret.
	m_pBeam->PointsInit( vEndPoint, vecMuzzle );

	m_pBeam->SetHaloScale( LaserEndPointSize() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float C_NPC_Hover_Turret::LaserEndPointSize( void )
{
	return ( ( MAX( 0.0f, sinf( gpGlobals->curtime * M_PI + m_fPulseOffset ) ) ) * HOVER_TURRET_END_POINT_PULSE_SCALE + 3.0f ) * 1.5f;
}
