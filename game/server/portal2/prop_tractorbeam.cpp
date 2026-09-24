//========= Portal 2 reconstruction ============================================//
//
// Purpose: Excursion funnel emitter (prop_tractor_beam).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "prop_tractorbeam.h"
#include "trigger_tractorbeam.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CPropTractorBeamProjector )

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetLinearForce", InputSetLinearForce ),

	DEFINE_KEYFIELD( m_flLinearForce, FIELD_FLOAT, "linearForce" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( prop_tractor_beam, CPropTractorBeamProjector );

IMPLEMENT_SERVERCLASS_ST( CPropTractorBeamProjector, DT_PropTractorBeamProjector )
	SendPropFloat( SENDINFO( m_flLinearForce ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vEndPos ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTractorBeamProjector::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

	SetModel( "models/props/tractor_beam_emitter.mdl" );
	SetSolid( SOLID_VPHYSICS );

	// Reconstruction note: both builds start sequence index 2 directly rather than looking
	// it up by name.
	ResetSequence( 2 );
	UseClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTractorBeamProjector::Precache( void )
{
	PrecacheModel( "models/props/tractor_beam_emitter.mdl" );

	PrecacheParticleSystem( "tractor_beam_arm" );
	PrecacheParticleSystem( "tractor_beam_core" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTractorBeamProjector::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Tractor beam projectors project tractor beams
//-----------------------------------------------------------------------------
CBaseProjectedEntity *CPropTractorBeamProjector::CreateNewProjectedEntity( void )
{
	return CProjectedTractorBeamEntity::CreateNewInstance();
}

//-----------------------------------------------------------------------------
// Purpose: Change the beam's force; a zero force turns the beam off
//-----------------------------------------------------------------------------
void CPropTractorBeamProjector::InputSetLinearForce( inputdata_t &indata )
{
	m_flLinearForce = indata.value.Float();

	if ( m_flLinearForce != 0.0f )
	{
		Project();
	}
	else
	{
		EnableProjection( false );
		m_vEndPos = vec3_origin;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropTractorBeamProjector::Project( void )
{
	BaseClass::Project();

	m_vEndPos = m_hFirstChild->GetEndPoint();
}
