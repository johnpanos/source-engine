// DWARF declaration skeleton for game/server/portal2/npc_personality_core.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xacd10 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CNPC_PersonalityCore>::CEntityFactory() at line 233
	// inlined ServerClass::ServerClass() at line 235
	// inlined CAI_ClassScheduleIdSpace::CAI_ClassScheduleIdSpace() at line 924
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
	void InputForcePickup( inputdata_t & );  // line 105
	void InputPlayLock( inputdata_t & );  // line 107
	void InputPlayAttach( inputdata_t & );  // line 108
	void InputPlayDetach( inputdata_t & );  // line 109
	void InputSetIdleSequence( inputdata_t & );  // line 111
	void InputClearIdleSequence( inputdata_t & );  // line 112
	void InputExplode( inputdata_t & );  // line 114
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo & );  // line 116
	virtual int SelectSchedule();  // line 117
	virtual bool ShouldIgnite( const CTakeDamageInfo & );  // line 119
	virtual bool ShouldPlayIdleSound();  // line 120
	virtual void IdleSound();  // line 121
	virtual void HandleAnimEvent( animevent_t * );  // line 122
	virtual int TranslateSchedule( int );  // line 123
	virtual void GatherConditions();  // line 124
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 125
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 126
	virtual void OnFizzled();  // line 127
	virtual bool CreateVPhysics();  // line 128
	virtual int ObjectCaps();  // line 129
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 130
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 131
	virtual QAngle PreferredCarryAngles();  // line 132
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 133
	virtual void ModifyOrAppendCriteria( ResponseRules::CriteriaSet & );  // line 134
	virtual CAI_Expresser *CreateExpresser();  // line 135
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 142
	virtual bool IsOkToSpeakInResponseToPlayer();  // line 144
	virtual bool StartSceneEvent( CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, CBaseEntity * );  // line 146
	virtual void PrescheduleThink();  // line 147
protected:
	virtual bool TestRemarkingUpon( CInfoRemarkable * );  // line 151
	bool IsBeingHeldByPlayer();  // line 152
	const char *GetCoreTypeName();  // line 153
	float GetPlayerSpeed();  // line 154
	CBasePlayer *GetFirstPlayer();  // line 155
	const char *GetPlayerHeldEntityName();  // line 156
	CoreType_e m_CoreType; // +0x1178  // line 158
	static const char *m_lpszCoreTypeNames[6];  // line 159
	float m_flNextIdleSoundTime; // +0x117c  // line 161
	float m_flLastPhysicsImpactTime; // +0x1180  // line 162
	bool m_bHasBeenPickedUp; // +0x1184  // line 163
	bool m_bPickupEnabled; // +0x1185  // line 164
	bool m_bAttached; // +0x1186  // line 165
	COutputEvent m_OnPlayerPickup; // +0x1188  // line 167
	COutputEvent m_OnPlayerDrop; // +0x11a0  // line 168
	float m_flAnimResetTime; // +0x11b8  // line 170
	int m_iIdleOverrideSequence; // +0x11bc  // line 172
	CNetworkVar( bool, m_bFlashlightEnabled ); // +0x11c0  // line 174
	EHANDLE m_hProjectedTexture; // +0x11c4  // line 176
public:
	// game/server/portal2/npc_personality_core.cpp:178 sizeof=0x1 (i386)
	struct CScheduleLoader
	{
	public:
		CScheduleLoader();  // line 178
	};  // line 178
protected:
	static AI_SchedLoadStatus_t gm_SchedLoadStatus;  // line 178
	static CAI_ClassScheduleIdSpace gm_ClassScheduleIdSpace;  // line 178
	static const char *gm_pszErrorClassName;  // line 178
	CNPC_PersonalityCore::CScheduleLoader m_ScheduleLoader; // +0x11c8  // line 178
	static CAI_LocalIdSpace gm_SquadSlotIdSpace;  // line 178
	static CAI_ClassScheduleIdSpace &AccessClassScheduleIdSpaceDirect();  // line 178
	virtual CAI_ClassScheduleIdSpace *GetClassScheduleIdSpace();  // line 178
	virtual const char *GetSchedulingErrorName();  // line 178
	static void InitCustomSchedules();  // line 178
	static bool LoadSchedules();  // line 178
	virtual bool LoadedSchedules();  // line 178
	virtual const char *SquadSlotName( int );  // line 178
};

// game/server/portal2/npc_personality_core.cpp:83 (declaration)
void CNPC_PersonalityCore();

// game/server/portal2/npc_personality_core.cpp:83 @0x72f020 _ZN20CNPC_PersonalityCoreC2Ev
CNPC_PersonalityCore::CNPC_PersonalityCore()
{
	// inlined CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader() at line 83
	// inlined CHandle<CBaseEntity>::CHandle() at line 83
	// inlined COutputEvent::COutputEvent() at line 83
	// inlined COutputEvent::COutputEvent() at line 83
	// inlined CAI_PlayerAlly::CAI_PlayerAlly() at line 83
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
	// inlined COutputEvent::~COutputEvent() at line 83
}

// game/server/portal2/npc_personality_core.cpp:86 @0x72baa0 _ZN20CNPC_PersonalityCore14GetDataDescMapEv
datamap_t *CNPC_PersonalityCore::GetDataDescMap()
{
}

// game/server/portal2/npc_personality_core.cpp:86 @0x72bab0 _ZN20CNPC_PersonalityCore10GetBaseMapEv
datamap_t *CNPC_PersonalityCore::GetBaseMap()
{
}

// game/server/portal2/npc_personality_core.cpp:87 @0x72bac0 _ZN20CNPC_PersonalityCore14GetServerClassEv
ServerClass *CNPC_PersonalityCore::GetServerClass()
{
}

// game/server/portal2/npc_personality_core.cpp:87 @0x72bad0 _ZN20CNPC_PersonalityCore40YouForgotToImplementOrDeclareServerClassEv
int CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/npc_personality_core.cpp:89 @0x72ea40 _ZN20CNPC_PersonalityCoreD0Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 245
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 249
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 249
}

// game/server/portal2/npc_personality_core.cpp:89 @0x72ec80 _ZN20CNPC_PersonalityCoreD2Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 245
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined COutputEvent::~COutputEvent() at line 249
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 249
	// inlined CAI_PlayerAlly::~CAI_PlayerAlly() at line 249
}

// game/server/portal2/npc_personality_core.cpp:89 @0x72eec0 _ZN20CNPC_PersonalityCoreD1Ev
CNPC_PersonalityCore::~CNPC_PersonalityCore()
{
}

// game/server/portal2/npc_personality_core.cpp:91 @0x72c250 _ZN20CNPC_PersonalityCore8PrecacheEv
void CNPC_PersonalityCore::Precache()
{
}

// game/server/portal2/npc_personality_core.cpp:92 @0x72d6f0 _ZN20CNPC_PersonalityCore5SpawnEv
void CNPC_PersonalityCore::Spawn()
{
	int iAttachmentIndex;  // line 309
	Vector vecAttachOrigin;  // line 311
	QAngle vecAttachAngles;  // line 312
	variant_t emptyVariant;  // line 315
	// inlined variant_t::variant_t() at line 329
	// inlined variant_t::variant_t() at line 315
	// inlined CHandle<CBaseEntity>::operator->() at line 329
	// inlined CHandle<CBaseEntity>::operator->() at line 327
	// inlined CHandle<CBaseEntity>::operator->() at line 326
	// inlined CHandle<CBaseEntity>::operator->() at line 325
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 323
	// inlined CBaseEntity::AddSpawnFlags() at line 321
	// inlined CHandle<CBaseEntity>::operator->() at line 321
	// inlined CHandle<CBaseEntity>::operator->() at line 320
	// inlined CHandle<CBaseEntity>::operator->() at line 319
	// inlined CHandle<CBaseEntity>::operator=() at line 317
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 306
	// inlined CBaseCombatCharacter::Spawn() at line 297
	// inlined CAI_BaseActor::EnableRemarkables() at line 293
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_takedamage>::operator=<int>() at line 286
	// inlined CBaseEntity::AddSolidFlags() at line 278
	// inlined CBaseEntity::SetSolid() at line 277
	// inlined CBaseCombatCharacter::SetHullType() at line 272
}

// game/server/portal2/npc_personality_core.cpp:93 @0x72bae0 _ZN20CNPC_PersonalityCore9StartTaskEPK6Task_t
void CNPC_PersonalityCore::StartTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_personality_core.cpp:94 @0x72baf0 _ZN20CNPC_PersonalityCore7RunTaskEPK6Task_t
void CNPC_PersonalityCore::RunTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_personality_core.cpp:96 @0x72bb60 _ZN20CNPC_PersonalityCore12EnableMotionEv
void CNPC_PersonalityCore::EnableMotion()
{
	IPhysicsObject *pPhysicsObject;  // line 564
	// inlined CBaseEntity::VPhysicsGetObject() at line 564
}

// game/server/portal2/npc_personality_core.cpp:97 @0x72cf60 _ZN20CNPC_PersonalityCore17InputEnableMotionER11inputdata_t
void CNPC_PersonalityCore::InputEnableMotion( inputdata_t &inputdata )
{
	// inlined CNPC_PersonalityCore::EnableMotion() at line 555
}

// game/server/portal2/npc_personality_core.cpp:98 @0x72c300 _ZN20CNPC_PersonalityCore18InputDisableMotionER11inputdata_t
void CNPC_PersonalityCore::InputDisableMotion( inputdata_t &inputdata )
{
	IPhysicsObject *pPhysicsObject;  // line 577
	// inlined CBaseEntity::VPhysicsGetObject() at line 577
}

// game/server/portal2/npc_personality_core.cpp:100 @0x72c340 _ZN20CNPC_PersonalityCore21InputEnableFlashlightER11inputdata_t
void CNPC_PersonalityCore::InputEnableFlashlight( inputdata_t &inputdata )
{
	variant_t emptyVariant;  // line 589
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 591
	// inlined variant_t::variant_t() at line 589
	// inlined variant_t::variant_t() at line 593
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 594
}

// game/server/util.h:100 @0x72f490 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE6CreateEPKc
IServerNetworkable *CEntityFactory<CNPC_PersonalityCore>::Create( const char *pClassName )
{
	CNPC_PersonalityCore *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CNPC_PersonalityCore>() at line 102
}

// game/server/portal2/npc_personality_core.cpp:101 @0x72c8c0 _ZN20CNPC_PersonalityCore22InputDisableFlashlightER11inputdata_t
void CNPC_PersonalityCore::InputDisableFlashlight( inputdata_t &inputdata )
{
	variant_t emptyVariant;  // line 603
	// inlined CNetworkVarBase<bool,CNPC_PersonalityCore::NetworkVar_m_bFlashlightEnabled>::operator=<bool>() at line 608
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 604
	// inlined variant_t::variant_t() at line 603
	// inlined variant_t::variant_t() at line 606
}

// game/server/portal2/npc_personality_core.cpp:103 @0x72bb00 _ZN20CNPC_PersonalityCore17InputEnablePickupER11inputdata_t
void CNPC_PersonalityCore::InputEnablePickup( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:104 @0x72bb10 _ZN20CNPC_PersonalityCore18InputDisablePickupER11inputdata_t
void CNPC_PersonalityCore::InputDisablePickup( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:105 @0x72c030 _ZN20CNPC_PersonalityCore16InputForcePickupER11inputdata_t
void CNPC_PersonalityCore::InputForcePickup( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer;  // line 453
}

// game/server/util.h:106 @0x72ef10 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE7DestroyEP18IServerNetworkable
void CEntityFactory<CNPC_PersonalityCore>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/npc_personality_core.cpp:107 @0x72bfe0 _ZN20CNPC_PersonalityCore13InputPlayLockER11inputdata_t
void CNPC_PersonalityCore::InputPlayLock( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:108 @0x72bf80 _ZN20CNPC_PersonalityCore15InputPlayAttachER11inputdata_t
void CNPC_PersonalityCore::InputPlayAttach( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:109 @0x72bb20 _ZN20CNPC_PersonalityCore15InputPlayDetachER11inputdata_t
void CNPC_PersonalityCore::InputPlayDetach( inputdata_t &inputdata )
{
}

// game/server/portal2/npc_personality_core.cpp:111 @0x72bf20 _ZN20CNPC_PersonalityCore20InputSetIdleSequenceER11inputdata_t
void CNPC_PersonalityCore::InputSetIdleSequence( inputdata_t &inputdata )
{
	// inlined variant_t::String() at line 510
}

// game/server/portal2/npc_personality_core.cpp:112 @0x72bb40 _ZN20CNPC_PersonalityCore22InputClearIdleSequenceER11inputdata_t
void CNPC_PersonalityCore::InputClearIdleSequence( inputdata_t &inputdata )
{
}

// game/server/util.h:114 @0x72ef40 _ZN14CEntityFactoryI20CNPC_PersonalityCoreE13GetEntitySizeEv
size_t CEntityFactory<CNPC_PersonalityCore>::GetEntitySize()
{
}

// game/server/portal2/npc_personality_core.cpp:114 @0x72cfb0 _ZN20CNPC_PersonalityCore12InputExplodeER11inputdata_t
void CNPC_PersonalityCore::InputExplode( inputdata_t &inputdata )
{
	CPVSFilter filter;  // line 535
	Vector gibVelocity;  // line 536
	int iModelIndex;  // line 537
	// inlined CPVSFilter::~CPVSFilter() at line 547
	{
		int i;  // line 538
		// inlined Vector::Vector() at line 540
	}
	// inlined RandomVector() at line 536
	// inlined CPVSFilter::CPVSFilter() at line 535
	// inlined CPVSFilter::~CPVSFilter() at line 547
}

// game/server/portal2/npc_personality_core.cpp:116 @0x72bdb0 _ZN20CNPC_PersonalityCore18OnTakeDamage_AliveERK15CTakeDamageInfo
int CNPC_PersonalityCore::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info;  // line 617
	bool bTookDamage;  // line 624
	// inlined CAI_Concept::CAI_Concept() at line 621
}

// game/server/portal2/npc_personality_core.cpp:117 @0x72bf10 _ZN20CNPC_PersonalityCore14SelectScheduleEv
int CNPC_PersonalityCore::SelectSchedule()
{
}

// game/server/portal2/npc_personality_core.cpp:119 @0x72bbb0 _ZN20CNPC_PersonalityCore12ShouldIgniteERK15CTakeDamageInfo
bool CNPC_PersonalityCore::ShouldIgnite( const CTakeDamageInfo &info )
{
}

// game/server/portal2/npc_personality_core.cpp:120 @0x72bbc0 _ZN20CNPC_PersonalityCore19ShouldPlayIdleSoundEv
bool CNPC_PersonalityCore::ShouldPlayIdleSound()
{
}

// game/server/portal2/npc_personality_core.cpp:121 @0x72c640 _ZN20CNPC_PersonalityCore9IdleSoundEv
void CNPC_PersonalityCore::IdleSound()
{
	// inlined CAI_Concept::CAI_Concept() at line 663
}

// game/server/portal2/npc_personality_core.cpp:122 @0x72cf20 _ZN20CNPC_PersonalityCore15HandleAnimEventEP11animevent_t
void CNPC_PersonalityCore::HandleAnimEvent( animevent_t *pEvent )
{
	// inlined CAI_BehaviorHost<CAI_BaseNPC>::HandleAnimEvent() at line 674
}

// game/server/portal2/npc_personality_core.cpp:123 @0x72bf00 _ZN20CNPC_PersonalityCore17TranslateScheduleEi
int CNPC_PersonalityCore::TranslateSchedule( int scheduleType )
{
}

// game/server/portal2/npc_personality_core.cpp:124 @0x72bef0 _ZN20CNPC_PersonalityCore16GatherConditionsEv
void CNPC_PersonalityCore::GatherConditions()
{
}

// game/server/portal2/npc_personality_core.cpp:125 @0x72c590 _ZN20CNPC_PersonalityCore15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CNPC_PersonalityCore::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	// inlined CAI_Concept::CAI_Concept() at line 723
}

// game/server/portal2/npc_personality_core.cpp:126 @0x72c7e0 _ZN20CNPC_PersonalityCore13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CNPC_PersonalityCore::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	// inlined CAI_Concept::CAI_Concept() at line 740
	// inlined CAI_Concept::CAI_Concept() at line 744
}

// game/server/portal2/npc_personality_core.cpp:127 @0x72c750 _ZN20CNPC_PersonalityCore9OnFizzledEv
void CNPC_PersonalityCore::OnFizzled()
{
	// inlined CAI_Concept::CAI_Concept() at line 756
}

// game/server/portal2/npc_personality_core.cpp:128 @0x72c1b0 _ZN20CNPC_PersonalityCore14CreateVPhysicsEv
bool CNPC_PersonalityCore::CreateVPhysics()
{
	IPhysicsObject *pPhysicsObject;  // line 342
	// inlined CBaseEntity::RemoveSolidFlags() at line 339
	// inlined CBaseEntity::GetSolidFlags() at line 342
}

// game/server/portal2/npc_personality_core.cpp:129 @0x72efd0 _ZN20CNPC_PersonalityCore10ObjectCapsEv
int CNPC_PersonalityCore::ObjectCaps()
{
	// inlined CAI_PlayerAlly::ObjectCaps() at line 129
}

// game/server/portal2/npc_personality_core.cpp:130 @0x72bc10 _ZN20CNPC_PersonalityCore3UseEP11CBaseEntityS1_8USE_TYPEf
void CNPC_PersonalityCore::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, $_170 useType, float value )
{
	{
		CBasePlayer *pPlayer;  // line 706
		// inlined ToBasePlayer() at line 706
	}
}

// game/server/portal2/npc_personality_core.cpp:131 @0x72d4f0 _ZN20CNPC_PersonalityCore17VPhysicsCollisionEiP21gamevcollisionevent_t
void CNPC_PersonalityCore::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	CBaseEntity *pHitEntity;  // line 844
	// inlined CAI_Concept::CAI_Concept() at line 868
	// inlined CNPC_PersonalityCore::IsBeingHeldByPlayer() at line 847
	{
		Vector vecVelocity;  // line 854
		// inlined CAI_Concept::CAI_Concept() at line 857
	}
	// inlined CAI_Concept::CAI_Concept() at line 849
}

// game/server/portal2/npc_personality_core.cpp:132 @0x72c2a0 _ZN20CNPC_PersonalityCore20PreferredCarryAnglesEv
QAngle CNPC_PersonalityCore::PreferredCarryAngles()
{
	// inlined ConVar::GetFloat() at line 359
	// inlined ConVar::GetFloat() at line 359
	// inlined QAngle::QAngle() at line 359
}

// game/server/portal2/npc_personality_core.cpp:133 @0x72eee0 _ZN20CNPC_PersonalityCore32HasPreferredCarryAnglesForPlayerEP11CBasePlayer
bool CNPC_PersonalityCore::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
}

// game/server/portal2/npc_personality_core.cpp:134 @0x72d2b0 _ZN20CNPC_PersonalityCore22ModifyOrAppendCriteriaERN13ResponseRules11CriteriaSetE
void CNPC_PersonalityCore::ModifyOrAppendCriteria( ResponseRules::CriteriaSet &set )
{
	// inlined CNPC_PersonalityCore::IsBeingHeldByPlayer() at line 902
	// inlined CNPC_PersonalityCore::GetPlayerSpeed() at line 904
	// inlined CNPC_PersonalityCore::GetPlayerHeldEntityName() at line 905
}

// game/server/portal2/npc_personality_core.cpp:135 @0x72ef50 _ZN20CNPC_PersonalityCore15CreateExpresserEv
CAI_Expresser *CNPC_PersonalityCore::CreateExpresser()
{
	CAI_Expresser *expresser;  // line 137
	// inlined CAI_Expresser::Connect() at line 138
	// inlined CAI_ExpresserWithFollowup::CAI_ExpresserWithFollowup() at line 137
}

// game/server/portal2/npc_personality_core.cpp:142 @0x72c6d0 _ZN20CNPC_PersonalityCore17NotifySystemEventEP11CBaseEntity21notify_system_event_tRK28notify_system_event_params_t
void CNPC_PersonalityCore::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// inlined CAI_Concept::CAI_Concept() at line 914
}

// game/server/portal2/npc_personality_core.cpp:144 @0x72c0a0 _ZN20CNPC_PersonalityCore29IsOkToSpeakInResponseToPlayerEv
bool CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer()
{
}

// game/server/portal2/npc_personality_core.cpp:146 @0x72c0d0 _ZN20CNPC_PersonalityCore15StartSceneEventEP15CSceneEventInfoP12CChoreoSceneP12CChoreoEventP12CChoreoActorP11CBaseEntity
bool CNPC_PersonalityCore::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	// inlined CAI_BaseNPC::SetIdealSequence() at line 376
}

// game/server/portal2/npc_personality_core.cpp:147 @0x72ccd0 _ZN20CNPC_PersonalityCore16PrescheduleThinkEv
void CNPC_PersonalityCore::PrescheduleThink()
{
	// inlined CAI_BaseNPC::SetIdealSequence() at line 398
	// inlined CBaseAnimating::SetCycle() at line 404
}

// game/server/portal2/npc_personality_core.cpp:151 @0x72bcc0 _ZN20CNPC_PersonalityCore17TestRemarkingUponEP15CInfoRemarkable
bool CNPC_PersonalityCore::TestRemarkingUpon( CInfoRemarkable *pRemarkable )
{
}

// game/server/portal2/npc_personality_core.cpp:152 @0x72be80 _ZN20CNPC_PersonalityCore19IsBeingHeldByPlayerEv
bool CNPC_PersonalityCore::IsBeingHeldByPlayer()
{
	{
		int i;  // line 766
		{
			CBasePlayer *pPlayer;  // line 768
		}
	}
}

// game/server/portal2/npc_personality_core.cpp:153 @0x72bca0 _ZN20CNPC_PersonalityCore15GetCoreTypeNameEv
const char *CNPC_PersonalityCore::GetCoreTypeName()
{
}

// game/server/portal2/npc_personality_core.cpp:154 @0x72cb80 _ZN20CNPC_PersonalityCore14GetPlayerSpeedEv
float CNPC_PersonalityCore::GetPlayerSpeed()
{
	CBasePlayer *pPlayer;  // line 801
	// inlined CNPC_PersonalityCore::GetFirstPlayer() at line 801
	// inlined CBaseEntity::GetAbsVelocity() at line 804
	// inlined Vector::Length() at line 804
}

// game/server/portal2/npc_personality_core.cpp:155 @0x72be30 _ZN20CNPC_PersonalityCore14GetFirstPlayerEv
CBasePlayer *CNPC_PersonalityCore::GetFirstPlayer()
{
	{
		int i;  // line 785
		{
			CBasePlayer *pPlayer;  // line 787
		}
	}
}

// game/server/portal2/npc_personality_core.cpp:156 @0x72cb10 _ZN20CNPC_PersonalityCore23GetPlayerHeldEntityNameEv
const char *CNPC_PersonalityCore::GetPlayerHeldEntityName()
{
	CBasePlayer *pPlayer;  // line 817
	// inlined CNPC_PersonalityCore::GetFirstPlayer() at line 817
	{
		CBaseEntity *heldItem;  // line 820
		// inlined CBaseEntity::GetClassname() at line 823
	}
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72eef0 _ZN20CNPC_PersonalityCore23GetClassScheduleIdSpaceEv
CAI_ClassScheduleIdSpace *CNPC_PersonalityCore::GetClassScheduleIdSpace()
{
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72ef00 _ZN20CNPC_PersonalityCore22GetSchedulingErrorNameEv
const char *CNPC_PersonalityCore::GetSchedulingErrorName()
{
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72bcf0 _ZN20CNPC_PersonalityCore13LoadSchedulesEv
bool CNPC_PersonalityCore::LoadSchedules()
{
	// inlined AI_DoLoadSchedules() at line 924
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72bd50 _ZN20CNPC_PersonalityCore15LoadedSchedulesEv
bool CNPC_PersonalityCore::LoadedSchedules()
{
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72bd70 _ZN20CNPC_PersonalityCore13SquadSlotNameEi
const char *CNPC_PersonalityCore::SquadSlotName( int slotEN )
{
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72cc30 _ZN20CNPC_PersonalityCore15CScheduleLoaderC1Ev
CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_PersonalityCore::LoadSchedules() at line 924
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72cc80 _ZN20CNPC_PersonalityCore15CScheduleLoaderC2Ev
CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_PersonalityCore::LoadSchedules() at line 924
}

// game/server/portal2/npc_personality_core.cpp:178 @0x72df50 _ZN20CNPC_PersonalityCore19InitCustomSchedulesEv
void CNPC_PersonalityCore::InitCustomSchedules()
{
	const char *pszClassName;  // line 924
	CUtlVector<char*,CUtlMemory<char*, int> > schedulesToLoad;  // line 924
	CUtlVector<bool (*)(),CUtlMemory<bool (*)(), int> > reqiredOthers;  // line 924
	CAI_NamespaceInfos scheduleIds;  // line 924
	CAI_NamespaceInfos taskIds;  // line 924
	CAI_NamespaceInfos conditionIds;  // line 924
	CAI_NamespaceInfos squadSlotIds;  // line 924
	int i;  // line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 954
	// inlined CAI_ClassScheduleIdSpace::AddSchedule() at line 954
	// inlined CAI_NamespaceInfos::Sort() at line 954
	// inlined CAI_NamespaceInfos::Sort() at line 954
	// inlined CAI_NamespaceInfos::Sort() at line 954
	// inlined CAI_NamespaceInfos::Sort() at line 954
	// inlined CAI_ClassScheduleIdSpace::Init() at line 954
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 924
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 924
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 924
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 924
	// inlined CAI_ClassScheduleIdSpace::AddTask() at line 954
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 954
	// inlined ValidateConditionLimits() at line 954
	// inlined CAI_ClassScheduleIdSpace::AddCondition() at line 954
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 954
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 954
}

// game/server/portal2/npc_personality_core.cpp:195 @0xacbe0 _Z11DataMapInitI20CNPC_PersonalityCoreEP9datamap_tPT_
datamap_t *DataMapInit<CNPC_PersonalityCore>( CNPC_PersonalityCore * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 195
	CDatadescGeneratedNameHolder nameHolder;  // line 195
	typedescription_t dataDesc[26];  // line 195
}

// game/server/portal2/npc_personality_core.cpp:195
datamap_t *g_DataMapHolder;

// game/server/portal2/npc_personality_core.cpp:233
static CEntityFactory<CNPC_PersonalityCore> npc_personality_core;

// game/server/portal2/npc_personality_core.cpp:235 @0xaca20 _Z15ServerClassInitIN23DT_NPC_Personality_Core7ignoredEEiPT_
int ServerClassInit<DT_NPC_Personality_Core::ignored>( DT_NPC_Personality_Core::ignored * )
{
	SendTable &sendTable;  // line 235
	char *const g_pSendTableName;  // line 235
	SendProp g_SendProps[3];  // line 235
}

// game/server/portal2/npc_personality_core.cpp:235
SendTable g_SendTable;

// game/server/portal2/npc_personality_core.cpp:235
int g_SendTableInit;

// game/server/portal2/npc_personality_core.cpp:235
static ServerClass g_CNPC_PersonalityCore_ClassReg;
