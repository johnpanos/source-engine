//========= Portal 2 reconstruction ============================================//
//
// Purpose: Physics prop that splashes paint around itself when it breaks.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "prop_paint_bomb.h"
#include "portal_player.h"
#include "paint_database.h"
#include "portal_base2d.h"
#include "portal_util_shared.h"
#include "particle_parse.h"
#include "debugoverlay_shared.h"
#include "datacache/imdlcache.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar paintbomb_explosion_radius( "paintbomb_explosion_radius", "100.f", FCVAR_CHEAT, "Radius of the trace from the center of the explosion" );

ConVar paintbomb_streak_speed_min( "paintbomb_streak_speed_min", "150.f", FCVAR_CHEAT );
ConVar paintbomb_streak_speed_max( "paintbomb_streak_speed_max", "250.f", FCVAR_CHEAT );

ConVar paintbomb_blobs_min_streak_time( "paintbomb_blobs_min_streak_time", "0.1f", FCVAR_CHEAT );
ConVar paintbomb_blobs_max_streak_time( "paintbomb_blobs_max_streak_time", "0.2f", FCVAR_CHEAT );
ConVar paintbomb_blobs_min_streak_speed_dampen( "paintbomb_blobs_min_streak_speed_dampen", "500.f", FCVAR_CHEAT );
ConVar paintbomb_blobs_max_streak_speed_dampen( "paintbomb_blobs_max_streak_speed_dampen", "800.0f", FCVAR_CHEAT );

ConVar paintbomb_vertical_angle_split( "paintbomb_vertical_angle_split", "8", FCVAR_CHEAT );
ConVar paintbomb_horizontal_angle_split( "paintbomb_horizontal_angle_split", "8", FCVAR_CHEAT );

extern ConVar player_can_use_painted_power;
extern ConVar player_paint_effects_enabled;

ConVar debug_paintbomb_explosion( "debug_paintbomb_explosion", "0", FCVAR_CHEAT );


BEGIN_DATADESC( CPropPaintBomb )
	DEFINE_KEYFIELD( m_nPaintPowerType, FIELD_INTEGER, "PaintType" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Dissolve", InputDissolve ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisablePortalFunnel", InputDisablePortalFunnel ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnablePortalFunnel", InputEnablePortalFunnel ),

	DEFINE_OUTPUT( m_OnFizzled, "OnFizzled" ),
	DEFINE_OUTPUT( m_OnExploded, "OnExploded" ),
END_DATADESC()


IMPLEMENT_SERVERCLASS_ST( CPropPaintBomb, DT_PropPaintBomb )
	SendPropInt( SENDINFO( m_nPaintPowerType ) ),
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS( prop_paint_bomb, CPropPaintBomb );


CPropPaintBomb::CPropPaintBomb()
{
	m_nPaintPowerType = NO_POWER;
}


CPropPaintBomb::~CPropPaintBomb()
{
}


void CPropPaintBomb::Precache()
{
	PrecacheModel( "models/props/futbol.mdl" );
	PrecacheModel( "models/props/futbol_gib01.mdl" );
	PrecacheModel( "models/props/futbol_gib02.mdl" );
	PrecacheModel( "models/props/futbol_gib03.mdl" );
	PrecacheModel( "models/props/futbol_gib04.mdl" );

	PrecacheScriptSound( "PaintBomb.Splat" );

	PrecacheParticleSystem( "paint_bomb_bounce" );
	PrecacheParticleSystem( "paint_bomb_speed" );
	PrecacheParticleSystem( "paint_bomb_stick" );
	PrecacheParticleSystem( "paint_bomb_erase" );

	BaseClass::Precache();
}


void CPropPaintBomb::Spawn()
{
	Precache();
	SetModel( "models/props/futbol.mdl" );

	// The client draws the bomb as a ball of paint blobs
	AddEffects( EF_NODRAW );

	AddSpawnFlags( SF_PHYSPROP_PREVENT_PICKUP );

	BaseClass::Spawn();

	int nPaintBombSurfaceIndex = physprops->GetSurfaceIndex( "PaintBomb" );
	IPhysicsObject *pPhysObject = VPhysicsGetObject();
	if( pPhysObject )
	{
		pPhysObject->SetMaterialIndex( nPaintBombSurfaceIndex );
	}
}


int CPropPaintBomb::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_PVSCHECK );
}


void CPropPaintBomb::SetPaintPowerType( PaintPowerType paintType )
{
	m_nPaintPowerType = paintType;

	Color color = MapPowerToVisualColor( m_nPaintPowerType );
	SetRenderColor( color.r(), color.g(), color.b() );
}


//-----------------------------------------------------------------------------
// Purpose: Builds evenly spread unit directions over a sphere
//-----------------------------------------------------------------------------
void GenerateBombDirections( CUtlVector<Vector> &directions )
{
	if( paintbomb_vertical_angle_split.GetInt() <= 0 || paintbomb_horizontal_angle_split.GetInt() <= 0 )
		return;

	directions.Purge();

	float flVerticalAngle = 180.f / paintbomb_vertical_angle_split.GetFloat();
	float flHorizontalAngle = 180.f / paintbomb_horizontal_angle_split.GetFloat();

	// Rotate up around Y to make one vertical slice
	Vector vecTemp( 0.f, 0.f, 1.f );
	VMatrix matRotation;
	MatrixBuildRotationAboutAxis( matRotation, Vector( 0.f, 1.f, 0.f ), flVerticalAngle );
	int nVerticalSplit = paintbomb_vertical_angle_split.GetInt();
	for( int i = 1; i < nVerticalSplit; ++i )
	{
		vecTemp = matRotation * vecTemp;
		directions.AddToTail( vecTemp );
		directions.AddToHead( -vecTemp );
	}

	// Rotate the last slice around Z to fill the sphere
	MatrixBuildRotateZ( matRotation, flHorizontalAngle );
	for( int x = 1; x < nVerticalSplit; ++x )
	{
		int startIndex = directions.Count() - ( nVerticalSplit - 1 );
		for( int y = 1; y < nVerticalSplit; ++y )
		{
			// Each pass also adds one direction at the head, so step over it
			vecTemp = directions[ startIndex + ( y - 1 ) * 2 ];
			vecTemp = matRotation * vecTemp;
			directions.AddToTail( vecTemp );
			directions.AddToHead( -vecTemp );
		}
	}

	// The poles
	directions.AddToHead( Vector( 0.f, 0.f, 1.f ) );
	directions.AddToTail( Vector( 0.f, 0.f, -1.f ) );
}


void DispatchDryBombParticleEffect( const Vector &pos, PaintPowerType paintType, QAngle angSplash )
{
	switch( paintType )
	{
	case BOUNCE_POWER:
		DispatchParticleEffect( "paint_bomb_bounce", pos, angSplash );
		break;
	case REFLECT_POWER:
		DispatchParticleEffect( "paint_bomb_stick", pos, angSplash );
		break;
	case SPEED_POWER:
		DispatchParticleEffect( "paint_bomb_speed", pos, angSplash );
		break;
	// Reconstruction note: both builds use the erase effect for portal paint.
	case PORTAL_POWER:
	case NO_POWER:
		DispatchParticleEffect( "paint_bomb_erase", pos, angSplash );
		break;
	default:
		break;
	}
}


void CreatePaintBombExplosion( PaintPowerType paintType, const Vector &vecExplosionPos, bool bSpawnBlobs )
{
	// Cleansed bombs just splash
	if( !bSpawnBlobs )
	{
		DispatchDryBombParticleEffect( vecExplosionPos, paintType, QAngle( 0.f, 0.f, 1.f ) );
		return;
	}

	CUtlVector<Vector> dirs;
	QAngle angSplashAngle( 0.f, 0.f, 1.f );
	GenerateBombDirections( dirs );

	// Paint the surfaces around the bomb
	const float radius = paintbomb_explosion_radius.GetFloat();
	for( int i = 0; i < dirs.Count(); ++i )
	{
		Vector vecBlobFireDir = dirs[i].Normalized();

		trace_t tr;
		CTraceFilterSimpleClassnameList filter( NULL, COLLISION_GROUP_NONE );
		filter.AddClassnameToIgnore( "prop_paint_bomb" );

		Ray_t ray;
		ray.Init( vecExplosionPos, vecExplosionPos + radius * vecBlobFireDir );
		CPortal_Base2D *pPortal = UTIL_Portal_TraceRay( ray, MASK_SHOT, &filter, &tr );
		if( tr.fraction < 1.0f && tr.m_pEnt && !pPortal )
		{
			PaintDatabase.AddPaint( tr, paintType );

			VectorAngles( -tr.plane.normal, angSplashAngle );
		}
	}

	// Paint the players close to the explosion
	if( player_can_use_painted_power.GetBool() || player_paint_effects_enabled.GetBool() )
	{
		for( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CPortal_Player *pPlayer = ToPortalPlayer( UTIL_PlayerByIndex( i ) );
			if( !pPlayer || !pPlayer->IsConnected() )
				continue;

			const Vector &playerCenter = pPlayer->WorldSpaceCenter();
			const Vector playerToBomb = vecExplosionPos - playerCenter;
			if( playerToBomb.LengthSqr() < radius * radius )
			{
				PaintDatabase.PaintEntity( pPlayer, paintType, playerCenter );
			}
		}
	}

	DispatchDryBombParticleEffect( vecExplosionPos, paintType, angSplashAngle );

	if( debug_paintbomb_explosion.GetBool() )
	{
		for( int i = 0; i < dirs.Count(); ++i )
		{
			Vector vecBlobFireDir = dirs[i].Normalized();
			NDebugOverlay::Line( vecExplosionPos, vecExplosionPos + vecBlobFireDir * 50.f, 0, 255, 0, false, 5.f );
		}
	}
}


void CPropPaintBomb::Event_Killed( const CTakeDamageInfo &info )
{
	Break( info.GetInflictor(), info );

	Vector vecPos = GetAbsOrigin();

	// A bomb broken by a cleanser doesn't paint anything
	bool bSpawnBlobs = !( info.GetInflictor() && FClassnameIs( info.GetInflictor(), "trigger_paint_cleanser" ) );

	CreatePaintBombExplosion( (PaintPowerType)m_nPaintPowerType.Get(), vecPos, bSpawnBlobs );

	m_OnExploded.FireOutput( this, this );
}


void CPropPaintBomb::CleansePaintPower()
{
	SetPaintPowerType( NO_POWER );
}


void CPropPaintBomb::Break( CBaseEntity *pBreaker, const CTakeDamageInfo &info )
{
	EmitSound( "PaintBomb.Splat" );
	UTIL_Remove( this );
}


void CPropPaintBomb::OnFizzled()
{
	m_OnFizzled.FireOutput( this, this );
}


void CPropPaintBomb::InputDissolve( inputdata_t &in )
{
	Dissolve( NULL, gpGlobals->curtime + 0.1f, false, ENTITY_DISSOLVE_NORMAL, vec3_origin, 0 );
	OnFizzled();
}


void CPropPaintBomb::InputDisablePortalFunnel( inputdata_t &in )
{
	m_bAllowPortalFunnel = false;
}


void CPropPaintBomb::InputEnablePortalFunnel( inputdata_t &in )
{
	m_bAllowPortalFunnel = true;
}


void CreatePaintBomb( PaintPowerType paintType )
{
	MDLCACHE_CRITICAL_SECTION();

	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	CPropPaintBomb *pPaintBomb = static_cast< CPropPaintBomb* >( CreateEntityByName( "prop_paint_bomb" ) );
	if( pPaintBomb )
	{
		pPaintBomb->Precache();
		pPaintBomb->SetPaintPowerType( paintType );
		DispatchSpawn( pPaintBomb );

		// Place it where the player is looking
		CBasePlayer *pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );
		UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + vecForward * MAX_TRACE_LENGTH, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if( tr.fraction != 1.0f )
		{
			tr.endpos.z += 12.f;
			pPaintBomb->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( pPaintBomb, MASK_SOLID );
		}

		pPaintBomb->UpdateObjectCapsCache();
	}

	CBaseEntity::SetAllowPrecache( bAllowPrecache );
}


static void ent_create_paint_bomb_jump( const CCommand &args )
{
	CreatePaintBomb( BOUNCE_POWER );
}
static ConCommand ent_create_paint_bomb_jump_command( "ent_create_paint_bomb_jump", ent_create_paint_bomb_jump, "Creates a paint bomb with the jump paint paint power", FCVAR_CHEAT );


static void ent_create_paint_bomb_speed( const CCommand &args )
{
	CreatePaintBomb( SPEED_POWER );
}
static ConCommand ent_create_paint_bomb_speed_command( "ent_create_paint_bomb_speed", ent_create_paint_bomb_speed, "Creates a paint bomb with the speed paint paint power", FCVAR_CHEAT );


static void ent_create_paint_bomb_stick( const CCommand &args )
{
	CreatePaintBomb( REFLECT_POWER );
}
static ConCommand ent_create_paint_bomb_stick_command( "ent_create_paint_bomb_stick", ent_create_paint_bomb_stick, "Creates a paint bomb with the stick paint paint power", FCVAR_CHEAT );


static void ent_create_paint_bomb_portal( const CCommand &args )
{
	CreatePaintBomb( PORTAL_POWER );
}
static ConCommand ent_create_paint_bomb_portal_command( "ent_create_paint_bomb_portal", ent_create_paint_bomb_portal, "Creates a paint bomb with the portal paint paint power", FCVAR_CHEAT );


static void ent_create_paint_bomb_erase( const CCommand &args )
{
	CreatePaintBomb( NO_POWER );
}
static ConCommand ent_create_paint_bomb_erase_command( "ent_create_paint_bomb_erase", ent_create_paint_bomb_erase, "Creates a paint bomb with the erase paint paint power", FCVAR_CHEAT );
