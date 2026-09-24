//--------------------------------------------------------------------------------------------------------
// Copyright (c) 2007 Turtle Rock Studios, Inc. - All Rights Reserved
//
// Portal 2 port: fog_volume, from the CS:GO source drop; the repository's provenance and distribution warning applies.
// Fog controller switching is applied by CFogVolumeSystem below (CS:GO does it
// in CBasePlayer::UpdateFXVolume). Per-volume post-process and color-correction
// controllers are not applied by this engine's client and are reported once.

#include "cbase.h"
#include "fogvolume.h"
#include "collisionutils.h"
#include "fogcontroller.h"
#include "postprocesscontroller.h"
#include "player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CUtlVector< CFogVolume * > TheFogVolumes;

ConVar fog_volume_debug( "fog_volume_debug", "0", 0, "If enabled, prints diagnostic information about the current fog volume" );

//--------------------------------------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(fog_volume, CFogVolume);

BEGIN_DATADESC( CFogVolume )

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	DEFINE_KEYFIELD( m_fogName, FIELD_STRING, "FogName" ),
	DEFINE_KEYFIELD( m_postProcessName, FIELD_STRING, "PostProcessName" ),
	DEFINE_KEYFIELD( m_colorCorrectionName, FIELD_STRING, "ColorCorrectionName" ),
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN,	"StartDisabled" ),

	DEFINE_FIELD( m_hFogController, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hPostProcessController, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hColorCorrectionController, FIELD_EHANDLE ),

END_DATADESC()


//--------------------------------------------------------------------------------------------------------
CFogVolume *CFogVolume::FindFogVolumeForPosition( const Vector &position )
{
	CFogVolume *fogVolume = NULL;
	for ( int i=0; i<TheFogVolumes.Count(); ++i )
	{
		fogVolume = TheFogVolumes[i];

		Vector vecRelativeCenter;
		fogVolume->CollisionProp()->WorldToCollisionSpace( position, &vecRelativeCenter );
		if ( IsBoxIntersectingSphere( fogVolume->CollisionProp()->OBBMins(), fogVolume->CollisionProp()->OBBMaxs(), vecRelativeCenter, 1.0f ) )
		{
			break;
		}
		fogVolume = NULL;
	}

	// This doesn't work well if there are multiple players or multiple fog volume queries per frame; might want to relocate this if that's the case
	if ( fog_volume_debug.GetBool() )
	{
		if ( fogVolume )
		{
			char fogVolumeName[256];
			fogVolume->GetKeyValue( "targetname", fogVolumeName, 256 );
			engine->Con_NPrintf( 0, "Fog Volume ""%s"" found at position (%f %f %f)", fogVolumeName, position.x, position.y, position.z );
			engine->Con_NPrintf( 1, "Fog: %s, post process: %s, color correct: %s", fogVolume->m_fogName.ToCStr(), fogVolume->m_postProcessName.ToCStr(), fogVolume->m_colorCorrectionName.ToCStr() );
		}
		else
		{
			engine->Con_NPrintf( 0, "No Fog Volume found at given position (%f %f %f)", position.x, position.y, position.z );
		}
	}
	
	return fogVolume;
}


//--------------------------------------------------------------------------------------------------------
CFogVolume::CFogVolume() :
	BaseClass(),
	m_bDisabled( false ),
	m_bInFogVolumesList( false )
{
}


//--------------------------------------------------------------------------------------------------------
CFogVolume::~CFogVolume()
{
	RemoveFromGlobalList();
}


//--------------------------------------------------------------------------------------------------------
void CFogVolume::Spawn( void )
{
	BaseClass::Spawn();

	SetSolid( SOLID_BSP );
	SetSolidFlags( FSOLID_NOT_SOLID );
	SetModel( STRING( GetModelName() ) );
}


//--------------------------------------------------------------------------------------------------------
void CFogVolume::AddToGlobalList()
{
	if ( !m_bInFogVolumesList )
	{
		TheFogVolumes.AddToTail( this );
		m_bInFogVolumesList = true;
	}
}


//--------------------------------------------------------------------------------------------------------
void CFogVolume::RemoveFromGlobalList()
{
	if ( m_bInFogVolumesList )
	{
		TheFogVolumes.FindAndRemove( this );
		m_bInFogVolumesList = false;
	}
}


//----------------------------------------------------------------------------
void CFogVolume::InputEnable( inputdata_t &data )
{
	m_bDisabled = false;
	AddToGlobalList();
}


//----------------------------------------------------------------------------
void CFogVolume::InputDisable( inputdata_t &data )
{
	m_bDisabled = true;
	RemoveFromGlobalList();
}


//----------------------------------------------------------------------------
// Called when the level loads or is restored
//----------------------------------------------------------------------------
void CFogVolume::Activate()
{
	BaseClass::Activate();

	m_hFogController = dynamic_cast< CFogController* >( gEntList.FindEntityByName( NULL, m_fogName ) );
	m_hPostProcessController = dynamic_cast< CPostProcessController* >( gEntList.FindEntityByName( NULL, m_postProcessName ) );
	CBaseEntity *pColorCorrection = gEntList.FindEntityByName( NULL, m_colorCorrectionName );
	m_hColorCorrectionController = ( pColorCorrection && FClassnameIs( pColorCorrection, "color_correction" ) ) ? pColorCorrection : NULL;

	if ( m_hPostProcessController || m_hColorCorrectionController )
	{
		// Portal 2 port: the client applies only the master post-process and
		// color-correction state, never a per-player override.
		static bool s_bWarned = false;
		if ( !s_bWarned )
		{
			s_bWarned = true;
			DevWarning( "Portal 2: fog_volume post-process/color-correction overrides are not supported by this engine\n" );
		}
	}

	if ( !m_bDisabled )
	{
		AddToGlobalList();
	}
}


//--------------------------------------------------------------------------------------------------------
// Portal 2 port: the fog half of CS:GO's CBasePlayer::UpdateFXVolume, run for
// every player after entities think. The player here has no per-player
// post-process or color-correction handles.
//--------------------------------------------------------------------------------------------------------
class CFogVolumeSystem : public CAutoGameSystemPerFrame
{
public:
	CFogVolumeSystem() : CAutoGameSystemPerFrame( "CFogVolumeSystem" ) {}

	virtual void FrameUpdatePostEntityThink()
	{
		if ( TheFogVolumes.Count() == 0 )
			return;

		for ( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( !pPlayer )
				continue;

			CBaseEntity *pViewEntity = pPlayer->GetViewEntity();
			Vector eyePos = pViewEntity ? pViewEntity->GetAbsOrigin() : pPlayer->EyePosition();

			CFogController *pFogController = NULL;
			CFogVolume *pFogVolume = CFogVolume::FindFogVolumeForPosition( eyePos );
			if ( pFogVolume )
			{
				pFogController = pFogVolume->GetFogController();
			}
			if ( !pFogController )
			{
				// Outside every volume (or a volume without fog): back to the master.
				pFogController = FogSystem()->GetMasterFogController();
			}

			if ( pFogController && pPlayer->m_Local.m_PlayerFog.m_hCtrl.Get() != pFogController )
			{
				pPlayer->m_Local.m_PlayerFog.m_hCtrl.Set( pFogController );
			}
		}
	}
};

static CFogVolumeSystem s_FogVolumeSystem;
