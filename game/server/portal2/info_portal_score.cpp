//========= Portal 2 reconstruction ============================================//
//
// Purpose: Networked score and round state for versus chambers.
//
// The send properties and entity I/O names come from the installed Portal 2
// server binary. Round and timer behavior is a conservative reconstruction,
// not original Valve source; the provenance warning applies.
//
//=============================================================================//

#include "cbase.h"

#include "tier0/memdbgon.h"

class CInfoPortalScore : public CBaseEntity
{
public:
	DECLARE_CLASS( CInfoPortalScore, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CInfoPortalScore();
	void Spawn() OVERRIDE;
	int UpdateTransmitState() OVERRIDE;

private:
	void InputStartGame( inputdata_t &inputData );
	void InputResetScore( inputdata_t &inputData );
	void InputAddBlueScore( inputdata_t &inputData );
	void InputAddOrangeScore( inputdata_t &inputData );
	void ScoreThink();

	CNetworkVar( int, m_nBlueScore );
	CNetworkVar( int, m_nBlueRounds );
	CNetworkVar( int, m_nOrangeScore );
	CNetworkVar( int, m_nOrangeRounds );
	CNetworkVar( int, m_nRoundsToWin );
	CNetworkVar( float, m_flTotalTime );
	CNetworkString( m_strGameDescription, 260 );

	float m_flStartTime;
	bool m_bShowCountdown;
	bool m_bGameStarted;
	bool m_bSuddenDeath;
	COutputEvent m_OnBlueWin;
	COutputEvent m_OnOrangeWin;
	COutputEvent m_OnTie;
	COutputEvent m_OnGameStart;
	COutputEvent m_OnGameEnd;
	COutputEvent m_OnRoundEnd;
	COutputEvent m_OnSuddenDeath;
};

LINK_ENTITY_TO_CLASS( info_portal_score, CInfoPortalScore );

// clang-format off
IMPLEMENT_SERVERCLASS_ST( CInfoPortalScore, DT_InfoPortalScore )
	SendPropInt( SENDINFO( m_nBlueScore ) ),
	SendPropInt( SENDINFO( m_nOrangeScore ) ),
	SendPropInt( SENDINFO( m_nBlueRounds ) ),
	SendPropInt( SENDINFO( m_nOrangeRounds ) ),
	SendPropInt( SENDINFO( m_nRoundsToWin ) ),
	SendPropFloat( SENDINFO( m_flTotalTime ) ),
	SendPropString( SENDINFO( m_strGameDescription ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CInfoPortalScore )
	DEFINE_KEYFIELD( m_flTotalTime, FIELD_FLOAT, "GameTime" ),
	DEFINE_KEYFIELD( m_nRoundsToWin, FIELD_INTEGER, "RoundsToWin" ),
	DEFINE_KEYFIELD( m_bShowCountdown, FIELD_BOOLEAN, "DisplayCountdown" ),
	DEFINE_AUTO_ARRAY_KEYFIELD( m_strGameDescription, FIELD_CHARACTER, "GameDescription" ),
	DEFINE_FIELD( m_nBlueScore, FIELD_INTEGER ),
	DEFINE_FIELD( m_nOrangeScore, FIELD_INTEGER ),
	DEFINE_FIELD( m_nBlueRounds, FIELD_INTEGER ),
	DEFINE_FIELD( m_nOrangeRounds, FIELD_INTEGER ),
	DEFINE_FIELD( m_flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_bGameStarted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bSuddenDeath, FIELD_BOOLEAN ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StartGame", InputStartGame ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ResetScore", InputResetScore ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddBlueScore", InputAddBlueScore ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddOrangeScore", InputAddOrangeScore ),
	DEFINE_THINKFUNC( ScoreThink ),
	DEFINE_OUTPUT( m_OnBlueWin, "OnBlueWin" ),
	DEFINE_OUTPUT( m_OnOrangeWin, "OnOrangeWin" ),
	DEFINE_OUTPUT( m_OnTie, "OnTie" ),
	DEFINE_OUTPUT( m_OnGameStart, "OnGameStart" ),
	DEFINE_OUTPUT( m_OnGameEnd, "OnGameEnd" ),
	DEFINE_OUTPUT( m_OnRoundEnd, "OnRoundEnd" ),
	DEFINE_OUTPUT( m_OnSuddenDeath, "OnSuddenDeath" ),
END_DATADESC()
    // clang-format on

    CInfoPortalScore::CInfoPortalScore()
{
	m_nBlueScore = 0;
	m_nBlueRounds = 0;
	m_nOrangeScore = 0;
	m_nOrangeRounds = 0;
	m_nRoundsToWin = 1;
	m_flTotalTime = 0.0f;
	m_flStartTime = 0.0f;
	m_bShowCountdown = false;
	m_bGameStarted = false;
	m_bSuddenDeath = false;
	m_strGameDescription.GetForModify()[0] = '\0';
}

void CInfoPortalScore::Spawn()
{
	BaseClass::Spawn();
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
}

int CInfoPortalScore::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CInfoPortalScore::InputStartGame( inputdata_t &inputData )
{
	m_flStartTime = gpGlobals->curtime;
	m_bGameStarted = true;
	m_bSuddenDeath = false;
	m_OnGameStart.FireOutput( inputData.pActivator, this );
	SetThink( &CInfoPortalScore::ScoreThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CInfoPortalScore::InputResetScore( inputdata_t &inputData )
{
	(void)inputData;
	m_nBlueScore = 0;
	m_nOrangeScore = 0;
}

void CInfoPortalScore::InputAddBlueScore( inputdata_t &inputData )
{
	m_nBlueScore += inputData.value.Int();
}

void CInfoPortalScore::InputAddOrangeScore( inputdata_t &inputData )
{
	m_nOrangeScore += inputData.value.Int();
}

void CInfoPortalScore::ScoreThink()
{
	if ( !m_bGameStarted )
	{
		return;
	}
	if ( gpGlobals->curtime < m_flStartTime + m_flTotalTime )
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
		return;
	}
	if ( m_nBlueScore == m_nOrangeScore )
	{
		if ( !m_bSuddenDeath )
		{
			m_bSuddenDeath = true;
			m_OnTie.FireOutput( this, this );
			m_OnSuddenDeath.FireOutput( this, this );
		}
		SetNextThink( gpGlobals->curtime + 0.1f );
		return;
	}
	if ( m_nBlueScore > m_nOrangeScore )
	{
		++m_nBlueRounds;
	}
	else
	{
		++m_nOrangeRounds;
	}
	m_OnRoundEnd.FireOutput( this, this );
	if ( m_nBlueRounds >= m_nRoundsToWin || m_nOrangeRounds >= m_nRoundsToWin )
	{
		m_bGameStarted = false;
		if ( m_nBlueRounds > m_nOrangeRounds )
		{
			m_OnBlueWin.FireOutput( this, this );
		}
		else
		{
			m_OnOrangeWin.FireOutput( this, this );
		}
		m_OnGameEnd.FireOutput( this, this );
		return;
	}
	m_nBlueScore = 0;
	m_nOrangeScore = 0;
	m_flStartTime = gpGlobals->curtime;
	m_bSuddenDeath = false;
	SetNextThink( gpGlobals->curtime + 0.1f );
}
