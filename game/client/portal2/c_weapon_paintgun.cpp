//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint gun weapon.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon_paintgun.h"
#include "c_portal_player.h"
#include "igameevents.h"
#include "iinput.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_erase_stream_color( "cl_erase_stream_color", "255 255 255 0", FCVAR_CHEAT, "Change the appearance of the erase particle stream." );

ConVar draw_paint_particle_stream( "draw_paint_particle_stream", "0", 0 );

// Reconstruction note: the 2010 builds read ConVar paintgun_blobs_per_second.
// The retail weapon_paintgun_shared.cpp replaced that ConVar with this constant.
#define paintgun_blobs_per_second	40.f

// Reconstruction note: the 2010 client took these from the portal base weapon
// unit, which the retail C_BasePortalCombatWeapon no longer provides.
static float g_lateralBob = 0.0f;
static float g_verticalBob = 0.0f;


static inline void ChangePaintColor( PaintPowerType power )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if( pPlayer )
	{
		C_WeaponPaintGun *pPaintGun = dynamic_cast< C_WeaponPaintGun* >( pPlayer->GetActiveWeapon() );
		if( pPaintGun )
		{
			pPaintGun->SetPaintPower( power );
		}
	}
}


IMPLEMENT_CLIENTCLASS_DT( C_WeaponPaintGun, DT_WeaponPaintGun, CWeaponPaintGun )
	RecvPropArray3( RECVINFO_ARRAY( m_bHasPaint ), RecvPropBool( RECVINFO( m_bHasPaint[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_hPaintStream ), RecvPropEHandle( RECVINFO( m_hPaintStream[0] ) ) ),
	RecvPropInt( RECVINFO( m_nCurrentColor ) ),
	RecvPropBool( RECVINFO( m_bFiringPaint ) ),
	RecvPropBool( RECVINFO( m_bFiringErase ) ),
	// Reconstruction note: the 841 table also carries this whole-array handle
	// prop; the server sends no matching prop, so it never receives data.
	RecvPropEHandle( RECVINFO( m_hPaintStream ) ),
END_RECV_TABLE()


BEGIN_PREDICTION_DATA( C_WeaponPaintGun )
	DEFINE_PRED_FIELD( m_nCurrentColor, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bFiringPaint, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bFiringErase, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_bHasPaint, FIELD_BOOLEAN, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()


LINK_ENTITY_TO_CLASS( weapon_paintgun, C_WeaponPaintGun );


C_WeaponPaintGun::C_WeaponPaintGun()
{
	m_flBobScale = 0.0f;
	m_flAccumulatedTime = 0.0f;
	m_flLastThinkTime = 0.0f;

	m_nCurrentColor = NO_POWER;
	m_bFiringPaint = false;
	m_bFiringErase = false;

	m_pLastOwner = NULL;
}


void C_WeaponPaintGun::Precache()
{
	PrecacheParticleSystem( "paint_splat_bounce_01" );
	PrecacheParticleSystem( "paint_splat_stick_01" );
	PrecacheParticleSystem( "paint_splat_speed_01" );
	PrecacheParticleSystem( "paint_splat_erase_01" );

	BaseClass::Precache();
}


void C_WeaponPaintGun::Spawn()
{
	BaseClass::Spawn();

	m_flLastThinkTime = gpGlobals->curtime;
	m_flAccumulatedTime = 1.0f / paintgun_blobs_per_second;
	m_nBlobRandomSeed = 0;

	// Reconstruction note: both builds call SetNextThink (not SetNextClientThink)
	// with CLIENT_THINK_ALWAYS here; OnDataChanged starts the client think.
	SetNextThink( CLIENT_THINK_ALWAYS );

	SetPaintPower( NO_POWER );
}


void C_WeaponPaintGun::ClientThink()
{
	float flDeltaTime = gpGlobals->curtime - m_flLastThinkTime;
	m_flLastThinkTime = gpGlobals->curtime;

	C_Portal_Player *pPlayer = ToPortalPlayer( GetOwner() );

	// Spray the predicted blobs
	if( IsCarrierAlive() && m_iState == WEAPON_IS_ACTIVE && flDeltaTime > 0.0f )
	{
		if( m_bFiringPaint || m_bFiringErase )
		{
			PaintPowerType blobType = m_bFiringPaint ? (PaintPowerType)m_nCurrentColor : NO_POWER;
			SprayPaint( flDeltaTime, blobType );
		}
	}

	if( pPlayer )
	{
		m_vecOldBlobFirePos = pPlayer->GetPaintGunShootPosition();
	}

	// The local player sees the predicted color, everyone else the networked one
	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Color color = pOwner && pOwner->IsLocalPlayer() ? MapPowerToColor( m_iSubType ) : MapPowerToColor( m_nCurrentColor );
	if( MapColorToPower( color ) == NO_POWER || !HasAnyPaintPower() )
	{
		color = Color( 255, 255, 255 );
	}

	SetRenderColor( color.r(), color.g(), color.b() );

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}


void C_WeaponPaintGun::Simulate()
{
	if( IsEffectActive( EF_NODRAW ) )
	{
		StopHoseEffect();
	}
}


void C_WeaponPaintGun::OnPreDataChanged( DataUpdateType_t type )
{
	BaseClass::OnPreDataChanged( type );

	m_pLastOwner = GetOwner();
	m_pLastEffect = m_pStreamEffect;
}


void C_WeaponPaintGun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	SetNextClientThink( CLIENT_THINK_ALWAYS );

	if( updateType == DATA_UPDATE_CREATED )
	{
		m_iSubType = m_nCurrentColor;

		C_Portal_Player *pOwner = ToPortalPlayer( GetOwner() );
		if( pOwner )
		{
			m_vecOldBlobFirePos = pOwner->GetPaintGunShootPosition();
		}
	}

	if( IsCarrierAlive() && m_iState == WEAPON_IS_ACTIVE )
	{
		if( ( m_bFiringPaint && HasAnyPaintPower() ) || m_bFiringErase )
		{
			StartHoseEffect();
		}
		else
		{
			StopHoseEffect();
		}

		ChangeRenderColor();
	}
	else
	{
		StopHoseEffect();
	}

	// The owner changed: stop the stream that was attached to the previous owner
	if( m_pLastOwner != GetOwner() )
	{
		StopHoseEffect();

		C_BaseCombatCharacter *pCurrentOwner = GetOwner();
		SetOwner( m_pLastOwner );
		m_pStreamEffect = m_pLastEffect;
		StopHoseEffect();
		SetOwner( pCurrentOwner );
	}
}


void C_WeaponPaintGun::ChangeRenderColor( bool bForce )
{
	C_BasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	Color color;

	color = pPlayer && pPlayer->IsLocalPlayer() ? MapPowerToColor( m_iSubType ) : MapPowerToColor( m_nCurrentColor );

	if( !HasAnyPaintPower() && !bForce )
	{
		color = Color( 255, 255, 255, 255 );
	}

	if( pPlayer && pPlayer->IsLocalPlayer() )
	{
		C_BaseViewModel *pGunModel = GetEffectViewModel();
		if( pGunModel )
		{
			pGunModel->SetRenderColor( color.r(), color.g(), color.b() );
		}
	}

	SetRenderColor( color.r(), color.g(), color.b() );
}


void C_WeaponPaintGun::StartHoseEffect()
{
	if( !draw_paint_particle_stream.GetBool() )
		return;

	const char *particleEffectName = "paint_hose";

	Color paintColor;

	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( pOwner && pOwner->IsLocalPlayer() )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( pOwner );

		// The local player sees the stream come out of the view model
		if( !m_pStreamEffect )
		{
			m_pStreamEffect = pOwner->GetViewModel()->ParticleProp()->Create( particleEffectName, PATTACH_POINT_FOLLOW, "rtube_muzzle" );
		}

		paintColor = MapPowerToVisualColor( m_bFiringPaint ? m_iSubType : NO_POWER );
	}
	else
	{
		if( !m_pStreamEffect )
		{
			m_pStreamEffect = ParticleProp()->Create( particleEffectName, PATTACH_POINT_FOLLOW, "muzzle" );
		}

		paintColor = MapPowerToVisualColor( m_bFiringPaint ? m_nCurrentColor : NO_POWER );
	}

	m_pStreamEffect->SetControlPoint( 1, Vector( paintColor.r(), paintColor.g(), paintColor.b() ) );
}


void C_WeaponPaintGun::StopHoseEffect()
{
	if( m_pStreamEffect )
	{
		C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if( pOwner && pOwner->IsLocalPlayer() )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( pOwner );

			if( pOwner->GetViewModel() )
			{
				pOwner->GetViewModel()->ParticleProp()->StopEmission( m_pStreamEffect );
				m_pStreamEffect = NULL;
			}
		}
		else
		{
			ParticleProp()->StopEmission( m_pStreamEffect );
			m_pStreamEffect = NULL;
		}
	}
}


C_BaseViewModel *C_WeaponPaintGun::GetEffectViewModel()
{
	C_BaseViewModel *pViewModel = NULL;
	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( pOwner && pOwner->IsLocalPlayer() )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( pOwner );
		pViewModel = pOwner->GetViewModel();
	}

	return pViewModel;
}


void C_WeaponPaintGun::AddViewmodelBob( C_BaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector forward, right, up;
	AngleVectors( angles, &forward, &right, &up );

	CalcViewmodelBob();

	C_Portal_Player *pPortalPlayer = ToPortalPlayer( GetOwner() );
	if( !pPortalPlayer )
		return;

	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * 0.1f, forward, origin );

	// Z bob a bit more, along the player's up (which can be reoriented by paint)
	origin += pPortalPlayer->GetPortalPlayerLocalData().m_Up * ( g_verticalBob * 0.1f );

	VectorMA( origin, g_lateralBob * 0.8f, right, origin );

	// Bob the angles about the view axes
	float rollAngle = g_verticalBob * 0.5f;
	VMatrix rotMatrix;
	Vector rotAxis = CrossProduct( right, up ).Normalized();
	MatrixBuildRotationAboutAxis( rotMatrix, rotAxis, rollAngle );
	up = rotMatrix * up;
	forward = rotMatrix * forward;
	right = rotMatrix * right;

	float pitchAngle = -g_verticalBob * 0.4f;
	rotAxis = right;
	MatrixBuildRotationAboutAxis( rotMatrix, rotAxis, pitchAngle );
	up = rotMatrix * up;
	forward = rotMatrix * forward;

	float yawAngle = -g_lateralBob * 0.3f;
	rotAxis = up;
	MatrixBuildRotationAboutAxis( rotMatrix, rotAxis, yawAngle );
	forward = rotMatrix * forward;

	VectorAngles( forward, up, angles );
}


#define PAINTGUN_BOB_CYCLE_MAX	0.45f
#define PAINTGUN_BOB_UP			0.5f

float C_WeaponPaintGun::CalcViewmodelBob()
{
	static float bobtime;
	static float lastbobtime;
	float cycle;

	C_BasePlayer *player = ToBasePlayer( GetOwner() );

	if( ( !gpGlobals->frametime ) || ( player == NULL ) )
	{
		return 0.0f;
	}

	// Find the speed of the player
	float speed = player->GetLocalVelocity().Length();
	speed = clamp( speed, -player->MaxSpeed(), player->MaxSpeed() );

	float bob_offset = RemapVal( speed, 0.0f, player->MaxSpeed(), 0.0f, 1.0f );

	bobtime += gpGlobals->frametime * bob_offset;
	lastbobtime = gpGlobals->curtime;

	// Calculate the vertical bob
	cycle = bobtime - (int)( bobtime / PAINTGUN_BOB_CYCLE_MAX ) * PAINTGUN_BOB_CYCLE_MAX;
	cycle /= PAINTGUN_BOB_CYCLE_MAX;

	if( cycle < PAINTGUN_BOB_UP )
	{
		cycle = M_PI * cycle / PAINTGUN_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI * ( cycle - PAINTGUN_BOB_UP ) / ( 1.0 - PAINTGUN_BOB_UP );
	}

	g_verticalBob = speed * 0.005f;
	g_verticalBob = g_verticalBob * 0.3 + g_verticalBob * 0.7 * sin( cycle );
	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

	// Calculate the lateral bob
	cycle = bobtime - (int)( bobtime / PAINTGUN_BOB_CYCLE_MAX * 2 ) * PAINTGUN_BOB_CYCLE_MAX * 2;
	cycle /= PAINTGUN_BOB_CYCLE_MAX * 2;

	if( cycle < PAINTGUN_BOB_UP )
	{
		cycle = M_PI * cycle / PAINTGUN_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI * ( cycle - PAINTGUN_BOB_UP ) / ( 1.0 - PAINTGUN_BOB_UP );
	}

	g_lateralBob = speed * 0.005f;
	g_lateralBob = g_lateralBob * 0.3 + g_lateralBob * 0.7 * sin( cycle );
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );

	return 0.0f;
}


void C_WeaponPaintGun::SetPaintPower( PaintPowerType type )
{
	if( HasPaintPower( type ) )
	{
		SetSubType( type );

		// Let the input system know so the server gets the new sub type
		input->MakeWeaponSelection( this );

		IGameEvent *event = gameeventmanager->CreateEvent( "player_changed_colors" );
		if ( event )
		{
			C_BasePlayer *pPlayer = ToBasePlayer( GetOwner() );
			if( pPlayer )
			{
				event->SetInt( "userid", pPlayer->GetUserID() );

				gameeventmanager->FireEventClientSide( event );
			}
		}
	}
}


// Order in which the paint powers are cycled through
static const PaintPowerType paintPowersInGunOrder[] = { BOUNCE_POWER, SPEED_POWER, REFLECT_POWER, PORTAL_POWER };

void C_WeaponPaintGun::CyclePaintPower( bool bForward )
{
	if( !HasAnyPaintPower() )
		return;

	const int nPowerCount = ARRAYSIZE( paintPowersInGunOrder );

	int nCurrentPowerIndex = -1;
	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
	int nCurrentColor = pOwner && pOwner->IsLocalPlayer() ? m_iSubType : m_nCurrentColor;

	for( int i = 0; i < nPowerCount; ++i )
	{
		if( paintPowersInGunOrder[i] == nCurrentColor )
		{
			nCurrentPowerIndex = i;
			break;
		}
	}

	// Find the next power the gun has
	int nNextPowerIndex = nCurrentPowerIndex;
	int nCounter = bForward ? 1 : -1;
	do
	{
		nNextPowerIndex += nCounter;
		if( nNextPowerIndex == nPowerCount )
		{
			nNextPowerIndex = 0;
		}
		else if( nNextPowerIndex < 0 )
		{
			nNextPowerIndex = nPowerCount - 1;
		}
	} while( !HasPaintPower( paintPowersInGunOrder[nNextPowerIndex] ) );

	SetPaintPower( paintPowersInGunOrder[nNextPowerIndex] );
}


static void NextPaint()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if( !pPlayer )
		return;

	C_WeaponPaintGun *pPaintGun = dynamic_cast< C_WeaponPaintGun* >( pPlayer->GetActiveWeapon() );
	if( pPaintGun )
	{
		pPaintGun->CyclePaintPower( true );
	}
}

static ConCommand nextpaint( "nextpaint", NextPaint );


static void PrevPaint()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if( !pPlayer )
		return;

	C_WeaponPaintGun *pPaintGun = dynamic_cast< C_WeaponPaintGun* >( pPlayer->GetActiveWeapon() );
	if( pPaintGun )
	{
		pPaintGun->CyclePaintPower( false );
	}
}

static ConCommand prevpaint( "prevpaint", PrevPaint );


static void ChangePaintTo( const CCommand &args )
{
	if( args.ArgC() != 2 )
	{
		DevMsg( "changepaintto bounce,speed,stick" );
		return;
	}

	if( !V_stricmp( args[1], "bounce" ) )
	{
		ChangePaintColor( BOUNCE_POWER );
	}
	else if( !V_stricmp( args[1], "speed" ) )
	{
		ChangePaintColor( SPEED_POWER );
	}
	else if( !V_stricmp( args[1], "stick" ) )
	{
		ChangePaintColor( REFLECT_POWER );
	}
	else if( !V_stricmp( args[1], "portal" ) )
	{
		ChangePaintColor( PORTAL_POWER );
	}
}

static ConCommand changepaintto( "changepaintto", ChangePaintTo );
