// DWARF declaration skeleton for game/server/portal2/trigger_tractorbeam.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xd0960 _Z41__static_initialization_and_destruction_0ii
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
	// inlined ServerClass::ServerClass() at line 28
	// inlined CEntityFactory<CProjectedTractorBeamEntity>::CEntityFactory() at line 32
	// inlined CEntityFactory<CTrigger_TractorBeam>::CEntityFactory() at line 81
	// inlined ServerClass::ServerClass() at line 83
}

// game/server/portal2/trigger_tractorbeam.cpp:24 (declaration)
void GetBaseMap();

// game/server/portal2/trigger_tractorbeam.cpp:24 @0x6fb000 _ZN27CProjectedTractorBeamEntity14GetDataDescMapEv
datamap_t *CProjectedTractorBeamEntity::GetDataDescMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:24 @0x6fb010 _ZN27CProjectedTractorBeamEntity10GetBaseMapEv
datamap_t *CProjectedTractorBeamEntity::GetBaseMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:24 @0xd0650 _Z11DataMapInitI27CProjectedTractorBeamEntityEP9datamap_tPT_
datamap_t *DataMapInit<CProjectedTractorBeamEntity>( CProjectedTractorBeamEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 24
	CDatadescGeneratedNameHolder nameHolder;  // line 24
	typedescription_t dataDesc[2];  // line 24
}

// game/server/portal2/trigger_tractorbeam.cpp:24
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_tractorbeam.cpp:24
void CProjectedTractorBeamEntity::m_DataMap;

// game/server/portal2/trigger_tractorbeam.cpp:28 @0x6fb020 _ZN27CProjectedTractorBeamEntity14GetServerClassEv
ServerClass *CProjectedTractorBeamEntity::GetServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:28 @0x6fb030 _ZN27CProjectedTractorBeamEntity40YouForgotToImplementOrDeclareServerClassEv
int CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:28 @0xd0480 _Z15ServerClassInitIN29DT_ProjectedTractorBeamEntity7ignoredEEiPT_
int ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>( DT_ProjectedTractorBeamEntity::ignored * )
{
	SendTable &sendTable;  // line 28
	char *const g_pSendTableName;  // line 28
	SendProp g_SendProps[3];  // line 28
}

// game/server/portal2/trigger_tractorbeam.cpp:28
SendTable g_SendTable;

// game/server/portal2/trigger_tractorbeam.cpp:28
int g_SendTableInit;

// game/server/portal2/trigger_tractorbeam.cpp:28
static ServerClass g_CProjectedTractorBeamEntity_ClassReg;

// game/server/portal2/trigger_tractorbeam.cpp:28
void CProjectedTractorBeamEntity::m_pClassSendTable;

// game/server/portal2/trigger_tractorbeam.cpp:32
static CEntityFactory<CProjectedTractorBeamEntity> projected_tractor_beam_entity;

// game/server/portal2/trigger_tractorbeam.cpp:37 (declaration)
void GetBaseMap();

// game/server/portal2/trigger_tractorbeam.cpp:37 @0x6fb040 _ZN20CTrigger_TractorBeam14GetDataDescMapEv
datamap_t *CTrigger_TractorBeam::GetDataDescMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:37 @0x6fb050 _ZN20CTrigger_TractorBeam10GetBaseMapEv
datamap_t *CTrigger_TractorBeam::GetBaseMap()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:37 @0xd0720 _Z11DataMapInitI20CTrigger_TractorBeamEP9datamap_tPT_
datamap_t *DataMapInit<CTrigger_TractorBeam>( CTrigger_TractorBeam * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 79
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 37
	CDatadescGeneratedNameHolder nameHolder;  // line 37
	typedescription_t dataDesc[33];  // line 37
}

// game/server/portal2/trigger_tractorbeam.cpp:37
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_tractorbeam.cpp:37
void CTrigger_TractorBeam::m_DataMap;

// game/server/portal2/trigger_tractorbeam.cpp:81
static CEntityFactory<CTrigger_TractorBeam> trigger_tractorbeam;

// game/server/portal2/trigger_tractorbeam.cpp:83 @0x6fb060 _ZN20CTrigger_TractorBeam14GetServerClassEv
ServerClass *CTrigger_TractorBeam::GetServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:83 @0x6fb070 _ZN20CTrigger_TractorBeam40YouForgotToImplementOrDeclareServerClassEv
int CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:83 @0xcfbf0 _Z15ServerClassInitIN22DT_Trigger_TractorBeam7ignoredEEiPT_
int ServerClassInit<DT_Trigger_TractorBeam::ignored>( DT_Trigger_TractorBeam::ignored * )
{
	SendTable &sendTable;  // line 83
	char *const g_pSendTableName;  // line 83
	SendProp g_SendProps[23];  // line 83
}

// game/server/portal2/trigger_tractorbeam.cpp:83
SendTable g_SendTable;

// game/server/portal2/trigger_tractorbeam.cpp:83
int g_SendTableInit;

// game/server/portal2/trigger_tractorbeam.cpp:83
static ServerClass g_CTrigger_TractorBeam_ClassReg;

// game/server/portal2/trigger_tractorbeam.cpp:83
void CTrigger_TractorBeam::m_pClassSendTable;

// game/server/util.h:99 @0x6fc7d0 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE6CreateEPKc
IServerNetworkable *CEntityFactory<CProjectedTractorBeamEntity>::Create( const char *pClassName )
{
	CProjectedTractorBeamEntity *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CProjectedTractorBeamEntity>() at line 101
}

// game/server/util.h:99 @0x6fcc00 _ZN14CEntityFactoryI20CTrigger_TractorBeamE6CreateEPKc
IServerNetworkable *CEntityFactory<CTrigger_TractorBeam>::Create( const char *pClassName )
{
	CTrigger_TractorBeam *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CTrigger_TractorBeam>() at line 101
}

// game/server/util.h:105 @0x6fc6c0 _ZN14CEntityFactoryI20CTrigger_TractorBeamE7DestroyEP18IServerNetworkable
void CEntityFactory<CTrigger_TractorBeam>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x6fc700 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE7DestroyEP18IServerNetworkable
void CEntityFactory<CProjectedTractorBeamEntity>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/trigger_tractorbeam.cpp:110 (declaration)
void CTrigger_TractorBeam();

// game/server/portal2/trigger_tractorbeam.cpp:110 @0x6fc250 _ZN20CTrigger_TractorBeamC2Ev
CTrigger_TractorBeam::CTrigger_TractorBeam()
{
	// inlined CTrigger_TractorBeam::SetBeamRadius() at line 114
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::AddToTail() at line 112
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::CNetworkHandleBase() at line 110
	// inlined CBaseVPhysicsTrigger::CBaseVPhysicsTrigger() at line 110
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 115
}

// game/server/portal2/trigger_tractorbeam.cpp:110 @0x6fc3b0 _ZN20CTrigger_TractorBeamC1Ev
CTrigger_TractorBeam::CTrigger_TractorBeam()
{
}

// game/server/util.h:113 @0x6fc6f0 _ZN14CEntityFactoryI20CTrigger_TractorBeamE13GetEntitySizeEv
size_t CEntityFactory<CTrigger_TractorBeam>::GetEntitySize()
{
}

// game/server/util.h:113 @0x6fc730 _ZN14CEntityFactoryI27CProjectedTractorBeamEntityE13GetEntitySizeEv
size_t CEntityFactory<CProjectedTractorBeamEntity>::GetEntitySize()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:117 (declaration)
~CTrigger_TractorBeam();

// game/server/portal2/trigger_tractorbeam.cpp:117 @0x6fc3c0 _ZN20CTrigger_TractorBeamD2Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 119
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
}

// game/server/portal2/trigger_tractorbeam.cpp:117 @0x6fc4c0 _ZN20CTrigger_TractorBeamD0Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 119
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
}

// game/server/portal2/trigger_tractorbeam.cpp:117 @0x6fc5c0 _ZN20CTrigger_TractorBeamD1Ev
CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::FindAndRemove() at line 119
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
	// inlined CBaseVPhysicsTrigger::~CBaseVPhysicsTrigger() at line 120
}

// game/server/portal2/trigger_tractorbeam.cpp:126 @0x6fb810 _ZN20CTrigger_TractorBeam5SpawnEv
void CTrigger_TractorBeam::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 130
	// inlined CBaseEntity::AddSolidFlags() at line 131
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bDisablePlayerMove>::operator=<bool>() at line 142
}

// game/server/portal2/trigger_tractorbeam.cpp:148 @0x6fb3f0 _ZN20CTrigger_TractorBeam8PrecacheEv
void CTrigger_TractorBeam::Precache()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:159 @0x6fbca0 _ZN20CTrigger_TractorBeam8ActivateEv
void CTrigger_TractorBeam::Activate()
{
	CSoundEnvelopeController &controller;  // line 163
	CPASFilter filter;  // line 164
	// inlined CPASFilter::~CPASFilter() at line 167
	// inlined CBaseEntity::entindex() at line 166
	// inlined CPASFilter::CPASFilter() at line 164
	// inlined CBaseEntity::GetAbsOrigin() at line 164
	// inlined CPASFilter::~CPASFilter() at line 167
}

// game/server/portal2/trigger_tractorbeam.cpp:174 @0x6fb940 _ZN20CTrigger_TractorBeam12TractorThinkEv
void CTrigger_TractorBeam::TractorThink()
{
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::Get() at line 176
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bFromPortal>::operator=<bool>() at line 179
	// inlined CNetworkHandleBase<CProjectedTractorBeamEntity,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>::operator->() at line 180
	// inlined CNetworkVarBase<bool,CTrigger_TractorBeam::NetworkVar_m_bToPortal>::operator=<bool>() at line 180
}

// game/server/portal2/trigger_tractorbeam.cpp:196 @0x6fb080 _ZN20CTrigger_TractorBeam14CreateVPhysicsEv
bool CTrigger_TractorBeam::CreateVPhysics()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:206 @0x6fb430 _ZN20CTrigger_TractorBeam19WakeTouchingObjectsEv
void CTrigger_TractorBeam::WakeTouchingObjects()
{
	CBaseEntity *list[256];  // line 208
	CBaseEntity *pEntity;  // line 209
	Ray_t ray;  // line 211
	Vector vExtents;  // line 212
	int nNumFound;  // line 214
	// inlined Vector::operator-() at line 213
	// inlined CTrigger_TractorBeam::GetEndPoint() at line 213
	// inlined CTrigger_TractorBeam::GetStartPoint() at line 213
	// inlined Ray_t::Init() at line 213
	// inlined UTIL_EntitiesAlongRay() at line 214
	{
		int i;  // line 215
		{
			IPhysicsObject *pPhysObj;  // line 222
			// inlined CBaseEntity::VPhysicsGetObject() at line 222
		}
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:233 @0x6fb310 _ZN20CTrigger_TractorBeam14UpdateOnRemoveEv
void CTrigger_TractorBeam::UpdateOnRemove()
{
	{
		const int nNumObjects;  // line 238
		IPhysicsObject **pObjects;  // line 239
		{
			int i;  // line 242
		}
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:267 @0x6fb2c0 _ZN20CTrigger_TractorBeam9OnRestoreEv
void CTrigger_TractorBeam::OnRestore()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:281 @0x6fb230 _ZN20CTrigger_TractorBeam17StopLoopingSoundsEv
void CTrigger_TractorBeam::StopLoopingSounds()
{
	{
		CSoundEnvelopeController &controller;  // line 286
	}
	{
		CSoundEnvelopeController &controller;  // line 295
	}
}

// game/server/portal2/trigger_tractorbeam.cpp:308 @0x6fbdd0 _ZN20CTrigger_TractorBeam10StartTouchEP11CBaseEntity
void CTrigger_TractorBeam::StartTouch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 315
	triggerevent_t event;  // line 345
	{
		CSoundEnvelopeController &controller;  // line 322
		EmitSound_t ep;  // line 330
		CPASAttenuationFilter filter;  // line 336
		// inlined EmitSound_t::~EmitSound_t() at line 342
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 342
		// inlined CBaseEntity::entindex() at line 341
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 336
		// inlined CBaseEntity::GetAbsOrigin() at line 336
		// inlined EmitSound_t::EmitSound_t() at line 330
		// inlined CBaseEntity::SetGravity() at line 320
		// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bSlowMovement>::operator=<bool>() at line 319
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 342
		// inlined EmitSound_t::~EmitSound_t() at line 342
	}
	// inlined ToBasePlayer() at line 315
}

// game/server/portal2/trigger_tractorbeam.cpp:357 @0x6fb6a0 _ZN20CTrigger_TractorBeam8EndTouchEP11CBaseEntity
void CTrigger_TractorBeam::EndTouch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 364
	triggerevent_t event;  // line 377
	// inlined CBaseEntity::SetGravity() at line 369
	// inlined CNetworkVarBase<bool,CPlayerLocalData::NetworkVar_m_bSlowMovement>::operator=<bool>() at line 368
	// inlined ToBasePlayer() at line 364
}

// game/server/portal2/trigger_tractorbeam.cpp:389 @0x6fbaa0 _ZN20CTrigger_TractorBeam25InputSetVelocityLimitTimeER11inputdata_t
void CTrigger_TractorBeam::InputSetVelocityLimitTime( inputdata_t &inputdata )
{
	float args[2];  // line 394
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitTime>::operator=<float>() at line 397
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimit>::operator=<float>() at line 396
	// inlined variant_t::String() at line 395
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitStartTime>::operator=<float>() at line 392
	// inlined CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimitStart>::operator=<float>() at line 391
}

// game/server/portal2/trigger_tractorbeam.cpp:403 (declaration)
void CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner );

// game/server/portal2/trigger_tractorbeam.cpp:403 @0x6fb1c0 _ZN20CTrigger_TractorBeam17CreateTractorBeamERK6VectorS2_P11CBaseEntity
CTrigger_TractorBeam *CTrigger_TractorBeam::CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner )
{
	CTrigger_TractorBeam *pBeam;  // line 406
}

// game/server/portal2/trigger_tractorbeam.cpp:420 (declaration)
void CreateNewInstance();

// game/server/portal2/trigger_tractorbeam.cpp:420 @0x6fb180 _ZN27CProjectedTractorBeamEntity17CreateNewInstanceEv
CProjectedTractorBeamEntity *CProjectedTractorBeamEntity::CreateNewInstance()
{
}

// game/server/portal2/trigger_tractorbeam.cpp:425 @0x6fb7d0 _ZN27CProjectedTractorBeamEntity24CreateNewProjectedEntityEv
CBaseProjectedEntity *CProjectedTractorBeamEntity::CreateNewProjectedEntity()
{
	// inlined CProjectedTractorBeamEntity::CreateNewInstance() at line 427
}

// game/server/portal2/trigger_tractorbeam.cpp:430 @0x6fb0d0 _ZN27CProjectedTractorBeamEntity11OnProjectedEv
void CProjectedTractorBeamEntity::OnProjected()
{
	CTrigger_TractorBeam *pBeam;  // line 434
	// inlined CBaseEntity::GetSimulatingPlayer() at line 439
	// inlined CNetworkHandleBase<CTrigger_TractorBeam,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>::Get() at line 434
}

// public/tier1/utlvector.h:655 @0x6fc940 _ZN10CUtlVectorIP20CTrigger_TractorBeam10CUtlMemoryIS1_iEE10GrowVectorEi
void CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<CTrigger_TractorBeam*,int>::NumAllocated() at line 657
	// inlined CUtlMemory<CTrigger_TractorBeam*,int>::Grow() at line 660
	// inlined CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*, int> >::ResetDbgInfo() at line 664
}
