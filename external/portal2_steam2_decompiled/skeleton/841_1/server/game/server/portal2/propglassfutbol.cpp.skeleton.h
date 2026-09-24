// DWARF declaration skeleton for game/server/portal2/propglassfutbol.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb3310 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CFutbolCatcher>::CEntityFactory() at line 86
	// inlined CEntityFactory<CPropFutbolSpawner>::CEntityFactory() at line 132
	// inlined CEntityFactory<CPropGlassFutbol>::CEntityFactory() at line 208
	// inlined CEntityFactory<CPropFutbolSocket>::CEntityFactory() at line 237
}

// game/server/portal2/propglassfutbol.cpp:18
ConVar sv_futbol_fake_force;

// game/server/portal2/propglassfutbol.cpp:19
ConVar sv_futbol_force_players_to_catch;

// game/server/portal2/propglassfutbol.cpp:20
ConVar sv_futbol_use_cooldown_time;

// game/server/portal2/propglassfutbol.cpp:21
ConVar sv_futbol_use_steals_from_holding_player;

// game/server/portal2/propglassfutbol.cpp:33
char *g_szFutbolAnimThinkContext;

// game/server/portal2/propglassfutbol.cpp:34
char *g_szFutbolThrownThinkContext;

// game/server/portal2/propglassfutbol.cpp:40 sizeof=0x4 (i386)
struct CFilterOnlyGlassFutbol : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *, int );  // line 43
	virtual TraceType_t GetTraceType() const;  // line 48
};

// game/server/portal2/propglassfutbol.cpp:40 (declaration)
void CFilterOnlyGlassFutbol();

// game/server/portal2/propglassfutbol.cpp:43 @0x74ed70 _ZN22CFilterOnlyGlassFutbol15ShouldHitEntityEP13IHandleEntityi
bool CFilterOnlyGlassFutbol::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	CBaseEntity *pEntity;  // line 45
	// inlined FClassnameIs() at line 46
	// inlined EntityFromEntityHandle() at line 45
}

// game/server/portal2/propglassfutbol.cpp:48 @0x74eb30 _ZNK22CFilterOnlyGlassFutbol12GetTraceTypeEv
TraceType_t CFilterOnlyGlassFutbol::GetTraceType()
{
}

// game/server/portal2/propglassfutbol.cpp:56 sizeof=0x57c (i386)
struct CFutbolCatcher : public CBaseAnimating
{
public:
	CFutbolCatcher();  // line 61
	virtual void Spawn();  // line 62
protected:
	EHANDLE m_hCaughtFutbol; // +0x514  // line 65
	Vector m_vCatcherBoxHalfDiagonal; // +0x518  // line 67
	Vector m_vecCatchBoxMins; // +0x524  // line 70
	Vector m_vecCatchBoxMaxs; // +0x530  // line 71
	Vector m_vecCatchBoxOrig; // +0x53c  // line 72
	bool m_bDisableRecaptureOnPlayerGrab; // +0x548  // line 74
	COutputEvent m_OnFutbolReleased; // +0x54c  // line 76
	COutputEvent m_OnFutbolCaught; // +0x564  // line 77
	virtual void CatchThink();  // line 79
	virtual void CaptureThink();  // line 80
	virtual void CaptureFutbol( CPropGlassFutbol * );  // line 82
};

// game/server/portal2/propglassfutbol.cpp:56 (declaration)
~CFutbolCatcher();

// game/server/portal2/propglassfutbol.cpp:56 @0x74edf0 _ZN14CFutbolCatcherD0Ev
CFutbolCatcher::~CFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 56
	// inlined COutputEvent::~COutputEvent() at line 56
	// inlined COutputEvent::~COutputEvent() at line 56
}

// game/server/portal2/propglassfutbol.cpp:56 @0x74ee70 _ZN14CFutbolCatcherD1Ev
CFutbolCatcher::~CFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 56
	// inlined COutputEvent::~COutputEvent() at line 56
	// inlined COutputEvent::~COutputEvent() at line 56
}

// game/server/portal2/propglassfutbol.cpp:59 @0x74d560 _ZN14CFutbolCatcher14GetDataDescMapEv
datamap_t *CFutbolCatcher::GetDataDescMap()
{
}

// game/server/portal2/propglassfutbol.cpp:59 @0x74d570 _ZN14CFutbolCatcher10GetBaseMapEv
datamap_t *CFutbolCatcher::GetBaseMap()
{
}

// game/server/portal2/propglassfutbol.cpp:61 @0x74e8e0 _ZN14CFutbolCatcherC2Ev
CFutbolCatcher::CFutbolCatcher()
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 253
	// inlined Vector::Vector() at line 253
	// inlined COutputEvent::COutputEvent() at line 253
	// inlined COutputEvent::COutputEvent() at line 253
}

// game/server/portal2/propglassfutbol.cpp:61 @0x74eae0 _ZN14CFutbolCatcherC1Ev
CFutbolCatcher::CFutbolCatcher()
{
}

// game/server/portal2/propglassfutbol.cpp:62 @0x74d760 _ZN14CFutbolCatcher5SpawnEv
void CFutbolCatcher::Spawn()
{
	// inlined Vector::operator-() at line 265
	// inlined Vector::operator=() at line 265
	// inlined Vector::operator=() at line 266
}

// game/server/portal2/propglassfutbol.cpp:79 @0x74d910 _ZN14CFutbolCatcher10CatchThinkEv
void CFutbolCatcher::CatchThink()
{
	Ray_t ray;  // line 278
	CFilterOnlyGlassFutbol filter;  // line 280
	trace_t tr;  // line 281
	// inlined UTIL_TraceRay() at line 282
	// inlined CFilterOnlyGlassFutbol::CFilterOnlyGlassFutbol() at line 280
	// inlined Ray_t::Init() at line 279
	{
		CPropGlassFutbol *pFutbol;  // line 286
		// inlined CPropGlassFutbol::GetHolder() at line 290
	}
}

// game/server/portal2/propglassfutbol.cpp:80 @0x74df30 _ZN14CFutbolCatcher12CaptureThinkEv
void CFutbolCatcher::CaptureThink()
{
	CPropGlassFutbol *pFutbol;  // line 334
	// inlined CHandle<CBaseEntity>::Set() at line 342
	// inlined CHandle<CBaseEntity>::Get() at line 334
}

// game/server/portal2/propglassfutbol.cpp:82 @0x74e3a0 _ZN14CFutbolCatcher13CaptureFutbolEP16CPropGlassFutbol
void CFutbolCatcher::CaptureFutbol( CPropGlassFutbol *pFutbol )
{
	IPhysicsObject *pPhysicsObject;  // line 316
	// inlined CPropGlassFutbol::GetLastPlayerToHold() at line 326
	// inlined CPropGlassFutbol::SetHolder() at line 325
	// inlined CBaseEntity::VPhysicsGetObject() at line 316
	// inlined CHandle<CBaseEntity>::Set() at line 313
}

// game/server/portal2/propglassfutbol.cpp:86
static CEntityFactory<CFutbolCatcher> futbol_catcher;

// game/server/portal2/propglassfutbol.cpp:88 @0xb3010 _Z11DataMapInitI14CFutbolCatcherEP9datamap_tPT_
datamap_t *DataMapInit<CFutbolCatcher>( CFutbolCatcher * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 103
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 103
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 88
	CDatadescGeneratedNameHolder nameHolder;  // line 88
	typedescription_t dataDesc[10];  // line 88
}

// game/server/portal2/propglassfutbol.cpp:88
datamap_t *g_DataMapHolder;

// game/server/util.h:100 @0x74eee0 _ZN14CEntityFactoryI16CPropGlassFutbolE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropGlassFutbol>::Create( const char *pClassName )
{
	CPropGlassFutbol *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropGlassFutbol>() at line 102
}

// game/server/util.h:100 @0x74ef30 _ZN14CEntityFactoryI14CFutbolCatcherE6CreateEPKc
IServerNetworkable *CEntityFactory<CFutbolCatcher>::Create( const char *pClassName )
{
	CFutbolCatcher *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CFutbolCatcher>() at line 102
}

// game/server/util.h:100 @0x74ef80 _ZN14CEntityFactoryI18CPropFutbolSpawnerE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropFutbolSpawner>::Create( const char *pClassName )
{
	CPropFutbolSpawner *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropFutbolSpawner>() at line 102
}

// game/server/util.h:100 @0x74efd0 _ZN14CEntityFactoryI17CPropFutbolSocketE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropFutbolSocket>::Create( const char *pClassName )
{
	CPropFutbolSocket *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropFutbolSocket>() at line 102
}

// game/server/util.h:106 @0x74eba0 _ZN14CEntityFactoryI17CPropFutbolSocketE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropFutbolSocket>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x74ebe0 _ZN14CEntityFactoryI16CPropGlassFutbolE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropGlassFutbol>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x74ec20 _ZN14CEntityFactoryI18CPropFutbolSpawnerE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropFutbolSpawner>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x74ec60 _ZN14CEntityFactoryI14CFutbolCatcherE7DestroyEP18IServerNetworkable
void CEntityFactory<CFutbolCatcher>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/propglassfutbol.cpp:108 sizeof=0x5b0 (i386)
struct CPropFutbolSpawner : public CFutbolCatcher
{
public:
	CPropFutbolSpawner();  // line 113
	virtual void Spawn();  // line 115
	virtual void Precache();  // line 116
	virtual void FutbolDestroyed();  // line 118
	virtual void FutbolGrabbed();  // line 119
private:
	bool m_bHasFutbol; // +0x57c  // line 122
	COutputEvent m_OnFutbolSpawned; // +0x580  // line 124
	COutputEvent m_OnFutbolGrabbed; // +0x598  // line 125
	void InputForceSpawn( inputdata_t & );  // line 127
	virtual void SpawnFutbol();  // line 129
};

// game/server/portal2/propglassfutbol.cpp:108 (declaration)
~CPropFutbolSpawner();

// game/server/portal2/propglassfutbol.cpp:108 @0x74f130 _ZN18CPropFutbolSpawnerD0Ev
CPropFutbolSpawner::~CPropFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 108
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 108
}

// game/server/portal2/propglassfutbol.cpp:108 @0x74f220 _ZN18CPropFutbolSpawnerD1Ev
CPropFutbolSpawner::~CPropFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 108
	// inlined COutputEvent::~COutputEvent() at line 108
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 108
}

// game/server/portal2/propglassfutbol.cpp:111 @0x74d580 _ZN18CPropFutbolSpawner14GetDataDescMapEv
datamap_t *CPropFutbolSpawner::GetDataDescMap()
{
}

// game/server/portal2/propglassfutbol.cpp:111 @0x74d590 _ZN18CPropFutbolSpawner10GetBaseMapEv
datamap_t *CPropFutbolSpawner::GetBaseMap()
{
}

// game/server/portal2/propglassfutbol.cpp:113 @0x74ea40 _ZN18CPropFutbolSpawnerC2Ev
CPropFutbolSpawner::CPropFutbolSpawner()
{
	// inlined COutputEvent::COutputEvent() at line 604
	// inlined COutputEvent::COutputEvent() at line 604
	// inlined Vector::Vector() at line 608
	// inlined Vector::operator=() at line 608
}

// game/server/portal2/propglassfutbol.cpp:113 @0x74ead0 _ZN18CPropFutbolSpawnerC1Ev
CPropFutbolSpawner::CPropFutbolSpawner()
{
}

// game/server/util.h:114 @0x74ebd0 _ZN14CEntityFactoryI17CPropFutbolSocketE13GetEntitySizeEv
size_t CEntityFactory<CPropFutbolSocket>::GetEntitySize()
{
}

// game/server/util.h:114 @0x74ec10 _ZN14CEntityFactoryI16CPropGlassFutbolE13GetEntitySizeEv
size_t CEntityFactory<CPropGlassFutbol>::GetEntitySize()
{
}

// game/server/util.h:114 @0x74ec50 _ZN14CEntityFactoryI18CPropFutbolSpawnerE13GetEntitySizeEv
size_t CEntityFactory<CPropFutbolSpawner>::GetEntitySize()
{
}

// game/server/util.h:114 @0x74ec90 _ZN14CEntityFactoryI14CFutbolCatcherE13GetEntitySizeEv
size_t CEntityFactory<CFutbolCatcher>::GetEntitySize()
{
}

// game/server/portal2/propglassfutbol.cpp:115 @0x74d890 _ZN18CPropFutbolSpawner5SpawnEv
void CPropFutbolSpawner::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 621
}

// game/server/portal2/propglassfutbol.cpp:116 @0x74d640 _ZN18CPropFutbolSpawner8PrecacheEv
void CPropFutbolSpawner::Precache()
{
}

// game/server/portal2/propglassfutbol.cpp:118 @0x74d5e0 _ZN18CPropFutbolSpawner15FutbolDestroyedEv
void CPropFutbolSpawner::FutbolDestroyed()
{
}

// game/server/portal2/propglassfutbol.cpp:119 @0x74d660 _ZN18CPropFutbolSpawner13FutbolGrabbedEv
void CPropFutbolSpawner::FutbolGrabbed()
{
}

// game/server/portal2/propglassfutbol.cpp:127 @0x74d600 _ZN18CPropFutbolSpawner15InputForceSpawnER11inputdata_t
void CPropFutbolSpawner::InputForceSpawn( inputdata_t &data )
{
}

// game/server/portal2/propglassfutbol.cpp:129 @0x74dce0 _ZN18CPropFutbolSpawner11SpawnFutbolEv
void CPropFutbolSpawner::SpawnFutbol()
{
	CPropGlassFutbol *pBall;  // line 647
	Vector vecBallSpawnPoint;  // line 658
	// inlined CPropGlassFutbol::SetHolder() at line 662
}

// game/server/portal2/propglassfutbol.cpp:132
static CEntityFactory<CPropFutbolSpawner> prop_glass_futbol_spawner;

// game/server/portal2/propglassfutbol.cpp:135 @0xb2bf0 _Z11DataMapInitI18CPropFutbolSpawnerEP9datamap_tPT_
datamap_t *DataMapInit<CPropFutbolSpawner>( CPropFutbolSpawner * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 135
	CDatadescGeneratedNameHolder nameHolder;  // line 135
	typedescription_t dataDesc[5];  // line 135
}

// game/server/portal2/propglassfutbol.cpp:135
datamap_t *g_DataMapHolder;

// game/server/portal2/propglassfutbol.cpp:147
enum futbol_holder_type_t
{
	FUTBOL_HELD_BY_NONE = 0,
	FUTBOL_HELD_BY_PLAYER = 1,
	FUTBOL_HELD_BY_SPAWNER = 2,
	FUTBOL_HELD_BY_CATCHER = 3,
	FUTBOL_HELD_BY_COUNT = 4,
};

// game/server/portal2/propglassfutbol.cpp:160 sizeof=0x784 (i386)
struct CPropGlassFutbol : public CPhysicsProp
{
public:
	CPropGlassFutbol();  // line 165
	virtual QAngle PreferredCarryAngles();  // line 167
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 168
	virtual void Precache();  // line 170
	virtual void Spawn();  // line 171
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 172
	void ThrownThink();  // line 175
	void AnimThink();  // line 178
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 181
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 184
	virtual void SetSpawner( CPropFutbolSpawner * );  // line 187
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 190
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 192
	futbol_holder_type_t GetHolder();  // line 195
	void SetHolder( futbol_holder_type_t );  // line 196
	CPortal_Player *GetLastPlayerToHold();  // line 198
private:
	CHandle<CPropFutbolSpawner> m_hSpawner; // +0x768  // line 201
	string_t m_strSpawnerName; // +0x76c  // line 202
	futbol_holder_type_t m_Holder; // +0x770  // line 203
	Vector m_vecThrowDirection; // +0x774  // line 204
	CHandle<CPortal_Player> m_hLastHeldByPlayer; // +0x780  // line 205
};

// game/server/portal2/propglassfutbol.cpp:160 (declaration)
~CPropGlassFutbol();

// game/server/portal2/propglassfutbol.cpp:160 @0x74eca0 _ZN16CPropGlassFutbolD0Ev
CPropGlassFutbol::~CPropGlassFutbol()
{
}

// game/server/portal2/propglassfutbol.cpp:160 @0x74ed10 _ZN16CPropGlassFutbolD1Ev
CPropGlassFutbol::~CPropGlassFutbol()
{
}

// game/server/portal2/propglassfutbol.cpp:163 @0x74d5a0 _ZN16CPropGlassFutbol14GetDataDescMapEv
datamap_t *CPropGlassFutbol::GetDataDescMap()
{
}

// game/server/portal2/propglassfutbol.cpp:163 @0x74d5b0 _ZN16CPropGlassFutbol10GetBaseMapEv
datamap_t *CPropGlassFutbol::GetBaseMap()
{
}

// game/server/portal2/propglassfutbol.cpp:165 @0x74e820 _ZN16CPropGlassFutbolC2Ev
CPropGlassFutbol::CPropGlassFutbol()
{
	// inlined CHandle<CPropFutbolSpawner>::CHandle() at line 363
	// inlined CHandle<CPortal_Player>::CHandle() at line 363
}

// game/server/portal2/propglassfutbol.cpp:165 @0x74e8d0 _ZN16CPropGlassFutbolC1Ev
CPropGlassFutbol::CPropGlassFutbol()
{
}

// game/server/portal2/propglassfutbol.cpp:167 @0x74eb50 _ZN16CPropGlassFutbol20PreferredCarryAnglesEv
QAngle CPropGlassFutbol::PreferredCarryAngles()
{
	// inlined QAngle::QAngle() at line 167
}

// game/server/portal2/propglassfutbol.cpp:168 @0x74eb90 _ZN16CPropGlassFutbol32HasPreferredCarryAnglesForPlayerEP11CBasePlayer
bool CPropGlassFutbol::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
}

// game/server/portal2/propglassfutbol.cpp:170 @0x74d730 _ZN16CPropGlassFutbol8PrecacheEv
void CPropGlassFutbol::Precache()
{
}

// game/server/portal2/propglassfutbol.cpp:171 @0x74e4c0 _ZN16CPropGlassFutbol5SpawnEv
void CPropGlassFutbol::Spawn()
{
	// inlined CBaseEntity::AddSpawnFlags() at line 429
	{
		CBaseEntity *pEnt;  // line 392
		// inlined string_t::operator!=() at line 393
		{
			CPropFutbolSpawner *pSpawner;  // line 399
			// inlined CHandle<CPropFutbolSpawner>::Set() at line 402
			// inlined CBaseEntity::GetClassname() at line 406
		}
	}
	// inlined CHandle<CPropFutbolSpawner>::Get() at line 390
}

// game/server/portal2/propglassfutbol.cpp:172 @0x74e010 _ZN16CPropGlassFutbol12Event_KilledERK15CTakeDamageInfo
void CPropGlassFutbol::Event_Killed( const CTakeDamageInfo &info )
{
	// inlined CTakeDamageInfo::GetAttacker() at line 434
	// inlined CHandle<CPropFutbolSpawner>::Get() at line 444
	// inlined FClassnameIs() at line 434
}

// game/server/portal2/propglassfutbol.cpp:175 @0x74ddd0 _ZN16CPropGlassFutbol11ThrownThinkEv
void CPropGlassFutbol::ThrownThink()
{
	Vector vecForce;  // line 572
	IPhysicsObject *pPhys;  // line 574
	// inlined ConVar::GetFloat() at line 572
	// inlined Vector::operator*() at line 572
	// inlined CBaseEntity::VPhysicsGetObject() at line 574
}

// game/server/portal2/propglassfutbol.cpp:178 @0x74d6a0 _ZN16CPropGlassFutbol9AnimThinkEv
void CPropGlassFutbol::AnimThink()
{
}

// game/server/portal2/propglassfutbol.cpp:181 @0x74e100 _ZN16CPropGlassFutbol13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CPropGlassFutbol::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	Vector forward;  // line 504
	// inlined Vector::operator=() at line 516
	// inlined ToPortalPlayer() at line 508
	{
		CPortal_Base2D *pPortal;  // line 510
		// inlined ToPortalPlayer() at line 510
		// inlined CPortal_Player::GetHeldObjectPortal() at line 510
	}
}

// game/server/portal2/propglassfutbol.cpp:184 @0x74e240 _ZN16CPropGlassFutbol15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CPropGlassFutbol::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CPortal_Player *pHoldingPlayer;  // line 456
	IPhysicsObject *pPhysicsObject;  // line 460
	// inlined CHandle<CPropFutbolSpawner>::Get() at line 476
	// inlined CBaseEntity::VPhysicsGetObject() at line 460
	// inlined CHandle<CPortal_Player>::operator=() at line 458
	{
		CBasePlayer *pOtherPlayer;  // line 483
	}
}

// game/server/portal2/propglassfutbol.cpp:187 @0x74eaf0 _ZN16CPropGlassFutbol10SetSpawnerEP18CPropFutbolSpawner
void CPropGlassFutbol::SetSpawner( CPropFutbolSpawner *pMySpawner )
{
	// inlined CHandle<CPropFutbolSpawner>::Set() at line 526
}

// game/server/portal2/propglassfutbol.cpp:190 @0x74d700 _ZN16CPropGlassFutbol12OnTakeDamageERK15CTakeDamageInfo
int CPropGlassFutbol::OnTakeDamage( const CTakeDamageInfo &info )
{
}

// game/server/portal2/propglassfutbol.cpp:192 @0x74dbd0 _ZN16CPropGlassFutbol17VPhysicsCollisionEiP21gamevcollisionevent_t
void CPropGlassFutbol::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	{
		CBaseEntity *pHitEntity;  // line 546
		{
			CPortal_Player *pPlayer;  // line 549
		}
	}
}

// game/server/portal2/propglassfutbol.cpp:196 @0x74d5c0 _ZN16CPropGlassFutbol9SetHolderE20futbol_holder_type_t
void CPropGlassFutbol::SetHolder( futbol_holder_type_t type )
{
}

// game/server/portal2/propglassfutbol.cpp:208
static CEntityFactory<CPropGlassFutbol> prop_glass_futbol;

// game/server/portal2/propglassfutbol.cpp:211 @0xb2d20 _Z11DataMapInitI16CPropGlassFutbolEP9datamap_tPT_
datamap_t *DataMapInit<CPropGlassFutbol>( CPropGlassFutbol * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 224
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 224
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 211
	CDatadescGeneratedNameHolder nameHolder;  // line 211
	typedescription_t dataDesc[8];  // line 211
}

// game/server/portal2/propglassfutbol.cpp:211
datamap_t *g_DataMapHolder;

// game/server/portal2/propglassfutbol.cpp:228 sizeof=0x57c (i386)
struct CPropFutbolSocket : public CFutbolCatcher
{
public:
	CPropFutbolSocket();  // line 231
	virtual void Spawn();  // line 233
	virtual void Precache();  // line 234
};

// game/server/portal2/propglassfutbol.cpp:228 (declaration)
~CPropFutbolSocket();

// game/server/portal2/propglassfutbol.cpp:228 @0x74f040 _ZN17CPropFutbolSocketD0Ev
CPropFutbolSocket::~CPropFutbolSocket()
{
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 228
}

// game/server/portal2/propglassfutbol.cpp:228 @0x74f0c0 _ZN17CPropFutbolSocketD1Ev
CPropFutbolSocket::~CPropFutbolSocket()
{
	// inlined CFutbolCatcher::~CFutbolCatcher() at line 228
}

// game/server/portal2/propglassfutbol.cpp:231 @0x74e9c0 _ZN17CPropFutbolSocketC1Ev
CPropFutbolSocket::CPropFutbolSocket()
{
}

// game/server/portal2/propglassfutbol.cpp:231 @0x74ea00 _ZN17CPropFutbolSocketC2Ev
CPropFutbolSocket::CPropFutbolSocket()
{
}

// game/server/portal2/propglassfutbol.cpp:233 @0x74dec0 _ZN17CPropFutbolSocket5SpawnEv
void CPropFutbolSocket::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 712
}

// game/server/portal2/propglassfutbol.cpp:234 @0x74d620 _ZN17CPropFutbolSocket8PrecacheEv
void CPropFutbolSocket::Precache()
{
}

// game/server/portal2/propglassfutbol.cpp:237
static CEntityFactory<CPropFutbolSocket> prop_glass_futbol_socket;
