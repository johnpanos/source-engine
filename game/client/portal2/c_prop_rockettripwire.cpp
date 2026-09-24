//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of prop_rocket_tripwire: draws the tripwire beam.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_baseanimating.h"
#include "beam_shared.h"
#include "portal_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ROCKET_TRIPWIRE_BEAM_SPRITE		"effects/bluelaser1.vmt"
#define ROCKET_TRIPWIRE_RANGE			8192.0f

class C_PropRocketTripwire : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PropRocketTripwire, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	C_PropRocketTripwire();
	virtual ~C_PropRocketTripwire();

	virtual void Spawn( void );
	virtual void ClientThink( void );

	virtual ITraceFilter *GetBeamTraceFilter( void ) { return &m_filterBeams; }

private:

	C_Beam *m_pBeam;
	int m_nBeamEndpointTexture;
	int m_iMuzzleAttachment;
	bool m_bDrawBeams;

	EHANDLE m_hRocket;

	// The beam passes through players and our own rocket
	CTraceFilterSkipClassname m_filterBeams;
};

IMPLEMENT_CLIENTCLASS_DT( C_PropRocketTripwire, DT_PropRocketTripwire, CPropRocketTripwire )
	RecvPropInt( RECVINFO( m_iMuzzleAttachment ) ),
	RecvPropInt( RECVINFO( m_nBeamEndpointTexture ) ),
	RecvPropBool( RECVINFO( m_bDrawBeams ) ),
	RecvPropEHandle( RECVINFO( m_hRocket ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_PropRocketTripwire::C_PropRocketTripwire() : m_filterBeams( this, "player", COLLISION_GROUP_DEBRIS )
{
	m_pBeam = NULL;
	m_hRocket = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_PropRocketTripwire::~C_PropRocketTripwire()
{
	if ( m_pBeam )
	{
		m_pBeam->Remove();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropRocketTripwire::Spawn( void )
{
	SetThink( &C_PropRocketTripwire::ClientThink );
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	m_pBeam = NULL;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Keep the tripwire beam aimed along the barrel
//-----------------------------------------------------------------------------
void C_PropRocketTripwire::ClientThink( void )
{
	if ( !IsBoneAccessAllowed() )
		return;

	Vector vecMuzzle;
	QAngle angMuzzleDir;
	GetAttachment( m_iMuzzleAttachment, vecMuzzle, angMuzzleDir );

	Vector vecAimDir;
	AngleVectors( angMuzzleDir, &vecAimDir );

	if ( !m_pBeam )
	{
		m_pBeam = C_Beam::BeamCreate( ROCKET_TRIPWIRE_BEAM_SPRITE, 0.1f );
		m_pBeam->SetHaloTexture( m_nBeamEndpointTexture );
		m_pBeam->SetColor( 100, 100, 255 );
		m_pBeam->SetBrightness( 100 );
		m_pBeam->SetNoise( 0 );
		m_pBeam->SetWidth( 1 );
		m_pBeam->SetEndWidth( 0 );
		m_pBeam->SetScrollRate( 0 );
		m_pBeam->SetFadeLength( 0 );
		m_pBeam->SetHaloScale( 16.0f );
		m_pBeam->SetCollisionGroup( COLLISION_GROUP_NONE );
		m_pBeam->SetBeamFlag( FBEAM_REVERSED );
		m_pBeam->PointsInit( vecMuzzle + vecAimDir, vecMuzzle );
		m_pBeam->SetStartEntity( this );

		m_pBeam->SetWidth( 1 );
		m_pBeam->SetScrollRate( 0 );
		m_pBeam->SetHaloScale( 25.0f );
	}
	else if ( !m_bDrawBeams )
	{
		m_pBeam->AddEffects( EF_NODRAW );
	}
	else
	{
		m_pBeam->RemoveEffects( EF_NODRAW );
	}

	Vector vEndPoint;
	float fEndFraction;
	Ray_t rayPath;
	rayPath.Init( vecMuzzle, vecMuzzle + vecAimDir * ROCKET_TRIPWIRE_RANGE );

	// The rocket must not cut its own tripwire
	m_filterBeams.SetPassEntity( m_hRocket.Get() );

	if ( UTIL_Portal_TraceRay_Beam( rayPath, MASK_VISIBLE_AND_NPCS, &m_filterBeams, &fEndFraction ) )
	{
		// Reconstruction note: through a portal the beam is drawn at full length.
		vEndPoint = vecMuzzle + vecAimDir * ROCKET_TRIPWIRE_RANGE;
	}
	else
	{
		vEndPoint = vecMuzzle + ( vecAimDir * ROCKET_TRIPWIRE_RANGE ) * fEndFraction;
	}

	m_pBeam->PointsInit( vEndPoint, vecMuzzle );
}
