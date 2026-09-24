//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that asks every client to show a survey panel.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "pointsurvey.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_disable_survey_panel( "cl_disable_survey_panel", "0", FCVAR_REPLICATED | FCVAR_ARCHIVE );

LINK_ENTITY_TO_CLASS( point_survey, CPointSurvey );

BEGIN_DATADESC( CPointSurvey )

	DEFINE_KEYFIELD( m_iszSurveyName, FIELD_STRING, "surveyname" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ShowSurvey", InputShowSurvey ),

	DEFINE_OUTPUT( m_OnSurveyComplete, "OnSurveyComplete" ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPointSurvey, DT_PointSurvey )
	SendPropStringT( SENDINFO( m_iszSurveyName ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Show the survey on every client, or skip straight to completion
//			when surveys are disabled.
//-----------------------------------------------------------------------------
void CPointSurvey::InputShowSurvey( inputdata_t &inputdata )
{
	if ( cl_disable_survey_panel.GetInt() )
	{
		OnSurveyCompleted();
		return;
	}

	CBroadcastRecipientFilter filter;
	filter.MakeReliable();
	UserMessageBegin( filter, "StartSurvey" );
		WRITE_EHANDLE( this );
	MessageEnd();
}

void CPointSurvey::OnSurveyCompleted( void )
{
	m_OnSurveyComplete.FireOutput( this, this );
}

void CPointSurvey::Activate( void )
{
	BaseClass::Activate();

	// Clients need the survey name whenever the panel opens
	SetTransmitState( FL_EDICT_ALWAYS );
}
