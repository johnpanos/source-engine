//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: stores map postprocess params
//
//
// Portal 2 port: from the CS:GO source drop; the repository's provenance and distribution warning applies.
// This engine's view post-processing has no local contrast, vignette, depth
// blur, screen blur, fade-to-black or film grain stages, so a master controller
// that asks for any of them is reported once instead of applied.
//
//=============================================================================
#include "cbase.h"
#include "c_postprocesscontroller.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CPostProcessController
#undef CPostProcessController
#endif

IMPLEMENT_CLIENTCLASS_DT( C_PostProcessController, DT_PostProcessController, CPostProcessController )
	RecvPropArray3( RECVINFO_NAME( m_PostProcessParameters.m_flParameters[0], m_flPostProcessParameters ), POST_PROCESS_PARAMETER_COUNT, RecvPropFloat( RECVINFO_NAME( m_PostProcessParameters.m_flParameters[0], m_flPostProcessParameters[0] ) ) ),
	RecvPropBool( RECVINFO(m_bMaster) )
END_RECV_TABLE()

C_PostProcessController* C_PostProcessController::ms_pMasterController = NULL;

//-----------------------------------------------------------------------------
C_PostProcessController::C_PostProcessController( void )
: 	m_bMaster( false )
{
	if ( ms_pMasterController == NULL )
	{
		ms_pMasterController = this;
	}
}

//-----------------------------------------------------------------------------
C_PostProcessController::~C_PostProcessController( void )
{
	if ( ms_pMasterController == this )
	{
		ms_pMasterController = NULL;
	}
}

void C_PostProcessController::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	if ( m_bMaster )
	{
		ms_pMasterController = this;
	}

	// Portal 2 port: CS:GO's viewpostprocess.cpp consumes these parameters; this
	// client's post-processing does not. Default parameters change nothing.
	if ( ms_pMasterController == this )
	{
		const PostProcessParameters_t defaults;
		for ( int i = 0; i < POST_PROCESS_PARAMETER_COUNT; ++i )
		{
			if ( i == PPPN_FADE_TIME || m_PostProcessParameters.m_flParameters[i] == defaults.m_flParameters[i] )
				continue;

			static bool s_bWarned = false;
			if ( !s_bWarned )
			{
				s_bWarned = true;
				DevWarning( "Portal 2: postprocess_controller parameters are not supported by this engine\n" );
			}
			break;
		}
	}
}
