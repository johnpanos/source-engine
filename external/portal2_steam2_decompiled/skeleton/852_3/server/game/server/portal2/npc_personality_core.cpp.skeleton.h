// DWARF declaration skeleton for game/server/portal2/npc_personality_core.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc2090 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CEntityFactory<CNPC_PersonalityCore>::CEntityFactory() at line 231
	// inlined ServerClass::ServerClass() at line 233
	// inlined CAI_ClassScheduleIdSpace::CAI_ClassScheduleIdSpace() at line 902
}

// game/server/portal2/npc_personality_core.cpp:22
ConVar sv_personality_core_pca_pitch;

// game/server/portal2/npc_personality_core.cpp:23
ConVar sv_personality_core_pca_yaw;

// game/server/portal2/npc_personality_core.cpp:24
ConVar sv_personality_core_pca_roll;

// game/server/portal2/npc_personality_core.cpp:70
enum CoreType_e
{
	CORETYPE_CURIOUS = 0,
	CORETYPE_AGGRESSIVE = 1,
	CORETYPE_CRAZY = 2,
	CORETYPE_RICHARD = 3,
	CORETYPE_AQUARIUM = 4,
	CORETYPE_NONE = 5,
	NUM_CORETYPES = 6,
};

// game/server/portal2/npc_personality_core.cpp:83 sizeof=0x11cc (i386)
struct CNPC_PersonalityCore : public CAI_PlayerAlly, public CDefaultPlayerPickupVPhysics
{
public:
	virtual ~CNPC_PersonalityCore();  // line 89
	virtual void Precache();  // line 91
	virtual void Spawn();  // line 92
	virtual void StartTask( const Task_t * );  // line 93
	virtual void RunTask( const Task_t * );  // line 94
	void EnableMotion();  // line 96
	void InputEnableMotion( inputdata_t & );  // line 97
	void InputDisableMotion( inputdata_t & );  // line 98
	void InputEnableFlashlight( inputdata_t & );  // line 100
	void InputDisableFlashlight( inputdata_t & );  // line 101
	void InputEnablePickup( inputdata_t & );  // line 103
	void InputDisablePickup( inputdata_t & );  // line 104
	void InputPlayLock( inputdata_t & );  // line 106
	void InputPlayAttach( inputdata_t & );  // line 107
	void InputPlayDetach( inputdata_t & );  // line 108
	void InputSetIdleSequence( inputdata_t & );  // line 110
	void InputClearIdleSequence( inputdata_t & );  // line 111
	void InputExplode( inputdata_t & );  // line 113
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo & );  // line 115
	virtual int SelectSchedule();  // line 116
	virtual bool ShouldIgnite( const CTakeDamageInfo & );  // line 118
	virtual bool ShouldPlayIdleSound();  // line 119
	virtual void IdleSound();  // line 120
	virtual void HandleAnimEvent( animevent_t * );  // line 121
	virtual int TranslateSchedule( int );  // line 122
	virtual void GatherConditions();  // line 123
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 124
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 125
	virtual void OnFizzled();  // line 126
	virtual bool CreateVPhysics();  // line 127
	virtual int ObjectCaps();  // line 128
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 129
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 130
	virtual QAngle PreferredCarryAngles();  // line 131
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 132
	virtual void ModifyOrAppendCriteria( ResponseRules::CriteriaSet & );  // line 133
	virtual CAI_Expresser *CreateExpresser();  // line 134
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 141
	virtual bool IsOkToSpeakInResponseToPlayer();  // line 143
	virtual bool StartSceneEvent( CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, CBaseEntity * );  // line 145
	virtual void PrescheduleThink();  // line 146
protected:
	virtual bool TestRemarkingUpon( CInfoRemarkable * );  // line 150
	bool IsBeingHeldByPlayer();  // line 151
	const char *GetCoreTypeName();  // line 152
	float GetPlayerSpeed();  // line 153
	CBasePlayer *GetFirstPlayer();  // line 154
	const char *GetPlayerHeldEntityName();  // line 155
	CoreType_e m_CoreType; // +0x1178  // line 157
	static const char *m_lpszCoreTypeNames[6];  // line 158
	float m_flNextIdleSoundTime; // +0x117c  // line 160
	float m_flLastPhysicsImpactTime; // +0x1180  // line 161
	bool m_bHasBeenPickedUp; // +0x1184  // line 162
	bool m_bPickupEnabled; // +0x1185  // line 163
	bool m_bAttached; // +0x1186  // line 164
	COutputEvent m_OnPlayerPickup; // +0x1188  // line 166
	COutputEvent m_OnPlayerDrop; // +0x11a0  // line 167
	float m_flAnimResetTime; // +0x11b8  // line 169
	int m_iIdleOverrideSequence; // +0x11bc  // line 171
	CNetworkVar( bool, m_bFlashlightEnabled ); // +0x11c0  // line 173
	EHANDLE m_hProjectedTexture; // +0x11c4  // line 175
public:
	// game/server/portal2/npc_personality_core.cpp:177 sizeof=0x1 (i386)
	struct CScheduleLoader
	{
	public:
		CScheduleLoader();  // line 177
	};  // line 177
protected:
	static AI_SchedLoadStatus_t gm_SchedLoadStatus;  // line 177
	static CAI_ClassScheduleIdSpace gm_ClassScheduleIdSpace;  // line 177
	static const char *gm_pszErrorClassName;  // line 177
	CNPC_PersonalityCore::CScheduleLoader m_ScheduleLoader; // +0x11c8  // line 177
	static CAI_LocalIdSpace gm_SquadSlotIdSpace;  // line 177
	static CAI_ClassScheduleIdSpace &AccessClassScheduleIdSpaceDirect();  // line 177
	virtual CAI_ClassScheduleIdSpace *GetClassScheduleIdSpace();  // line 177
	virtual const char *GetSchedulingErrorName();  // line 177
	static void InitCustomSchedules();  // line 177
	static bool LoadSchedules();  // line 177
	virtual bool LoadedSchedules();  // line 177
	virtual const char *SquadSlotName( int );  // line 177
};

// game/server/portal2/npc_personality_core.cpp:83 (declaration)
void CNPC_PersonalityCore();

// game/server/portal2/npc_personality_core.cpp:86 @0x6c5f00 _ZN20CNPC_PersonalityCore14GetDataDescMapEv
datamap_t *CNPC_PersonalityCore::GetDataDescMap()
{
}

// game/server/portal2/npc_personality_core.cpp:86 @0x6c5f10 _ZN20CNPC_PersonalityCore10GetBaseMapEv
datamap_t *CNPC_PersonalityCore::GetBaseMap()
{
}

// game/server/portal2/npc_personality_core.cpp:87 @0x6c5f20 _ZN20CNPC_PersonalityCore14GetServerClassEv
ServerClass *CNPC_PersonalityCore::GetServerClass()
{
}

// game/server/portal2/npc_personality_core.cpp:87 @0x6c5f30 _ZN20CNPC_PersonalityCore40YouForgotToImplementOrDeclareServerClassEv
int CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/npc_personality_core.cpp:89 @0x6c8780 _ZN20CNPC_PersonalityCoreD0Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 243
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 247
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 247
}

// game/server/portal2/npc_personality_core.cpp:89 @0x6c89c0 _ZN20CNPC_PersonalityCoreD2Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 243
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined COutputEvent::~COutputEvent() at line 247
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 247
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 247
}

// game/server/portal2/npc_personality_core.cpp:89 @0x6c8c00 _ZN20CNPC_PersonalityCoreD1Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
}

// game/server/portal2/npc_personality_core.cpp:91 @0x6c64c0 _ZN20CNPC_PersonalityCore8PrecacheEv
void CNPC_PersonalityCore::Precache()
{
}

// game/server/portal2/npc_personality_core.cpp:92 @0x6c6c20 _ZN20CNPC_PersonalityCore5SpawnEv
void CNPC_PersonalityCore::Spawn()
{
	int iAttachmentIndex;  // line 307
	Vector vecAttachOrigin;  // line 309
	QAngle vecAttachAngles;  // line 310
	variant_t emptyVariant;  // line 313
	// inlined variant_t::variant_t() at line 327
	// inlined variant_t::variant_t() at line 313
	// inlined CHandle<CBaseEntity>::operator->() at line 327
	// inlined CHandle<CBaseEntity>::operator->() at line 325
	// inlined CHandle<CBaseEntity>::operator->() at line 324
	// inlined CHandle<CBaseEntity>::operator->() at line 323
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 321
	// inlined CBaseEntity::AddSpawnFlags() at line 319
	// inlined CHandle<CBaseEntity>::operator->() at line 319
	// inlined CHandle<CBaseEntity>::operator->() at line 318
	// inlined CHandle<CBaseEntity>::operator->() at line 317
	// inlined CHandle<CBaseEntity>::operator=() at line 315
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 304
	// inlined CBaseCombatCharacter::Spawn() at line 295
	// inlined CAI_BaseActor::EnableRemarkables() at line 291
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_takedamage>::operator=<int>() at line 284
	// inlined CBaseEntity::AddSolidFlags() at line 276
	// inlined CBaseEntity::SetSolid() at line 275
	// inlined CBaseCombatCharacter::SetHullType() at line 270
}

// game/server/portal2/npc_personality_core.cpp:93 @0x6c5f40 _ZN20CNPC_PersonalityCore9StartTaskEPK6Task_t
void CNPC_PersonalityCore::StartTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_personality_core.cpp:94 @0x6c5f50 _ZN20CNPC_PersonalityCore7RunTaskEPK6Task_t
void CNPC_PersonalityCore::RunTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_personality_core.cpp:96 @0x6c5fc0 _ZN20CNPC_PersonalityCore12EnableMotionEv
void CNPC_PersonalityCore::EnableMotion()
{
	IPhysicsObject *pPhysicsObject;  // line 548
	// inlined CBaseEntity::VPhysicsGetObject() at line 548
}

// game/server/portal2/npc_personality_core.cpp:97 @0x6c73b0 _ZN20CNPC_PersonalityCore17InputEnableMotionER11inputdata_t
void CNPC_PersonalityCore::InputEnableMotion( inputdata_t &inputdata )
{
	// inlined CNPC_PersonalityCore::EnableMotion() at line 539
}

// game/server/portal2/npc_personality_core.cpp:98 @0x6c6570 _ZN20CNPC_PersonalityCore18InputDisableMotionER11inputdata_t
void CNPC_PersonalityCore::InputDisableMotion( inputdata_t &inputdata )
{
	IPhysicsObject *pPhysicsObject;  // line 561
	// inlined CBaseEntity::VPhysicsGetObject() at line 561
}

// game/server/util.h:99 @0x6c8d60 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE6CreateEPKc
IServerNetworkable *CEntityFactory<CNPC_PersonalityCore>::Create( const char *pClassName )
{
	CNPC_PersonalityCore *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CNPC_PersonalityCore>() at line 101
}

// game/server/portal2/npc_personality_core.cpp:100 @0x6c6970 _ZN20CNPC_PersonalityCore21InputEnableFlashlightER11inputdata_t
void CNPC_PersonalityCore::InputEnableFlashlight( inputdata_t &inputdata )
{
	variant_t emptyVariant;  // line 573
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 578
	// inlined variant_t::variant_t() at line 577
	// inlined variant_t::variant_t() at line 573
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 575
}

// game/server/portal2/npc_personality_core.cpp:101 @0x6c7b40 _ZN20CNPC_PersonalityCore22InputDisableFlashlightER11inputdata_t
void CNPC_PersonalityCore::InputDisableFlashlight( inputdata_t &inputdata )
{
	variant_t emptyVariant;  // line 587
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 592
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 588
	// inlined variant_t::variant_t() at line 587
	// inlined variant_t::variant_t() at line 590
}

// game/server/portal2/npc_personality_core.cpp:103 @0x6c5f60 _ZN20CNPC_PersonalityCore17InputEnablePickupER11inputdata_t
void CNPC_PersonalityCore::InputEnablePickup( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:104 @0x6c5f70 _ZN20CNPC_PersonalityCore18InputDisablePickupER11inputdata_t
void CNPC_PersonalityCore::InputDisablePickup( inputdata_t &inputdata )
{
}

// game/server/util.h:105 @0x6c8c50 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE7DestroyEP18IServerNetworkable
void CEntityFactory<CNPC_PersonalityCore>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/npc_personality_core.cpp:106 @0x6c63a0 _ZN20CNPC_PersonalityCore13InputPlayLockER11inputdata_t
void CNPC_PersonalityCore::InputPlayLock( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:107 @0x6c6340 _ZN20CNPC_PersonalityCore15InputPlayAttachER11inputdata_t
void CNPC_PersonalityCore::InputPlayAttach( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:108 @0x6c5f80 _ZN20CNPC_PersonalityCore15InputPlayDetachER11inputdata_t
void CNPC_PersonalityCore::InputPlayDetach( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:110 @0x6c62e0 _ZN20CNPC_PersonalityCore20InputSetIdleSequenceER11inputdata_t
void CNPC_PersonalityCore::InputSetIdleSequence( inputdata_t &inputdata )
{
	// inlined variant_t::String() at line 494
}

// game/server/portal2/npc_personality_core.cpp:111 @0x6c5fa0 _ZN20CNPC_PersonalityCore22InputClearIdleSequenceER11inputdata_t
void CNPC_PersonalityCore::InputClearIdleSequence( inputdata_t &inputdata )
{
}

// game/server/util.h:113 @0x6c8c80 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE13GetEntitySizeEv
size_t CEntityFactory<CNPC_PersonalityCore>::GetEntitySize()
{
}

// game/server/portal2/npc_personality_core.cpp:113 @0x6c7400 _ZN20CNPC_PersonalityCore12InputExplodeER11inputdata_t
void CNPC_PersonalityCore::InputExplode( inputdata_t &inputdata )
{
	CPVSFilter filter;  // line 519
	Vector gibVelocity;  // line 520
	int iModelIndex;  // line 521
	// inlined CPVSFilter::~CPVSFilter() at line 531
	{
		int i;  // line 522
		// inlined Vector::Vector() at line 524
	}
	// inlined RandomVector() at line 520
	// inlined CPVSFilter::CPVSFilter() at line 519
	// inlined CPVSFilter::~CPVSFilter() at line 531
}

// game/server/portal2/npc_personality_core.cpp:115 @0x6c67f0 _ZN20CNPC_PersonalityCore18OnTakeDamage_AliveERK15CTakeDamageInfo
int CNPC_PersonalityCore::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info;  // line 601
	bool bTookDamage;  // line 608
	// inlined CAI_Concept::CAI_Concept() at line 605
}

// game/server/portal2/npc_personality_core.cpp:116 @0x6c62d0 _ZN20CNPC_PersonalityCore14SelectScheduleEv
int CNPC_PersonalityCore::SelectSchedule()
{
}

// game/server/portal2/npc_personality_core.cpp:118 @0x6c6010 _ZN20CNPC_PersonalityCore12ShouldIgniteERK15CTakeDamageInfo
bool CNPC_PersonalityCore::ShouldIgnite( const CTakeDamageInfo &info )
{
}

// game/server/portal2/npc_personality_core.cpp:119 @0x6c6020 _ZN20CNPC_PersonalityCore19ShouldPlayIdleSoundEv
bool CNPC_PersonalityCore::ShouldPlayIdleSound()
{
}

// game/server/portal2/npc_personality_core.cpp:120 @0x6c6760 _ZN20CNPC_PersonalityCore9IdleSoundEv
void CNPC_PersonalityCore::IdleSound()
{
	// inlined CAI_Concept::CAI_Concept() at line 641
}

// game/server/portal2/npc_personality_core.cpp:121 @0x6c7370 _ZN20CNPC_PersonalityCore15HandleAnimEventEP11animevent_t
void CNPC_PersonalityCore::HandleAnimEvent( animevent_t *pEvent )
{
	// inlined CAI_BehaviorHost<CAI_BaseNPC>::HandleAnimEvent() at line 652
}

// game/server/portal2/npc_personality_core.cpp:122 @0x6c62c0 _ZN20CNPC_PersonalityCore17TranslateScheduleEi
int CNPC_PersonalityCore::TranslateSchedule( int scheduleType )
{
}

// game/server/portal2/npc_personality_core.cpp:123 @0x6c62b0 _ZN20CNPC_PersonalityCore16GatherConditionsEv
void CNPC_PersonalityCore::GatherConditions()
{
}

// game/server/portal2/npc_personality_core.cpp:124 @0x6c66b0 _ZN20CNPC_PersonalityCore15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CNPC_PersonalityCore::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	// inlined CAI_Concept::CAI_Concept() at line 701
}

// game/server/portal2/npc_personality_core.cpp:125 @0x6c65c0 _ZN20CNPC_PersonalityCore13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CNPC_PersonalityCore::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	// inlined CAI_Concept::CAI_Concept() at line 718
	// inlined CAI_Concept::CAI_Concept() at line 722
}

// game/server/portal2/npc_personality_core.cpp:126 @0x6c68f0 _ZN20CNPC_PersonalityCore9OnFizzledEv
void CNPC_PersonalityCore::OnFizzled()
{
	// inlined CAI_Concept::CAI_Concept() at line 734
}

// game/server/portal2/npc_personality_core.cpp:127 @0x6c6420 _ZN20CNPC_PersonalityCore14CreateVPhysicsEv
bool CNPC_PersonalityCore::CreateVPhysics()
{
	IPhysicsObject *pPhysicsObject;  // line 340
	// inlined CBaseEntity::RemoveSolidFlags() at line 337
	// inlined CBaseEntity::GetSolidFlags() at line 340
}

// game/server/portal2/npc_personality_core.cpp:128 @0x6c8d10 _ZN20CNPC_PersonalityCore10ObjectCapsEv
int CNPC_PersonalityCore::ObjectCaps()
{
	// inlined CAI_PlayerAlly::ObjectCaps() at line 128
}

// game/server/portal2/npc_personality_core.cpp:129 @0x6c6050 _ZN20CNPC_PersonalityCore3UseEP11CBaseEntityS1_8USE_TYPEf
void CNPC_PersonalityCore::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, $_170 useType, float value )
{
	{
		CBasePlayer *pPlayer;  // line 684
		// inlined ToBasePlayer() at line 684
	}
}

// game/server/portal2/npc_personality_core.cpp:130 @0x6c7940 _ZN20CNPC_PersonalityCore17VPhysicsCollisionEiP21gamevcollisionevent_t
void CNPC_PersonalityCore::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	CBaseEntity *pHitEntity;  // line 822
	// inlined CAI_Concept::CAI_Concept() at line 846
	// inlined CNPC_PersonalityCore::IsBeingHeldByPlayer() at line 825
	{
		Vector vecVelocity;  // line 832
		// inlined CAI_Concept::CAI_Concept() at line 835
	}
	// inlined CAI_Concept::CAI_Concept() at line 827
}

// game/server/portal2/npc_personality_core.cpp:131 @0x6c6510 _ZN20CNPC_PersonalityCore20PreferredCarryAnglesEv
QAngle CNPC_PersonalityCore::PreferredCarryAngles()
{
	// inlined ConVar::GetFloat() at line 357
	// inlined ConVar::GetFloat() at line 357
	// inlined QAngle::QAngle() at line 357
}

// game/server/portal2/npc_personality_core.cpp:132 @0x6c8c20 _ZN20CNPC_PersonalityCore32HasPreferredCarryAnglesForPlayerEP11CBasePlayer
bool CNPC_PersonalityCore::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
}

// game/server/portal2/npc_personality_core.cpp:133 @0x6c7700 _ZN20CNPC_PersonalityCore22ModifyOrAppendCriteriaERN13ResponseRules11CriteriaSetE
void CNPC_PersonalityCore::ModifyOrAppendCriteria( ResponseRules::CriteriaSet &set )
{
	// inlined CNPC_PersonalityCore::IsBeingHeldByPlayer() at line 880
	// inlined CNPC_PersonalityCore::GetPlayerSpeed() at line 882
	// inlined CNPC_PersonalityCore::GetPlayerHeldEntityName() at line 883
}

// game/server/portal2/npc_personality_core.cpp:134 @0x6c8c90 _ZN20CNPC_PersonalityCore15CreateExpresserEv
CAI_Expresser *CNPC_PersonalityCore::CreateExpresser()
{
	CAI_Expresser *expresser;  // line 136
	// inlined CAI_Expresser::Connect() at line 137
	// inlined CAI_ExpresserWithFollowup::CAI_ExpresserWithFollowup() at line 136
}

// game/server/portal2/npc_personality_core.cpp:141 @0x6c6870 _ZN20CNPC_PersonalityCore17NotifySystemEventEP11CBaseEntity21notify_system_event_tRK28notify_system_event_params_t
void CNPC_PersonalityCore::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// inlined CAI_Concept::CAI_Concept() at line 892
}

// game/server/portal2/npc_personality_core.cpp:143 @0x6c63f0 _ZN20CNPC_PersonalityCore29IsOkToSpeakInResponseToPlayerEv
bool CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer()
{
}

// game/server/portal2/npc_personality_core.cpp:145 @0x6c7170 _ZN20CNPC_PersonalityCore15StartSceneEventEP15CSceneEventInfoP12CChoreoSceneP12CChoreoEventP12CChoreoActorP11CBaseEntity
bool CNPC_PersonalityCore::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	// inlined CAI_BaseNPC::SetIdealSequence() at line 374
}

// game/server/portal2/npc_personality_core.cpp:146 @0x6c7250 _ZN20CNPC_PersonalityCore16PrescheduleThinkEv
void CNPC_PersonalityCore::PrescheduleThink()
{
	// inlined CAI_BaseNPC::SetIdealSequence() at line 396
	// inlined CBaseAnimating::SetCycle() at line 402
}

// game/server/portal2/npc_personality_core.cpp:150 @0x6c6100 _ZN20CNPC_PersonalityCore17TestRemarkingUponEP15CInfoRemarkable
bool CNPC_PersonalityCore::TestRemarkingUpon( CInfoRemarkable *pRemarkable )
{
}

// game/server/portal2/npc_personality_core.cpp:151 @0x6c6240 _ZN20CNPC_PersonalityCore19IsBeingHeldByPlayerEv
bool CNPC_PersonalityCore::IsBeingHeldByPlayer()
{
	{
		int i;  // line 744
		{
			CBasePlayer *pPlayer;  // line 746
		}
	}
}

// game/server/portal2/npc_personality_core.cpp:152 @0x6c60e0 _ZN20CNPC_PersonalityCore15GetCoreTypeNameEv
const char *CNPC_PersonalityCore::GetCoreTypeName()
{
}

// game/server/portal2/npc_personality_core.cpp:153 @0x6c6ad0 _ZN20CNPC_PersonalityCore14GetPlayerSpeedEv
float CNPC_PersonalityCore::GetPlayerSpeed()
{
	CBasePlayer *pPlayer;  // line 779
	// inlined CNPC_PersonalityCore::GetFirstPlayer() at line 779
	// inlined CBaseEntity::GetAbsVelocity() at line 782
	// inlined Vector::Length() at line 782
}

// game/server/portal2/npc_personality_core.cpp:154 @0x6c61f0 _ZN20CNPC_PersonalityCore14GetFirstPlayerEv
CBasePlayer *CNPC_PersonalityCore::GetFirstPlayer()
{
	{
		int i;  // line 763
		{
			CBasePlayer *pPlayer;  // line 765
		}
	}
}

// game/server/portal2/npc_personality_core.cpp:155 @0x6c6a60 _ZN20CNPC_PersonalityCore23GetPlayerHeldEntityNameEv
const char *CNPC_PersonalityCore::GetPlayerHeldEntityName()
{
	CBasePlayer *pPlayer;  // line 795
	// inlined CNPC_PersonalityCore::GetFirstPlayer() at line 795
	{
		CBaseEntity *heldItem;  // line 798
		// inlined CBaseEntity::GetClassname() at line 801
	}
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c8c30 _ZN20CNPC_PersonalityCore23GetClassScheduleIdSpaceEv
CAI_ClassScheduleIdSpace *CNPC_PersonalityCore::GetClassScheduleIdSpace()
{
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c8c40 _ZN20CNPC_PersonalityCore22GetSchedulingErrorNameEv
const char *CNPC_PersonalityCore::GetSchedulingErrorName()
{
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c6130 _ZN20CNPC_PersonalityCore13LoadSchedulesEv
bool CNPC_PersonalityCore::LoadSchedules()
{
	// inlined AI_DoLoadSchedules() at line 902
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c6190 _ZN20CNPC_PersonalityCore15LoadedSchedulesEv
bool CNPC_PersonalityCore::LoadedSchedules()
{
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c61b0 _ZN20CNPC_PersonalityCore13SquadSlotNameEi
const char *CNPC_PersonalityCore::SquadSlotName( int slotEN )
{
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c6b80 _ZN20CNPC_PersonalityCore15CScheduleLoaderC1Ev
CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_PersonalityCore::LoadSchedules() at line 902
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c6bd0 _ZN20CNPC_PersonalityCore15CScheduleLoaderC2Ev
CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_PersonalityCore::LoadSchedules() at line 902
}

// game/server/portal2/npc_personality_core.cpp:177 @0x6c7c20 _ZN20CNPC_PersonalityCore19InitCustomSchedulesEv
void CNPC_PersonalityCore::InitCustomSchedules()
{
	const char *pszClassName;  // line 902
	CUtlVector<char*,CUtlMemory<char*, int> > schedulesToLoad;  // line 902
	CUtlVector<bool (*)(),CUtlMemory<bool (*)(), int> > reqiredOthers;  // line 902
	CAI_NamespaceInfos scheduleIds;  // line 902
	CAI_NamespaceInfos taskIds;  // line 902
	CAI_NamespaceInfos conditionIds;  // line 902
	CAI_NamespaceInfos squadSlotIds;  // line 902
	int i;  // line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 932
	// inlined CAI_ClassScheduleIdSpace::AddSchedule() at line 932
	// inlined CAI_NamespaceInfos::Sort() at line 932
	// inlined CAI_NamespaceInfos::Sort() at line 932
	// inlined CAI_NamespaceInfos::Sort() at line 932
	// inlined CAI_NamespaceInfos::Sort() at line 932
	// inlined CAI_ClassScheduleIdSpace::Init() at line 932
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 902
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 902
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 902
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 902
	// inlined CAI_ClassScheduleIdSpace::AddTask() at line 932
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 932
	// inlined ValidateConditionLimits() at line 932
	// inlined CAI_ClassScheduleIdSpace::AddCondition() at line 932
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 932
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 932
}

// game/server/portal2/npc_personality_core.cpp:194 @0xc1f60 _Z11DataMapInitI20CNPC_PersonalityCoreEP9datamap_tPT_
datamap_t *DataMapInit<CNPC_PersonalityCore>( CNPC_PersonalityCore * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 194
	CDatadescGeneratedNameHolder nameHolder;  // line 194
	typedescription_t dataDesc[25];  // line 194
}

// game/server/portal2/npc_personality_core.cpp:194
datamap_t *g_DataMapHolder;

// game/server/portal2/npc_personality_core.cpp:231
static CEntityFactory<CNPC_PersonalityCore> npc_personality_core;

// game/server/portal2/npc_personality_core.cpp:233 @0xc1da0 _Z15ServerClassInitIN23DT_NPC_Personality_Core7ignoredEEiPT_
int ServerClassInit<DT_NPC_Personality_Core::ignored>( DT_NPC_Personality_Core::ignored * )
{
	SendTable &sendTable;  // line 233
	char *const g_pSendTableName;  // line 233
	SendProp g_SendProps[3];  // line 233
}

// game/server/portal2/npc_personality_core.cpp:233
SendTable g_SendTable;

// game/server/portal2/npc_personality_core.cpp:233
int g_SendTableInit;

// game/server/portal2/npc_personality_core.cpp:233
static ServerClass g_CNPC_PersonalityCore_ClassReg;
