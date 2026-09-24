// DWARF declaration skeleton for game/server/portal2/trigger_tractorbeam.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbb790 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 32
	// inlined CEntityFactory<CProjectedTractorBeamEntity>::CEntityFactory() at line 36
	// inlined CEntityFactory<CTrigger_TractorBeam>::CEntityFactory() at line 85
	// inlined ServerClass::ServerClass() at line 87
}

// game/server/portal2/trigger_tractorbeam.cpp:28 (declaration)
void GetBaseMap();

// game/server/portal2/trigger_tractorbeam.cpp:28 @0x7763d0 _ZN27CProjectedTractorBeamEntity14GetDataDescMapEv
datamap_t *CProjectedTractorBeamEntity::GetDataDescMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:28 @0x7763e0 _ZN27CProjectedTractorBeamEntity10GetBaseMapEv
datamap_t *CProjectedTractorBeamEntity::GetBaseMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:28 @0xbb3a0 _Z11DataMapInitI27CProjectedTractorBeamEntityEP9datamap_tPT_
datamap_t *DataMapInit<CProjectedTractorBeamEntity>( CProjectedTractorBeamEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 28
	CDatadescGeneratedNameHolder nameHolder;  // line 28
	typedescription_t dataDesc[2];  // line 28
}

// game/server/portal2/trigger_tractorbeam.cpp:28
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_tractorbeam.cpp:28
void CProjectedTractorBeamEntity::m_DataMap;

// game/server/portal2/trigger_tractorbeam.cpp:32 @0x7763f0 _ZN27CProjectedTractorBeamEntity14GetServerClassEv
ServerClass *CProjectedTractorBeamEntity::GetServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:32 @0x776400 _ZN27CProjectedTractorBeamEntity40YouForgotToImplementOrDeclareServerClassEv
int CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:32 @0xbb1d0 _Z15ServerClassInitIN29DT_ProjectedTractorBeamEntity7ignoredEEiPT_
int ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>( DT_ProjectedTractorBeamEntity::ignored * )
{
	SendTable &sendTable;  // line 32
	char *const g_pSendTableName;  // line 32
	SendProp g_SendProps[3];  // line 32
}

// game/server/portal2/trigger_tractorbeam.cpp:32
SendTable g_SendTable;

// game/server/portal2/trigger_tractorbeam.cpp:32
int g_SendTableInit;

// game/server/portal2/trigger_tractorbeam.cpp:32
static ServerClass g_CProjectedTractorBeamEntity_ClassReg;

// game/server/portal2/trigger_tractorbeam.cpp:32
void CProjectedTractorBeamEntity::m_pClassSendTable;

// game/server/portal2/trigger_tractorbeam.cpp:36
static CEntityFactory<CProjectedTractorBeamEntity> projected_tractor_beam_entity;

// game/server/portal2/trigger_tractorbeam.cpp:41 (declaration)
void GetBaseMap();

// game/server/portal2/trigger_tractorbeam.cpp:41 @0x776410 _ZN20CTrigger_TractorBeam14GetDataDescMapEv
datamap_t *CTrigger_TractorBeam::GetDataDescMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:41 @0x776420 _ZN20CTrigger_TractorBeam10GetBaseMapEv
datamap_t *CTrigger_TractorBeam::GetBaseMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:41 @0xbb470 _Z11DataMapInitI20CTrigger_TractorBeamEP9datamap_tPT_
datamap_t *DataMapInit<CTrigger_TractorBeam>( CTrigger_TractorBeam * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 83
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 41
	CDatadescGeneratedNameHolder nameHolder;  // line 41
	typedescription_t dataDesc[33];  // line 41
}

// game/server/portal2/trigger_tractorbeam.cpp:41
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_tractorbeam.cpp:41
void CTrigger_TractorBeam::m_DataMap;

// game/server/portal2/trigger_tractorbeam.cpp:85
static CEntityFactory<CTrigger_TractorBeam> trigger_tractorbeam;

// game/server/portal2/trigger_tractorbeam.cpp:87 @0x776430 _ZN20CTrigger_TractorBeam14GetServerClassEv
ServerClass *CTrigger_TractorBeam::GetServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:87 @0x776440 _ZN20CTrigger_TractorBeam40YouForgotToImplementOrDeclareServerClassEv
int CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:87 @0xba940 _Z15ServerClassInitIN22DT_Trigger_TractorBeam7ignoredEEiPT_
int ServerClassInit<DT_Trigger_TractorBeam::ignored>( DT_Trigger_TractorBeam::ignored * )
{
	SendTable &sendTable;  // line 87
	char *const g_pSendTableName;  // line 87
	SendProp g_SendProps[23];  // line 87
}

// game/server/portal2/trigger_tractorbeam.cpp:87
SendTable g_SendTable;

// game/server/portal2/trigger_tractorbeam.cpp:87
int g_SendTableInit;

// game/server/portal2/trigger_tractorbeam.cpp:87
static ServerClass g_CTrigger_TractorBeam_ClassReg;

// game/server/portal2/trigger_tractorbeam.cpp:87
void CTrigger_TractorBeam::m_pClassSendTable;

// game/server/util.h:100 @0x778620 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE6CreateEPKc
IServerNetworkable *CEntityFactory<CProjectedTractorBeamEntity>::Create( const char *pClassName )
{
	CProjectedTractorBeamEntity *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CProjectedTractorBeamEntity>() at line 102
}

// game/server/util.h:100 @0x778cc0 _ZN14CEntityFactoryI20CTrigger_TractorBeamE6CreateEPKc
IServerNetworkable *CEntityFactory<CTrigger_TractorBeam>::Create( const char *pClassName )
{
	CTrigger_TractorBeam *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CTrigger_TractorBeam>() at line 102
}

// game/server/util.h:106 @0x778510 _ZN14CEntityFactoryI20CTrigger_TractorBeamE7DestroyEP18IServerNetworkable
void CEntityFactory<CTrigger_TractorBeam>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x778550 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE7DestroyEP18IServerNetworkable
void CEntityFactory<CProjectedTractorBeamEntity>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/trigger_tractorbeam.cpp:114 (declaration)
void CTrigger_TractorBeam();

// game/server/util.h:114 @0x778540 _ZN14CEntityFactoryI20CTrigger_TractorBeamE13GetEntitySizeEv
size_t CEntityFactory<CTrigger_TractorBeam>::GetEntitySize()
{
}

// game/server/util.h:114 @0x778580 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE13GetEntitySizeEv
size_t CEntityFactory<CProjectedTractorBeamEntity>::GetEntitySize()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:114 @0x777fa0 _ZN20CTrigger_TractorBeamC2Ev
CTrigger_TractorBeam::CTrigger_TractorBeam()
{
	// inlined CTrigger_TractorBeam::SetBeamRadius() at line 118
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::AddToTail() at line 116
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::CNetworkHandleBase() at line 114
	// inlined CBaseVPhysicsTrigger::CBaseVPhysicsTrigger() at line 114
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 119
}

// game/server/portal2/trigger_tractorbeam.cpp:114 @0x778200 _ZN20CTrigger_TractorBeamC1Ev
CTrigger_TractorBeam::CTrigger_TractorBeam()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:121 (declaration)
~CTrigger_TractorBeam();

// game/server/portal2/trigger_tractorbeam.cpp:121 @0x778210 _ZN20CTrigger_TractorBeamD0Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 123
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
}

// game/server/portal2/trigger_tractorbeam.cpp:121 @0x778310 _ZN20CTrigger_TractorBeamD1Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 123
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
}

// game/server/portal2/trigger_tractorbeam.cpp:121 @0x778410 _ZN20CTrigger_TractorBeamD2Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 123
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 124
}

// game/server/portal2/trigger_tractorbeam.cpp:130 @0x776bf0 _ZN20CTrigger_TractorBeam5SpawnEv
void CTrigger_TractorBeam::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 134
	// inlined CBaseEntity::AddSolidFlags() at line 135
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bDisablePlayerMove>::operator=<bool>() at line 146
}

// game/server/portal2/trigger_tractorbeam.cpp:152 @0x7767c0 _ZN20CTrigger_TractorBeam8PrecacheEv
void CTrigger_TractorBeam::Precache()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:163 @0x7779c0 _ZN20CTrigger_TractorBeam8ActivateEv
void CTrigger_TractorBeam::Activate()
{
	CSoundEnvelopeController &controller;  // line 167
	CPASFilter filter;  // line 168
	// inlined CPASFilter::~CPASFilter() at line 171
	// inlined CBaseEntity::entindex() at line 170
	// inlined CPASFilter::CPASFilter() at line 168
	// inlined CBaseEntity::GetAbsOrigin() at line 168
	// inlined CPASFilter::~CPASFilter() at line 171
}

// game/server/portal2/trigger_tractorbeam.cpp:178 @0x776e20 _ZN20CTrigger_TractorBeam12TractorThinkEv
void CTrigger_TractorBeam::TractorThink()
{
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::Get() at line 180
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bFromPortal>::operator=<bool>() at line 183
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::operator->() at line 184
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bToPortal>::operator=<bool>() at line 184
}

// game/server/portal2/trigger_tractorbeam.cpp:200 @0x776450 _ZN20CTrigger_TractorBeam14CreateVPhysicsEv
bool CTrigger_TractorBeam::CreateVPhysics()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:210 @0x776800 _ZN20CTrigger_TractorBeam19WakeTouchingObjectsEv
void CTrigger_TractorBeam::WakeTouchingObjects()
{
	CBaseEntity *list[256];  // line 212
	CBaseEntity *pEntity;  // line 213
	Ray_t ray;  // line 215
	Vector vExtents;  // line 216
	int nNumFound;  // line 218
	// inlined Vector::operator-() at line 217
	// inlined Ray_t::Init() at line 217
	// inlined UTIL_EntitiesAlongRay() at line 218
	{
		int i;  // line 219
		{
			IPhysicsObject *pPhysObj;  // line 232
			// inlined CBaseEntity::VPhysicsGetObject() at line 232
			{
				CPropWeightedCube *pReflectiveCube;  // line 228
			}
		}
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:243 @0x7766e0 _ZN20CTrigger_TractorBeam14UpdateOnRemoveEv
void CTrigger_TractorBeam::UpdateOnRemove()
{
	{
		const int nNumObjects;  // line 248
		IPhysicsObject **pObjects;  // line 249
		{
			int i;  // line 252
		}
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:277 @0x776690 _ZN20CTrigger_TractorBeam9OnRestoreEv
void CTrigger_TractorBeam::OnRestore()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:291 @0x776600 _ZN20CTrigger_TractorBeam17StopLoopingSoundsEv
void CTrigger_TractorBeam::StopLoopingSounds()
{
	{
		CSoundEnvelopeController &controller;  // line 296
	}
	{
		CSoundEnvelopeController &controller;  // line 305
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:318 @0x777af0 _ZN20CTrigger_TractorBeam10StartTouchEP11CBaseEntity
void CTrigger_TractorBeam::StartTouch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 335
	triggerevent_t event;  // line 365
	{
		CSoundEnvelopeController &controller;  // line 342
		EmitSound_t ep;  // line 350
		CPASAttenuationFilter filter;  // line 356
		// inlined EmitSound_t::~EmitSound_t() at line 362
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 362
		// inlined CBaseEntity::entindex() at line 361
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 356
		// inlined CBaseEntity::GetAbsOrigin() at line 356
		// inlined EmitSound_t::EmitSound_t() at line 350
		// inlined CBaseEntity::SetGravity() at line 340
		// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bSlowMovement>::operator=<bool>() at line 339
		// inlined EmitSound_t::~EmitSound_t() at line 362
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 362
	}
	// inlined ToBasePlayer() at line 335
	{
		CNPC_Portal_FloorTurret *pTurret;  // line 327
	}
	// inlined FClassnameIs() at line 325
}

// game/server/portal2/trigger_tractorbeam.cpp:377 @0x776a40 _ZN20CTrigger_TractorBeam8EndTouchEP11CBaseEntity
void CTrigger_TractorBeam::EndTouch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 394
	triggerevent_t event;  // line 407
	// inlined CBaseEntity::SetGravity() at line 399
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bSlowMovement>::operator=<bool>() at line 398
	// inlined ToBasePlayer() at line 394
	{
		CNPC_Portal_FloorTurret *pTurret;  // line 386
	}
	// inlined FClassnameIs() at line 384
}

// game/server/portal2/trigger_tractorbeam.cpp:419 @0x777210 _ZN20CTrigger_TractorBeam25InputSetVelocityLimitTimeER11inputdata_t
void CTrigger_TractorBeam::InputSetVelocityLimitTime( inputdata_t &inputdata )
{
	float args[2];  // line 424
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitTime>::operator=<float>() at line 427
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimit>::operator=<float>() at line 426
	// inlined variant_t::String() at line 425
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitStartTime>::operator=<float>() at line 422
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitStart>::operator=<float>() at line 421
}

// game/server/portal2/trigger_tractorbeam.cpp:433 (declaration)
void CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner );

// game/server/portal2/trigger_tractorbeam.cpp:433 @0x776590 _ZN20CTrigger_TractorBeam17CreateTractorBeamERK6VectorS2_P11CBaseEntity
CTrigger_TractorBeam *CTrigger_TractorBeam::CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner )
{
	CTrigger_TractorBeam *pBeam;  // line 436
}

// game/server/portal2/trigger_tractorbeam.cpp:450 (declaration)
void CreateNewInstance();

// game/server/portal2/trigger_tractorbeam.cpp:450 @0x776550 _ZN27CProjectedTractorBeamEntity17CreateNewInstanceEv
CProjectedTractorBeamEntity *CProjectedTractorBeamEntity::CreateNewInstance()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:455 @0x776bb0 _ZN27CProjectedTractorBeamEntity24CreateNewProjectedEntityEv
CBaseProjectedEntity *CProjectedTractorBeamEntity::CreateNewProjectedEntity()
{
	// inlined CProjectedTractorBeamEntity::CreateNewInstance() at line 457
}

// game/server/portal2/trigger_tractorbeam.cpp:460 @0x7764a0 _ZN27CProjectedTractorBeamEntity11OnProjectedEv
void CProjectedTractorBeamEntity::OnProjected()
{
	CTrigger_TractorBeam *pBeam;  // line 464
	// inlined CBaseEntity::GetSimulatingPlayer() at line 469
	// inlined CNetworkHandleBase<CTrigger_TractorBeam,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>::Get() at line 464
}

// public/tier1/utlvector.h:655 @0x778790 _ZN10CUtlVectorIP20CTrigger_TractorBeam10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CTrigger_TractorBeam*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CTrigger_TractorBeam*,int>::Grow() at line 660
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::ResetDbgInfo() at line 664
}
