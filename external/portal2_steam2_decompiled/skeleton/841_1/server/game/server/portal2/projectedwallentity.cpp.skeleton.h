// DWARF declaration skeleton for game/server/portal2/projectedwallentity.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb2890 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CProjectedWallEntity>::CEntityFactory() at line 34
	// inlined ServerClass::ServerClass() at line 54
	// inlined ServerClass::ServerClass() at line 468
	// inlined CTEWallPaintedEvent::CTEWallPaintedEvent() at line 474
}

// game/server/portal2/projectedwallentity.cpp:34
static CEntityFactory<CProjectedWallEntity> projected_wall_entity;

// game/server/portal2/projectedwallentity.cpp:36 (declaration)
void GetBaseMap();

// game/server/portal2/projectedwallentity.cpp:36 @0x748700 _ZN20CProjectedWallEntity14GetDataDescMapEv
datamap_t *CProjectedWallEntity::GetDataDescMap()
{
}

// game/server/portal2/projectedwallentity.cpp:36 @0x748710 _ZN20CProjectedWallEntity10GetBaseMapEv
datamap_t *CProjectedWallEntity::GetBaseMap()
{
}

// game/server/portal2/projectedwallentity.cpp:36 @0xb27c0 _Z11DataMapInitI20CProjectedWallEntityEP9datamap_tPT_
datamap_t *DataMapInit<CProjectedWallEntity>( CProjectedWallEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 36
	CDatadescGeneratedNameHolder nameHolder;  // line 36
	typedescription_t dataDesc[9];  // line 36
}

// game/server/portal2/projectedwallentity.cpp:36
datamap_t *g_DataMapHolder;

// game/server/portal2/projectedwallentity.cpp:36
void CProjectedWallEntity::m_DataMap;

// game/server/portal2/projectedwallentity.cpp:54 @0x748720 _ZN20CProjectedWallEntity14GetServerClassEv
ServerClass *CProjectedWallEntity::GetServerClass()
{
}

// game/server/portal2/projectedwallentity.cpp:54 @0x748730 _ZN20CProjectedWallEntity40YouForgotToImplementOrDeclareServerClassEv
int CProjectedWallEntity::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/projectedwallentity.cpp:54 @0xb22a0 _Z15ServerClassInitIN22DT_ProjectedWallEntity7ignoredEEiPT_
int ServerClassInit<DT_ProjectedWallEntity::ignored>( DT_ProjectedWallEntity::ignored * )
{
	SendTable &sendTable;  // line 54
	char *const g_pSendTableName;  // line 54
	SendProp g_SendProps[12];  // line 54
}

// game/server/portal2/projectedwallentity.cpp:54
SendTable g_SendTable;

// game/server/portal2/projectedwallentity.cpp:54
int g_SendTableInit;

// game/server/portal2/projectedwallentity.cpp:54
static ServerClass g_CProjectedWallEntity_ClassReg;

// game/server/portal2/projectedwallentity.cpp:54
void CProjectedWallEntity::m_pClassSendTable;

// game/server/portal2/projectedwallentity.cpp:72 (declaration)
void CProjectedWallEntity();

// game/server/portal2/projectedwallentity.cpp:72 @0x74a290 _ZN20CProjectedWallEntityC2Ev
CProjectedWallEntity::CProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::CUtlVector() at line 79
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flParticleUpdateTime>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flSegmentLength>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<int,CProjectedWallEntity::NetworkVar_m_nNumSegments>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<bool,CProjectedWallEntity::NetworkVar_m_bIsHorizontal>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flWidth>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flHeight>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flLength>::CNetworkVarBase() at line 79
	// inlined CNetworkVarBase<CHandle<CPortal_Base2D>,CProjectedWallEntity::NetworkVar_m_hColorPortal>::CNetworkVarBase() at line 79
	// inlined CPaintableEntity<CBaseProjectedEntity>::CPaintableEntity() at line 79
	// inlined CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity() at line 81
}

// game/server/portal2/projectedwallentity.cpp:72 @0x74ad90 _ZN20CProjectedWallEntityC1Ev
CProjectedWallEntity::CProjectedWallEntity()
{
}

// game/server/portal2/projectedwallentity.cpp:83 (declaration)
~CProjectedWallEntity();

// game/server/portal2/projectedwallentity.cpp:83 @0x749e70 _ZN20CProjectedWallEntityD0Ev
CProjectedWallEntity::~CProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 89
	// inlined CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity() at line 89
	// inlined CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity() at line 89
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 89
}

// game/server/portal2/projectedwallentity.cpp:83 @0x74a070 _ZN20CProjectedWallEntityD2Ev
CProjectedWallEntity::~CProjectedWallEntity()
{
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 89
	// inlined CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity() at line 89
	// inlined CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity() at line 89
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::~CUtlVector() at line 89
}

// game/server/portal2/projectedwallentity.cpp:83 @0x74a280 _ZN20CProjectedWallEntityD1Ev
CProjectedWallEntity::~CProjectedWallEntity()
{
}

// game/server/portal2/projectedwallentity.cpp:91 @0x749e40 _ZN20CProjectedWallEntity14UpdateOnRemoveEv
void CProjectedWallEntity::UpdateOnRemove()
{
}

// game/server/portal2/projectedwallentity.cpp:97 @0x748af0 _ZN20CProjectedWallEntity8PrecacheEv
void CProjectedWallEntity::Precache()
{
}

// game/server/util.h:100 @0x74cf90 _ZN14CEntityFactoryI20CProjectedWallEntityE6CreateEPKc
IServerNetworkable *CEntityFactory<CProjectedWallEntity>::Create( const char *pClassName )
{
	CProjectedWallEntity *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CProjectedWallEntity>() at line 102
}

// game/server/portal2/projectedwallentity.cpp:102 @0x748a50 _ZN20CProjectedWallEntity5SpawnEv
void CProjectedWallEntity::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 108
	// inlined CBaseEntity::SetSolidFlags() at line 109
}

// game/server/util.h:106 @0x74cd00 _ZN14CEntityFactoryI20CProjectedWallEntityE7DestroyEP18IServerNetworkable
void CEntityFactory<CProjectedWallEntity>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x74cd30 _ZN14CEntityFactoryI20CProjectedWallEntityE13GetEntitySizeEv
size_t CEntityFactory<CProjectedWallEntity>::GetEntitySize()
{
}

// game/server/portal2/projectedwallentity.cpp:121 @0x748a20 _ZN20CProjectedWallEntity9OnRestoreEv
void CProjectedWallEntity::OnRestore()
{
}

// game/server/portal2/projectedwallentity.cpp:134 @0x74ada0 _ZN20CProjectedWallEntity11ProjectWallEv
void CProjectedWallEntity::ProjectWall()
{
	Vector vWallStartPoint;  // line 157
	Vector vWallEndPoint;  // line 158
	Vector vecForward;  // line 161
	Vector vecRight;  // line 161
	Vector vecUp;  // line 161
	solid_t solid;  // line 262
	IPhysicsObject *pWallObj;  // line 266
	Vector vMins;  // line 286
	Vector vMaxs;  // line 286
	Vector vUp;  // line 321
	Vector vRight;  // line 321
	{
		int i;  // line 145
		{
			CBasePlayer *pPlayer;  // line 147
		}
	}
	{
		Vector vScaledRight;  // line 166
		Vector vStart;  // line 167
		Vector vEnd;  // line 168
		Vector vVerts[4];  // line 170
		Vector *pVerts[4];  // line 185
		CPhysConvex *pTempConvex;  // line 186
		// inlined Vector::operator*() at line 166
		// inlined Vector::operator+() at line 171
		// inlined Vector::operator=() at line 171
		// inlined Vector::operator-() at line 172
		// inlined Vector::operator=() at line 172
		// inlined Vector::operator-() at line 173
		// inlined Vector::operator=() at line 173
		// inlined Vector::operator+() at line 174
		// inlined Vector::operator=() at line 174
	}
	{
		float flWallPlanes[24];  // line 203
		Vector vWallPointRight;  // line 210
		Vector vWallPointLeft;  // line 214
		Vector vWallPointUp;  // line 218
		Vector vWallPointDown;  // line 222
		CPolyhedron *pTempPolyhedron;  // line 227
		CPhysConvex *pTempConvex;  // line 234
		// inlined Vector::operator-() at line 224
		// inlined Vector::CopyToArray() at line 223
		// inlined Vector::operator-() at line 223
		// inlined Vector::operator-() at line 222
		// inlined Vector::operator/() at line 222
		// inlined Vector::CopyToArray() at line 219
		// inlined Vector::operator+() at line 218
		// inlined Vector::operator/() at line 218
		// inlined Vector::operator*() at line 218
		// inlined Vector::operator-() at line 216
		// inlined Vector::CopyToArray() at line 215
		// inlined Vector::operator-() at line 215
		// inlined Vector::operator-() at line 214
		// inlined Vector::operator/() at line 214
		// inlined Vector::CopyToArray() at line 211
		// inlined Vector::operator+() at line 210
		// inlined Vector::operator/() at line 210
		// inlined Vector::operator*() at line 210
		// inlined Vector::CopyToArray() at line 207
		// inlined Vector::operator-() at line 205
		// inlined Vector::CopyToArray() at line 204
		// inlined Vector::operator-() at line 204
	}
	// inlined Vector::operator=() at line 287
	// inlined Vector::operator=() at line 287
	// inlined CNetworkVarBase<Vector,CProjectedWallEntity::NetworkVar_m_vWorldSpace_WallMins>::operator=<Vector>() at line 293
	// inlined CNetworkVarBase<Vector,CProjectedWallEntity::NetworkVar_m_vWorldSpace_WallMaxs>::operator=<Vector>() at line 294
	{
		Vector vSetMins;  // line 298
		Vector vSetMaxs;  // line 299
		// inlined Vector::operator-() at line 298
		// inlined Vector::operator-() at line 299
	}
	// inlined Vector::operator-() at line 310
	// inlined Vector::Length() at line 310
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flLength>::operator=<vec_t>() at line 310
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flWidth>::operator=<float>() at line 313
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flHeight>::operator=<float>() at line 314
	// inlined CBaseEntity::GetAbsAngles() at line 322
	// inlined CNetworkVarBase<bool,CProjectedWallEntity::NetworkVar_m_bIsHorizontal>::operator=<bool>() at line 323
	// inlined CNetworkVarBase<int,CProjectedWallEntity::NetworkVar_m_nNumSegments>::operator=<double>() at line 327
	// inlined CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType, int> >::SetCount() at line 329
}

// game/server/portal2/projectedwallentity.cpp:335 @0x749200 _ZN20CProjectedWallEntity11CleanupWallEv
void CProjectedWallEntity::CleanupWall()
{
	// inlined CNetworkVarBase<Vector,CProjectedWallEntity::NetworkVar_m_vWorldSpace_WallMins>::operator=<Vector>() at line 348
	// inlined CNetworkVarBase<Vector,CProjectedWallEntity::NetworkVar_m_vWorldSpace_WallMaxs>::operator=<Vector>() at line 348
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flHeight>::operator=<float>() at line 349
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flWidth>::operator=<float>() at line 349
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flLength>::operator=<vec_t>() at line 349
	// inlined CNetworkHandleBase<CPortal_Base2D,CBaseProjectedEntity::NetworkVar_m_hHitPortal>::operator=() at line 350
}

// game/server/portal2/projectedwallentity.cpp:354 @0x7489f0 _ZN20CProjectedWallEntity20GetProjectionExtentsER6VectorS1_
void CProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
}

// game/server/portal2/projectedwallentity.cpp:365 @0x74c8c0 _ZN20CProjectedWallEntity14CreateVPhysicsEv
bool CProjectedWallEntity::CreateVPhysics()
{
}

// game/server/portal2/projectedwallentity.cpp:372 @0x748740 _ZN20CProjectedWallEntity13TestCollisionERK5Ray_tjR10CGameTrace
bool CProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
	// inlined CGameTrace::DidHit() at line 382
}

// game/server/portal2/projectedwallentity.cpp:386 @0x748c00 _ZN20CProjectedWallEntity31ComputeWorldSpaceSurroundingBoxEP6VectorS1_
void CProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	Vector vMins;  // line 388
	Vector vMaxs;  // line 389
	// inlined Vector::operator=() at line 394
	// inlined Vector::operator=() at line 395
}

// game/server/portal2/projectedwallentity.cpp:399 @0x748900 _ZN20CProjectedWallEntity23DrawWallBoundsHighlightEv
void CProjectedWallEntity::DrawWallBoundsHighlight()
{
	Vector *outVerts;  // line 405
	int vertCount;  // line 406
	int triCount;  // line 407
	int vert;  // line 408
	{
		int i;  // line 409
	}
}

// game/server/portal2/projectedwallentity.cpp:417 @0x7488b0 _ZN20CProjectedWallEntity17NotifyPortalEventE13PortalEvent_tP14CPortal_Base2D
void CProjectedWallEntity::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 422
}

// game/server/portal2/projectedwallentity.cpp:429 @0x748d90 _ZN20CProjectedWallEntity15SetSourcePortalEP14CPortal_Base2D
void CProjectedWallEntity::SetSourcePortal( CPortal_Base2D *pPortal )
{
	// inlined CNetworkVarBase<CHandle<CPortal_Base2D>,CProjectedWallEntity::NetworkVar_m_hColorPortal>::operator=<CPortal_Base2D*>() at line 436
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flParticleUpdateTime>::operator=<float>() at line 433
}

// game/server/portal2/projectedwallentity.cpp:440 @0x74cab0 _ZN20CProjectedWallEntity12SetHitPortalEP14CPortal_Base2D
void CProjectedWallEntity::SetHitPortal( CPortal_Base2D *pPortal )
{
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flParticleUpdateTime>::operator=<float>() at line 444
}

// game/server/portal2/projectedwallentity.cpp:455 sizeof=0x18 (i386)
struct CTEWallPaintedEvent : public CBaseTempEntity
{
public:
	CTEWallPaintedEvent( const char * );  // line 460
	CNetworkHandle( CBaseEntity, m_hEntity ); // +0xc  // line 463
	CNetworkVar( int, m_colorIndex ); // +0x10  // line 464
	CNetworkVar( int, m_nSegment ); // +0x14  // line 465
};

// game/server/portal2/projectedwallentity.cpp:455 (declaration)
~CTEWallPaintedEvent();

// game/server/portal2/projectedwallentity.cpp:455 @0x74cd40 _ZN19CTEWallPaintedEventD0Ev
CTEWallPaintedEvent::~CTEWallPaintedEvent()
{
}

// game/server/portal2/projectedwallentity.cpp:455 @0x74cd80 _ZN19CTEWallPaintedEventD1Ev
CTEWallPaintedEvent::~CTEWallPaintedEvent()
{
}

// game/server/portal2/projectedwallentity.cpp:458 @0x7487e0 _ZN19CTEWallPaintedEvent14GetServerClassEv
ServerClass *CTEWallPaintedEvent::GetServerClass()
{
}

// game/server/portal2/projectedwallentity.cpp:458 @0x7487f0 _ZN19CTEWallPaintedEvent40YouForgotToImplementOrDeclareServerClassEv
int CTEWallPaintedEvent::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/projectedwallentity.cpp:468 @0xb2070 _Z15ServerClassInitIN21DT_TEWallPaintedEvent7ignoredEEiPT_
int ServerClassInit<DT_TEWallPaintedEvent::ignored>( DT_TEWallPaintedEvent::ignored * )
{
	SendTable &sendTable;  // line 468
	char *const g_pSendTableName;  // line 468
	SendProp g_SendProps[4];  // line 468
}

// game/server/portal2/projectedwallentity.cpp:468
SendTable g_SendTable;

// game/server/portal2/projectedwallentity.cpp:468
int g_SendTableInit;

// game/server/portal2/projectedwallentity.cpp:468
static ServerClass g_CTEWallPaintedEvent_ClassReg;

// game/server/portal2/projectedwallentity.cpp:474
static CTEWallPaintedEvent g_TEWallPaintedEvent;

// game/server/portal2/projectedwallentity.cpp:476 @0x748c70 _Z11WallPaintediiP11CBaseEntity
WallPainted( int colorIndex, int nSegment, CBaseEntity *pWall )
{
	CRecipientFilter filter;  // line 478
	// inlined CNetworkVarBase<int,CTEWallPaintedEvent::NetworkVar_m_nSegment>::operator=<int>() at line 483
	// inlined CNetworkVarBase<int,CTEWallPaintedEvent::NetworkVar_m_colorIndex>::operator=<int>() at line 482
	// inlined CNetworkHandleBase<CBaseEntity,CTEWallPaintedEvent::NetworkVar_m_hEntity>::operator=() at line 481
}

// game/server/portal2/projectedwallentity.cpp:487 @0x748840 _ZNK20CProjectedWallEntity13IsWallPaintedERK6Vector
bool CProjectedWallEntity::IsWallPainted( const Vector &vecPosition )
{
}

// game/server/portal2/projectedwallentity.cpp:494 @0x748810 _ZNK20CProjectedWallEntity14GetNumSegmentsEv
int CProjectedWallEntity::GetNumSegments()
{
}

// game/server/portal2/projectedwallentity.cpp:499 @0x748800 _ZNK20CProjectedWallEntity16GetSegmentLengthEv
float CProjectedWallEntity::GetSegmentLength()
{
}

// game/server/portal2/projectedwallentity.cpp:504 @0x748820 _ZNK20CProjectedWallEntity22GetPaintPowerAtSegmentEi
PaintPowerType CProjectedWallEntity::GetPaintPowerAtSegment( int i )
{
}

// game/server/portal2/projectedwallentity.cpp:509 (declaration)
void CreateNewInstance();

// game/server/portal2/projectedwallentity.cpp:509 @0x748870 _ZN20CProjectedWallEntity17CreateNewInstanceEv
CProjectedWallEntity *CProjectedWallEntity::CreateNewInstance()
{
}

// game/server/portal2/projectedwallentity.cpp:514 @0x748d50 _ZN20CProjectedWallEntity24CreateNewProjectedEntityEv
CBaseProjectedEntity *CProjectedWallEntity::CreateNewProjectedEntity()
{
	// inlined CProjectedWallEntity::CreateNewInstance() at line 516
}

// game/server/portal2/projectedwallentity.cpp:524 @0x748b10 _ZN20CProjectedWallEntity20PlacePlacementHelperEP20CInfoPlacementHelper
void CProjectedWallEntity::PlacePlacementHelper( CInfoPlacementHelper *pHelper )
{
	float flZOffset;  // line 526
	Vector vecPosition;  // line 527
	// inlined Vector::Vector() at line 527
	// inlined Vector::operator+() at line 527
}

// game/server/portal2/projectedwallentity.cpp:537 @0x74c8e0 _ZN20CProjectedWallEntity11OnProjectedEv
void CProjectedWallEntity::OnProjected()
{
	// inlined CNetworkVarBase<float,CProjectedWallEntity::NetworkVar_m_flParticleUpdateTime>::operator=<float>() at line 542
}

// public/tier1/utlmemory.h:707 @0x74cdf0 _ZN10CUtlMemoryI14PaintPowerTypeiE4GrowEi
void CUtlMemory<PaintPowerType,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<PaintPowerType,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
