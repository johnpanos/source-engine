// DWARF declaration skeleton for game/server/portal2/point_laser_target.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaff00 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPortalLaserTarget>::CEntityFactory() at line 19
	// inlined CEntityFactory<CLaserCatcher>::CEntityFactory() at line 111
	// inlined CEntityFactory<CLaserRelay>::CEntityFactory() at line 395
}

// game/server/portal2/point_laser_target.cpp:19
static CEntityFactory<CPortalLaserTarget> point_laser_target;

// game/server/portal2/point_laser_target.cpp:21 (declaration)
void GetBaseMap();

// game/server/portal2/point_laser_target.cpp:21 @0x7406e0 _ZN18CPortalLaserTarget14GetDataDescMapEv
datamap_t *CPortalLaserTarget::GetDataDescMap()
{
}

// game/server/portal2/point_laser_target.cpp:21 @0x7406f0 _ZN18CPortalLaserTarget10GetBaseMapEv
datamap_t *CPortalLaserTarget::GetBaseMap()
{
}

// game/server/portal2/point_laser_target.cpp:21 @0xafce0 _Z11DataMapInitI18CPortalLaserTargetEP9datamap_tPT_
datamap_t *DataMapInit<CPortalLaserTarget>( CPortalLaserTarget * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 33
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 21
	CDatadescGeneratedNameHolder nameHolder;  // line 21
	typedescription_t dataDesc[8];  // line 21
}

// game/server/portal2/point_laser_target.cpp:21
datamap_t *g_DataMapHolder;

// game/server/portal2/point_laser_target.cpp:21
void CPortalLaserTarget::m_DataMap;

// game/server/portal2/point_laser_target.cpp:38 @0x740de0 _ZN18CPortalLaserTarget5SpawnEv
void CPortalLaserTarget::Spawn()
{
	const float flWidth;  // line 43
	// inlined Vector::Vector() at line 44
	// inlined Vector::operator VectorByValue&() at line 44
	// inlined Vector::operator-() at line 44
	// inlined Vector::operator VectorByValue&() at line 44
}

// game/server/portal2/point_laser_target.cpp:51 @0x740c40 _ZN18CPortalLaserTarget12OnTakeDamageERK15CTakeDamageInfo
int CPortalLaserTarget::OnTakeDamage( const CTakeDamageInfo &info )
{
	// inlined CTakeDamageInfo::GetAttacker() at line 60
	// inlined FClassnameIs() at line 60
}

// game/server/portal2/point_laser_target.cpp:81 @0x740aa0 _ZN18CPortalLaserTarget12DisableThinkEv
void CPortalLaserTarget::DisableThink()
{
}

// game/server/portal2/point_laser_target.cpp:99 (declaration)
void IsPowered();

// game/server/portal2/point_laser_target.cpp:99 @0x740700 _ZN18CPortalLaserTarget9IsPoweredEv
bool CPortalLaserTarget::IsPowered()
{
}

// game/server/util.h:100 @0x741c80 _ZN14CEntityFactoryI11CLaserRelayE6CreateEPKc
IServerNetworkable *CEntityFactory<CLaserRelay>::Create( const char *pClassName )
{
	CLaserRelay *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CLaserRelay>() at line 102
}

// game/server/util.h:100 @0x741d00 _ZN14CEntityFactoryI18CPortalLaserTargetE6CreateEPKc
IServerNetworkable *CEntityFactory<CPortalLaserTarget>::Create( const char *pClassName )
{
	CPortalLaserTarget *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPortalLaserTarget>() at line 102
}

// game/server/util.h:100 @0x741da0 _ZN14CEntityFactoryI13CLaserCatcherE6CreateEPKc
IServerNetworkable *CEntityFactory<CLaserCatcher>::Create( const char *pClassName )
{
	CLaserCatcher *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CLaserCatcher>() at line 102
}

// game/server/util.h:106 @0x741b30 _ZN14CEntityFactoryI11CLaserRelayE7DestroyEP18IServerNetworkable
void CEntityFactory<CLaserRelay>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x741b70 _ZN14CEntityFactoryI13CLaserCatcherE7DestroyEP18IServerNetworkable
void CEntityFactory<CLaserCatcher>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x741bb0 _ZN14CEntityFactoryI18CPortalLaserTargetE7DestroyEP18IServerNetworkable
void CEntityFactory<CPortalLaserTarget>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/point_laser_target.cpp:111
static CEntityFactory<CLaserCatcher> prop_laser_catcher;

// game/server/portal2/point_laser_target.cpp:113 (declaration)
void GetBaseMap();

// game/server/portal2/point_laser_target.cpp:113 @0x740710 _ZN13CLaserCatcher14GetDataDescMapEv
datamap_t *CLaserCatcher::GetDataDescMap()
{
}

// game/server/portal2/point_laser_target.cpp:113 @0x740720 _ZN13CLaserCatcher10GetBaseMapEv
datamap_t *CLaserCatcher::GetBaseMap()
{
}

// game/server/portal2/point_laser_target.cpp:113 @0xafab0 _Z11DataMapInitI13CLaserCatcherEP9datamap_tPT_
datamap_t *DataMapInit<CLaserCatcher>( CLaserCatcher * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 126
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 113
	CDatadescGeneratedNameHolder nameHolder;  // line 113
	typedescription_t dataDesc[9];  // line 113
}

// game/server/portal2/point_laser_target.cpp:113
datamap_t *g_DataMapHolder;

// game/server/portal2/point_laser_target.cpp:113
void CLaserCatcher::m_DataMap;

// game/server/util.h:114 @0x741b60 _ZN14CEntityFactoryI11CLaserRelayE13GetEntitySizeEv
size_t CEntityFactory<CLaserRelay>::GetEntitySize()
{
}

// game/server/util.h:114 @0x741ba0 _ZN14CEntityFactoryI13CLaserCatcherE13GetEntitySizeEv
size_t CEntityFactory<CLaserCatcher>::GetEntitySize()
{
}

// game/server/util.h:114 @0x741be0 _ZN14CEntityFactoryI18CPortalLaserTargetE13GetEntitySizeEv
size_t CEntityFactory<CPortalLaserTarget>::GetEntitySize()
{
}

// game/server/portal2/point_laser_target.cpp:131 (declaration)
void CLaserCatcher();

// game/server/portal2/point_laser_target.cpp:131 @0x740bb0 _ZN13CLaserCatcherC2Ev
CLaserCatcher::CLaserCatcher()
{
	// inlined COutputEvent::COutputEvent() at line 131
	// inlined COutputEvent::COutputEvent() at line 131
}

// game/server/portal2/point_laser_target.cpp:131 @0x740c30 _ZN13CLaserCatcherC1Ev
CLaserCatcher::CLaserCatcher()
{
}

// game/server/portal2/point_laser_target.cpp:140 @0x740b40 _ZN13CLaserCatcher8PrecacheEv
void CLaserCatcher::Precache()
{
}

// game/server/portal2/point_laser_target.cpp:157 @0x740f20 _ZN13CLaserCatcher5SpawnEv
void CLaserCatcher::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 166
	// inlined CBaseAnimating::SetPlaybackRate() at line 176
}

// game/server/portal2/point_laser_target.cpp:186 @0x740b20 _ZN13CLaserCatcher14CreateVPhysicsEv
bool CLaserCatcher::CreateVPhysics()
{
}

// game/server/portal2/point_laser_target.cpp:195 @0x740a10 _ZN13CLaserCatcher8ActivateEv
void CLaserCatcher::Activate()
{
}

// game/server/portal2/point_laser_target.cpp:206 @0x741630 _ZN13CLaserCatcher12AnimateThinkEv
void CLaserCatcher::AnimateThink()
{
	// inlined CBaseAnimating::GetPlaybackRate() at line 214
	// inlined CBaseAnimating::SetPlaybackRate() at line 214
	// inlined CBaseAnimating::GetPlaybackRate() at line 210
	// inlined CBaseAnimating::SetPlaybackRate() at line 210
}

// game/server/portal2/point_laser_target.cpp:227 @0x7409b0 _ZN13CLaserCatcher14UpdateOnRemoveEv
void CLaserCatcher::UpdateOnRemove()
{
}

// game/server/portal2/point_laser_target.cpp:245 @0x7411a0 _ZN13CLaserCatcher9OnPoweredEv
void CLaserCatcher::OnPowered()
{
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 257
}

// game/server/portal2/point_laser_target.cpp:263 @0x741420 _ZN13CLaserCatcher11OnUnPoweredEv
void CLaserCatcher::OnUnPowered()
{
	// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin>::operator=<int>() at line 274
}

// game/server/portal2/point_laser_target.cpp:280 @0x740870 _ZN13CLaserCatcher20CreateHelperEntitiesEv
void CLaserCatcher::CreateHelperEntities()
{
	{
		Vector vecOrigin;  // line 285
		// inlined CBaseEntity::GetAbsAngles() at line 290
		// inlined CPortalLaserTarget::SetCatcher() at line 297
	}
}

// game/server/portal2/point_laser_target.cpp:305 @0x740da0 _ZN13CLaserCatcher19GetCatcherModelNameEv
const char *CLaserCatcher::GetCatcherModelName()
{
	// inlined string_t::ToCStr() at line 307
	// inlined string_t::ToCStr() at line 307
	// inlined string_t::ToCStr() at line 307
}

// game/server/portal2/point_laser_target.cpp:313 @0x740730 _ZN13CLaserCatcher15IsTerminalPointEv
bool CLaserCatcher::IsTerminalPoint()
{
}

// game/server/portal2/point_laser_target.cpp:322 @0x740740 _ZN13CLaserCatcher8SpinDownEf
float CLaserCatcher::SpinDown( float flSpinRate )
{
	float flNewSpinRate;  // line 327
}

// game/server/portal2/point_laser_target.cpp:340 @0x7407a0 _ZN13CLaserCatcher6SpinUpEf
float CLaserCatcher::SpinUp( float flSpinRate )
{
	float flNewSpinRate;  // line 345
}

// game/server/portal2/point_laser_target.cpp:358 @0x740800 _ZN13CLaserCatcher12PoweringDownEv
bool CLaserCatcher::PoweringDown()
{
}

// game/server/portal2/point_laser_target.cpp:366 @0x740820 _ZN13CLaserCatcher10PoweringUpEv
bool CLaserCatcher::PoweringUp()
{
}

// game/server/portal2/point_laser_target.cpp:375 @0x740e60 _ZN13CLaserCatcher9OnRestoreEv
void CLaserCatcher::OnRestore()
{
	// inlined CPortalLaserTarget::SetCatcher() at line 382
	// inlined CPortalLaserTarget::IsPowered() at line 385
}

// game/server/portal2/point_laser_target.cpp:395
static CEntityFactory<CLaserRelay> prop_laser_relay;

// game/server/portal2/point_laser_target.cpp:397 (declaration)
void GetBaseMap();

// game/server/portal2/point_laser_target.cpp:397 @0x740840 _ZN11CLaserRelay14GetDataDescMapEv
datamap_t *CLaserRelay::GetDataDescMap()
{
}

// game/server/portal2/point_laser_target.cpp:397 @0x740850 _ZN11CLaserRelay10GetBaseMapEv
datamap_t *CLaserRelay::GetBaseMap()
{
}

// game/server/portal2/point_laser_target.cpp:397 @0xaf9e0 _Z11DataMapInitI11CLaserRelayEP9datamap_tPT_
datamap_t *DataMapInit<CLaserRelay>( CLaserRelay * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 397
	CDatadescGeneratedNameHolder nameHolder;  // line 397
	typedescription_t dataDesc[1];  // line 397
}

// game/server/portal2/point_laser_target.cpp:397
datamap_t *g_DataMapHolder;

// game/server/portal2/point_laser_target.cpp:397
void CLaserRelay::m_DataMap;

// game/server/portal2/point_laser_target.cpp:403 @0x740d60 _ZN11CLaserRelay19GetCatcherModelNameEv
const char *CLaserRelay::GetCatcherModelName()
{
	// inlined string_t::ToCStr() at line 405
	// inlined string_t::ToCStr() at line 405
	// inlined string_t::ToCStr() at line 405
}

// game/server/portal2/point_laser_target.cpp:411 @0x740860 _ZN11CLaserRelay15IsTerminalPointEv
bool CLaserRelay::IsTerminalPoint()
{
}
