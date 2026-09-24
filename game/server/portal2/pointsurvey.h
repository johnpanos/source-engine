//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that asks every client to show a survey panel.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef POINTSURVEY_H
#define POINTSURVEY_H
#ifdef _WIN32
#pragma once
#endif

class CPointSurvey : public CBaseEntity
{
public:
	DECLARE_CLASS( CPointSurvey, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void Activate( void );

	// Called when a client reports the survey panel was closed
	void OnSurveyCompleted( void );

protected:

	void InputShowSurvey( inputdata_t &inputdata );

	CNetworkVar( string_t, m_iszSurveyName );
	COutputEvent m_OnSurveyComplete;
};

#endif // POINTSURVEY_H
