// DWARF declaration skeleton for game/server/portal2/portal_pointpush.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb1920 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPortalVortexEffectManager::CPortalVortexEffectManager() at line 107
	// inlined ServerClass::ServerClass() at line 128
	// inlined CEntityFactory<CPortal_PointPush>::CEntityFactory() at line 136
}

// game/server/portal2/portal_pointpush.cpp:18 sizeof=0x8 (i386)
struct PortalVortexEffect_t
{
public:
	PortalVortexEffect_t( CParticleSystem *, CPortal_Base2D * );  // line 19
	CHandle<CParticleSystem> hParticleSystem; // +0x0  // line 21
	CHandle<CPortal_Base2D> hPortalOwner; // +0x4  // line 22
};

// game/server/portal2/portal_pointpush.cpp:18 (declaration)
void PortalVortexEffect_t( const PortalVortexEffect_t & );

// game/server/portal2/portal_pointpush.cpp:26 sizeof=0x20 (i386)
struct CPortalVortexEffectManager : public CAutoGameSystem
{
public:
	void AddVortexEffect( CPortal_Base2D *, bool );  // line 28
	void RemoveVortexEffect( CPortal_Base2D * );  // line 71
	void Purge();  // line 88
	virtual void LevelShutdownPostEntity();  // line 98
private:
	CUtlVector<PortalVortexEffect_t,CUtlMemory<PortalVortexEffect_t, int> > m_Effects; // +0xc  // line 104
};

// game/server/portal2/portal_pointpush.cpp:26 (declaration)
~CPortalVortexEffectManager();

// game/server/portal2/portal_pointpush.cpp:26 @0x7467a0 _ZN26CPortalVortexEffectManagerD1Ev
CPortalVortexEffectManager::~CPortalVortexEffectManager()
{
	// inlined CUtlVector<PortalVortexEffect_t,CUtlMemory<PortalVortexEffect_t, int> >::~CUtlVector() at line 26
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
}

// game/server/portal2/portal_pointpush.cpp:26 (declaration)
void CPortalVortexEffectManager();

// game/server/portal2/portal_pointpush.cpp:26 @0x7468a0 _ZN26CPortalVortexEffectManagerD0Ev
CPortalVortexEffectManager::~CPortalVortexEffectManager()
{
	// inlined CUtlVector<PortalVortexEffect_t,CUtlMemory<PortalVortexEffect_t, int> >::~CUtlVector() at line 26
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 26
}

// game/server/portal2/portal_pointpush.cpp:98 @0x746690 _ZN26CPortalVortexEffectManager23LevelShutdownPostEntityEv
void CPortalVortexEffectManager::LevelShutdownPostEntity()
{
	// inlined CPortalVortexEffectManager::Purge() at line 100
}

// game/server/util.h:100 @0x746730 _ZN14CEntityFactoryI17CPortal_PointPushE6CreateEPKc
IServerNetworkable *CEntityFactory<CPortal_PointPush>::Create( const char *pClassName )
{
	CPortal_PointPush *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPortal_PointPush>() at line 102
}

// game/server/util.h:106 @0x7465f0 _ZN14CEntityFactoryI17CPortal_PointPushE7DestroyEP18IServerNetworkable
void CEntityFactory<CPortal_PointPush>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/portal_pointpush.cpp:107
CPortalVortexEffectManager g_PortalVortexManager;

// game/server/portal2/portal_pointpush.cpp:114 (declaration)
void GetBaseMap();

// game/server/portal2/portal_pointpush.cpp:114 @0x744d10 _ZN17CPortal_PointPush14GetDataDescMapEv
datamap_t *CPortal_PointPush::GetDataDescMap()
{
}

// game/server/portal2/portal_pointpush.cpp:114 @0x744d20 _ZN17CPortal_PointPush10GetBaseMapEv
datamap_t *CPortal_PointPush::GetBaseMap()
{
}

// game/server/util.h:114 @0x746620 _ZN14CEntityFactoryI17CPortal_PointPushE13GetEntitySizeEv
size_t CEntityFactory<CPortal_PointPush>::GetEntitySize()
{
}

// game/server/portal2/portal_pointpush.cpp:114 @0xb1710 _Z11DataMapInitI17CPortal_PointPushEP9datamap_tPT_
datamap_t *DataMapInit<CPortal_PointPush>( CPortal_PointPush * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 125
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 114
	CDatadescGeneratedNameHolder nameHolder;  // line 114
	typedescription_t dataDesc[9];  // line 114
}

// game/server/portal2/portal_pointpush.cpp:114
datamap_t *g_DataMapHolder;

// game/server/portal2/portal_pointpush.cpp:114
void CPortal_PointPush::m_DataMap;

// game/server/portal2/portal_pointpush.cpp:128 @0x744d30 _ZN17CPortal_PointPush14GetServerClassEv
ServerClass *CPortal_PointPush::GetServerClass()
{
}

// game/server/portal2/portal_pointpush.cpp:128 @0x744d40 _ZN17CPortal_PointPush40YouForgotToImplementOrDeclareServerClassEv
int CPortal_PointPush::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/portal_pointpush.cpp:128 @0xb13d0 _Z15ServerClassInitIN18DT_PortalPointPush7ignoredEEiPT_
int ServerClassInit<DT_PortalPointPush::ignored>( DT_PortalPointPush::ignored * )
{
	SendTable &sendTable;  // line 128
	char *const g_pSendTableName;  // line 128
	SendProp g_SendProps[7];  // line 128
}

// game/server/portal2/portal_pointpush.cpp:128
SendTable g_SendTable;

// game/server/portal2/portal_pointpush.cpp:128
int g_SendTableInit;

// game/server/portal2/portal_pointpush.cpp:128
static ServerClass g_CPortal_PointPush_ClassReg;

// game/server/portal2/portal_pointpush.cpp:128
void CPortal_PointPush::m_pClassSendTable;

// game/server/portal2/portal_pointpush.cpp:136
static CEntityFactory<CPortal_PointPush> point_push;

// game/server/portal2/portal_pointpush.cpp:138 @0x745ee0 _ZN17CPortal_PointPush25DrawDebugGeometryOverlaysEv
void CPortal_PointPush::DrawDebugGeometryOverlays()
{
	// inlined Vector::Vector() at line 141
	// inlined Vector::operator-() at line 141
	// inlined CBaseEntity::GetAbsOrigin() at line 141
	// inlined Vector::operator VectorByValue&() at line 141
	// inlined Vector::operator VectorByValue&() at line 141
	// inlined CBaseEntity::GetAbsOrigin() at line 144
}

// game/server/portal2/portal_pointpush.cpp:150 @0x744d70 _ZN17CPortal_PointPush8ActivateEv
void CPortal_PointPush::Activate()
{
}

// game/server/portal2/portal_pointpush.cpp:161 @0x744d50 _ZN17CPortal_PointPush19UpdateTransmitStateEv
int CPortal_PointPush::UpdateTransmitState()
{
}

// game/server/portal2/portal_pointpush.cpp:167 @0x745ff0 _ZN17CPortal_PointPush24CreatePortalVortexEffectERK6VectorP14CPortal_Base2D
void CPortal_PointPush::CreatePortalVortexEffect( const Vector &vecOrigin, CPortal_Base2D *pPortal )
{
	CPortal_Base2D *pPartnerPortal;  // line 169
	float flDistFromOriginSqr;  // line 172
	float flMinRadiusSqr;  // line 173
	// inlined CHandle<CBaseEntity>::CHandle() at line 181
	// inlined CHandle<CBaseEntity>::CHandle() at line 180
	// inlined CPortalVortexEffectManager::AddVortexEffect() at line 177
	// inlined Vector::operator-() at line 172
	// inlined CPortal_Base2D::GetLinkedPortal() at line 169
}

// game/server/portal2/portal_pointpush.cpp:190 (declaration)
void PushEntity( CBaseEntity *pTarget, const Vector &vecPushPoint );

// game/server/portal2/portal_pointpush.cpp:260 @0x744fb0 _ZN17CPortal_PointPush11InputEnableER11inputdata_t
void CPortal_PointPush::InputEnable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CPortal_PointPush::NetworkVar_m_bEnabled>::operator=<bool>() at line 262
}

// game/server/portal2/portal_pointpush.cpp:270 @0x744df0 _ZN17CPortal_PointPush12InputDisableER11inputdata_t
void CPortal_PointPush::InputDisable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CPortal_PointPush::NetworkVar_m_bEnabled>::operator=<bool>() at line 272
}

// game/server/portal2/portal_pointpush.cpp:278 @0x745190 _ZN17CPortal_PointPush17NotifyPortalEventE13PortalEvent_tP14CPortal_Base2D
void CPortal_PointPush::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	// inlined CPortalVortexEffectManager::RemoveVortexEffect() at line 285
	// inlined CHandle<CBaseEntity>::CHandle() at line 286
	// inlined CPortal_Base2D::GetLinkedPortal() at line 287
	// inlined CHandle<CBaseEntity>::CHandle() at line 289
	// inlined CPortal_Base2D::GetLinkedPortal() at line 289
}

// game/server/portal2/portal_pointpush.cpp:296 @0x745400 _ZN17CPortal_PointPush19PushEntitiesAtPointERK6VectorRK6QAngleP14CPortal_Base2D
void CPortal_PointPush::PushEntitiesAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal )
{
	CBaseEntity *pEnts[256];  // line 299
	int numEnts;  // line 300
	{
		int i;  // line 301
		// inlined CPortal_PointPush::PushEntity() at line 370
		// inlined CBaseEntity::GetMoveType() at line 314
		// inlined CBaseEntity::GetMoveParent() at line 310
		// inlined CBaseEntity::IsSolid() at line 306
		// inlined CBaseEntity::GetAbsAngles() at line 338
		// inlined CBaseEntity::GetAbsOrigin() at line 338
		// inlined CBaseEntity::GetAbsOrigin() at line 338
		// inlined CBaseEntity::HasSpawnFlags() at line 343
		{
			float flMinAngle;  // line 330
		}
		{
			Vector vecStartPos;  // line 345
			Vector vecEndPos;  // line 346
			trace_t tr;  // line 356
			{
				Vector vecDirToTarget;  // line 351
				// inlined Vector::operator-() at line 351
				// inlined Vector::operator*() at line 353
				// inlined Vector::operator+() at line 353
				// inlined Vector::operator=() at line 353
			}
			// inlined Vector::operator VectorByValue&() at line 346
			// inlined UTIL_TraceLine() at line 362
		}
	}
	// inlined UTIL_EntitiesInSphere() at line 300
}
