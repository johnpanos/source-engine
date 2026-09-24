//========= Portal 2 reconstruction ============================================//
//
// Purpose: Personality core (npc_personality_core): a talking, carryable sphere.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "ai_playerally.h"
#include "ai_speech.h"
#include "ai_default.h"
#include "player_pickup.h"
#include "sceneentity.h"
#include "choreoevent.h"
#include "explode.h"
#include "props.h"
#include "te_effect_dispatch.h"
#include "particle_parse.h"
#include "portal_grabcontroller_shared.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PERSONALITY_CORE_MODEL	"models/npcs/personality_sphere/personality_sphere.mdl"

ConVar sv_personality_core_pca_pitch( "sv_personality_core_pca_pitch", "180", 0, "Pitch value for personality core perferred carry angles." );
ConVar sv_personality_core_pca_yaw( "sv_personality_core_pca_yaw", "-90", 0, "Yaw value for personality core perferred carry angles." );
ConVar sv_personality_core_pca_roll( "sv_personality_core_pca_roll", "195", 0, "Roll value for personality core perferred carry angles." );

//-----------------------------------------------------------------------------
// Core personalities (m_lpszCoreTypeNames holds the response rule names)
//-----------------------------------------------------------------------------
enum CoreType_e
{
	CORETYPE_CURIOUS = 0,
	CORETYPE_AGGRESSIVE,
	CORETYPE_CRAZY,
	CORETYPE_RICHARD,
	CORETYPE_AQUARIUM,
	CORETYPE_NONE,

	NUM_CORETYPES,
};

class CNPC_PersonalityCore : public CAI_PlayerAlly, public CDefaultPlayerPickupVPhysics
{
public:
	DECLARE_CLASS( CNPC_PersonalityCore, CAI_PlayerAlly );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual ~CNPC_PersonalityCore();

	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );

	void			EnableMotion( void );
	void			InputEnableMotion( inputdata_t &inputdata );
	void			InputDisableMotion( inputdata_t &inputdata );

	void			InputEnableFlashlight( inputdata_t &inputdata );
	void			InputDisableFlashlight( inputdata_t &inputdata );

	void			InputEnablePickup( inputdata_t &inputdata );
	void			InputDisablePickup( inputdata_t &inputdata );
	void			InputForcePickup( inputdata_t &inputdata );

	void			InputPlayLock( inputdata_t &inputdata );
	void			InputPlayAttach( inputdata_t &inputdata );
	void			InputPlayDetach( inputdata_t &inputdata );

	void			InputSetIdleSequence( inputdata_t &inputdata );
	void			InputClearIdleSequence( inputdata_t &inputdata );

	void			InputExplode( inputdata_t &inputdata );

	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );
	virtual int		SelectSchedule( void );

	virtual bool	ShouldIgnite( const CTakeDamageInfo &info ) { return false; }
	virtual bool	ShouldPlayIdleSound( void );
	virtual void	IdleSound( void );
	virtual void	HandleAnimEvent( animevent_t *pEvent );
	virtual int		TranslateSchedule( int scheduleType );
	virtual void	GatherConditions( void );
	virtual void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void	OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );
	virtual void	OnFizzled( void );
	virtual bool	CreateVPhysics( void );
	virtual int		ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS; }
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual QAngle	PreferredCarryAngles( void );
	virtual bool	HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return true; }
	virtual void	ModifyOrAppendCriteria( ResponseRules::CriteriaSet &set );
	virtual CAI_Expresser *CreateExpresser( void )
	{
		CAI_Expresser *expresser = new CAI_ExpresserWithFollowup( this );
		expresser->Connect( this );
		return expresser;
	}

	virtual void	NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );

	virtual bool	IsOkToSpeakInResponseToPlayer( void );

	virtual bool	StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget );
	virtual void	PrescheduleThink( void );

protected:
	virtual bool	TestRemarkingUpon( CInfoRemarkable *pRemarkable );
	bool			IsBeingHeldByPlayer( void );
	const char		*GetCoreTypeName( void );
	float			GetPlayerSpeed( void );
	CBasePlayer		*GetFirstPlayer( void );
	const char		*GetPlayerHeldEntityName( void );

	CoreType_e		m_CoreType;
	static const char *m_lpszCoreTypeNames[NUM_CORETYPES];

	float			m_flNextIdleSoundTime;
	float			m_flLastPhysicsImpactTime;
	bool			m_bHasBeenPickedUp;
	bool			m_bPickupEnabled;
	bool			m_bAttached;

	COutputEvent	m_OnPlayerPickup;
	COutputEvent	m_OnPlayerDrop;

	float			m_flAnimResetTime;

	int				m_iIdleOverrideSequence;

	CNetworkVar( bool, m_bFlashlightEnabled );

	EHANDLE			m_hProjectedTexture;

	DEFINE_CUSTOM_AI;
};

const char *CNPC_PersonalityCore::m_lpszCoreTypeNames[NUM_CORETYPES] =
{
	"curious",
	"aggressive",
	"crazy",
	"sphere03",
	"sphere02",
	"none",
};

BEGIN_DATADESC( CNPC_PersonalityCore )
	DEFINE_FIELD( m_flNextIdleSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastPhysicsImpactTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flAnimResetTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_bHasBeenPickedUp, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPickupEnabled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAttached, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFlashlightEnabled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iIdleOverrideSequence, FIELD_INTEGER ),
	DEFINE_FIELD( m_hProjectedTexture, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_CoreType, FIELD_INTEGER, "CoreType" ),

	DEFINE_OUTPUT( m_OnPlayerPickup, "OnPlayerPickup" ),
	DEFINE_OUTPUT( m_OnPlayerDrop, "OnPlayerDrop" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "EnableMotion", InputEnableMotion ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableMotion", InputDisableMotion ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableFlashlight", InputEnableFlashlight ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableFlashlight", InputDisableFlashlight ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForcePickup", InputForcePickup ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnablePickup", InputEnablePickup ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisablePickup", InputDisablePickup ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PlayLock", InputPlayLock ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PlayAttach", InputPlayAttach ),
	DEFINE_INPUTFUNC( FIELD_VOID, "PlayDetach", InputPlayDetach ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetIdleSequence", InputSetIdleSequence ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ClearIdleSequence", InputClearIdleSequence ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Explode", InputExplode ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_personality_core, CNPC_PersonalityCore );

IMPLEMENT_SERVERCLASS_ST( CNPC_PersonalityCore, DT_NPC_Personality_Core )
	SendPropBool( SENDINFO( m_bFlashlightEnabled ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
	if ( m_hProjectedTexture )
	{
		UTIL_Remove( m_hProjectedTexture );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::Precache( void )
{
	PrecacheModel( PERSONALITY_CORE_MODEL );
	PrecacheInstancedScene( "scenes/npc/sp_proto_sphere/sphere_plug_attach.vcd" );
	PrecacheParticleSystem( "flashlight_thirdperson" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::Spawn( void )
{
	Precache();

	CapabilitiesClear();

	SetModel( PERSONALITY_CORE_MODEL );

	SetHullType( HULL_SMALL_CENTERED );
	SetHullSizeNormal();
	SetDefaultEyeOffset();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetCollisionGroup( COLLISION_GROUP_NONE );
	SetMoveType( MOVETYPE_VPHYSICS );

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );

	m_NPCState = NPC_STATE_NONE;
	m_takedamage = DAMAGE_EVENTS_ONLY;
	SetHealth( 250 );
	SetBloodColor( DONT_BLEED );
	m_flFieldOfView = -1.0f;
	m_bRemarkablePolling = true;

	NPCInit();

	BaseClass::Spawn();

	SetBlocksLOS( false );

	m_flLastPhysicsImpactTime = 0.0f;
	m_flAnimResetTime = 0.0f;
	m_bHasBeenPickedUp = false;
	m_bPickupEnabled = true;
	m_bAttached = false;
	m_iIdleOverrideSequence = -1;
	m_bFlashlightEnabled = false;

	// Build the (initially off) flashlight at the eye
	int iAttachmentIndex = LookupAttachment( "eyes" );

	Vector vecAttachOrigin;
	QAngle vecAttachAngles;
	GetAttachment( iAttachmentIndex, vecAttachOrigin, vecAttachAngles );

	m_hProjectedTexture = CreateEntityByName( "env_projectedtexture" );
	m_hProjectedTexture->KeyValue( "lightfov", "100.0f" );
	m_hProjectedTexture->KeyValue( "enableshadows", "1" );
	// Reconstruction note: spawnflag 2 of env_projectedtexture is "always update".
	m_hProjectedTexture->AddSpawnFlags( 2 );
	DispatchSpawn( m_hProjectedTexture );
	m_hProjectedTexture->SetAbsAngles( vecAttachAngles );
	m_hProjectedTexture->SetAbsOrigin( vecAttachOrigin );
	m_hProjectedTexture->SetParent( this, iAttachmentIndex );

	variant_t emptyVariant;
	m_hProjectedTexture->AcceptInput( "TurnOff", this, this, emptyVariant, 0 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::CreateVPhysics( void )
{
	VPhysicsDestroyObject();

	RemoveSolidFlags( FSOLID_NOT_SOLID );
	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );
	if ( pPhysicsObject )
	{
		pPhysicsObject->SetMass( 75.0f );
		SetMoveType( MOVETYPE_VPHYSICS );
	}

	return ( pPhysicsObject != NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
QAngle CNPC_PersonalityCore::PreferredCarryAngles( void )
{
	return QAngle( sv_personality_core_pca_pitch.GetFloat(), sv_personality_core_pca_yaw.GetFloat(), sv_personality_core_pca_roll.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: Play raw sequences from choreo directly; everything else goes to the base.
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	if ( event->GetType() == CChoreoEvent::SEQUENCE )
	{
		info->m_nSequence = LookupSequence( event->GetParameters() );
		if ( info->m_nSequence < 0 )
			return false;

		SetIdealSequence( info->m_nSequence, true );
		m_flAnimResetTime = gpGlobals->curtime + event->GetEndTime();
		return true;
	}

	return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
}

//-----------------------------------------------------------------------------
// Purpose: Return to the idle (or override) sequence once a scene sequence ends.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::PrescheduleThink( void )
{
	if ( gpGlobals->curtime > m_flAnimResetTime && m_flAnimResetTime != 0.0f )
	{
		if ( m_iIdleOverrideSequence >= 0 )
		{
			SetIdealSequence( m_iIdleOverrideSequence );
		}
		else if ( m_bAttached )
		{
			SetIdealSequence( LookupSequence( "sphere_plug_idle_neutral" ) );
		}
		else
		{
			SetIdealActivity( ACT_IDLE );
		}

		SetCycle( 0 );
		m_flAnimResetTime = 0.0f;
	}

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer( void )
{
	return IsOkToSpeak( SPEECH_PRIORITY, true );
}

//-----------------------------------------------------------------------------
// Purpose: The core runs no tasks of its own.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::StartTask( const Task_t *pTask )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::RunTask( const Task_t *pTask )
{
}

//-----------------------------------------------------------------------------
// Purpose: Put the core in the local player's hands.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputForcePickup( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if ( pPlayer )
	{
		SetParent( NULL );
		CreateVPhysics();
		pPlayer->PickupObject( this, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputEnablePickup( inputdata_t &inputdata )
{
	m_bPickupEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputDisablePickup( inputdata_t &inputdata )
{
	m_bPickupEnabled = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputPlayLock( inputdata_t &inputdata )
{
	InstancedScriptedScene( NULL, "scenes/npc/sp_proto_sphere/sphere_plug_lock.vcd" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputPlayAttach( inputdata_t &inputdata )
{
	InstancedScriptedScene( NULL, "scenes/npc/sp_proto_sphere/sphere_plug_attach.vcd" );
	m_bAttached = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputPlayDetach( inputdata_t &inputdata )
{
	m_bAttached = false;
	m_flAnimResetTime = -1.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputSetIdleSequence( inputdata_t &inputdata )
{
	m_iIdleOverrideSequence = LookupSequence( inputdata.value.String() );
	m_flAnimResetTime = -1.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputClearIdleSequence( inputdata_t &inputdata )
{
	m_iIdleOverrideSequence = -1;
	m_flAnimResetTime = -1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Blow up in a shower of metal chunks.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputExplode( inputdata_t &inputdata )
{
	ExplosionCreate( WorldSpaceCenter(), vec3_angle, this, 100, 500,
		SF_ENVEXPLOSION_NODAMAGE | SF_ENVEXPLOSION_NOSMOKE | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NOFIREBALLSMOKE | SF_ENVEXPLOSION_NODLIGHTS, 0.0f );

	UTIL_ScreenShake( WorldSpaceCenter(), 10.0f, 150.0f, 1.0f, 750.0f, SHAKE_START );

	CPVSFilter filter( WorldSpaceCenter() );
	Vector gibVelocity = RandomVector( -15.0f, 15.0f );
	int iModelIndex = modelinfo->GetModelIndex( g_PropDataSystem.GetRandomChunkModel( "MetalChunks" ) );
	for ( int i = 0; i < 12; i++ )
	{
		te->BreakModel( filter, 0.0, WorldSpaceCenter(), vec3_angle, Vector( 16, 16, 72 ), gibVelocity, iModelIndex, 400, 1, 2.5, BREAK_METAL );
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputEnableMotion( inputdata_t &inputdata )
{
	EnableMotion();
	CreateVPhysics();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::EnableMotion( void )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
		pPhysicsObject->Wake();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputDisableMotion( inputdata_t &inputdata )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputEnableFlashlight( inputdata_t &inputdata )
{
	if ( m_hProjectedTexture )
	{
		variant_t emptyVariant;
		m_hProjectedTexture->AcceptInput( "TurnOn", this, this, emptyVariant, 0 );
		m_bFlashlightEnabled = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::InputDisableFlashlight( inputdata_t &inputdata )
{
	if ( m_hProjectedTexture )
	{
		variant_t emptyVariant;
		m_hProjectedTexture->AcceptInput( "TurnOff", this, this, emptyVariant, 0 );
	}

	m_bFlashlightEnabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: Cores cannot be hurt; they only complain about being burned.
//-----------------------------------------------------------------------------
int CNPC_PersonalityCore::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	if ( info.GetDamageType() & DMG_BURN )
	{
		Speak( "TLK_BURNED" );
	}

	// Reconstruction note: both builds return 1 without applying the damage.
	bool bTookDamage = true;
	return bTookDamage;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CNPC_PersonalityCore::SelectSchedule( void )
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::ShouldPlayIdleSound( void )
{
	if ( ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT ) &&
		 gpGlobals->curtime > m_flNextIdleSoundTime )
	{
		return !HasSpawnFlags( SF_NPC_GAG );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::IdleSound( void )
{
	Speak( "TLK_IDLE" );
	m_flNextIdleSoundTime = gpGlobals->curtime + 0.1f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::HandleAnimEvent( animevent_t *pEvent )
{
	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CNPC_PersonalityCore::TranslateSchedule( int scheduleType )
{
	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::GatherConditions( void )
{
	BaseClass::GatherConditions();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( m_bPickupEnabled && pActivator && pActivator->IsPlayer() )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pActivator );

		SetParent( NULL );
		CreateVPhysics();
		pPlayer->PickupObject( this, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if ( reason == PICKED_UP_BY_PLAYER )
	{
		Speak( "TLK_PLAYER_PICKUP" );
		m_bHasBeenPickedUp = true;
		m_OnPlayerPickup.FireOutput( pPhysGunUser, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	if ( reason == DROPPED_BY_PLAYER )
	{
		Speak( "TLK_PLAYER_DROP" );
	}
	else if ( reason == THROWN_BY_PLAYER )
	{
		Speak( "TLK_PLAYER_THROW" );
	}

	m_OnPlayerDrop.FireOutput( pPhysGunUser, this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::OnFizzled( void )
{
	Speak( "TLK_FIZZLED" );
	BaseClass::OnFizzled();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::IsBeingHeldByPlayer( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && GetPlayerHeldEntity( pPlayer ) == this )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CNPC_PersonalityCore::GetFirstPlayer( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
			return pPlayer;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CNPC_PersonalityCore::GetPlayerSpeed( void )
{
	CBasePlayer *pPlayer = GetFirstPlayer();
	if ( pPlayer )
		return pPlayer->GetAbsVelocity().Length();

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CNPC_PersonalityCore::GetPlayerHeldEntityName( void )
{
	static const char *pDefaultHeldItemName = "None";

	CBasePlayer *pPlayer = GetFirstPlayer();
	if ( pPlayer )
	{
		CBaseEntity *heldItem = GetPlayerHeldEntity( pPlayer );
		if ( heldItem )
			return heldItem->GetClassname();
	}

	return pDefaultHeldItemName;
}

//-----------------------------------------------------------------------------
// Purpose: Comment on hard impacts.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	if ( gpGlobals->curtime - m_flLastPhysicsImpactTime < 0.1f )
		return;

	CBaseEntity *pHitEntity = pEvent->pEntities[!index];
	if ( pHitEntity == GetWorldEntity() )
	{
		if ( IsBeingHeldByPlayer() )
		{
			Speak( "TLK_HELD_PHYSICS_IMPACT" );
			m_flLastPhysicsImpactTime = gpGlobals->curtime;
		}
		else
		{
			Vector vecVelocity = pEvent->preVelocity[index];
			if ( vecVelocity.LengthSqr() > 10000.0f )
			{
				Speak( "TLK_PHYSICS_IMPACT" );
				m_flLastPhysicsImpactTime = gpGlobals->curtime;
			}
		}
	}
	else if ( !pHitEntity->IsPlayer() )
	{
		Speak( "TLK_PHYSICS_IMPACT" );
		m_flLastPhysicsImpactTime = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CNPC_PersonalityCore::GetCoreTypeName( void )
{
	return m_lpszCoreTypeNames[ m_CoreType ];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore::TestRemarkingUpon( CInfoRemarkable *pRemarkable )
{
	return IsLineOfSightClear( pRemarkable, IGNORE_ACTORS );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::ModifyOrAppendCriteria( ResponseRules::CriteriaSet &set )
{
	BaseClass::ModifyOrAppendCriteria( set );

	set.AppendCriteria( "core_type", GetCoreTypeName() );
	set.AppendCriteria( "core_held", IsBeingHeldByPlayer() ? "true" : "false" );
	set.AppendCriteria( "core_picked_up", m_bHasBeenPickedUp ? "true" : "false" );
	set.AppendCriteria( "player_speed", GetPlayerSpeed() );
	set.AppendCriteria( "player_held_item", GetPlayerHeldEntityName() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	if ( eventType == NOTIFY_EVENT_TELEPORT )
	{
		Speak( "TLK_PORTALED" );
	}
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_personality_core, CNPC_PersonalityCore )

AI_END_CUSTOM_NPC()
