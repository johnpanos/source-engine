//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server side of the paint gun weapon.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_paintgun.h"
#include "portal_player.h"
#include "paint_stream_shared.h"
#include "soundenvelope.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Reconstruction note: the 2010 builds read ConVar paintgun_blobs_per_second.
// The retail weapon_paintgun_shared.cpp replaced that ConVar with this constant,
// so the server keeps the same value for the initial accumulator.
#define paintgun_blobs_per_second	40.f

extern void PaintPowerPickup( int colorIndex, CBasePlayer *pPlayer );


IMPLEMENT_SERVERCLASS_ST( CWeaponPaintGun, DT_WeaponPaintGun )
	SendPropArray3( SENDINFO_ARRAY3( m_bHasPaint ), SendPropBool( SENDINFO_ARRAY( m_bHasPaint ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_hPaintStream ), SendPropEHandle( SENDINFO_ARRAY( m_hPaintStream ) ) ),
	SendPropInt( SENDINFO( m_nCurrentColor ) ),
	SendPropBool( SENDINFO( m_bFiringPaint ) ),
	SendPropBool( SENDINFO( m_bFiringErase ) ),
END_SEND_TABLE()


BEGIN_DATADESC( CWeaponPaintGun )
	DEFINE_FIELD( m_nCurrentColor, FIELD_INTEGER ),
	DEFINE_FIELD( m_bFiringPaint, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFiringErase, FIELD_BOOLEAN ),
	DEFINE_ARRAY( m_hPaintStream, FIELD_EHANDLE, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER ),
	DEFINE_ARRAY( m_bHasPaint, FIELD_BOOLEAN, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER ),

	DEFINE_THINKFUNC( PaintGunThink ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( weapon_paintgun, CWeaponPaintGun );


CWeaponPaintGun::CWeaponPaintGun()
{
	m_flAccumulatedTime = 0.0f;
	m_nCurrentColor = NO_POWER;

	m_bFiringPaint = false;
	m_bFiringErase = false;

	m_bFireOnEmpty = true;
	m_bReloadsSingly = false;

	ResetPaint();

	// NO_POWER is always available so the gun can erase
	ActivatePaint( NO_POWER );

	SetSubType( m_nCurrentColor );
}


CWeaponPaintGun::~CWeaponPaintGun()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if( m_pLiquidLoop )
	{
		controller.SoundDestroy( m_pLiquidLoop );
		m_pLiquidLoop = NULL;
	}

	if( m_pLiquidStart )
	{
		controller.SoundDestroy( m_pLiquidStart );
		m_pLiquidStart = NULL;
	}
}


void CWeaponPaintGun::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetContextThink( &CWeaponPaintGun::PaintGunThink, gpGlobals->curtime, "Paint Gun Think" );

	m_flAccumulatedTime = 1.0f / paintgun_blobs_per_second;
	m_nBlobRandomSeed = 0;

	// One paint stream per paint type (including erase)
	for( int i = 0; i < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER; ++i )
	{
		if( m_hPaintStream[i] == NULL )
		{
			CPaintStream *pPaintStream = static_cast< CPaintStream* >( CreateEntityByName( "paint_stream" ) );
			if( pPaintStream )
			{
				DispatchSpawn( pPaintStream );
				m_hPaintStream.Set( i, pPaintStream );
			}
		}
	}
}


void CWeaponPaintGun::UpdateOnRemove()
{
	for( int i = 0; i < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER; ++i )
	{
		UTIL_Remove( m_hPaintStream[i] );
	}

	BaseClass::UpdateOnRemove();
}


void CWeaponPaintGun::Precache()
{
	PrecacheScriptSound( "Paintgun.FireLoop" );
	PrecacheScriptSound( "NPC_CScanner.DiveBombFlyby" );
	PrecacheScriptSound( "Paintblob.Impact" );

	PrecacheModel( "models/weapons/w_ball_small.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_01.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_02.mdl" );
	PrecacheModel( "models/weapons/w_ball_small_chunk_03.mdl" );

	PrecacheParticleSystem( "paint_splat_bounce_01" );
	PrecacheParticleSystem( "paint_splat_stick_01" );
	PrecacheParticleSystem( "paint_splat_speed_01" );
	PrecacheParticleSystem( "paint_splat_erase_01" );

	BaseClass::Precache();
}


bool CWeaponPaintGun::SendWeaponAnim( int iActivity )
{
	int newActivity = iActivity;

	// Dry fire if we don't have the selected paint
	if( iActivity == ACT_VM_PRIMARYATTACK )
	{
		newActivity = HasCurrentColor() ? ACT_VM_PRIMARYATTACK : ACT_VM_DRYFIRE;
	}

	return BaseClass::SendWeaponAnim( newActivity );
}


bool CWeaponPaintGun::HasCurrentColor()
{
	return m_bHasPaint[ m_nCurrentColor ];
}


void CWeaponPaintGun::StartShootingSound()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if( m_pLiquidLoop )
	{
		controller.SoundDestroy( m_pLiquidLoop );
		m_pLiquidLoop = NULL;
	}

	{
		CPASAttenuationFilter filter( this, ATTN_NORM );
		m_pLiquidLoop = controller.SoundCreate( filter, entindex(), "Paintgun.FireLoop" );
		controller.Play( m_pLiquidLoop, 0.0f, 100.0f );
		controller.SoundChangeVolume( m_pLiquidLoop, 1.0f, 1.0f );
	}
}


void CWeaponPaintGun::StopShootingSound()
{
	if( m_pLiquidLoop )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundFadeOut( m_pLiquidLoop, 0.5f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Gives the gun a paint power
//-----------------------------------------------------------------------------
void CWeaponPaintGun::ActivatePaint( PaintPowerType nIndex )
{
	m_bHasPaint.Set( nIndex, true );

	IGameEvent *event = gameeventmanager->CreateEvent( "picked_up_paint" );
	if ( event )
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if( pPlayer )
		{
			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetInt( "paintcount", GetPaintCount() );

			gameeventmanager->FireEvent( event );
		}
	}

	SetSubType( nIndex );
}


int CWeaponPaintGun::GetPaintCount()
{
	int nPaintCount = 0;
	for( int i = 0; i < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER; ++i )
	{
		if( m_bHasPaint[i] )
		{
			++nPaintCount;
		}
	}

	return nPaintCount;
}


//-----------------------------------------------------------------------------
// Purpose: Removes every paint power from the gun
//-----------------------------------------------------------------------------
void CWeaponPaintGun::ResetPaint()
{
	for( int i = 0; i < PAINT_POWER_TYPE_COUNT; ++i )
	{
		DeactivatePaint( (PaintPowerType)i );
	}

	// Reconstruction note: both 2010 builds construct and mark this filter
	// reliable, but no message is sent with it any more.
	CBroadcastRecipientFilter filter;
	filter.MakeReliable();

	SetCurrentPaint( NO_POWER );
}


void CWeaponPaintGun::DeactivatePaint( PaintPowerType nIndex )
{
	m_bHasPaint.Set( nIndex, false );
}


void CWeaponPaintGun::CleansePaint()
{
	if( !HasAnyPaintPower() )
		return;

	EmitSound( "NPC_CScanner.DiveBombFlyby" );

	ResetPaint();
}


void CWeaponPaintGun::SetCurrentPaint( PaintPowerType nIndex )
{
	if( HasPaintPower( nIndex ) )
	{
		m_nCurrentColor = nIndex;
	}

	CBaseEntity *pOwner = GetOwner();
	if( pOwner )
	{
		// Tell the owner's client to change its predicted color
		CSingleUserRecipientFilter filter( ToBasePlayer( pOwner ) );
		filter.MakeReliable();

		UserMessageBegin( filter, "ChangePaintColor" );
			WRITE_EHANDLE( this );
			WRITE_BYTE( nIndex );
		MessageEnd();
	}
}


void CWeaponPaintGun::PaintGunThink()
{
	SetContextThink( &CWeaponPaintGun::PaintGunThink, gpGlobals->curtime, "Paint Gun Think" );

	// Remember where the blobs came out last think so the blobs can be interpolated
	CPortal_Player *pPlayer = ToPortalPlayer( GetOwner() );
	if( pPlayer )
	{
		m_vecOldBlobFirePos = pPlayer->GetPaintGunShootPosition();
	}
}


static void GiveAllPaintPowers()
{
	CBaseEntity *pFoundEnt = gEntList.FindEntityByClassname( NULL, "weapon_paintgun" );
	while( pFoundEnt )
	{
		CWeaponPaintGun *pPaintGun = dynamic_cast< CWeaponPaintGun* >( pFoundEnt );
		if( pPaintGun )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pPaintGun->GetOwner() );

			pPaintGun->ActivatePaint( BOUNCE_POWER );
			pPaintGun->ActivatePaint( SPEED_POWER );
			pPaintGun->ActivatePaint( REFLECT_POWER );
			pPaintGun->ActivatePaint( PORTAL_POWER );

			PaintPowerPickup( BOUNCE_POWER, pPlayer );
			PaintPowerPickup( SPEED_POWER, pPlayer );
			PaintPowerPickup( REFLECT_POWER, pPlayer );
			PaintPowerPickup( PORTAL_POWER, pPlayer );
		}

		pFoundEnt = gEntList.FindEntityByClassname( pFoundEnt, "weapon_paintgun" );
	}
}

static ConCommand giveallpaintpowers( "giveallpaintpowers", GiveAllPaintPowers );
