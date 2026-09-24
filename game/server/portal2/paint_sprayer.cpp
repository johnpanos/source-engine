//========= Portal 2 reconstruction ============================================//
//
// Purpose: info_paint_sprayer: sprays paint blobs into a paint stream.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "paint_sprayer.h"
#include "paint_stream.h"
#include "paint_sprayer_shared.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void SendProxy_Angles( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

IMPLEMENT_SERVERCLASS_ST( CPaintSprayer, DT_PaintSprayer )
	SendPropVector( SENDINFO( m_vecOrigin ), -1, SPROP_COORD | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropEHandle( SENDINFO_NAME( m_hMoveParent, moveparent ) ),
	SendPropQAngles( SENDINFO( m_angRotation ), 13, SPROP_CHANGES_OFTEN, SendProxy_Angles ),
	SendPropBool( SENDINFO( m_bActive ) ),
	SendPropBool( SENDINFO( m_bSilent ) ),
	SendPropInt( SENDINFO( m_PaintPowerType ) ),
	SendPropFloat( SENDINFO( m_flBlobsPerSecond ) ),
	SendPropFloat( SENDINFO( m_flMinSpeed ) ),
	SendPropFloat( SENDINFO( m_flMaxSpeed ) ),
	SendPropFloat( SENDINFO( m_flBlobSpreadRadius ) ),
	SendPropFloat( SENDINFO( m_flBlobSpreadAngle ) ),
	SendPropFloat( SENDINFO( m_flStreakPercentage ) ),
	SendPropFloat( SENDINFO( m_flMinStreakTime ) ),
	SendPropFloat( SENDINFO( m_flMaxStreakTime ) ),
	SendPropFloat( SENDINFO( m_flMinStreakSpeedDampen ) ),
	SendPropFloat( SENDINFO( m_flMaxStreakSpeedDampen ) ),
	SendPropFloat( SENDINFO( m_flStartTime ) ),
	SendPropEHandle( SENDINFO( m_hPaintStream ) ),
	// Reconstruction note: not in the 2010 table; the imported shared SprayPaint()
	// reads these on the client too, and C_PaintSprayer receives them.
	SendPropInt( SENDINFO( m_nMaxBlobCount ) ),
	SendPropBool( SENDINFO( m_bDrawOnly ) ),
	SendPropFloat( SENDINFO( m_flNoisyBlobPercentage ) ),
END_SEND_TABLE()


BEGIN_DATADESC( CPaintSprayer )
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hPaintStream, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_bSilent, FIELD_BOOLEAN, "Silent" ),
	DEFINE_KEYFIELD( m_bStartActive, FIELD_BOOLEAN, "start_active" ),
	DEFINE_KEYFIELD( m_PaintPowerType, FIELD_INTEGER, "PaintType" ),
	DEFINE_KEYFIELD( m_flBlobsPerSecond, FIELD_FLOAT, "blobs_per_second" ),
	DEFINE_KEYFIELD( m_flMinSpeed, FIELD_FLOAT, "min_speed" ),
	DEFINE_KEYFIELD( m_flMaxSpeed, FIELD_FLOAT, "max_speed" ),
	DEFINE_KEYFIELD( m_flBlobSpreadRadius, FIELD_FLOAT, "blob_spread_radius" ),
	DEFINE_KEYFIELD( m_flBlobSpreadAngle, FIELD_FLOAT, "blob_spread_angle" ),
	DEFINE_KEYFIELD( m_flStreakPercentage, FIELD_FLOAT, "blob_streak_percentage" ),
	DEFINE_KEYFIELD( m_flMinStreakTime, FIELD_FLOAT, "min_streak_time" ),
	DEFINE_KEYFIELD( m_flMaxStreakTime, FIELD_FLOAT, "max_streak_time" ),
	DEFINE_KEYFIELD( m_flMinStreakSpeedDampen, FIELD_FLOAT, "min_streak_speed_dampen" ),
	DEFINE_KEYFIELD( m_flMaxStreakSpeedDampen, FIELD_FLOAT, "max_streak_speed_dampen" ),
	DEFINE_KEYFIELD( m_nRenderMode, FIELD_INTEGER, "RenderMode" ),

	// Reconstruction note: absent from the 2010 table; key names are the retail
	// FGD's for the members the imported shared code uses.
	DEFINE_KEYFIELD( m_nMaxBlobCount, FIELD_INTEGER, "maxblobcount" ),
	DEFINE_KEYFIELD( m_bDrawOnly, FIELD_BOOLEAN, "DrawOnly" ),
	DEFINE_KEYFIELD( m_nAmbientSound, FIELD_INTEGER, "AmbientSound" ),
	DEFINE_KEYFIELD( m_strLightPositionName, FIELD_STRING, "light_position_name" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Start", InputStart ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Stop", InputStop ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "ChangePaintType", InputChangePaintType ),

	DEFINE_THINKFUNC( PaintSprayerThink ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( info_paint_sprayer, CPaintSprayer );

CPaintSprayer::CPaintSprayer()
	: m_bStartActive( false ),
	  m_flLastThinkTime( 0.0f ),
	  m_flAccumulatedTime( 0.0f ),
	  m_nBlobSpawned( 0 ),
	  m_nRenderMode( BLOB_RENDER_BLOBULATOR ),
	  m_nAmbientSound( PAINT_SPRAYER_SOUND_NONE )
{
	m_bActive = false;
	m_flStartTime = 0.0f;
	m_hPaintStream = NULL;

	// Reconstruction note: retail-only members; defaults are the retail FGD's.
	m_bDrawOnly = false;
	m_nBlobRandomSeed = 0;
	m_nMaxBlobCount = 250;
	m_flNoisyBlobPercentage = 0.0f;
	m_flPercentageSinceLastNoisyBlob = 0.0f;
}


CPaintSprayer::~CPaintSprayer()
{
}


void CPaintSprayer::Precache()
{
	PrecacheModel( "models/weapons/w_ball_small.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_01.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_02.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_03.mdl" );

	PrecacheParticleSystem( "paint_splat_bounce_01" );
	PrecacheParticleSystem( "paint_splat_stick_01" );
	PrecacheParticleSystem( "paint_splat_speed_01" );
	PrecacheParticleSystem( "paint_splat_erase_01" );
}


void CPaintSprayer::Spawn()
{
	BaseClass::Spawn();

	Precache();

	SetThink( &CPaintSprayer::PaintSprayerThink );
	SetNextThink( TICK_NEVER_THINK );

	if ( m_bStartActive )
	{
		StartPaintSpray();
	}

	// Every sprayer owns the stream its blobs live in
	if ( m_hPaintStream == NULL )
	{
		CPaintStream *pPaintStream = static_cast< CPaintStream* >( CreateEntityByName( "paint_stream" ) );
		if ( pPaintStream )
		{
			pPaintStream->SetPaintType( m_PaintPowerType );
			pPaintStream->SetStreamRenderMode( m_nRenderMode );

			// Reconstruction note: the 2010 stream had no light position or blob budget.
			// The imported stream needs both before it spawns (its client allocates the
			// blob pool from m_nMaxBlobCount), so pass them through the stream's setup.
			Vector vLightPosition = GetAbsOrigin();
			if ( m_strLightPositionName != NULL_STRING )
			{
				CBaseEntity *pLightPosition = gEntList.FindEntityByName( NULL, m_strLightPositionName );
				if ( pLightPosition )
				{
					vLightPosition = pLightPosition->GetAbsOrigin();
				}
			}
			pPaintStream->Init( vLightPosition, m_PaintPowerType, m_nRenderMode, m_nMaxBlobCount );

			DispatchSpawn( pPaintStream );

			m_hPaintStream = pPaintStream;
		}
	}
}


void CPaintSprayer::Activate()
{
	BaseClass::Activate();

	m_flLastThinkTime = gpGlobals->curtime;

	if ( m_bStartActive )
	{
		m_bStartActive = false;
		StartPaintSpray();
	}
}


void CPaintSprayer::UpdateOnRemove()
{
	UTIL_Remove( m_hPaintStream );

	BaseClass::UpdateOnRemove();
}


void CPaintSprayer::SetPaintSprayerData( bool bSilent, float flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float flMinStreakSpeedDampen, float flMaxStreakSpeedDampen )
{
	m_bSilent = bSilent;

	m_flBlobsPerSecond = flBlobsPerSecond;
	m_flMinSpeed = flMinSpeed;
	m_flMaxSpeed = flMaxSpeed;
	m_flBlobSpreadRadius = flBlobSpreadRadius;
	m_flBlobSpreadAngle = flBlobSpreadAngle;

	m_flStreakPercentage = flBlobStreakPercent;
	m_flMinStreakTime = flMinStreakTime;
	m_flMaxStreakTime = flMaxStreakTime;
	m_flMinStreakSpeedDampen = flMinStreakSpeedDampen;
	m_flMaxStreakSpeedDampen = flMaxStreakSpeedDampen;
}


void CPaintSprayer::SetPaintPowerType( PaintPowerType paintType )
{
	m_PaintPowerType = paintType;
}


int CPaintSprayer::UpdateTransmitState()
{
	// Silent sprayers only paint, so the clients never need them
	if ( m_bSilent )
	{
		return SetTransmitState( FL_EDICT_DONTSEND );
	}

	return SetTransmitState( FL_EDICT_ALWAYS );
}


void CPaintSprayer::PaintSprayerThink()
{
	if ( m_bActive )
	{
		float flDeltaTime = gpGlobals->curtime - m_flLastThinkTime;
		m_flLastThinkTime = gpGlobals->curtime;

		SprayPaint( flDeltaTime );

		SetNextThink( gpGlobals->curtime );
	}
}


void CPaintSprayer::InputStart( inputdata_t &inputData )
{
	StartPaintSpray();
}


void CPaintSprayer::InputStop( inputdata_t &inputData )
{
	StopPaintSpray();
}


void CPaintSprayer::InputChangePaintType( inputdata_t &inputData )
{
	SetPaintPowerType( static_cast< PaintPowerType >( inputData.value.Int() ) );
}


void CPaintSprayer::StartPaintSpray()
{
	if ( !m_bActive )
	{
		m_bActive = true;

		// Fire the first blob right away
		m_flAccumulatedTime = 1.0f / m_flBlobsPerSecond;

		m_flStartTime = gpGlobals->curtime;
		m_flLastThinkTime = gpGlobals->curtime;
		m_nBlobSpawned = 0;

		SetNextThink( gpGlobals->curtime );
	}
}


void CPaintSprayer::StopPaintSpray()
{
	if ( m_bActive )
	{
		m_bActive = false;
	}

	SetNextThink( TICK_NEVER_THINK );
}
