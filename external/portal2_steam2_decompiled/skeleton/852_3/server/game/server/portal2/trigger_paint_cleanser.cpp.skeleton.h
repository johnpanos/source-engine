// DWARF declaration skeleton for game/server/portal2/trigger_paint_cleanser.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xcf390 _Z41__static_initialization_and_destruction_0ii
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
	// inlined ServerClass::ServerClass() at line 20
	// inlined CEntityFactory<CTriggerPaintCleanser>::CEntityFactory() at line 24
}

// game/server/portal2/trigger_paint_cleanser.cpp:16 (declaration)
void GetBaseMap();

// game/server/portal2/trigger_paint_cleanser.cpp:16 @0x6f9e70 _ZN21CTriggerPaintCleanser14GetDataDescMapEv
datamap_t *CTriggerPaintCleanser::GetDataDescMap()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:16 @0x6f9e80 _ZN21CTriggerPaintCleanser10GetBaseMapEv
datamap_t *CTriggerPaintCleanser::GetBaseMap()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:16 @0xcf2c0 _Z11DataMapInitI21CTriggerPaintCleanserEP9datamap_tPT_
datamap_t *DataMapInit<CTriggerPaintCleanser>( CTriggerPaintCleanser * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 16
	CDatadescGeneratedNameHolder nameHolder;  // line 16
	typedescription_t dataDesc[1];  // line 16
}

// game/server/portal2/trigger_paint_cleanser.cpp:16
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_paint_cleanser.cpp:16
void CTriggerPaintCleanser::m_DataMap;

// game/server/portal2/trigger_paint_cleanser.cpp:20 @0x6f9e90 _ZN21CTriggerPaintCleanser14GetServerClassEv
ServerClass *CTriggerPaintCleanser::GetServerClass()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:20 @0x6f9ea0 _ZN21CTriggerPaintCleanser40YouForgotToImplementOrDeclareServerClassEv
int CTriggerPaintCleanser::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:20 @0xcf100 _Z15ServerClassInitIN23DT_TriggerPaintCleanser7ignoredEEiPT_
int ServerClassInit<DT_TriggerPaintCleanser::ignored>( DT_TriggerPaintCleanser::ignored * )
{
	SendTable &sendTable;  // line 20
	char *const g_pSendTableName;  // line 20
	SendProp g_SendProps[3];  // line 20
}

// game/server/portal2/trigger_paint_cleanser.cpp:20
SendTable g_SendTable;

// game/server/portal2/trigger_paint_cleanser.cpp:20
int g_SendTableInit;

// game/server/portal2/trigger_paint_cleanser.cpp:20
static ServerClass g_CTriggerPaintCleanser_ClassReg;

// game/server/portal2/trigger_paint_cleanser.cpp:20
void CTriggerPaintCleanser::m_pClassSendTable;

// game/server/portal2/trigger_paint_cleanser.cpp:24
static CEntityFactory<CTriggerPaintCleanser> trigger_paint_cleanser;

// game/server/portal2/trigger_paint_cleanser.cpp:26 (declaration)
void CTriggerPaintCleanser();

// game/server/portal2/trigger_paint_cleanser.cpp:26 @0x6f9f80 _ZN21CTriggerPaintCleanserC1Ev
CTriggerPaintCleanser::CTriggerPaintCleanser()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:26 @0x6f9fc0 _ZN21CTriggerPaintCleanserC2Ev
CTriggerPaintCleanser::CTriggerPaintCleanser()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:31 @0x6f9f60 _ZN21CTriggerPaintCleanser5SpawnEv
void CTriggerPaintCleanser::Spawn()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:39 @0x6f9f20 _ZN21CTriggerPaintCleanser8ActivateEv
void CTriggerPaintCleanser::Activate()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:47 @0x6f9ee0 _ZN21CTriggerPaintCleanser14UpdateOnRemoveEv
void CTriggerPaintCleanser::UpdateOnRemove()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:55 @0x6fa000 _ZN21CTriggerPaintCleanser5TouchEP11CBaseEntity
void CTriggerPaintCleanser::Touch( CBaseEntity *pOther )
{
	CWeaponPaintGun *pPaintGun;  // line 58
	{
		CTakeDamageInfo info;  // line 98
		// inlined CTakeDamageInfo::SetDamagePosition() at line 99
		// inlined CBaseEntity::GetAbsOrigin() at line 99
	}
	// inlined FClassnameIs() at line 95
	// inlined FClassnameIs() at line 91
	// inlined FClassnameIs() at line 86
	{
		CPortal_Player *pPlayer;  // line 61
		// inlined ToPortalPlayer() at line 61
		{
			IGameEvent *event;  // line 67
			// inlined CBasePlayer::GetUserID() at line 70
			// inlined CBaseEntity::entindex() at line 71
		}
	}
}

// game/server/util.h:99 @0x6fa3c0 _ZN14CEntityFactoryI21CTriggerPaintCleanserE6CreateEPKc
IServerNetworkable *CEntityFactory<CTriggerPaintCleanser>::Create( const char *pClassName )
{
	CTriggerPaintCleanser *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CTriggerPaintCleanser>() at line 101
}

// game/server/util.h:105 @0x6fa300 _ZN14CEntityFactoryI21CTriggerPaintCleanserE7DestroyEP18IServerNetworkable
void CEntityFactory<CTriggerPaintCleanser>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:106 @0x6f9ed0 _ZN21CTriggerPaintCleanser6EnableEv
void CTriggerPaintCleanser::Enable()
{
}

// game/server/portal2/trigger_paint_cleanser.cpp:112 @0x6f9eb0 _ZN21CTriggerPaintCleanser19UpdateTransmitStateEv
int CTriggerPaintCleanser::UpdateTransmitState()
{
}

// game/server/util.h:113 @0x6fa330 _ZN14CEntityFactoryI21CTriggerPaintCleanserE13GetEntitySizeEv
size_t CEntityFactory<CTriggerPaintCleanser>::GetEntitySize()
{
}
