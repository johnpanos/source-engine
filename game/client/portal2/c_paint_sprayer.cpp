//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint sprayer (info_paint_sprayer).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_paint_sprayer.h"
#include "paint_sprayer_shared.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_PaintSprayer, DT_PaintSprayer, CPaintSprayer )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropInt( RECVINFO_NAME( m_hNetworkMoveParent, moveparent ), 0, RecvProxy_IntToMoveParent ),
	RecvPropVector( RECVINFO_NAME( m_angNetworkAngles, m_angRotation ) ),
	RecvPropBool( RECVINFO( m_bActive ) ),
	RecvPropBool( RECVINFO( m_bSilent ) ),
	RecvPropInt( RECVINFO( m_PaintPowerType ) ),
	RecvPropFloat( RECVINFO( m_flBlobsPerSecond ) ),
	RecvPropFloat( RECVINFO( m_flMinSpeed ) ),
	RecvPropFloat( RECVINFO( m_flMaxSpeed ) ),
	RecvPropFloat( RECVINFO( m_flBlobSpreadRadius ) ),
	RecvPropFloat( RECVINFO( m_flBlobSpreadAngle ) ),
	RecvPropFloat( RECVINFO( m_flStreakPercentage ) ),
	RecvPropFloat( RECVINFO( m_flMinStreakTime ) ),
	RecvPropFloat( RECVINFO( m_flMaxStreakTime ) ),
	RecvPropFloat( RECVINFO( m_flMinStreakSpeedDampen ) ),
	RecvPropFloat( RECVINFO( m_flMaxStreakSpeedDampen ) ),
	RecvPropFloat( RECVINFO( m_flStartTime ) ),
	RecvPropEHandle( RECVINFO( m_hPaintStream ) ),
	// Reconstruction note: not in the 2010 table; required by the shared
	// SprayPaint so the client sprays with the server's settings.
	RecvPropInt( RECVINFO( m_nMaxBlobCount ) ),
	RecvPropBool( RECVINFO( m_bDrawOnly ) ),
	RecvPropFloat( RECVINFO( m_flNoisyBlobPercentage ) ),
END_RECV_TABLE()


C_PaintSprayer::C_PaintSprayer()
	: m_flAccumulatedTime( 0.0f ),
	  m_nBlobSpawned( 0 ),
	  m_flStartTime( 0.0f ),
	  m_nMaxBlobCount( 0 ),
	  m_bDrawOnly( false ),
	  m_flNoisyBlobPercentage( 0.0f ),
	  m_nBlobRandomSeed( 0 ),
	  m_flPercentageSinceLastNoisyBlob( 0.0f )
{
	m_hPaintStream = NULL;
}


C_PaintSprayer::~C_PaintSprayer()
{
}


void C_PaintSprayer::Precache()
{
	PrecacheParticleSystem( "paint_splat_bounce_01" );
	PrecacheParticleSystem( "paint_splat_stick_01" );
	PrecacheParticleSystem( "paint_splat_speed_01" );
	PrecacheParticleSystem( "paint_splat_erase_01" );

	BaseClass::Precache();
}


void C_PaintSprayer::Spawn()
{
	Precache();

	BaseClass::Spawn();
}


void C_PaintSprayer::PreDataUpdate( DataUpdateType_t updateType )
{
	m_bOldActive = m_bActive;

	BaseClass::PreDataUpdate( updateType );
}


void C_PaintSprayer::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		if ( !m_bActive )
			return;
	}
	else
	{
		if ( m_bOldActive == m_bActive )
			return;

		// The sprayer was turned off
		if ( !m_bActive )
		{
			SetNextClientThink( CLIENT_THINK_NEVER );
			return;
		}
	}

	// The sprayer was turned on, fire the first blob right away
	m_flAccumulatedTime = 1.0f / m_flBlobsPerSecond;
	m_flLastThinkTime = m_flStartTime;
	m_nBlobSpawned = 0;

	SetNextClientThink( gpGlobals->curtime );
}


void C_PaintSprayer::ClientThink()
{
	if ( m_bActive )
	{
		float flDeltaTime = gpGlobals->curtime - m_flLastThinkTime;
		m_flLastThinkTime = gpGlobals->curtime;

		SprayPaint( flDeltaTime );

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}
