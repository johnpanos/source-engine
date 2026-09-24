//========= Portal 2 reconstruction ============================================//
//
// Purpose: logic_timescale: blends the game timescale through CGameTimescale.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Behavior follows the
// retail Portal 2 server's CLogicTimeScale input handlers; the timescale
// itself is owned by game/shared/game_timescale_shared.cpp. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "logic_timescale.h"
#include "game_timescale_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CLogicTimeScale )

	DEFINE_KEYFIELD( m_flBlendTime, FIELD_FLOAT, "BlendTime" ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetDesiredTimescale", InputSetDesiredTimescale ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimescaleBlendTime", InputSetTimescaleBlendTime ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_timescale, CLogicTimeScale );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CLogicTimeScale::CLogicTimeScale( void )
{
	m_flBlendTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Starts blending toward the requested timescale over m_flBlendTime
//-----------------------------------------------------------------------------
void CLogicTimeScale::InputSetDesiredTimescale( inputdata_t &inputdata )
{
	GameTimescale()->SetDesiredTimescale( inputdata.value.Float(), m_flBlendTime, CGameTimescale::INTERPOLATOR_LINEAR, 0.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicTimeScale::InputSetTimescaleBlendTime( inputdata_t &inputdata )
{
	// The retail handler leaves a negative blend time untouched
	if ( m_flBlendTime >= 0.0f )
	{
		m_flBlendTime = inputdata.value.Float();
	}
}
