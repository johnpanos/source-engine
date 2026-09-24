// DWARF declaration skeleton for game/server/portal2/weapon_paintgun.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xd1270 _Z41__static_initialization_and_destruction_0ii
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
	// inlined ServerClass::ServerClass() at line 37
	// inlined CEntityFactory<CWeaponPaintGun>::CEntityFactory() at line 59
}

// game/server/portal2/weapon_paintgun.cpp:37 @0x6fcc50 _ZN15CWeaponPaintGun14GetServerClassEv
ServerClass *CWeaponPaintGun::GetServerClass()
{
}

// game/server/portal2/weapon_paintgun.cpp:37 @0x6fcc60 _ZN15CWeaponPaintGun40YouForgotToImplementOrDeclareServerClassEv
int CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/weapon_paintgun.cpp:37
static ServerClass g_CWeaponPaintGun_ClassReg;

// game/server/portal2/weapon_paintgun.cpp:37
void CWeaponPaintGun::m_pClassSendTable;

// game/server/portal2/weapon_paintgun.cpp:38 @0xd0d40 _Z15ServerClassInitIN17DT_WeaponPaintGun7ignoredEEiPT_
int ServerClassInit<DT_WeaponPaintGun::ignored>( DT_WeaponPaintGun::ignored * )
{
	SendTable &sendTable;  // line 38
	char *const g_pSendTableName;  // line 38
	SendProp g_SendProps[6];  // line 38
}

// game/server/portal2/weapon_paintgun.cpp:38
SendTable g_SendTable;

// game/server/portal2/weapon_paintgun.cpp:38
int g_SendTableInit;

// game/server/portal2/weapon_paintgun.cpp:48 (declaration)
void GetBaseMap();

// game/server/portal2/weapon_paintgun.cpp:48 @0x6fcc70 _ZN15CWeaponPaintGun14GetDataDescMapEv
datamap_t *CWeaponPaintGun::GetDataDescMap()
{
}

// game/server/portal2/weapon_paintgun.cpp:48 @0x6fcc80 _ZN15CWeaponPaintGun10GetBaseMapEv
datamap_t *CWeaponPaintGun::GetBaseMap()
{
}

// game/server/portal2/weapon_paintgun.cpp:48 @0xd1050 _Z11DataMapInitI15CWeaponPaintGunEP9datamap_tPT_
datamap_t *DataMapInit<CWeaponPaintGun>( CWeaponPaintGun * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 57
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 48
	CDatadescGeneratedNameHolder nameHolder;  // line 48
	typedescription_t dataDesc[6];  // line 48
}

// game/server/portal2/weapon_paintgun.cpp:48
datamap_t *g_DataMapHolder;

// game/server/portal2/weapon_paintgun.cpp:48
void CWeaponPaintGun::m_DataMap;

// game/server/portal2/weapon_paintgun.cpp:59
static CEntityFactory<CWeaponPaintGun> weapon_paintgun;

// game/server/portal2/weapon_paintgun.cpp:63 (declaration)
void CWeaponPaintGun();

// game/server/portal2/weapon_paintgun.cpp:63 @0x6fd9c0 _ZN15CWeaponPaintGunC2Ev
CWeaponPaintGun::CWeaponPaintGun()
{
	// inlined CNetworkVarBase<bool,CWeaponPaintGun::NetworkVar_m_bFiringErase>::CNetworkVarBase() at line 68
	// inlined CNetworkVarBase<bool,CWeaponPaintGun::NetworkVar_m_bFiringPaint>::CNetworkVarBase() at line 68
	// inlined CNetworkVarBase<int,CWeaponPaintGun::NetworkVar_m_nCurrentColor>::CNetworkVarBase() at line 68
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 83
}

// game/server/portal2/weapon_paintgun.cpp:63 @0x6fdb20 _ZN15CWeaponPaintGunC1Ev
CWeaponPaintGun::CWeaponPaintGun()
{
}

// game/server/portal2/weapon_paintgun.cpp:85 (declaration)
~CWeaponPaintGun();

// game/server/portal2/weapon_paintgun.cpp:85 @0x6fd4b0 _ZN15CWeaponPaintGunD0Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
	{
		CSoundEnvelopeController &controller;  // line 87
	}
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 100
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 100
}

// game/server/portal2/weapon_paintgun.cpp:85 @0x6fd570 _ZN15CWeaponPaintGunD2Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
	{
		CSoundEnvelopeController &controller;  // line 87
	}
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 100
	// inlined CBasePortalCombatWeapon::~CBasePortalCombatWeapon() at line 100
}

// game/server/portal2/weapon_paintgun.cpp:85 @0x6fd630 _ZN15CWeaponPaintGunD1Ev
CWeaponPaintGun::~CWeaponPaintGun()
{
}

// game/server/util.h:99 @0x6fdb70 _ZN14CEntityFactoryI15CWeaponPaintGunE6CreateEPKc
IServerNetworkable *CEntityFactory<CWeaponPaintGun>::Create( const char *pClassName )
{
	CWeaponPaintGun *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CWeaponPaintGun>() at line 101
}

// game/server/portal2/weapon_paintgun.cpp:103 @0x6fcdf0 _ZN15CWeaponPaintGun5SpawnEv
void CWeaponPaintGun::Spawn()
{
}

// game/server/util.h:105 @0x6fdb30 _ZN14CEntityFactoryI15CWeaponPaintGunE7DestroyEP18IServerNetworkable
void CEntityFactory<CWeaponPaintGun>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6fdb60 _ZN14CEntityFactoryI15CWeaponPaintGunE13GetEntitySizeEv
size_t CEntityFactory<CWeaponPaintGun>::GetEntitySize()
{
}

// game/server/portal2/weapon_paintgun.cpp:119 @0x6fcd30 _ZN15CWeaponPaintGun8PrecacheEv
void CWeaponPaintGun::Precache()
{
}

// game/server/portal2/weapon_paintgun.cpp:140 @0x6fcf60 _ZN15CWeaponPaintGun14SendWeaponAnimEi
bool CWeaponPaintGun::SendWeaponAnim( int iActivity )
{
	int newActivity;  // line 142
}

// game/server/portal2/weapon_paintgun.cpp:157 (declaration)
void HasCurrentColor();

// game/server/portal2/weapon_paintgun.cpp:157 @0x6fcc90 _ZN15CWeaponPaintGun15HasCurrentColorEv
bool CWeaponPaintGun::HasCurrentColor()
{
}

// game/server/portal2/weapon_paintgun.cpp:162 @0x6fd020 _ZN15CWeaponPaintGun18StartShootingSoundEv
void CWeaponPaintGun::StartShootingSound()
{
	CSoundEnvelopeController &controller;  // line 164
	{
		CPASAttenuationFilter filter;  // line 174
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 177
		// inlined CBaseEntity::entindex() at line 175
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 174
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 177
	}
}

// game/server/portal2/weapon_paintgun.cpp:181 @0x6fccf0 _ZN15CWeaponPaintGun17StopShootingSoundEv
void CWeaponPaintGun::StopShootingSound()
{
	{
		CSoundEnvelopeController &controller;  // line 185
	}
}

// game/server/portal2/weapon_paintgun.cpp:192 @0x6fd1b0 _ZN15CWeaponPaintGun13ActivatePaintE14PaintPowerType
void CWeaponPaintGun::ActivatePaint( PaintPowerType nIndex )
{
	IGameEvent *event;  // line 196
	{
		CBasePlayer *pPlayer;  // line 199
		// inlined ToBasePlayer() at line 199
		// inlined CBasePlayer::GetUserID() at line 202
		// inlined CWeaponPaintGun::GetPaintCount() at line 203
	}
	// inlined CWeaponPaintGun::NetworkVar_m_bHasPaint::Set() at line 194
}

// game/server/portal2/weapon_paintgun.cpp:213 (declaration)
void GetPaintCount();

// game/server/portal2/weapon_paintgun.cpp:213 @0x6fccb0 _ZN15CWeaponPaintGun13GetPaintCountEv
int CWeaponPaintGun::GetPaintCount()
{
	int nPaintCount;  // line 215
	{
		int i;  // line 216
	}
}

// game/server/portal2/weapon_paintgun.cpp:228 @0x6fd790 _ZN15CWeaponPaintGun10ResetPaintEv
void CWeaponPaintGun::ResetPaint()
{
	CBroadcastRecipientFilter filter;  // line 235
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 238
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 235
	{
		int i;  // line 230
		// inlined CWeaponPaintGun::DeactivatePaint() at line 232
	}
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 238
}

// game/server/portal2/weapon_paintgun.cpp:242 (declaration)
void DeactivatePaint( PaintPowerType nIndex );

// game/server/portal2/weapon_paintgun.cpp:242 @0x6fcfa0 _ZN15CWeaponPaintGun15DeactivatePaintE14PaintPowerType
void CWeaponPaintGun::DeactivatePaint( PaintPowerType nIndex )
{
	// inlined CWeaponPaintGun::NetworkVar_m_bHasPaint::Set() at line 244
}

// game/server/portal2/weapon_paintgun.cpp:248 @0x6fd950 _ZN15CWeaponPaintGun12CleansePaintEv
void CWeaponPaintGun::CleansePaint()
{
}

// game/server/portal2/weapon_paintgun.cpp:258 @0x6fd640 _ZN15CWeaponPaintGun15SetCurrentPaintE14PaintPowerType
void CWeaponPaintGun::SetCurrentPaint( PaintPowerType nIndex )
{
	CBaseEntity *pOwner;  // line 263
	CSingleUserRecipientFilter filter;  // line 267
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 273
	// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 267
	// inlined ToBasePlayer() at line 267
	// inlined CNetworkVarBase<int,CWeaponPaintGun::NetworkVar_m_nCurrentColor>::operator=<PaintPowerType>() at line 261
	// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 273
}

// game/server/portal2/weapon_paintgun.cpp:290 @0x6fce80 _ZN15CWeaponPaintGun13PaintGunThinkEv
void CWeaponPaintGun::PaintGunThink()
{
	CPortal_Player *pPlayer;  // line 300
	// inlined ToPortalPlayer() at line 300
	// inlined Vector::operator=() at line 303
}

// game/server/portal2/weapon_paintgun.cpp:310 @0x6fd340 _ZL18GiveAllPaintPowersv
GiveAllPaintPowers()
{
	CBaseEntity *pFoundEnt;  // line 312
	{
		CWeaponPaintGun *pPaintGun;  // line 315
		{
			CBasePlayer *pPlayer;  // line 318
			// inlined ToBasePlayer() at line 318
		}
	}
}

// game/server/portal2/weapon_paintgun.cpp:333
static ConCommand giveallpaintpowers;
