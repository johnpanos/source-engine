// DWARF declaration skeleton for game/server/portal2/item_paint_power_pickup.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xab700 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPaintPowerPickup>::CEntityFactory() at line 53
}

// game/server/portal2/item_paint_power_pickup.cpp:16 @0x71ceb0 _Z16PaintPowerPickupiP11CBasePlayer
PaintPowerPickup( int colorIndex, CBasePlayer *pPlayer )
{
	CBaseCombatWeapon *pWeapon;  // line 21
	CBroadcastRecipientFilter filter;  // line 23
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 29
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 23
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 29
}

// game/server/portal2/item_paint_power_pickup.cpp:34 sizeof=0x56c (i386)
struct CPaintPowerPickup : public CItem
{
public:
	CPaintPowerPickup();  // line 39
	virtual void Spawn();  // line 41
	virtual void Precache();  // line 42
	virtual void Touch( CBaseEntity * );  // line 43
private:
	int m_PaintPowerType; // +0x568  // line 46
};

// game/server/portal2/item_paint_power_pickup.cpp:34 (declaration)
~CPaintPowerPickup();

// game/server/portal2/item_paint_power_pickup.cpp:34 @0x71d660 _ZN17CPaintPowerPickupD0Ev
CPaintPowerPickup::~CPaintPowerPickup()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:34 @0x71d6b0 _ZN17CPaintPowerPickupD1Ev
CPaintPowerPickup::~CPaintPowerPickup()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:37 @0x71cdd0 _ZN17CPaintPowerPickup14GetDataDescMapEv
datamap_t *CPaintPowerPickup::GetDataDescMap()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:37 @0x71cde0 _ZN17CPaintPowerPickup10GetBaseMapEv
datamap_t *CPaintPowerPickup::GetBaseMap()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:39 @0x71ce10 _ZN17CPaintPowerPickupC1Ev
CPaintPowerPickup::CPaintPowerPickup()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:39 @0x71ce60 _ZN17CPaintPowerPickupC2Ev
CPaintPowerPickup::CPaintPowerPickup()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:41 @0x71d0b0 _ZN17CPaintPowerPickup5SpawnEv
void CPaintPowerPickup::Spawn()
{
	Color renderColor;  // line 66
	// inlined CBaseEntity::SetRenderColor() at line 67
}

// game/server/portal2/item_paint_power_pickup.cpp:42 @0x71cdf0 _ZN17CPaintPowerPickup8PrecacheEv
void CPaintPowerPickup::Precache()
{
}

// game/server/portal2/item_paint_power_pickup.cpp:43 @0x71cf80 _ZN17CPaintPowerPickup5TouchEP11CBaseEntity
void CPaintPowerPickup::Touch( CBaseEntity *pOther )
{
	CBasePlayer *pPlayer;  // line 90
	CBaseCombatWeapon *pWeapon;  // line 92
	// inlined FClassnameIs() at line 93
	{
		CWeaponPaintGun *pPaintGun;  // line 95
	}
}

// game/server/portal2/item_paint_power_pickup.cpp:49 @0xab630 _Z11DataMapInitI17CPaintPowerPickupEP9datamap_tPT_
datamap_t *DataMapInit<CPaintPowerPickup>( CPaintPowerPickup * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 49
	CDatadescGeneratedNameHolder nameHolder;  // line 49
	typedescription_t dataDesc[2];  // line 49
}

// game/server/portal2/item_paint_power_pickup.cpp:49
datamap_t *g_DataMapHolder;

// game/server/portal2/item_paint_power_pickup.cpp:53
static CEntityFactory<CPaintPowerPickup> item_paint_power_pickup;

// game/server/util.h:100 @0x71d6f0 _ZN14CEntityFactoryI17CPaintPowerPickupE6CreateEPKc
IServerNetworkable *CEntityFactory<CPaintPowerPickup>::Create( const char *pClassName )
{
	CPaintPowerPickup *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPaintPowerPickup>() at line 102
}

// game/server/util.h:106 @0x71d620 _ZN14CEntityFactoryI17CPaintPowerPickupE7DestroyEP18IServerNetworkable
void CEntityFactory<CPaintPowerPickup>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x71d650 _ZN14CEntityFactoryI17CPaintPowerPickupE13GetEntitySizeEv
size_t CEntityFactory<CPaintPowerPickup>::GetEntitySize()
{
}
