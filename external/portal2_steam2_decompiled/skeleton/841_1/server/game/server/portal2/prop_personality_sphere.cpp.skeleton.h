// DWARF declaration skeleton for game/server/portal2/prop_personality_sphere.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb8250 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CInfoTargetPersonalitySphere>::CEntityFactory() at line 92
	// inlined CEntityFactory<CPropPersonalitySphere>::CEntityFactory() at line 252
}

// game/server/portal2/prop_personality_sphere.cpp:17
static const char *const s_pAnimateThinkContext;

// game/shared/saverestore_utlvector.h:30 @0x766c20 _ZN17CUtlVectorDataOpsI10CUtlVectorI6Vector10CUtlMemoryIS1_iEELi3EE4SaveERK22SaveRestoreFieldInfo_tP5ISave
void CUtlVectorDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> >,3>::Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
{
	datamap_t *pArrayTypeDatamap;  // line 32
	typedescription_t dataDesc;  // line 33
	datamap_t dataMap;  // line 47
	CUtlVector<Vector,CUtlMemory<Vector, int> > *pUtlVector;  // line 60
	int nElems;  // line 61
}

// game/server/portal2/prop_personality_sphere.cpp:36
enum SphereEvent_e
{
	SPHERE_EVENT_INFO_TARGET = 0,
	SPHERE_EVENT_WAITING = 1,
	SPHERE_EVENT_HELD = 2,
	SPHERE_EVENT_HELD_PANIC = 3,
	SPHERE_EVENT_DROP = 4,
	SPHERE_EVENT_NOTHELD = 5,
	SPHERE_EVENT_NOTHELD_RELIEVED = 6,
	SPHERE_EVENT_FIRST_PICKUP = 7,
	SPHERE_EVENT_FRUSTRATION = 8,
	SPHERE_EVENT_HURT = 9,
	SPHERE_EVENT_HURT_REPEATEDLY = 10,
	SPHERE_EVENT_VELOCITY_UP_HIGH = 11,
	SPHERE_EVENT_VELOCITY_DOWN_HIGH = 12,
	SPHERE_EVENT_VELOCITY_LATERAL_LOW = 13,
	SPHERE_EVENT_VELOCITY_LATERAL_HIGH = 14,
	SPHERE_EVENT_TOTAL = 15,
};

// game/server/portal2/prop_personality_sphere.cpp:56
const char *g_LineNames[15];

// game/server/portal2/prop_personality_sphere.cpp:76 sizeof=0x3c8 (i386)
struct CInfoTargetPersonalitySphere : public CPointEntity
{
public:
	float GetTargetRadius();  // line 79
	string_t GetSphereLine();  // line 80
private:
	virtual void Spawn();  // line 84
	virtual void Precache();  // line 85
	string_t m_strSphereLine; // +0x3c0  // line 87
	float m_flRadius; // +0x3c4  // line 88
};

// game/server/portal2/prop_personality_sphere.cpp:76 (declaration)
~CInfoTargetPersonalitySphere();

// game/server/portal2/prop_personality_sphere.cpp:76 @0x766b70 _ZN28CInfoTargetPersonalitySphereD1Ev
CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere()
{
	// inlined CPointEntity::~CPointEntity() at line 76
}

// game/server/portal2/prop_personality_sphere.cpp:76 (declaration)
void CInfoTargetPersonalitySphere();

// game/server/portal2/prop_personality_sphere.cpp:76 @0x766f80 _ZN28CInfoTargetPersonalitySphereD0Ev
CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere()
{
	// inlined CPointEntity::~CPointEntity() at line 76
}

// game/server/portal2/prop_personality_sphere.cpp:82 @0x763210 _ZN28CInfoTargetPersonalitySphere14GetDataDescMapEv
datamap_t *CInfoTargetPersonalitySphere::GetDataDescMap()
{
}

// game/server/portal2/prop_personality_sphere.cpp:82 @0x763220 _ZN28CInfoTargetPersonalitySphere10GetBaseMapEv
datamap_t *CInfoTargetPersonalitySphere::GetBaseMap()
{
}

// game/shared/saverestore_utlvector.h:82 @0x766fd0 _ZN17CUtlVectorDataOpsI10CUtlVectorI6Vector10CUtlMemoryIS1_iEELi3EE7RestoreERK22SaveRestoreFieldInfo_tP8IRestore
void CUtlVectorDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> >,3>::Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
{
	datamap_t *pArrayTypeDatamap;  // line 84
	typedescription_t dataDesc;  // line 85
	datamap_t dataMap;  // line 99
	CUtlVector<Vector,CUtlMemory<Vector, int> > *pUtlVector;  // line 112
	int nElems;  // line 114
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::SetCount() at line 116
}

// game/server/portal2/prop_personality_sphere.cpp:84 @0x766b50 _ZN28CInfoTargetPersonalitySphere5SpawnEv
void CInfoTargetPersonalitySphere::Spawn()
{
}

// game/server/portal2/prop_personality_sphere.cpp:85 @0x766b20 _ZN28CInfoTargetPersonalitySphere8PrecacheEv
void CInfoTargetPersonalitySphere::Precache()
{
	// inlined string_t::ToCStr() at line 85
}

// game/server/portal2/prop_personality_sphere.cpp:92
static CEntityFactory<CInfoTargetPersonalitySphere> info_target_personality_sphere;

// game/server/portal2/prop_personality_sphere.cpp:93 @0xb7c40 _Z11DataMapInitI28CInfoTargetPersonalitySphereEP9datamap_tPT_
datamap_t *DataMapInit<CInfoTargetPersonalitySphere>( CInfoTargetPersonalitySphere * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 93
	CDatadescGeneratedNameHolder nameHolder;  // line 93
	typedescription_t dataDesc[3];  // line 93
}

// game/server/portal2/prop_personality_sphere.cpp:93
datamap_t *g_DataMapHolder;

// game/server/util.h:100 @0x766d40 _ZN14CEntityFactoryI28CInfoTargetPersonalitySphereE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfoTargetPersonalitySphere>::Create( const char *pClassName )
{
	CInfoTargetPersonalitySphere *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CInfoTargetPersonalitySphere>() at line 102
}

// game/server/util.h:100 @0x7672d0 _ZN14CEntityFactoryI22CPropPersonalitySphereE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropPersonalitySphere>::Create( const char *pClassName )
{
	CPropPersonalitySphere *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropPersonalitySphere>() at line 102
}

// game/server/portal2/prop_personality_sphere.cpp:101 sizeof=0x14 (i386)
struct CInfoTargetEntityEnumerator : public IEntityEnumerator
{
public:
	CInfoTargetEntityEnumerator( const Ray_t & );  // line 103
	virtual bool EnumEntity( IHandleEntity * );  // line 113
	CBaseEntity *GetBestInfoTargetEntity();  // line 150
private:
	CBaseEntity *m_pBestEnt; // +0x4  // line 153
	const Ray_t &m_ray; // +0x8  // line 154
	float m_flClosestDist; // +0xc  // line 155
	float m_flLargestDot; // +0x10  // line 156
};

// game/server/util.h:106 @0x766aa0 _ZN14CEntityFactoryI22CPropPersonalitySphereE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropPersonalitySphere>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x766ae0 _ZN14CEntityFactoryI28CInfoTargetPersonalitySphereE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfoTargetPersonalitySphere>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_personality_sphere.cpp:113 @0x766db0 _ZN27CInfoTargetEntityEnumerator10EnumEntityEP13IHandleEntity
bool CInfoTargetEntityEnumerator::EnumEntity( IHandleEntity *pHandleEntity )
{
	CBaseEntity *pEnt;  // line 115
	{
		Vector vDirToEnt;  // line 125
		float flDist;  // line 126
		Vector vRayDir;  // line 128
		float flDot;  // line 130
		float flToleranceAngle;  // line 132
		float flTargetAngle;  // line 133
		// inlined Vector::Dot() at line 130
		// inlined Vector::operator-() at line 125
		// inlined CBaseEntity::GetAbsOrigin() at line 125
	}
	// inlined FClassnameIs() at line 117
	// inlined CBaseHandle::CBaseHandle() at line 115
	// inlined CGlobalEntityList::GetBaseEntity() at line 115
}

// game/server/util.h:114 @0x766ad0 _ZN14CEntityFactoryI22CPropPersonalitySphereE13GetEntitySizeEv
size_t CEntityFactory<CPropPersonalitySphere>::GetEntitySize()
{
}

// game/server/util.h:114 @0x766b10 _ZN14CEntityFactoryI28CInfoTargetPersonalitySphereE13GetEntitySizeEv
size_t CEntityFactory<CInfoTargetPersonalitySphere>::GetEntitySize()
{
}

// game/shared/saverestore_utlvector.h:135 @0x767100 _ZN17CUtlVectorDataOpsI10CUtlVectorI6Vector10CUtlMemoryIS1_iEELi3EE9MakeEmptyERK22SaveRestoreFieldInfo_t
void CUtlVectorDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> >,3>::MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
	CUtlVector<Vector,CUtlMemory<Vector, int> > *pUtlVector;  // line 137
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::SetCount() at line 138
}

// game/shared/saverestore_utlvector.h:141 @0x766d20 _ZN17CUtlVectorDataOpsI10CUtlVectorI6Vector10CUtlMemoryIS1_iEELi3EE7IsEmptyERK22SaveRestoreFieldInfo_t
bool CUtlVectorDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> >,3>::IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
{
}

// game/shared/saverestore_utlvector.h:156 @0x766bc0 _ZN29CUtlVectorDataopsInstantiatorILi3EE10GetDataOpsI10CUtlVectorI6Vector10CUtlMemoryIS3_iEEEEP15ISaveRestoreOpsPT_
ISaveRestoreOps *CUtlVectorDataopsInstantiator<3>::GetDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> > >( CUtlVector<Vector,CUtlMemory<Vector, int> > * )
{
	// inlined CUtlVectorDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> >,3>::CUtlVectorDataOps() at line 158
}

// game/server/portal2/prop_personality_sphere.cpp:161 sizeof=0x7ec (i386)
struct CPropPersonalitySphere : public CPhysicsProp
{
public:
	CPropPersonalitySphere();  // line 166
	virtual ~CPropPersonalitySphere();  // line 167
	// game/server/portal2/prop_personality_sphere.cpp:169
	enum $_367
	{
		CORETYPE_CURIOUS = 0,
		CORETYPE_AGGRESSIVE = 1,
		CORETYPE_CRAZY = 2,
		CORETYPE_RICHARD = 3,
		CORETYPE_AQUARIUM = 4,
		CORETYPE_NONE = 5,
		CORETYPE_TOTAL = 6,
	};  // line 169
	virtual void Spawn();  // line 181
	virtual void Precache();  // line 182
	void RegisterSoundEvent( SphereEvent_e, int );  // line 183
	virtual QAngle PreferredCarryAngles();  // line 185
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer * );  // line 186
	void InputPanic( inputdata_t & );  // line 189
	void InputStartTalking( inputdata_t & );  // line 190
	void StartTalking( float );  // line 191
	void CheckForInfoTargets();  // line 193
	void UpdatePositions();  // line 194
	void TalkingThink();  // line 196
	void AnimateThink();  // line 197
	void SetupVOList();  // line 199
	void TrySpeakLine( int, int );  // line 201
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 203
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 204
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 206
	virtual void NotifySystemEvent( CBaseEntity *, notify_system_event_t, const notify_system_event_params_t & );  // line 208
	virtual int ObjectCaps();  // line 210
private:
	bool m_bHasEvent[15]; // +0x766  // line 217
	int m_iEyeballAttachment; // +0x778  // line 218
	float m_flBetweenVOPadding; // +0x77c  // line 219
	bool m_bFirstPickup; // +0x780  // line 220
	bool m_bHeld; // +0x781  // line 221
	bool m_bPanicked; // +0x782  // line 222
	float m_flFirstNoSkipTime; // +0x784  // line 223
	float m_flLineCompletionTime; // +0x788  // line 224
	float m_flInfoTargetTime; // +0x78c  // line 225
	float m_flIdleWaitTime; // +0x790  // line 226
	float m_flLastHeldTime; // +0x794  // line 228
	float m_flAbuseLevel; // +0x798  // line 230
	CUtlVector<Vector,CUtlMemory<Vector, int> > m_pastPositions; // +0x79c  // line 232
	CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> > m_iLineOrder; // +0x7b0  // line 234
	int m_iCurrentLine; // +0x7c4  // line 236
	int m_iCurrentPriority; // +0x7c8  // line 237
	int m_iPendingLine; // +0x7cc  // line 239
	int m_iPendingPriority; // +0x7d0  // line 240
	string_t m_iszInfoTargetScriptName; // +0x7d4  // line 242
	string_t m_iszLastLinePlayed; // +0x7d8  // line 243
	string_t m_iszDeathSoundScriptName; // +0x7dc  // line 245
	string_t m_iszLookAnimationName; // +0x7e0  // line 246
	string_t m_iszVoiceName; // +0x7e4  // line 247
	CPropPersonalitySphere::$_367 m_iCoreType; // +0x7e8  // line 249
};

// game/server/portal2/prop_personality_sphere.cpp:164 @0x763230 _ZN22CPropPersonalitySphere14GetDataDescMapEv
datamap_t *CPropPersonalitySphere::GetDataDescMap()
{
}

// game/server/portal2/prop_personality_sphere.cpp:164 @0x763240 _ZN22CPropPersonalitySphere10GetBaseMapEv
datamap_t *CPropPersonalitySphere::GetBaseMap()
{
}

// game/server/portal2/prop_personality_sphere.cpp:166 @0x765270 _ZN22CPropPersonalitySphereC2Ev
CPropPersonalitySphere::CPropPersonalitySphere()
{
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::CUtlVector() at line 291
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::CUtlVector() at line 291
}

// game/server/portal2/prop_personality_sphere.cpp:166 @0x765390 _ZN22CPropPersonalitySphereC1Ev
CPropPersonalitySphere::CPropPersonalitySphere()
{
}

// game/server/portal2/prop_personality_sphere.cpp:167 @0x764eb0 _ZN22CPropPersonalitySphereD0Ev
CPropPersonalitySphere::~CPropPersonalitySphere()
{
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
}

// game/server/portal2/prop_personality_sphere.cpp:167 @0x7653a0 _ZN22CPropPersonalitySphereD1Ev
CPropPersonalitySphere::~CPropPersonalitySphere()
{
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
}

// game/server/portal2/prop_personality_sphere.cpp:167 @0x765760 _ZN22CPropPersonalitySphereD2Ev
CPropPersonalitySphere::~CPropPersonalitySphere()
{
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 305
}

// game/server/portal2/prop_personality_sphere.cpp:181 @0x764320 _ZN22CPropPersonalitySphere5SpawnEv
void CPropPersonalitySphere::Spawn()
{
	// inlined CBaseAnimating::SetCycle() at line 315
	// inlined CPropPersonalitySphere::StartTalking() at line 325
}

// game/server/portal2/prop_personality_sphere.cpp:182 @0x7666a0 _ZN22CPropPersonalitySphere8PrecacheEv
void CPropPersonalitySphere::Precache()
{
	{
		unsigned int i;  // line 360
		// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::AddToTail() at line 363
	}
}

// game/server/portal2/prop_personality_sphere.cpp:183 @0x7639b0 _ZN22CPropPersonalitySphere18RegisterSoundEventE13SphereEvent_ei
void CPropPersonalitySphere::RegisterSoundEvent( SphereEvent_e eventName, int count )
{
	// inlined string_t::ToCStr() at line 350
	{
		int i;  // line 337
		{
			int iSwapIdx;  // line 343
			// inlined CUtlVector<int,CUtlMemory<int, int> >::operator[]() at line 344
			// inlined CUtlVector<int,CUtlMemory<int, int> >::operator[]() at line 344
			// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::operator[]() at line 342
			// inlined CUtlVector<int,CUtlMemory<int, int> >::AddToTail() at line 342
			// inlined string_t::ToCStr() at line 340
		}
	}
}

// game/server/portal2/prop_personality_sphere.cpp:185 @0x766a50 _ZN22CPropPersonalitySphere20PreferredCarryAnglesEv
QAngle CPropPersonalitySphere::PreferredCarryAngles()
{
	// inlined QAngle::QAngle() at line 185
}

// game/server/portal2/prop_personality_sphere.cpp:186 @0x766a90 _ZN22CPropPersonalitySphere32HasPreferredCarryAnglesForPlayerEP11CBasePlayer
bool CPropPersonalitySphere::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
}

// game/server/portal2/prop_personality_sphere.cpp:189 @0x763250 _ZN22CPropPersonalitySphere10InputPanicER11inputdata_t
void CPropPersonalitySphere::InputPanic( inputdata_t &inputdata )
{
}

// game/server/portal2/prop_personality_sphere.cpp:190 @0x7642a0 _ZN22CPropPersonalitySphere17InputStartTalkingER11inputdata_t
void CPropPersonalitySphere::InputStartTalking( inputdata_t &inputdata )
{
	// inlined CPropPersonalitySphere::StartTalking() at line 407
}

// game/server/portal2/prop_personality_sphere.cpp:191 @0x763300 _ZN22CPropPersonalitySphere12StartTalkingEf
void CPropPersonalitySphere::StartTalking( float flDelay )
{
}

// game/server/portal2/prop_personality_sphere.cpp:193 @0x764740 _ZN22CPropPersonalitySphere19CheckForInfoTargetsEv
void CPropPersonalitySphere::CheckForInfoTargets()
{
	CPortal_Player *pPlayer;  // line 470
	Vector vecStart;  // line 477
	Vector vecForward;  // line 478
	Vector vecRight;  // line 478
	Vector vecUp;  // line 478
	// inlined AI_GetSinglePlayer() at line 470
	// inlined Vector::operator VectorByValue&() at line 477
	{
		Ray_t ray;  // line 482
		CInfoTargetEntityEnumerator InfoTargetEnum;  // line 485
		trace_t tr;  // line 488
		CBaseEntity *ppEnts[256];  // line 491
		int nEntCount;  // line 492
		int i;  // line 494
		CInfoTargetPersonalitySphere *pEnt;  // line 503
		// inlined operator*() at line 483
		// inlined Vector::operator+() at line 483
		// inlined Ray_t::Init() at line 483
		// inlined UTIL_TraceRay() at line 489
		// inlined UTIL_EntitiesInSphere() at line 492
		// inlined CInfoTargetEntityEnumerator::EnumEntity() at line 500
	}
}

// game/server/portal2/prop_personality_sphere.cpp:194 @0x763e80 _ZN22CPropPersonalitySphere15UpdatePositionsEv
void CPropPersonalitySphere::UpdatePositions()
{
	CPortal_Player *pPlayer;  // line 521
	float flBestFit;  // line 527
	int iBestLine;  // line 528
	Vector vecVelocity;  // line 554
	// inlined Vector::Length() at line 589
	// inlined CBaseEntity::GetAbsVelocity() at line 589
	// inlined CBaseEntity::GetAbsVelocity() at line 584
	// inlined CBaseEntity::GetAbsVelocity() at line 579
	// inlined Vector::operator/=() at line 561
	{
		int i;  // line 555
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::operator[]() at line 557
		// inlined Vector::operator-() at line 557
		// inlined Vector::operator+=() at line 557
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Count() at line 555
	}
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Count() at line 532
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 531
	// inlined CBaseEntity::GetAbsOrigin() at line 531
	// inlined AI_GetSinglePlayer() at line 521
	// inlined Vector::Length() at line 567
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::RemoveMultipleFromHead() at line 534
}

// game/server/portal2/prop_personality_sphere.cpp:196 @0x764d00 _ZN22CPropPersonalitySphere12TalkingThinkEv
void CPropPersonalitySphere::TalkingThink()
{
	int iCurSequence;  // line 430
	int iLookSequence;  // line 431
	float flInterval;  // line 437
	// inlined string_t::ToCStr() at line 431
	// inlined CBaseAnimating::GetSequence() at line 430
}

// game/server/portal2/prop_personality_sphere.cpp:197 @0x763280 _ZN22CPropPersonalitySphere12AnimateThinkEv
void CPropPersonalitySphere::AnimateThink()
{
}

// game/server/portal2/prop_personality_sphere.cpp:199 @0x765b20 _ZN22CPropPersonalitySphere11SetupVOListEv
void CPropPersonalitySphere::SetupVOList()
{
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 774
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 755
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 762
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 737
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 768
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 746
}

// game/server/portal2/prop_personality_sphere.cpp:201 @0x763380 _ZN22CPropPersonalitySphere12TrySpeakLineEii
void CPropPersonalitySphere::TrySpeakLine( int line, int priority )
{
	float flFinishTime;  // line 614
	float flAdditionalPadding;  // line 646
	float flCurDuration;  // line 701
	// inlined string_t::ToCStr() at line 688
	{
		int iIndex;  // line 677
		// inlined string_t::ToCStr() at line 684
		// inlined CUtlVector<int,CUtlMemory<int, int> >::AddToTail() at line 682
		// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::operator[]() at line 682
		// inlined CUtlVector<int,CUtlMemory<int, int> >::RemoveMultipleFromHead() at line 681
		// inlined CUtlVector<int,CUtlMemory<int, int> >::Head() at line 680
		// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::operator[]() at line 680
	}
	// inlined string_t::ToCStr() at line 701
	// inlined string_t::ToCStr() at line 693
	// inlined string_t::ToCStr() at line 688
	// inlined string_t::ToCStr() at line 674
}

// game/server/portal2/prop_personality_sphere.cpp:203 @0x764630 _ZN22CPropPersonalitySphere15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CPropPersonalitySphere::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	// inlined CPropPersonalitySphere::StartTalking() at line 791
}

// game/server/portal2/prop_personality_sphere.cpp:204 @0x763940 _ZN22CPropPersonalitySphere13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CPropPersonalitySphere::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
}

// game/server/portal2/prop_personality_sphere.cpp:206 @0x763d70 _ZN22CPropPersonalitySphere17VPhysicsCollisionEiP21gamevcollisionevent_t
void CPropPersonalitySphere::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	CBaseEntity *pHitEntity;  // line 829
	{
		Vector vecVelocity;  // line 843
		// inlined Vector::Length() at line 845
	}
}

// game/server/portal2/prop_personality_sphere.cpp:208 @0x763260 _ZN22CPropPersonalitySphere17NotifySystemEventEP11CBaseEntity21notify_system_event_tRK28notify_system_event_params_t
void CPropPersonalitySphere::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::RemoveAll() at line 867
}

// game/server/portal2/prop_personality_sphere.cpp:210 @0x766ba0 _ZN22CPropPersonalitySphere10ObjectCapsEv
int CPropPersonalitySphere::ObjectCaps()
{
}

// game/server/portal2/prop_personality_sphere.cpp:252
static CEntityFactory<CPropPersonalitySphere> prop_personality_sphere;

// game/server/portal2/prop_personality_sphere.cpp:257 @0xb7d10 _Z11DataMapInitI22CPropPersonalitySphereEP9datamap_tPT_
datamap_t *DataMapInit<CPropPersonalitySphere>( CPropPersonalitySphere * )
{
	// inlined CUtlVectorDataopsInstantiator<3>::GetDataOps<CUtlVector<Vector, CUtlMemory<Vector, int> > >() at line 289
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 289
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 289
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 257
	CDatadescGeneratedNameHolder nameHolder;  // line 257
	typedescription_t dataDesc[24];  // line 257
}

// game/server/portal2/prop_personality_sphere.cpp:257
datamap_t *g_DataMapHolder;

// public/tier1/utlvector.h:778 @0x767120 _ZN10CUtlVectorIS_Ii10CUtlMemoryIiiEES0_IS2_iEE12InsertBeforeEi
int CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::InsertBefore( int elem )
{
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::GrowVector() at line 783
	// inlined CUtlVector<CUtlVector<int, CUtlMemory<int, int> >,CUtlMemory<CUtlVector<int, CUtlMemory<int, int> >, int> >::ShiftElementsRight() at line 784
	// inlined Construct<CUtlVector<int, CUtlMemory<int, int> > >() at line 785
}
