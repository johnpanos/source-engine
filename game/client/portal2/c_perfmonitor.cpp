//========= Portal 2 reconstruction ============================================//
//
// Purpose: Playtest performance monitor: logs low-framerate moments and a
//			per-map summary as XML nodes on the developer console.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "igamesystem.h"
#include "view.h"
#include "c_portal_player.h"
#include "c_weapon_portalgun.h"
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "portal2_engine_compat.h"
#include "tier0/memdbgon.h"

void cl_perfmon_frames_to_average_changed_f( IConVar *var, const char *pOldValue, float flOldValue );

static ConVar cl_perfmon_desired_framerate( "cl_perfmon_desired_framerate", "30", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL );
static ConVar cl_perfmon_enabled( "cl_perfmon_enabled", "0", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL );
static ConVar cl_perfmon_interval( "cl_perfmon_interval", "10", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL );
static ConVar cl_perfmon_frames_until_dump( "cl_perfmon_frames_until_dump", "5", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL );
static ConVar cl_perfmon_frames_to_average( "cl_perfmon_frames_to_average", "15", FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL, "The number of frames to use when computing the simple moving average of the framerate.", cl_perfmon_frames_to_average_changed_f );

//-----------------------------------------------------------------------------
// Purpose: Where performance reports go
//-----------------------------------------------------------------------------
void SendPerfMsg( const char *pMsg )
{
	DevMsg( 2, "%s", pMsg );
}

//-----------------------------------------------------------------------------
// Purpose: Tracks frame times for the current map
//-----------------------------------------------------------------------------
class C_PerfMonitor : public CAutoGameSystemPerFrame
{
public:
	C_PerfMonitor();

	// IGameSystem
	virtual bool Init( void );
	virtual void Shutdown( void );

	virtual void LevelInitPostEntity( void );
	virtual void LevelShutdownPreEntity( void );

	// IGameSystemPerFrame
	virtual void PreRender( void );

	// Gets called each frame
	virtual void Update( float frametime );

	virtual void PostRender( void );

	uint64 GetLocalTimeInSeconds( void );

	void UpdateFramesToAverage( void );

	void Reset( void );

private:

	uint32 m_uFramesBelowDesired;
	float m_fPreviousFrameTime;
	float m_fPeakFrametime;
	float m_fMinFrametime;
	float m_fLevelStartTime;
	uint32 m_uTotalFrames;
	float m_fLastLowTime;
	float m_fLastLowFramerate;
	char m_szLastPerfMsg[2048];
	CUtlVector< float > m_FrametimeHistory;
	uint32 m_uOldestFrame;
};

C_PerfMonitor::C_PerfMonitor() : CAutoGameSystemPerFrame( "PerfMonitor" )
{
	m_uFramesBelowDesired = 0;
	m_fPreviousFrameTime = 0.0f;
	m_uOldestFrame = 0;
	m_szLastPerfMsg[0] = '\0';
}

void C_PerfMonitor::Reset( void )
{
	UpdateFramesToAverage();
	for ( int i = 0; i < m_FrametimeHistory.Count(); ++i )
	{
		m_FrametimeHistory[i] = FLT_MIN;
	}

	double dbCurrTime = Plat_FloatTime();
	m_fLevelStartTime = dbCurrTime;
	m_fMinFrametime = FLT_MAX;
	m_fPeakFrametime = 0.0f;
	m_fPreviousFrameTime = dbCurrTime;
	m_uTotalFrames = 0;
	m_fLastLowFramerate = FLT_MAX;
	m_fLastLowTime = dbCurrTime;
	m_szLastPerfMsg[0] = '\0';
}

bool C_PerfMonitor::Init( void )
{
	Reset();
	return true;
}

void C_PerfMonitor::Shutdown( void )
{
}

void C_PerfMonitor::LevelInitPostEntity( void )
{
	Reset();
}

void C_PerfMonitor::LevelShutdownPreEntity( void )
{
	if ( !cl_perfmon_enabled.GetInt() )
		return;

	char strTemp[2048];
	V_snprintf( strTemp, sizeof( strTemp ), "<MapEndNode map=\"%s\" timesubmitted=\"%llu\" totalframes=\"%i\" minframe=\"%f\" peakframe=\"%f\" avgframe=\"%f\"/>",
		Portal2Engine::GetLevelNameShort(), GetLocalTimeInSeconds(), m_uTotalFrames, m_fMinFrametime, m_fPeakFrametime,
		(double)m_uTotalFrames / ( Plat_FloatTime() - m_fLevelStartTime ) );
	SendPerfMsg( strTemp );
}

void C_PerfMonitor::PreRender( void )
{
}

void C_PerfMonitor::Update( float frametime )
{
	if ( !cl_perfmon_enabled.GetInt() )
		return;

	float fCurrTime = Plat_FloatTime();
	float fElapsedTime = fCurrTime - m_fPreviousFrameTime;
	m_fPreviousFrameTime = fCurrTime;
	++m_uTotalFrames;

	if ( fElapsedTime > m_fPeakFrametime )
	{
		m_fPeakFrametime = fElapsedTime;
	}

	if ( fElapsedTime < m_fMinFrametime )
	{
		m_fMinFrametime = fElapsedTime;
	}

	float fFramerate = 1.0f / fElapsedTime;

	// Report the worst frame seen during the last interval
	if ( fCurrTime - m_fLastLowTime > cl_perfmon_interval.GetFloat() && m_szLastPerfMsg[0] )
	{
		m_fLastLowTime = fCurrTime;
		m_fLastLowFramerate = FLT_MAX;

		char strTemp[2048];
		V_snprintf( strTemp, sizeof( strTemp ), "%s", m_szLastPerfMsg );
		SendPerfMsg( strTemp );

		m_szLastPerfMsg[0] = '\0';
		m_uFramesBelowDesired = 0;
	}

	// Simple moving average over the frame time ring buffer
	m_FrametimeHistory[m_uOldestFrame] = fElapsedTime;

	float fAvgFrameTime = 0.0f;
	int iFramesToAvg = cl_perfmon_frames_to_average.GetInt();
	for ( int i = 0; i < iFramesToAvg; ++i )
	{
		fAvgFrameTime += m_FrametimeHistory[( i + m_uOldestFrame ) % iFramesToAvg] / (float)iFramesToAvg;
	}

	float fSMAFramerate = 1.0f / fAvgFrameTime;
	m_uOldestFrame = ( m_uOldestFrame + 1 ) % iFramesToAvg;

	if ( fSMAFramerate <= cl_perfmon_desired_framerate.GetInt() && fFramerate <= m_fLastLowFramerate )
	{
		m_fLastLowFramerate = fSMAFramerate;

		char szLowFrameMsg[2048];
		int iCurrChar = V_snprintf( szLowFrameMsg, sizeof( szLowFrameMsg ), "<LowFrameNode map=\"%s\" timesubmitted=\"%llu\" framerate=\"%f\" frame=\"%i\">\n",
			Portal2Engine::GetLevelNameShort(), GetLocalTimeInSeconds(), fSMAFramerate, m_uTotalFrames );

		FOR_EACH_VALID_SPLITSCREEN_PLAYER( hh )
		{
			C_BasePlayer *pl = C_BasePlayer::GetLocalPlayer();
			if ( !pl )
				continue;

			Vector vPlayerPos = pl->GetAbsOrigin();
			QAngle vPlayerAng = MainViewAngles();
			C_Portal_Player *pPortalPlayer = ToPortalPlayer( pl );

			Vector vBluePortalPos( 0.0f, 0.0f, 0.0f );
			QAngle vBluePortalAng( 0.0f, 0.0f, 0.0f );
			Vector vOrangePortalPos( 0.0f, 0.0f, 0.0f );
			QAngle vOrangePortalAng( 0.0f, 0.0f, 0.0f );

			C_WeaponPortalgun *pPortalgun = dynamic_cast< C_WeaponPortalgun * >( pPortalPlayer->GetActiveWeapon() );
			(void)pPortalgun;

			iCurrChar += V_snprintf( szLowFrameMsg + iCurrChar, sizeof( szLowFrameMsg ) - iCurrChar,
				"\t\t<Player index=\"%i\">\n"
				"\t\t\t<Position x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"\t\t\t<Angle x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"\t\t\t<BluePortalPos x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"\t\t\t<BluePortalAng x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"\t\t\t<OrangePortalPos x=\"%f\" y=\"%f\" z=\"%f\"/>\n"
				"\t\t\t<OrangePortalAng x=\"%f\" y=\"%f\" z=\"%f\"/>\n",
				hh,
				vPlayerPos.x, vPlayerPos.y, vPlayerPos.z,
				vPlayerAng.x, vPlayerAng.y, vPlayerAng.z,
				vBluePortalPos.x, vBluePortalPos.y, vBluePortalPos.z,
				vBluePortalAng.x, vBluePortalAng.y, vBluePortalAng.z,
				vOrangePortalPos.x, vOrangePortalPos.y, vOrangePortalPos.z,
				vOrangePortalAng.x, vOrangePortalAng.y, vOrangePortalAng.z );
		}

		// Held until the interval elapses so only the worst frame is reported
		V_snprintf( m_szLastPerfMsg, sizeof( m_szLastPerfMsg ), "%s%s", szLowFrameMsg, "\t</Players>\n" );
	}
}

void C_PerfMonitor::PostRender( void )
{
}

uint64 C_PerfMonitor::GetLocalTimeInSeconds( void )
{
	time_t now;
	time( &now );
	return (uint64)now;
}

void C_PerfMonitor::UpdateFramesToAverage( void )
{
	m_FrametimeHistory.SetCount( cl_perfmon_frames_to_average.GetInt() );
}

static C_PerfMonitor *s_pPerfMonitor = new C_PerfMonitor();

void cl_perfmon_frames_to_average_changed_f( IConVar *var, const char *pOldValue, float flOldValue )
{
	s_pPerfMonitor->UpdateFramesToAverage();
}
