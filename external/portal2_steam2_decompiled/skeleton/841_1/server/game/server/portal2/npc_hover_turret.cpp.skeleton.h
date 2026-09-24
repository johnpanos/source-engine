// DWARF declaration skeleton for game/server/portal2/npc_hover_turret.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xac6e0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CHoverTurretTether>::CEntityFactory() at line 174
	// inlined CEntityFactory<CNPC_HoverTurret>::CEntityFactory() at line 525
	// inlined ServerClass::ServerClass() at line 574
	// inlined CAI_ClassScheduleIdSpace::CAI_ClassScheduleIdSpace() at line 1892
}

// game/server/portal2/npc_hover_turret.cpp:86
ConVar sk_hover_turret_health;

// game/server/util.h:100 @0x72ba00 _ZN14CEntityFactoryI18CHoverTurretTetherE6CreateEPKc
IServerNetworkable *CEntityFactory<CHoverTurretTether>::Create( const char *pClassName )
{
	CHoverTurretTether *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CHoverTurretTether>() at line 102
}

// game/server/util.h:100 @0x72ba50 _ZN14CEntityFactoryI16CNPC_HoverTurretE6CreateEPKc
IServerNetworkable *CEntityFactory<CNPC_HoverTurret>::Create( const char *pClassName )
{
	CNPC_HoverTurret *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CNPC_HoverTurret>() at line 102
}

// game/server/portal2/npc_hover_turret.cpp:106
enum hoverTurretAttackState_e
{
	HOVER_TURRET_FIND_TARGET = 0,
	HOVER_TURRET_AIM_TARGET = 1,
	HOVER_TURRET_SHOT_DISABLED = 2,
	HOVER_TURRET_HIT_WITH_PHYSICS = 3,
};

// game/server/util.h:106 @0x72b710 _ZN14CEntityFactoryI16CNPC_HoverTurretE7DestroyEP18IServerNetworkable
void CEntityFactory<CNPC_HoverTurret>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x72b750 _ZN14CEntityFactoryI18CHoverTurretTetherE7DestroyEP18IServerNetworkable
void CEntityFactory<CHoverTurretTether>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x72b740 _ZN14CEntityFactoryI16CNPC_HoverTurretE13GetEntitySizeEv
size_t CEntityFactory<CNPC_HoverTurret>::GetEntitySize()
{
}

// game/server/util.h:114 @0x72b780 _ZN14CEntityFactoryI18CHoverTurretTetherE13GetEntitySizeEv
size_t CEntityFactory<CHoverTurretTether>::GetEntitySize()
{
}

// game/server/portal2/npc_hover_turret.cpp:114
int ACT_HOVER_TURRET_SEARCH;

// game/server/portal2/npc_hover_turret.cpp:115
int ACT_HOVER_TURRET_ALERT;

// game/server/portal2/npc_hover_turret.cpp:116
int ACT_HOVER_TURRET_ANGRY;

// game/server/portal2/npc_hover_turret.cpp:117
int ACT_HOVER_TURRET_DISABLED;

// game/server/portal2/npc_hover_turret.cpp:124
const char *g_HoverTurretTalkNames[8];

// game/server/portal2/npc_hover_turret.cpp:137 (declaration)
const char *GetHoverTurretTalkName( int iState );

// game/server/portal2/npc_hover_turret.cpp:137 @0x7202b0 _Z22GetHoverTurretTalkNamei
const char *GetHoverTurretTalkName( int iState )
{
}

// game/server/portal2/npc_hover_turret.cpp:147 sizeof=0x528 (i386)
struct CHoverTurretTether : public CBaseAnimating
{
public:
	CHoverTurretTether();  // line 150
	virtual void UpdateOnRemove();  // line 151
	void AttachEntities( EHANDLE, EHANDLE );  // line 153
	void ToggleThroughPortal( CPortal_Base2D * );  // line 155
	void ReleaseTether();  // line 157
private:
	void PullThink();  // line 162
	virtual void Spawn();  // line 163
	float m_flNaturalDistance; // +0x514  // line 165
	CHandle<CBaseEntity> m_hAttachPoint; // +0x518  // line 166
	CHandle<CBaseEntity> m_hTurret; // +0x51c  // line 167
	IPhysicsSpring *m_pSpring; // +0x520  // line 169
	CHandle<CRopeKeyframe> m_hRope; // +0x524  // line 171
};

// game/server/portal2/npc_hover_turret.cpp:147 (declaration)
~CHoverTurretTether();

// game/server/portal2/npc_hover_turret.cpp:147 @0x72b790 _ZN18CHoverTurretTetherD0Ev
CHoverTurretTether::~CHoverTurretTether()
{
}

// game/server/portal2/npc_hover_turret.cpp:147 @0x72b7d0 _ZN18CHoverTurretTetherD1Ev
CHoverTurretTether::~CHoverTurretTether()
{
}

// game/server/portal2/npc_hover_turret.cpp:150 @0x721580 _ZN18CHoverTurretTetherC2Ev
CHoverTurretTether::CHoverTurretTether()
{
	// inlined CHandle<CBaseEntity>::operator=() at line 194
	// inlined CHandle<CBaseEntity>::operator=() at line 195
	// inlined CHandle<CRopeKeyframe>::operator=() at line 197
}

// game/server/portal2/npc_hover_turret.cpp:150 @0x7215e0 _ZN18CHoverTurretTetherC1Ev
CHoverTurretTether::CHoverTurretTether()
{
}

// game/server/portal2/npc_hover_turret.cpp:151 @0x720e60 _ZN18CHoverTurretTether14UpdateOnRemoveEv
void CHoverTurretTether::UpdateOnRemove()
{
	// inlined CHandle<CRopeKeyframe>::operator!=() at line 229
}

// game/server/portal2/npc_hover_turret.cpp:153 @0x7202f0 _ZN18CHoverTurretTether14AttachEntitiesE7CHandleI11CBaseEntityES2_
void CHoverTurretTether::AttachEntities( EHANDLE &hTurret, EHANDLE &hAttachPoint )
{
}

// game/server/portal2/npc_hover_turret.cpp:155 @0x720610 _ZN18CHoverTurretTether19ToggleThroughPortalEP14CPortal_Base2D
void CHoverTurretTether::ToggleThroughPortal( CPortal_Base2D *pEnteredPortal )
{
	// inlined CHandle<CBaseEntity>::operator!=() at line 247
	{
		CAI_BaseNPC *pTurret;  // line 249
	}
}

// game/server/portal2/npc_hover_turret.cpp:157 @0x720ef0 _ZN18CHoverTurretTether13ReleaseTetherEv
void CHoverTurretTether::ReleaseTether()
{
	// inlined CHandle<CRopeKeyframe>::operator!=() at line 211
}

// game/server/portal2/npc_hover_turret.cpp:160 @0x7202d0 _ZN18CHoverTurretTether14GetDataDescMapEv
datamap_t *CHoverTurretTether::GetDataDescMap()
{
}

// game/server/portal2/npc_hover_turret.cpp:160 @0x7202e0 _ZN18CHoverTurretTether10GetBaseMapEv
datamap_t *CHoverTurretTether::GetBaseMap()
{
}

// game/server/portal2/npc_hover_turret.cpp:162 @0x721b20 _ZN18CHoverTurretTether9PullThinkEv
void CHoverTurretTether::PullThink()
{
	CAI_BaseNPC *pTurret;  // line 287
	// inlined CHandle<CBaseEntity>::Get() at line 287
	{
		Vector vecStart;  // line 326
		Vector vecEnd;  // line 326
		{
			Vector vecStart;  // line 338
			Ray_t ray;  // line 340
			trace_t tr;  // line 343
			// inlined UTIL_TraceRay() at line 344
			// inlined Ray_t::Init() at line 341
			// inlined CBaseEntity::GetAbsOrigin() at line 341
			// inlined Ray_t::Ray_t() at line 340
			// inlined CBaseEntity::GetAbsOrigin() at line 338
		}
		// inlined Vector::Length() at line 330
		// inlined Vector::operator-=() at line 329
	}
	{
		Vector vecDistance;  // line 296
		IPhysicsObject *pPhysObject;  // line 299
		IPhysicsObject *pAttachPhysObject;  // line 300
		springparams_t spring;  // line 302
		Vector up;  // line 308
		// inlined CBaseEntity::GetAbsOrigin() at line 296
		// inlined CBaseEntity::GetAbsOrigin() at line 296
		// inlined Vector::operator-() at line 296
		// inlined Vector::Length() at line 297
		// inlined CBaseEntity::VPhysicsGetObject() at line 299
		// inlined CBaseEntity::VPhysicsGetObject() at line 300
		// inlined springparams_t::springparams_t() at line 302
		// inlined operator*() at line 311
		// inlined CBaseEntity::GetAbsOrigin() at line 311
		// inlined Vector::operator+() at line 311
		// inlined Vector::operator=() at line 311
		// inlined CBaseEntity::GetAbsOrigin() at line 312
		// inlined Vector::operator=() at line 312
		// inlined CHandle<CRopeKeyframe>::operator!() at line 318
		// inlined CHandle<CRopeKeyframe>::operator=() at line 320
		// inlined CHandle<CRopeKeyframe>::operator->() at line 321
	}
}

// game/server/portal2/npc_hover_turret.cpp:163 @0x722250 _ZN18CHoverTurretTether5SpawnEv
void CHoverTurretTether::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 262
	// inlined CBaseEntity::AddSolidFlags() at line 263
	// inlined CHandle<CBaseEntity>::operator!=() at line 271
}

// game/server/portal2/npc_hover_turret.cpp:174
static CEntityFactory<CHoverTurretTether> ent_hover_turret_tether;

// game/server/portal2/npc_hover_turret.cpp:176 @0xac380 _Z11DataMapInitI18CHoverTurretTetherEP9datamap_tPT_
datamap_t *DataMapInit<CHoverTurretTether>( CHoverTurretTether * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 187
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 176
	CDatadescGeneratedNameHolder nameHolder;  // line 176
	typedescription_t dataDesc[7];  // line 176
}

// game/server/portal2/npc_hover_turret.cpp:176
datamap_t *g_DataMapHolder;

// game/server/portal2/npc_hover_turret.cpp:281
static ConVar hover_turret_break_dist;

// game/server/portal2/npc_hover_turret.cpp:360 sizeof=0xfec (i386)
struct CNPC_HoverTurret : public CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>, public CDefaultPlayerPickupVPhysics
{
public:
	CNPC_HoverTurret();  // line 365
	virtual ~CNPC_HoverTurret();  // line 366
	virtual Class_T Classify();  // line 368
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 371
	virtual void TraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 372
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 373
	void ShootLaser( const Vector &, const Vector & );  // line 375
	void TalkThink();  // line 377
	void FindTargetThink();  // line 378
	void AimThink();  // line 379
	void SetFiringState( hoverTurretAttackState_e );  // line 381
	virtual void UpdateOnRemove();  // line 383
	virtual bool OverrideMove( float );  // line 385
	virtual void MoveToTarget( float, const Vector & );  // line 386
	void MoveExecute_Alive( float );  // line 387
	void MoveExecute_Dead( float );  // line 388
	void CreateSmokeTrail();  // line 390
	void DestroySmokeTrail();  // line 391
	virtual Vector BodyTarget( const Vector &, bool );  // line 393
	virtual float GetHeadTurnRate();  // line 395
	virtual void TurnHeadToTarget( float, const Vector & );  // line 396
	bool UpdateFacing();  // line 397
	void UpdateMuzzleMatrix();  // line 398
	Vector GetClosestVisibleEnemyPosition();  // line 399
	virtual void Precache();  // line 401
	virtual void RunTask( const Task_t * );  // line 402
	virtual void Spawn();  // line 403
	virtual void StartTask( const Task_t * );  // line 404
	virtual void GatherConditions();  // line 406
	virtual void PrescheduleThink();  // line 407
	void Explode();  // line 408
	void StartDeathSequence();  // line 409
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 412
	virtual void ClampMotorForces( Vector &, AngularImpulse & );  // line 413
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 416
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 417
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 418
	virtual QAngle PreferredCarryAngles();  // line 419
	void ToggleUse( CBaseEntity *, CBaseEntity *, $_170, float );  // line 422
	virtual int ObjectCaps();  // line 424
	virtual void Use( CBaseEntity *, CBaseEntity *, $_170, float );  // line 429
	virtual CBasePlayer *HasPhysicsAttacker( float );  // line 438
	float GetMaxEnginePower();  // line 440
	bool IsMovementDisabled();  // line 441
	// game/server/portal2/npc_hover_turret.cpp:443 sizeof=0x1 (i386)
	struct CScheduleLoader
	{
	public:
		CScheduleLoader();  // line 443
	};  // line 443
	static AI_SchedLoadStatus_t gm_SchedLoadStatus;  // line 443
	static CAI_ClassScheduleIdSpace gm_ClassScheduleIdSpace;  // line 443
	static const char *gm_pszErrorClassName;  // line 443
	CNPC_HoverTurret::CScheduleLoader m_ScheduleLoader; // +0xee8  // line 443
	static CAI_LocalIdSpace gm_SquadSlotIdSpace;  // line 443
	static CAI_ClassScheduleIdSpace &AccessClassScheduleIdSpaceDirect();  // line 443
	virtual CAI_ClassScheduleIdSpace *GetClassScheduleIdSpace();  // line 443
	virtual const char *GetSchedulingErrorName();  // line 443
	static void InitCustomSchedules();  // line 443
	static bool LoadSchedules();  // line 443
	virtual bool LoadedSchedules();  // line 443
	virtual const char *SquadSlotName( int );  // line 443
private:
	void MaintainGroundHeight();  // line 448
	void TakeDamageFromPhysicsImpact( int, gamevcollisionevent_t * );  // line 449
	virtual bool AllowedToIgnite();  // line 451
	bool OnBurning();  // line 452
	float m_fNextTalk; // +0xeec  // line 455
	int m_iDesiredState; // +0xef0  // line 456
	int m_iLastState; // +0xef4  // line 457
	bool m_bAimingAtTarget; // +0xef8  // line 460
	Vector m_vecTargetPos; // +0xefc  // line 461
	float m_flAimStartTime; // +0xf08  // line 462
	CNetworkVar( int, m_iLaserState ); // +0xf0c  // line 465
	CNetworkVar( int, m_sLaserHaloSprite ); // +0xf10  // line 466
	bool m_bCarriedByPlayer; // +0xf14  // line 469
	bool m_bUseCarryAngles; // +0xf15  // line 470
	Vector m_vForceVelocity; // +0xf18  // line 473
	Vector m_vForceMoveTarget; // +0xf24  // line 474
	Vector m_vTargetBanking; // +0xf30  // line 476
	EHANDLE m_hAttachTarget; // +0xf3c  // line 478
	string_t m_strAttachName; // +0xf40  // line 479
	EHANDLE m_hTether; // +0xf44  // line 481
	float m_flSentryTurnSpeed; // +0xf48  // line 482
	bool m_bCanPushPlayer; // +0xf4c  // line 484
	float m_flEngineStallTime; // +0xf50  // line 486
	bool m_bInitialPositionSet; // +0xf54  // line 488
	COutputEvent m_OnPhysGunPickup; // +0xf58  // line 490
	COutputEvent m_OnPhysGunDrop; // +0xf70  // line 491
	float m_fSparkTime; // +0xf88  // line 493
	float m_flDeathTime; // +0xf8c  // line 494
	CHandle<SmokeTrail> m_hSmokeTrail; // +0xf90  // line 495
	hoverTurretAttackState_e m_iFiringState; // +0xf94  // line 497
	CHandle<CSprite> m_hEyeGlow; // +0xf98  // line 498
	int m_iMuzzleAttachment; // +0xf9c  // line 500
	matrix3x4_t m_muzzleToWorld; // +0xfa0  // line 501
	int m_muzzleToWorldTick; // +0xfd0  // line 502
	QAngle m_vecGoalAngles; // +0xfd4  // line 504
	QAngle m_vInitialLookAngles; // +0xfe0  // line 505
};

// game/server/portal2/npc_hover_turret.cpp:362 @0x720330 _ZN16CNPC_HoverTurret14GetServerClassEv
ServerClass *CNPC_HoverTurret::GetServerClass()
{
}

// game/server/portal2/npc_hover_turret.cpp:362 @0x720340 _ZN16CNPC_HoverTurret40YouForgotToImplementOrDeclareServerClassEv
int CNPC_HoverTurret::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/npc_hover_turret.cpp:365 @0x726160 _ZN16CNPC_HoverTurretC2Ev
CNPC_HoverTurret::CNPC_HoverTurret()
{
	// inlined CHandle<CBaseEntity>::operator=() at line 599
	// inlined CHandle<CBaseEntity>::operator=() at line 598
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_sLaserHaloSprite>::operator=<int>() at line 596
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::operator=<int>() at line 595
	// inlined CHandle<CSprite>::CHandle() at line 588
	// inlined CHandle<SmokeTrail>::CHandle() at line 588
	// inlined COutputEvent::COutputEvent() at line 588
	// inlined COutputEvent::COutputEvent() at line 588
	// inlined CHandle<CBaseEntity>::CHandle() at line 588
	// inlined CHandle<CBaseEntity>::CHandle() at line 588
	// inlined CNPC_HoverTurret::CScheduleLoader::CScheduleLoader() at line 588
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::CNPCBaseInteractive() at line 588
	// inlined COutputEvent::~COutputEvent() at line 600
	// inlined COutputEvent::~COutputEvent() at line 600
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 600
}

// game/server/portal2/npc_hover_turret.cpp:365 @0x726620 _ZN16CNPC_HoverTurretC1Ev
CNPC_HoverTurret::CNPC_HoverTurret()
{
}

// game/server/portal2/npc_hover_turret.cpp:366 @0x7258b0 _ZN16CNPC_HoverTurretD2Ev
CNPC_HoverTurret::~CNPC_HoverTurret()
{
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
}

// game/server/portal2/npc_hover_turret.cpp:366 @0x725a70 _ZN16CNPC_HoverTurretD0Ev
CNPC_HoverTurret::~CNPC_HoverTurret()
{
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
}

// game/server/portal2/npc_hover_turret.cpp:366 @0x725c30 _ZN16CNPC_HoverTurretD1Ev
CNPC_HoverTurret::~CNPC_HoverTurret()
{
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
	// inlined COutputEvent::~COutputEvent() at line 607
	// inlined CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive() at line 607
}

// game/server/portal2/npc_hover_turret.cpp:368 @0x72b5c0 _ZN16CNPC_HoverTurret8ClassifyEv
Class_T CNPC_HoverTurret::Classify()
{
}

// game/server/portal2/npc_hover_turret.cpp:371 @0x72aa80 _ZN16CNPC_HoverTurret12Event_KilledERK15CTakeDamageInfo
void CNPC_HoverTurret::Event_Killed( const CTakeDamageInfo &info )
{
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 1058
	// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1043
	// inlined CHandle<SmokeTrail>::operator==() at line 1031
	{
		CHoverTurretTether *pTether;  // line 1045
		// inlined CHoverTurretTether::ReleaseTether() at line 1046
		// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1048
		// inlined CHandle<CBaseEntity>::operator=() at line 1049
	}
}

// game/server/portal2/npc_hover_turret.cpp:372 @0x720e40 _ZN16CNPC_HoverTurret11TraceAttackERK15CTakeDamageInfoRK6VectorP10CGameTrace
void CNPC_HoverTurret::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
}

// game/server/portal2/npc_hover_turret.cpp:373 @0x7215f0 _ZN16CNPC_HoverTurret17NotifySystemEventEP11CBaseEntity21notify_system_event_tRK28notify_system_event_params_t
void CNPC_HoverTurret::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// inlined CHandle<CBaseEntity>::operator!=() at line 1085
	{
		CHoverTurretTether *pHoverTurretTether;  // line 1087
		CPortal_Base2D *pEnteredPortal;  // line 1088
		// inlined CHoverTurretTether::ToggleThroughPortal() at line 1091
	}
}

// game/server/portal2/npc_hover_turret.cpp:375 @0x722d30 _ZN16CNPC_HoverTurret10ShootLaserERK6VectorS2_
void CNPC_HoverTurret::ShootLaser( const Vector &vecSrc, const Vector &vecDirToEnemy )
{
	trace_t tr;  // line 1737
	Vector vecCenter;  // line 1738
	{
		CBaseEntity *pHit;  // line 1745
		{
			Vector dir;  // line 1749
			int nDamageType;  // line 1751
			CTakeDamageInfo info;  // line 1753
			// inlined Vector::operator-() at line 1749
			// inlined CBaseEntity::GetAbsOrigin() at line 1749
		}
	}
	// inlined UTIL_TraceLine() at line 1739
	// inlined Vector::operator+() at line 1739
	// inlined Vector::operator*() at line 1739
	// inlined CBaseEntity::GetAbsOrigin() at line 1738
}

// game/server/portal2/npc_hover_turret.cpp:377 @0x7207f0 _ZN16CNPC_HoverTurret9TalkThinkEv
void CNPC_HoverTurret::TalkThink()
{
	{
		const char *pchScriptName;  // line 777
		// inlined GetHoverTurretTalkName() at line 777
	}
	// inlined CBaseAnimating::IsDissolving() at line 761
}

// game/server/portal2/npc_hover_turret.cpp:378 @0x72b150 _ZN16CNPC_HoverTurret15FindTargetThinkEv
void CNPC_HoverTurret::FindTargetThink()
{
	// inlined CNPC_HoverTurret::IsMovementDisabled() at line 814
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::GetForModify() at line 822
	// inlined CBaseEntity::GetAbsAngles() at line 852
	// inlined QAngle::operator=() at line 852
	{
		CBaseEntity *pEnemy;  // line 827
		{
			bool bEnemyInFOV;  // line 831
			bool bEnemyVisible;  // line 832
			bool bEnemyVisibleThroughPortal;  // line 833
			CPortal_Base2D *pPortal;  // line 836
		}
	}
}

// game/server/portal2/npc_hover_turret.cpp:379 @0x72aca0 _ZN16CNPC_HoverTurret8AimThinkEv
void CNPC_HoverTurret::AimThink()
{
	bool bAbortAim;  // line 864
	// inlined QAngle::operator=() at line 887
	// inlined CBaseEntity::GetAbsAngles() at line 887
	// inlined CNPC_HoverTurret::IsMovementDisabled() at line 867
	{
		CBaseEntity *pEnemy;  // line 907
		{
			bool bEnemyInFOV;  // line 910
			bool bEnemyVisible;  // line 911
			bool bEnemyVisibleThroughPortal;  // line 912
			CPortal_Base2D *pPortal;  // line 915
			{
				Vector vecMuzzle;  // line 929
				Vector vecMuzzleDir;  // line 929
				// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::GetForModify() at line 936
				// inlined CNPC_HoverTurret::UpdateMuzzleMatrix() at line 927
			}
		}
	}
	{
		Vector vecEnemyPosition;  // line 874
		Vector vecEnemyMovement;  // line 875
		// inlined Vector::operator VectorByValue&() at line 874
		// inlined Vector::operator-() at line 875
		// inlined Vector::operator=() at line 877
		// inlined Vector::Length() at line 879
	}
}

// game/server/portal2/npc_hover_turret.cpp:381 @0x726630 _ZN16CNPC_HoverTurret14SetFiringStateE24hoverTurretAttackState_e
void CNPC_HoverTurret::SetFiringState( hoverTurretAttackState_e state )
{
	bool bNewState;  // line 678
	// inlined CHandle<CSprite>::operator->() at line 746
	// inlined CHandle<CSprite>::operator->() at line 699
	// inlined CSprite::SetColor() at line 698
	// inlined CHandle<CSprite>::operator->() at line 698
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 694
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::GetForModify() at line 690
	// inlined CHandle<CSprite>::operator!() at line 667
	// inlined CHandle<CSprite>::operator=() at line 670
	// inlined CHandle<CSprite>::operator!() at line 671
	// inlined CSprite::SetTransparency() at line 674
	// inlined CHandle<CSprite>::operator->() at line 675
	// inlined CSprite::SetAttachment() at line 675
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 741
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::GetForModify() at line 742
	// inlined CHandle<CSprite>::operator->() at line 744
	// inlined CSprite::SetColor() at line 744
	// inlined CHandle<CSprite>::operator->() at line 745
	// inlined CHandle<CSprite>::operator->() at line 732
	// inlined CSprite::SetColor() at line 732
	// inlined CHandle<CSprite>::operator->() at line 733
	// inlined CHandle<CSprite>::operator->() at line 734
	// inlined Vector::operator=() at line 705
	// inlined CHandle<CSprite>::operator->() at line 716
	// inlined CSprite::SetColor() at line 716
	// inlined CHandle<CSprite>::operator->() at line 717
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 712
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 728
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_iLaserState>::GetForModify() at line 729
}

// game/server/portal2/npc_hover_turret.cpp:383 @0x720a00 _ZN16CNPC_HoverTurret14UpdateOnRemoveEv
void CNPC_HoverTurret::UpdateOnRemove()
{
}

// game/server/portal2/npc_hover_turret.cpp:385 @0x7218a0 _ZN16CNPC_HoverTurret12OverrideMoveEf
bool CNPC_HoverTurret::OverrideMove( float flInterval )
{
	// inlined CBaseEntity::VPhysicsGetObject() at line 1288
	// inlined CNPC_HoverTurret::IsMovementDisabled() at line 1292
	// inlined CNPC_HoverTurret::MoveExecute_Alive() at line 1305
	// inlined CBaseEntity::GetAbsOrigin() at line 1283
	// inlined Vector::operator=() at line 1283
}

// game/server/portal2/npc_hover_turret.cpp:386 @0x724ab0 _ZN16CNPC_HoverTurret12MoveToTargetEfRK6Vector
void CNPC_HoverTurret::MoveToTarget( float flInterval, const Vector &vMoveTarget )
{
	{
		Vector vecMoveDir;  // line 1718
		Vector vecLookDir;  // line 1721
		// inlined Vector::operator VectorByValue&() at line 1726
		// inlined Vector::operator+() at line 1726
		// inlined CBaseEntity::GetAbsOrigin() at line 1726
		// inlined Vector::operator=() at line 1724
		// inlined Vector::operator+() at line 1724
		// inlined operator*() at line 1724
		// inlined Vector::NormalizeInPlace() at line 1719
		// inlined Vector::operator-() at line 1718
		// inlined CBaseEntity::GetAbsOrigin() at line 1718
	}
	// inlined CNPC_HoverTurret::IsMovementDisabled() at line 1700
	{
		float newYaw;  // line 1709
	}
}

// game/server/portal2/npc_hover_turret.cpp:387 @0x720a80 _ZN16CNPC_HoverTurret17MoveExecute_AliveEf
void CNPC_HoverTurret::MoveExecute_Alive( float flInterval )
{
	Vector vCurrentVelocity;  // line 1320
	// inlined Vector::operator=() at line 1332
	// inlined CAI_BasePhysicsFlyingBot::SetCurrentVelocity() at line 1331
	// inlined Vector::operator+() at line 1331
	// inlined CHandle<CBaseEntity>::operator!=() at line 1323
	// inlined CBaseEntity::VPhysicsGetObject() at line 1323
}

// game/server/portal2/npc_hover_turret.cpp:388 @0x721700 _ZN16CNPC_HoverTurret16MoveExecute_DeadEf
void CNPC_HoverTurret::MoveExecute_Dead( float flInterval )
{
	Vector newVelocity;  // line 1342
	Vector forward;  // line 1344
	Vector up;  // line 1344
	// inlined Vector::operator=() at line 1359
	// inlined CAI_BasePhysicsFlyingBot::SetCurrentVelocity() at line 1358
	// inlined Vector::operator+=() at line 1357
	// inlined CBaseEntity::GetAbsOrigin() at line 1350
}

// game/server/portal2/npc_hover_turret.cpp:390 @0x723140 _ZN16CNPC_HoverTurret16CreateSmokeTrailEv
void CNPC_HoverTurret::CreateSmokeTrail()
{
	SmokeTrail *pSmokeTrail;  // line 1856
	// inlined CHandle<SmokeTrail>::operator=() at line 1874
	// inlined CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>::Init() at line 1869
	// inlined CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>::Init() at line 1868
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_MaxSpeed>::operator=<int>() at line 1866
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_MinSpeed>::operator=<int>() at line 1865
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_SpawnRadius>::operator=<int>() at line 1864
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_EndSize>::operator=<int>() at line 1863
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_StartSize>::operator=<int>() at line 1862
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_ParticleLifetime>::operator=<float>() at line 1861
	// inlined CNetworkVarBase<float,SmokeTrail::NetworkVar_m_SpawnRate>::operator=<int>() at line 1860
	// inlined CHandle<SmokeTrail>::operator!=() at line 1853
}

// game/server/portal2/npc_hover_turret.cpp:391 @0x721500 _ZN16CNPC_HoverTurret17DestroySmokeTrailEv
void CNPC_HoverTurret::DestroySmokeTrail()
{
	// inlined CHandle<SmokeTrail>::Get() at line 1879
	// inlined CHandle<SmokeTrail>::operator=() at line 1882
}

// game/server/portal2/npc_hover_turret.cpp:393 @0x72b5d0 _ZN16CNPC_HoverTurret10BodyTargetERK6Vectorb
Vector CNPC_HoverTurret::BodyTarget( const Vector &posSrc, bool bNoisy )
{
}

// game/server/portal2/npc_hover_turret.cpp:395 @0x72b610 _ZN16CNPC_HoverTurret15GetHeadTurnRateEv
float CNPC_HoverTurret::GetHeadTurnRate()
{
}

// game/server/portal2/npc_hover_turret.cpp:396 @0x720f70 _ZN16CNPC_HoverTurret16TurnHeadToTargetEfRK6Vector
void CNPC_HoverTurret::TurnHeadToTarget( float flInterval, const Vector &MoveTarget )
{
	float flDestYaw;  // line 1194
	float newYaw;  // line 1196
	// inlined Vector::operator-() at line 1194
	// inlined Vector::operator VectorByValue&() at line 1194
}

// game/server/portal2/npc_hover_turret.cpp:397 @0x721230 _ZN16CNPC_HoverTurret12UpdateFacingEv
bool CNPC_HoverTurret::UpdateFacing()
{
	bool bMoved;  // line 1209
	Vector vecGoalDir;  // line 1212
	Vector vecGoalLocalDir;  // line 1215
	QAngle vecGoalLocalAngles;  // line 1234
	float flDiff;  // line 1237
	// inlined CBaseAnimating::SetPoseParameter() at line 1247
	// inlined UTIL_ApproachAngle() at line 1246
	// inlined CBaseAnimating::SetPoseParameter() at line 1238
	// inlined UTIL_ApproachAngle() at line 1237
	// inlined VectorIRotate() at line 1216
	// inlined CNPC_HoverTurret::UpdateMuzzleMatrix() at line 1210
}

// game/server/portal2/npc_hover_turret.cpp:398 @0x720360 _ZN16CNPC_HoverTurret18UpdateMuzzleMatrixEv
void CNPC_HoverTurret::UpdateMuzzleMatrix()
{
}

// game/server/portal2/npc_hover_turret.cpp:399 @0x724320 _ZN16CNPC_HoverTurret30GetClosestVisibleEnemyPositionEv
Vector CNPC_HoverTurret::GetClosestVisibleEnemyPosition()
{
	Vector vecMidEnemy;  // line 957
	CBaseEntity *pEnemy;  // line 951
	Vector vecMuzzlePos;  // line 955
	Vector vecMidEnemy;  // line 957
	Vector vecMid;  // line 959
	Vector vecBody;  // line 960
	bool bEnemyInFOV;  // line 963
	bool bEnemyVisible;  // line 964
	Vector vecDirToEnemy;  // line 967
	float flDistToEnemy;  // line 968
	CPortal_Base2D *pPortal;  // line 971
	QAngle vecAnglesToEnemy;  // line 1006
	// inlined QAngle::operator=() at line 1011
	// inlined Vector::operator-() at line 967
	// inlined Vector::operator+() at line 957
	// inlined operator*() at line 957
	// inlined CCollisionProperty::WorldSpaceCenter() at line 957
	// inlined Vector::operator VectorByValue&() at line 957
	// inlined operator*() at line 957
	// inlined CNPC_HoverTurret::UpdateMuzzleMatrix() at line 954
	{
		Vector vecMidEnemyTransformed;  // line 979
		Vector vecDirToEnemyTransformed;  // line 983
		float flDistToEnemyTransformed;  // line 984
		// inlined Vector::operator=() at line 992
		// inlined Vector::operator=() at line 991
		// inlined Vector::operator-() at line 983
		// inlined CNetworkHandleBase<CPortal_Base2D,CPortal_Base2D::NetworkVar_m_hLinkedPortal>::operator->() at line 980
	}
}

// game/server/portal2/npc_hover_turret.cpp:401 @0x725df0 _ZN16CNPC_HoverTurret8PrecacheEv
void CNPC_HoverTurret::Precache()
{
	{
		int iTalkScript;  // line 1483
	}
	// inlined CNetworkVarBase<int,CNPC_HoverTurret::NetworkVar_m_sLaserHaloSprite>::GetForModify() at line 1481
}

// game/server/portal2/npc_hover_turret.cpp:402 @0x720a50 _ZN16CNPC_HoverTurret7RunTaskEPK6Task_t
void CNPC_HoverTurret::RunTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_hover_turret.cpp:403 @0x729e30 _ZN16CNPC_HoverTurret5SpawnEv
void CNPC_HoverTurret::Spawn()
{
	Vector bobAmount;  // line 1551
	{
		CHoverTurretTether *pHoverTurretTether;  // line 1591
		CBaseEntity *pAttachEnt;  // line 1595
		Vector vecToAttach;  // line 1596
		// inlined CBaseEntity::GetAbsOrigin() at line 1596
		// inlined CBaseEntity::GetAbsOrigin() at line 1596
		// inlined CHandle<CBaseEntity>::Get() at line 1595
		// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 1593
		// inlined CHandle<CBaseEntity>::CHandle() at line 1592
		// inlined CHandle<CBaseEntity>::CHandle() at line 1592
		// inlined CHandle<CBaseEntity>::Get() at line 1591
		// inlined CHandle<CBaseEntity>::operator->() at line 1590
		// inlined CBaseEntity::GetAbsOrigin() at line 1590
		// inlined CHandle<CBaseEntity>::Get() at line 1590
		// inlined CHandle<CBaseEntity>::operator=() at line 1589
	}
	// inlined CHandle<CBaseEntity>::operator==() at line 1587
	// inlined CBaseCombatCharacter::SetHullType() at line 1519
	// inlined CBaseEntity::SetSolid() at line 1522
	// inlined CBaseEntity::AddSolidFlags() at line 1523
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<float>() at line 1527
	// inlined CBaseAnimating::SetPoseParameter() at line 1530
	// inlined CBaseAnimating::SetPoseParameter() at line 1531
	// inlined CBaseEntity::AddEFlags() at line 1539
	// inlined CAI_BasePhysicsFlyingBot::SetCurrentVelocity() at line 1543
	// inlined Vector::Init() at line 1544
	// inlined Vector::Init() at line 1545
	// inlined Vector::Init() at line 1546
	// inlined CAI_BasePhysicsFlyingBot::SetNoiseMod() at line 1556
	// inlined Vector::Vector() at line 1562
	// inlined Vector::operator VectorByValue&() at line 1562
	// inlined CBaseEntity::GetAbsAngles() at line 1569
	// inlined QAngle::operator=() at line 1569
	// inlined CBaseEntity::GetAbsAngles() at line 1570
	// inlined QAngle::operator=() at line 1570
	// inlined CBaseEntity::GetAbsOrigin() at line 1575
	// inlined Vector::operator==() at line 1575
	// inlined CBaseEntity::GetAbsOrigin() at line 1582
	// inlined Vector::operator=() at line 1582
	// inlined CGlobalEntityList::FindEntityByName() at line 1585
	// inlined CHandle<CBaseEntity>::operator=() at line 1585
	// inlined CHandle<CBaseEntity>::operator!=() at line 1587
}

// game/server/portal2/npc_hover_turret.cpp:404 @0x720a20 _ZN16CNPC_HoverTurret9StartTaskEPK6Task_t
void CNPC_HoverTurret::StartTask( const Task_t *pTask )
{
}

// game/server/portal2/npc_hover_turret.cpp:406 @0x720e50 _ZN16CNPC_HoverTurret16GatherConditionsEv
void CNPC_HoverTurret::GatherConditions()
{
}

// game/server/portal2/npc_hover_turret.cpp:407 @0x72b4d0 _ZN16CNPC_HoverTurret16PrescheduleThinkEv
void CNPC_HoverTurret::PrescheduleThink()
{
	// inlined CNPC_HoverTurret::OnBurning() at line 625
}

// game/server/portal2/npc_hover_turret.cpp:408 @0x722380 _ZN16CNPC_HoverTurret7ExplodeEv
void CNPC_HoverTurret::Explode()
{
	CBreakableProp *pFutbol;  // line 1792
	string_t strTurretName;  // line 1802
	string_t strPropName;  // line 1803
	Vector vecRadius;  // line 1805
	CBaseEntity *pEnts[128];  // line 1806
	int nNumFound;  // line 1807
	CPVSFilter filter;  // line 1835
	Vector gibVelocity;  // line 1836
	int iModelIndex;  // line 1837
	// inlined CPVSFilter::~CPVSFilter() at line 1845
	{
		int i;  // line 1838
		// inlined Vector::Vector() at line 1840
	}
	// inlined RandomVector() at line 1836
	// inlined CPVSFilter::CPVSFilter() at line 1835
	// inlined UTIL_EntitiesInBox() at line 1807
	// inlined Vector::operator VectorByValue&() at line 1807
	// inlined Vector::operator VectorByValue&() at line 1807
	// inlined Vector::operator-() at line 1807
	// inlined CBaseEntity::GetAbsOrigin() at line 1807
	// inlined Vector::operator+() at line 1807
	// inlined CBaseEntity::GetAbsOrigin() at line 1807
	{
		Vector vecVelocity;  // line 1796
	}
	// inlined CBaseEntity::GetAbsOrigin() at line 1792
	{
		int i;  // line 1808
		{
			trace_t tr;  // line 1816
			CTakeDamageInfo info;  // line 1821
			// inlined string_t::operator!=() at line 1813
			// inlined CBaseEntity::GetAbsOrigin() at line 1817
			// inlined UTIL_TraceLine() at line 1817
			// inlined CBaseEntity::GetAbsOrigin() at line 1822
			// inlined CTakeDamageInfo::SetDamagePosition() at line 1822
			// inlined CBaseEntity::GetAbsOrigin() at line 1823
			// inlined CBaseEntity::GetAbsOrigin() at line 1823
			// inlined CBaseEntity::GetAbsOrigin() at line 1823
			// inlined Vector::operator-() at line 1823
			// inlined Vector::operator VectorByValue&() at line 1823
		}
	}
	// inlined CBaseEntity::GetAbsAngles() at line 1792
	// inlined CPVSFilter::~CPVSFilter() at line 1845
}

// game/server/portal2/npc_hover_turret.cpp:409 @0x7206b0 _ZN16CNPC_HoverTurret18StartDeathSequenceEv
void CNPC_HoverTurret::StartDeathSequence()
{
}

// game/server/portal2/npc_hover_turret.cpp:412 @0x72a960 _ZN16CNPC_HoverTurret17VPhysicsCollisionEiP21gamevcollisionevent_t
void CNPC_HoverTurret::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	int otherIndex;  // line 1149
	CBaseEntity *pHitEntity;  // line 1150
}

// game/server/portal2/npc_hover_turret.cpp:413 @0x7204d0 _ZN16CNPC_HoverTurret16ClampMotorForcesER6VectorS1_
void CNPC_HoverTurret::ClampMotorForces( Vector &linear, AngularImpulse &angular )
{
}

// game/server/portal2/npc_hover_turret.cpp:416 @0x72a8e0 _ZN16CNPC_HoverTurret15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CNPC_HoverTurret::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
}

// game/server/portal2/npc_hover_turret.cpp:417 @0x72a650 _ZN16CNPC_HoverTurret13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CNPC_HoverTurret::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason )
{
	// inlined CHandle<CBaseEntity>::operator!=() at line 1400
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 1409
}

// game/server/portal2/npc_hover_turret.cpp:418 @0x7203c0 _ZN16CNPC_HoverTurret32HasPreferredCarryAnglesForPlayerEP11CBasePlayer
bool CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
}

// game/server/portal2/npc_hover_turret.cpp:419 @0x721080 _ZN16CNPC_HoverTurret20PreferredCarryAnglesEv
QAngle CNPC_HoverTurret::PreferredCarryAngles()
{
	Vector vecUserForward;  // line 1436
	CBasePlayer *pPlayer;  // line 1437
	// inlined AI_GetSinglePlayer() at line 1437
	// inlined QAngle::Init() at line 1445
	QAngle g_prefAngles;  // line 1434
}

// game/server/portal2/npc_hover_turret.cpp:424 @0x72b9e0 _ZN16CNPC_HoverTurret10ObjectCapsEv
int CNPC_HoverTurret::ObjectCaps()
{
	// inlined CAI_BaseNPC::ObjectCaps() at line 426
}

// game/server/portal2/npc_hover_turret.cpp:429 @0x72b620 _ZN16CNPC_HoverTurret3UseEP11CBaseEntityS1_8USE_TYPEf
void CNPC_HoverTurret::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, $_170 useType, float value )
{
	CBasePlayer *pPlayer;  // line 431
	// inlined ToBasePlayer() at line 431
}

// game/server/portal2/npc_hover_turret.cpp:438 @0x7203f0 _ZN16CNPC_HoverTurret18HasPhysicsAttackerEf
CBasePlayer *CNPC_HoverTurret::HasPhysicsAttacker( float dt )
{
}

// game/server/portal2/npc_hover_turret.cpp:440 @0x720400 _ZN16CNPC_HoverTurret17GetMaxEnginePowerEv
float CNPC_HoverTurret::GetMaxEnginePower()
{
}

// game/server/portal2/npc_hover_turret.cpp:441 @0x720410 _ZN16CNPC_HoverTurret18IsMovementDisabledEv
bool CNPC_HoverTurret::IsMovementDisabled()
{
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x72b670 _ZN16CNPC_HoverTurret23GetClassScheduleIdSpaceEv
CAI_ClassScheduleIdSpace *CNPC_HoverTurret::GetClassScheduleIdSpace()
{
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x72b680 _ZN16CNPC_HoverTurret22GetSchedulingErrorNameEv
const char *CNPC_HoverTurret::GetSchedulingErrorName()
{
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x720450 _ZN16CNPC_HoverTurret13LoadSchedulesEv
bool CNPC_HoverTurret::LoadSchedules()
{
	// inlined AI_DoLoadSchedules() at line 1892
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x7204b0 _ZN16CNPC_HoverTurret15LoadedSchedulesEv
bool CNPC_HoverTurret::LoadedSchedules()
{
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x7205d0 _ZN16CNPC_HoverTurret13SquadSlotNameEi
const char *CNPC_HoverTurret::SquadSlotName( int slotEN )
{
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x721190 _ZN16CNPC_HoverTurret15CScheduleLoaderC1Ev
CNPC_HoverTurret::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_HoverTurret::LoadSchedules() at line 1892
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x7211e0 _ZN16CNPC_HoverTurret15CScheduleLoaderC2Ev
CNPC_HoverTurret::CScheduleLoader::CScheduleLoader()
{
	// inlined CNPC_HoverTurret::LoadSchedules() at line 1892
}

// game/server/portal2/npc_hover_turret.cpp:443 @0x724d70 _ZN16CNPC_HoverTurret19InitCustomSchedulesEv
void CNPC_HoverTurret::InitCustomSchedules()
{
	const char *pszClassName;  // line 1892
	CUtlVector<char*,CUtlMemory<char*, int> > schedulesToLoad;  // line 1892
	CUtlVector<bool (*)(),CUtlMemory<bool (*)(), int> > reqiredOthers;  // line 1892
	CAI_NamespaceInfos scheduleIds;  // line 1892
	CAI_NamespaceInfos taskIds;  // line 1892
	CAI_NamespaceInfos conditionIds;  // line 1892
	CAI_NamespaceInfos squadSlotIds;  // line 1892
	int i;  // line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 1894
	// inlined CAI_ClassScheduleIdSpace::AddSchedule() at line 1894
	// inlined CAI_NamespaceInfos::Sort() at line 1894
	// inlined CAI_NamespaceInfos::Sort() at line 1894
	// inlined CAI_NamespaceInfos::Sort() at line 1894
	// inlined CAI_NamespaceInfos::Sort() at line 1894
	// inlined CAI_ClassScheduleIdSpace::Init() at line 1894
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 1892
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 1892
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 1892
	// inlined CAI_NamespaceInfos::CAI_NamespaceInfos() at line 1892
	// inlined CAI_ClassScheduleIdSpace::AddTask() at line 1894
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 1894
	// inlined ValidateConditionLimits() at line 1894
	// inlined CAI_ClassScheduleIdSpace::AddCondition() at line 1894
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 1894
	// inlined CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t, int> >::operator[]() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
	// inlined CAI_NamespaceInfos::~CAI_NamespaceInfos() at line 1894
}

// game/server/portal2/npc_hover_turret.cpp:445 @0x720310 _ZN16CNPC_HoverTurret14GetDataDescMapEv
datamap_t *CNPC_HoverTurret::GetDataDescMap()
{
}

// game/server/portal2/npc_hover_turret.cpp:445 @0x720320 _ZN16CNPC_HoverTurret10GetBaseMapEv
datamap_t *CNPC_HoverTurret::GetBaseMap()
{
}

// game/server/portal2/npc_hover_turret.cpp:448 @0x720350 _ZN16CNPC_HoverTurret20MaintainGroundHeightEv
void CNPC_HoverTurret::MaintainGroundHeight()
{
}

// game/server/portal2/npc_hover_turret.cpp:449 @0x720bc0 _ZN16CNPC_HoverTurret27TakeDamageFromPhysicsImpactEiP21gamevcollisionevent_t
void CNPC_HoverTurret::TakeDamageFromPhysicsImpact( int index, gamevcollisionevent_t *pEvent )
{
	CBaseEntity *pHitEntity;  // line 1102
	float flDamageScale;  // line 1105
	int damageType;  // line 1110
	float damage;  // line 1111
	Vector damagePos;  // line 1121
	Vector damageForce;  // line 1123
	// inlined FClassnameIs() at line 1113
	// inlined Vector::operator*() at line 1123
	// inlined Vector::operator==() at line 1124
	// inlined Vector::operator*() at line 1128
	// inlined Vector::operator=() at line 1128
}

// game/server/portal2/npc_hover_turret.cpp:451 @0x72b690 _ZN16CNPC_HoverTurret15AllowedToIgniteEv
bool CNPC_HoverTurret::AllowedToIgnite()
{
}

// game/server/portal2/npc_hover_turret.cpp:452 @0x722cc0 _ZN16CNPC_HoverTurret9OnBurningEv
bool CNPC_HoverTurret::OnBurning()
{
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator-=<int>() at line 1776
}

// game/server/portal2/npc_hover_turret.cpp:525
static CEntityFactory<CNPC_HoverTurret> npc_hover_turret;

// game/server/portal2/npc_hover_turret.cpp:527 @0xac5b0 _Z11DataMapInitI16CNPC_HoverTurretEP9datamap_tPT_
datamap_t *DataMapInit<CNPC_HoverTurret>( CNPC_HoverTurret * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 527
	CDatadescGeneratedNameHolder nameHolder;  // line 527
	typedescription_t dataDesc[33];  // line 527
}

// game/server/portal2/npc_hover_turret.cpp:527
datamap_t *g_DataMapHolder;

// game/server/portal2/npc_hover_turret.cpp:574 @0xac150 _Z15ServerClassInitIN18DT_NPC_HoverTurret7ignoredEEiPT_
int ServerClassInit<DT_NPC_HoverTurret::ignored>( DT_NPC_HoverTurret::ignored * )
{
	SendTable &sendTable;  // line 574
	char *const g_pSendTableName;  // line 574
	SendProp g_SendProps[4];  // line 574
}

// game/server/portal2/npc_hover_turret.cpp:574
SendTable g_SendTable;

// game/server/portal2/npc_hover_turret.cpp:574
int g_SendTableInit;

// game/server/portal2/npc_hover_turret.cpp:574
static ServerClass g_CNPC_HoverTurret_ClassReg;

// game/server/ai_basenpc.h:3184 @0x72b870 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotED0Ev
CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive()
{
	// inlined COutputEvent::~COutputEvent() at line 3184
	// inlined COutputEvent::~COutputEvent() at line 3184
	// inlined COutputEvent::~COutputEvent() at line 3184
}

// game/server/ai_basenpc.h:3184 @0x72b930 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotED1Ev
CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::~CNPCBaseInteractive()
{
	// inlined COutputEvent::~COutputEvent() at line 3184
	// inlined COutputEvent::~COutputEvent() at line 3184
	// inlined COutputEvent::~COutputEvent() at line 3184
}

// game/server/ai_basenpc.h:3187 @0x72b6c0 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE15CanInteractWithEP11CAI_BaseNPC
bool CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::CanInteractWith( CAI_BaseNPC *pUser )
{
}

// game/server/ai_basenpc.h:3188 @0x72b6e0 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE21HasBeenInteractedWithEv
bool CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::HasBeenInteractedWith()
{
}

// game/server/ai_basenpc.h:3189 @0x72b700 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE17NotifyInteractionEP11CAI_BaseNPC
void CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::NotifyInteraction( CAI_BaseNPC *pUser )
{
}

// game/server/ai_basenpc.h:3191 @0x72b6a0 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE14InputPowerdownER11inputdata_t
void CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::InputPowerdown( inputdata_t &inputdata )
{
}

// game/server/ai_basenpc.h:3197 @0x72b840 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE22AlyxStartedInteractionEv
void CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::AlyxStartedInteraction()
{
}

// game/server/ai_basenpc.h:3201 @0x72b800 _ZN19CNPCBaseInteractiveI24CAI_BasePhysicsFlyingBotE23AlyxFinishedInteractionEv
void CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>::AlyxFinishedInteraction()
{
}
