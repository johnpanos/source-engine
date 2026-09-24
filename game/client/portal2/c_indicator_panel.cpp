//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the test chamber indicator / countdown panel
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_indicator_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_LabIndicatorPanel, DT_LabIndicatorPanel, CLabIndicatorPanel )
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropBool( RECVINFO( m_bStopped ) ),
	RecvPropBool( RECVINFO( m_bCountingDown ) ),
	RecvPropBool( RECVINFO( m_bIsCountdownTimer ) ),
	RecvPropBool( RECVINFO( m_bIsChecked ) ),
	RecvPropInt( RECVINFO( m_iPlayerPinged ) ),
	RecvPropFloat( RECVINFO( m_flTimerDuration ) ),
	RecvPropFloat( RECVINFO( m_flTimerStart ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Fraction of the countdown remaining (1 = full time left, 0 = done)
//-----------------------------------------------------------------------------
float C_LabIndicatorPanel::GetCountdownPercentage( void )
{
	if ( m_bCountingDown == false )
		return 1.0f;

	return RemapValClamped( gpGlobals->curtime, m_flTimerStart, m_flTimerStart + m_flTimerDuration, 1.0f, 0.0f );
}
