// DWARF declaration skeleton for game/server/portal2/weapon_paintgun.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbc0c0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 37
	// inlined CEntityFactory<CWeaponPaintGun>::CEntityFactory() at line 71
}

// game/server/portal2/weapon_paintgun.cpp:37 @0x778d10 _ZN15CWeaponPaintGun14GetServerClassEv
ServerClass *CWeaponPaintGun::GetServerClass()
{
}

// game/server/portal2/weapon_paintgun.cpp:37 @0x778d20 _ZN15CWeaponPaintGun40YouForgotToImplementOrDeclareServerClassEv
int CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/weapon_paintgun.cpp:37
static ServerClass g_CWeaponPaintGun_ClassReg;

// game/server/portal2/weapon_paintgun.cpp:37
void CWeaponPaintGun::m_pClassSendTable;

// game/server/portal2/weapon_paintgun.cpp:38 @0xbbae0 _Z15ServerClassInitIN17DT_WeaponPaintGun7ignoredEEiPT_
int ServerClassInit<DT_WeaponPaintGun::ignored>( DT_WeaponPaintGun::ignored * )
{
	SendTable &sendTable;  // line 38
	char *const g_pSendTableName;  // line 38
	SendProp g_SendProps[7];  // line 38
}

// game/server/portal2/weapon_paintgun.cpp:38
SendTable g_SendTable;

// game/server/portal2/weapon_paintgun.cpp:38
int g_SendTableInit;

// game/server/portal2/weapon_paintgun.cpp:59 (declaration)
void GetBaseMap();

// game/server/portal2/weapon_paintgun.cpp:59 @0x778d30 _ZN15CWeaponPaintGun14GetDataDescMapEv
datamap_t *CWeaponPaintGun::GetDataDescMap()
{
}

// game/server/portal2/weapon_paintgun.cpp:59 @0x778d40 _ZN15CWeaponPaintGun10GetBaseMapEv
datamap_t *CWeaponPaintGun::GetBaseMap()
{
}

// game/server/portal2/weapon_paintgun.cpp:59 @0xbbea0 _Z11DataMapInitI15CWeaponPaintGunEP9datamap_tPT_
datamap_t *DataMapInit<CWeaponPaintGun>( CWeaponPaintGun * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 69
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 59
	CDatadescGeneratedNameHolder nameHolder;  // line 59
	typedescription_t dataDesc[7];  // line 59
}

// game/server/portal2/weapon_paintgun.cpp:59
datamap_t *g_DataMapHolder;

// game/server/portal2/weapon_paintgun.cpp:59
void CWeaponPaintGun::m_DataMap;

// game/server/portal2/weapon_paintgun.cpp:71
static CEntityFactory<CWeaponPaintGun> weapon_paintgun;

// game/server/portal2/weapon_paintgun.cpp:75 (declaration)
void CWeaponPaintGun();

// game/server/portal2/weapon_paintgun.cpp:75 @0x77a1c0 _ZN15CWeaponPaintGunC2Ev
CWeaponPaintGun::CWeaponPaintGun()
{
	// inlined CNetworkVarBase<bool,CWeaponPaintGun::NetworkVar_m_bFiringErase>::CNetworkVarBase() at line 80
	// inlined CNetworkVarBase<bool,CWeaponPaintGun::NetworkVar_m_bFiringPaint>::CNetworkVarBase() at line 80
	// inlined CNetworkVarBase<int,CWeaponPaintGun::NetworkVar_m_nCurrentColor>::CNetworkVarBase() at line 80
	// inlined CWeaponPaintGun::NetworkVar_m_hPaintStream::NetworkVar_m_hPaintStream() at line 80
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 95
}

// game/server/portal2/weapon_paintgun.cpp:75 @0x77a680 _ZN15CWeaponPaintGunC1Ev
CWeaponPaintGun::CWeaponPaintGun()
{
}

// game/server/portal2/weapon_paintgun.cpp:97 (declaration)
~CWeaponPaintGun();

// game/server/portal2/weapon_paintgun.cpp:97 @0x779700 _ZN15CWeaponPaintGunD0Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
	{
		CSoundEnvelopeController &controller;  // line 99
	}
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 112
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 112
}

// game/server/portal2/weapon_paintgun.cpp:97 @0x7797c0 _ZN15CWeaponPaintGunD2Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
	{
		CSoundEnvelopeController &controller;  // line 99
	}
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 112
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 112
}

// game/server/portal2/weapon_paintgun.cpp:97 @0x779880 _ZN15CWeaponPaintGunD1Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
}

// game/server/util.h:100 @0x77a6d0 _ZN14CEntityFactoryI15CWeaponPaintGunE6CreateEPKc
IServerNetworkable *CEntityFactory<CWeaponPaintGun>::Create( const char *pClassName )
{
	CWeaponPaintGun *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CWeaponPaintGun>() at line 102
}

// game/server/util.h:106 @0x77a690 _ZN14CEntityFactoryI15CWeaponPaintGunE7DestroyEP18IServerNetworkable
void CEntityFactory<CWeaponPaintGun>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x77a6c0 _ZN14CEntityFactoryI15CWeaponPaintGunE13GetEntitySizeEv
size_t CEntityFactory<CWeaponPaintGun>::GetEntitySize()
{
}

// game/server/portal2/weapon_paintgun.cpp:115 @0x7790c0 _ZN15CWeaponPaintGun5SpawnEv
void CWeaponPaintGun::Spawn()
{
	{
		int i;  // line 127
		// inlined CHandle<CBaseEntity>::operator==() at line 129
		{
			CPaintStream *pPaintStream;  // line 131
			// inlined CWeaponPaintGun::NetworkVar_m_hPaintStream::Set() at line 135
			// inlined CHandle<CBaseEntity>::CHandle() at line 135
		}
	}
}

// game/server/portal2/weapon_paintgun.cpp:144 @0x778f70 _ZN15CWeaponPaintGun14UpdateOnRemoveEv
void CWeaponPaintGun::UpdateOnRemove()
{
	{
		int i;  // line 146
		// inlined CHandle<CBaseEntity>::operator CBaseEntity*() at line 148
	}
}

// game/server/portal2/weapon_paintgun.cpp:155 @0x778df0 _ZN15CWeaponPaintGun8PrecacheEv
void CWeaponPaintGun::Precache()
{
}

// game/server/portal2/weapon_paintgun.cpp:176 @0x779080 _ZN15CWeaponPaintGun14SendWeaponAnimEi
bool CWeaponPaintGun::SendWeaponAnim( int iActivity )
{
	int newActivity;  // line 178
}

// game/server/portal2/weapon_paintgun.cpp:193 (declaration)
void HasCurrentColor();

// game/server/portal2/weapon_paintgun.cpp:193 @0x778d50 _ZN15CWeaponPaintGun15HasCurrentColorEv
bool CWeaponPaintGun::HasCurrentColor()
{
}

// game/server/portal2/weapon_paintgun.cpp:198 @0x779570 _ZN15CWeaponPaintGun18StartShootingSoundEv
void CWeaponPaintGun::StartShootingSound()
{
	CSoundEnvelopeController &controller;  // line 200
	{
		CPASAttenuationFilter filter;  // line 210
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 213
		// inlined CBaseEntity::entindex() at line 211
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 210
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 213
	}
}

// game/server/portal2/weapon_paintgun.cpp:217 @0x778db0 _ZN15CWeaponPaintGun17StopShootingSoundEv
void CWeaponPaintGun::StopShootingSound()
{
	{
		CSoundEnvelopeController &controller;  // line 221
	}
}

// game/server/portal2/weapon_paintgun.cpp:228 @0x779890 _ZN15CWeaponPaintGun13ActivatePaintE14PaintPowerType
void CWeaponPaintGun::ActivatePaint( PaintPowerType nIndex )
{
	IGameEvent *event;  // line 232
	{
		CBasePlayer *pPlayer;  // line 235
		// inlined ToBasePlayer() at line 235
		// inlined CBasePlayer::GetUserID() at line 238
		// inlined CWeaponPaintGun::GetPaintCount() at line 239
	}
	// inlined CWeaponPaintGun::NetworkVar_m_bHasPaint::Set() at line 230
}

// game/server/portal2/weapon_paintgun.cpp:249 (declaration)
void GetPaintCount();

// game/server/portal2/weapon_paintgun.cpp:249 @0x778d70 _ZN15CWeaponPaintGun13GetPaintCountEv
int CWeaponPaintGun::GetPaintCount()
{
	int nPaintCount;  // line 251
	{
		int i;  // line 252
	}
}

// game/server/portal2/weapon_paintgun.cpp:264 @0x779ef0 _ZN15CWeaponPaintGun10ResetPaintEv
void CWeaponPaintGun::ResetPaint()
{
	CBroadcastRecipientFilter filter;  // line 271
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 274
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 271
	{
		int i;  // line 266
		// inlined CWeaponPaintGun::DeactivatePaint() at line 268
	}
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 274
}

// game/server/portal2/weapon_paintgun.cpp:278 (declaration)
void DeactivatePaint( PaintPowerType nIndex );

// game/server/portal2/weapon_paintgun.cpp:278 @0x779390 _ZN15CWeaponPaintGun15DeactivatePaintE14PaintPowerType
void CWeaponPaintGun::DeactivatePaint( PaintPowerType nIndex )
{
	// inlined CWeaponPaintGun::NetworkVar_m_bHasPaint::Set() at line 280
}

// game/server/portal2/weapon_paintgun.cpp:284 @0x77a150 _ZN15CWeaponPaintGun12CleansePaintEv
void CWeaponPaintGun::CleansePaint()
{
}

// game/server/portal2/weapon_paintgun.cpp:294 @0x779c90 _ZN15CWeaponPaintGun15SetCurrentPaintE14PaintPowerType
void CWeaponPaintGun::SetCurrentPaint( PaintPowerType nIndex )
{
	CBaseEntity *pOwner;  // line 299
	CSingleUserRecipientFilter filter;  // line 303
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 309
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 303
	// inlined ToBasePlayer() at line 303
	// inlined CNetworkVarBase<int,CWeaponPaintGun::NetworkVar_m_nCurrentColor>::operator=<PaintPowerType>() at line 297
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 309
}

// game/server/portal2/weapon_paintgun.cpp:326 @0x778eb0 _ZN15CWeaponPaintGun13PaintGunThinkEv
void CWeaponPaintGun::PaintGunThink()
{
	CPortal_Player *pPlayer;  // line 336
	// inlined ToPortalPlayer() at line 336
	// inlined Vector::operator=() at line 339
}

// game/server/portal2/weapon_paintgun.cpp:346 @0x779b20 _ZL18GiveAllPaintPowersv
GiveAllPaintPowers()
{
	CBaseEntity *pFoundEnt;  // line 348
	{
		CWeaponPaintGun *pPaintGun;  // line 351
		{
			CBasePlayer *pPlayer;  // line 354
			// inlined ToBasePlayer() at line 354
		}
	}
}

// game/server/portal2/weapon_paintgun.cpp:369
static ConCommand giveallpaintpowers;
