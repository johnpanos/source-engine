// DWARF declaration skeleton for game/server/portal2/prop_paint_bomb.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xcc770 _Z41__static_initialization_and_destruction_0ii
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
	// inlined ServerClass::ServerClass() at line 57
	// inlined CEntityFactory<CPropPaintBomb>::CEntityFactory() at line 61
}

// game/server/portal2/prop_paint_bomb.cpp:25
ConVar paintbomb_explosion_radius;

// game/server/portal2/prop_paint_bomb.cpp:27
ConVar paintbomb_streak_speed_min;

// game/server/portal2/prop_paint_bomb.cpp:28
ConVar paintbomb_streak_speed_max;

// game/server/portal2/prop_paint_bomb.cpp:29
ConVar paintbomb_streak_enabled;

// game/server/portal2/prop_paint_bomb.cpp:31
ConVar paintbomb_blobs_min_streak_time;

// game/server/portal2/prop_paint_bomb.cpp:32
ConVar paintbomb_blobs_max_streak_time;

// game/server/portal2/prop_paint_bomb.cpp:33
ConVar paintbomb_blobs_min_streak_speed_dampen;

// game/server/portal2/prop_paint_bomb.cpp:34
ConVar paintbomb_blobs_max_streak_speed_dampen;

// game/server/portal2/prop_paint_bomb.cpp:36
ConVar paintbomb_vertical_angle_split;

// game/server/portal2/prop_paint_bomb.cpp:37
ConVar paintbomb_horizontal_angle_split;

// game/server/portal2/prop_paint_bomb.cpp:41
ConVar debug_paintbomb_explosion;

// game/server/portal2/prop_paint_bomb.cpp:45 (declaration)
void GetBaseMap();

// game/server/portal2/prop_paint_bomb.cpp:45 @0x6ea970 _ZN14CPropPaintBomb14GetDataDescMapEv
datamap_t *CPropPaintBomb::GetDataDescMap()
{
}

// game/server/portal2/prop_paint_bomb.cpp:45 @0x6ea980 _ZN14CPropPaintBomb10GetBaseMapEv
datamap_t *CPropPaintBomb::GetBaseMap()
{
}

// game/server/portal2/prop_paint_bomb.cpp:45 @0xcc640 _Z11DataMapInitI14CPropPaintBombEP9datamap_tPT_
datamap_t *DataMapInit<CPropPaintBomb>( CPropPaintBomb * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 45
	CDatadescGeneratedNameHolder nameHolder;  // line 45
	typedescription_t dataDesc[7];  // line 45
}

// game/server/portal2/prop_paint_bomb.cpp:45
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_paint_bomb.cpp:45
void CPropPaintBomb::m_DataMap;

// game/server/portal2/prop_paint_bomb.cpp:57 @0x6ea990 _ZN14CPropPaintBomb14GetServerClassEv
ServerClass *CPropPaintBomb::GetServerClass()
{
}

// game/server/portal2/prop_paint_bomb.cpp:57 @0x6ea9a0 _ZN14CPropPaintBomb40YouForgotToImplementOrDeclareServerClassEv
int CPropPaintBomb::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/prop_paint_bomb.cpp:57 @0xcc460 _Z15ServerClassInitIN16DT_PropPaintBomb7ignoredEEiPT_
int ServerClassInit<DT_PropPaintBomb::ignored>( DT_PropPaintBomb::ignored * )
{
	SendTable &sendTable;  // line 57
	char *const g_pSendTableName;  // line 57
	SendProp g_SendProps[3];  // line 57
}

// game/server/portal2/prop_paint_bomb.cpp:57
SendTable g_SendTable;

// game/server/portal2/prop_paint_bomb.cpp:57
int g_SendTableInit;

// game/server/portal2/prop_paint_bomb.cpp:57
static ServerClass g_CPropPaintBomb_ClassReg;

// game/server/portal2/prop_paint_bomb.cpp:57
void CPropPaintBomb::m_pClassSendTable;

// game/server/portal2/prop_paint_bomb.cpp:61
static CEntityFactory<CPropPaintBomb> prop_paint_bomb;

// game/server/portal2/prop_paint_bomb.cpp:63 (declaration)
void CPropPaintBomb();

// game/server/portal2/prop_paint_bomb.cpp:63 @0x6eaf20 _ZN14CPropPaintBombC2Ev
CPropPaintBomb::CPropPaintBomb()
{
	// inlined CNetworkVarBase<int,CPropPaintBomb::NetworkVar_m_nPaintPowerType>::CNetworkVarBase() at line 64
	// inlined COutputEvent::COutputEvent() at line 64
	// inlined COutputEvent::COutputEvent() at line 64
	// inlined COutputEvent::~COutputEvent() at line 66
	// inlined COutputEvent::~COutputEvent() at line 66
}

// game/server/portal2/prop_paint_bomb.cpp:63 @0x6eb030 _ZN14CPropPaintBombC1Ev
CPropPaintBomb::CPropPaintBomb()
{
}

// game/server/portal2/prop_paint_bomb.cpp:69 (declaration)
~CPropPaintBomb();

// game/server/portal2/prop_paint_bomb.cpp:69 @0x6ead30 _ZN14CPropPaintBombD0Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
}

// game/server/portal2/prop_paint_bomb.cpp:69 @0x6eade0 _ZN14CPropPaintBombD1Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
}

// game/server/portal2/prop_paint_bomb.cpp:69 @0x6eae80 _ZN14CPropPaintBombD2Ev
CPropPaintBomb::~CPropPaintBomb()
{
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
	// inlined COutputEvent::~COutputEvent() at line 71
}

// game/server/portal2/prop_paint_bomb.cpp:73 @0x6eac80 _ZN14CPropPaintBomb8PrecacheEv
void CPropPaintBomb::Precache()
{
}

// game/server/portal2/prop_paint_bomb.cpp:89 @0x6eb040 _ZN14CPropPaintBomb5SpawnEv
void CPropPaintBomb::Spawn()
{
	int nPaintBombSurfaceIndex;  // line 105
	IPhysicsObject *pPhysObject;  // line 106
	// inlined CBaseEntity::VPhysicsGetObject() at line 106
	// inlined CBaseEntity::AddSpawnFlags() at line 99
}

// game/server/util.h:99 @0x6ecbc0 _ZN14CEntityFactoryI14CPropPaintBombE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropPaintBomb>::Create( const char *pClassName )
{
	CPropPaintBomb *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropPaintBomb>() at line 101
}

// game/server/util.h:105 @0x6ecb80 _ZN14CEntityFactoryI14CPropPaintBombE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropPaintBomb>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6ecbb0 _ZN14CEntityFactoryI14CPropPaintBombE13GetEntitySizeEv
size_t CEntityFactory<CPropPaintBomb>::GetEntitySize()
{
}

// game/server/portal2/prop_paint_bomb.cpp:113 @0x6eac60 _ZN14CPropPaintBomb19UpdateTransmitStateEv
int CPropPaintBomb::UpdateTransmitState()
{
}

// game/server/portal2/prop_paint_bomb.cpp:119 @0x6eb120 _ZN14CPropPaintBomb17SetPaintPowerTypeE14PaintPowerType
void CPropPaintBomb::SetPaintPowerType( PaintPowerType paintType )
{
	Color color;  // line 123
	// inlined CBaseEntity::SetRenderColor() at line 124
	// inlined CNetworkVarBase<int,CPropPaintBomb::NetworkVar_m_nPaintPowerType>::operator=<PaintPowerType>() at line 121
}

// game/server/portal2/prop_paint_bomb.cpp:128 @0x6eb660 _Z22GenerateBombDirectionsR10CUtlVectorI6Vector10CUtlMemoryIS0_iEE
GenerateBombDirections( CUtlVector<Vector,CUtlMemory<Vector, int> > &directions )
{
	float flVerticalAngle;  // line 136
	float flHorizontalAngle;  // line 137
	Vector vecTemp;  // line 140
	VMatrix matRotation;  // line 141
	int nVerticalSplit;  // line 143
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 166
	// inlined Vector::Vector() at line 166
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 165
	// inlined Vector::Vector() at line 165
	{
		int x;  // line 153
		{
			int startIndex;  // line 155
			{
				int y;  // line 156
				// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 160
				// inlined Vector::operator-() at line 161
				// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 161
				// inlined Vector::operator=() at line 158
				// inlined VMatrix::operator*() at line 159
				// inlined Vector::operator=() at line 159
			}
			// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Count() at line 155
		}
	}
	{
		int i;  // line 144
		// inlined Vector::operator=() at line 146
		// inlined VMatrix::operator*() at line 146
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToHead() at line 148
		// inlined Vector::operator-() at line 148
		// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::AddToTail() at line 147
	}
	// inlined ConVar::GetInt() at line 143
	// inlined Vector::operator VectorByValue&() at line 142
	// inlined Vector::Vector() at line 142
	// inlined Vector::operator VectorByValue&() at line 140
	// inlined Vector::Vector() at line 140
	// inlined ConVar::GetFloat() at line 137
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::Purge() at line 133
}

// game/server/portal2/prop_paint_bomb.cpp:170 @0x6eaaf0 _Z29DispatchDryBombParticleEffectRK6Vector14PaintPowerType6QAngle
DispatchDryBombParticleEffect( const Vector &pos, PaintPowerType paintType, QAngle angSplash )
{
}

// game/server/portal2/prop_paint_bomb.cpp:193 @0x6ebf50 _Z24CreatePaintBombExplosion14PaintPowerTypeRK6Vectorb
CreatePaintBombExplosion( PaintPowerType paintType, const Vector &vecExplosionPos, bool bSpawnBlobs )
{
	CUtlVector<Vector,CUtlMemory<Vector, int> > dirs;  // line 204
	QAngle angSplashAngle;  // line 205
	const float radius;  // line 208
	// inlined QAngle::QAngle() at line 199
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::CUtlVector() at line 204
	// inlined QAngle::QAngle() at line 205
	// inlined ConVar::GetFloat() at line 208
	{
		int i;  // line 209
		{
			Vector vecBlobFireDir;  // line 212
			trace_t tr;  // line 214
			CTraceFilterSimpleClassnameList filter;  // line 215
			Ray_t ray;  // line 218
			CPortal_Base2D *pPortal;  // line 220
			// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 209
			// inlined Vector::Normalized() at line 212
			// inlined Vector::operator VectorByValue&() at line 212
			// inlined operator*() at line 219
			// inlined Vector::operator+() at line 219
			// inlined Ray_t::Init() at line 219
			{
				float flStreakSpeed;  // line 226
				float flStreakTime;  // line 227
				float flStreakDampen;  // line 228
				// inlined operator*() at line 229
				// inlined Vector::operator VectorByValue&() at line 229
				// inlined operator*() at line 229
				// inlined Vector::operator+() at line 229
				// inlined Vector::operator VectorByValue&() at line 229
			}
			// inlined Vector::operator-() at line 238
			// inlined Vector::operator VectorByValue&() at line 238
			// inlined CTraceFilterSimpleClassnameList::~CTraceFilterSimpleClassnameList() at line 209
		}
	}
	{
		int i;  // line 244
		{
			CPortal_Player *pPlayer;  // line 246
			{
				const Vector &playerCenter;  // line 251
				const Vector playerToBomb;  // line 252
				// inlined Vector::operator-() at line 252
			}
			// inlined ToPortalPlayer() at line 246
		}
	}
	{
		int i;  // line 266
		{
			Vector vecBlobFireDir;  // line 269
			// inlined Vector::operator VectorByValue&() at line 270
			// inlined Vector::Normalized() at line 269
			// inlined Vector::operator VectorByValue&() at line 269
			// inlined Vector::operator*() at line 270
			// inlined Vector::operator+() at line 270
		}
	}
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 272
	// inlined CUtlVector<Vector,CUtlMemory<Vector, int> >::~CUtlVector() at line 272
}

// game/server/portal2/prop_paint_bomb.cpp:276 @0x6eca40 _ZN14CPropPaintBomb12Event_KilledERK15CTakeDamageInfo
void CPropPaintBomb::Event_Killed( const CTakeDamageInfo &info )
{
	Vector vecPos;  // line 281
	bool bSpawnBlobs;  // line 284
	// inlined FClassnameIs() at line 284
	// inlined CTakeDamageInfo::GetInflictor() at line 284
	// inlined CBaseEntity::GetAbsOrigin() at line 281
	// inlined CTakeDamageInfo::GetInflictor() at line 279
}

// game/server/portal2/prop_paint_bomb.cpp:294 @0x6eb640 _ZN14CPropPaintBomb17CleansePaintPowerEv
void CPropPaintBomb::CleansePaintPower()
{
}

// game/server/portal2/prop_paint_bomb.cpp:300 @0x6eaaa0 _ZN14CPropPaintBomb5BreakEP11CBaseEntityRK15CTakeDamageInfo
void CPropPaintBomb::Break( CBaseEntity *pBreaker, const CTakeDamageInfo &info )
{
}

// game/server/portal2/prop_paint_bomb.cpp:307 @0x6eaa70 _ZN14CPropPaintBomb9OnFizzledEv
void CPropPaintBomb::OnFizzled()
{
}

// game/server/portal2/prop_paint_bomb.cpp:313 @0x6ea9b0 _ZN14CPropPaintBomb13InputDissolveER11inputdata_t
void CPropPaintBomb::InputDissolve( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:319 @0x6eaa50 _ZN14CPropPaintBomb24InputDisablePortalFunnelER11inputdata_t
void CPropPaintBomb::InputDisablePortalFunnel( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:324 @0x6eaa60 _ZN14CPropPaintBomb23InputEnablePortalFunnelER11inputdata_t
void CPropPaintBomb::InputEnablePortalFunnel( inputdata_t &in )
{
}

// game/server/portal2/prop_paint_bomb.cpp:329 @0x6eb2a0 _Z15CreatePaintBomb14PaintPowerType
CreatePaintBomb( PaintPowerType paintType )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 331
	bool bAllowPrecache;  // line 333
	CPropPaintBomb *pPaintBomb;  // line 337
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 362
	{
		CBasePlayer *pPlayer;  // line 345
		trace_t tr;  // line 346
		Vector vecForward;  // line 347
		// inlined UTIL_TraceLine() at line 349
		// inlined Vector::operator+() at line 349
		// inlined Vector::operator*() at line 349
	}
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 331
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 362
}

// game/server/portal2/prop_paint_bomb.cpp:365 @0x6eb630 _ZL26ent_create_paint_bomb_jumpRK8CCommand
ent_create_paint_bomb_jump( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:365
static ConCommand ent_create_paint_bomb_jump_command;

// game/server/portal2/prop_paint_bomb.cpp:370 @0x6eb620 _ZL27ent_create_paint_bomb_speedRK8CCommand
ent_create_paint_bomb_speed( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:370
static ConCommand ent_create_paint_bomb_speed_command;

// game/server/portal2/prop_paint_bomb.cpp:375 @0x6eb610 _ZL27ent_create_paint_bomb_stickRK8CCommand
ent_create_paint_bomb_stick( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:375
static ConCommand ent_create_paint_bomb_stick_command;

// game/server/portal2/prop_paint_bomb.cpp:380 @0x6eb600 _ZL28ent_create_paint_bomb_portalRK8CCommand
ent_create_paint_bomb_portal( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:380
static ConCommand ent_create_paint_bomb_portal_command;

// game/server/portal2/prop_paint_bomb.cpp:385 @0x6eb5f0 _ZL27ent_create_paint_bomb_eraseRK8CCommand
ent_create_paint_bomb_erase( const CCommand &args )
{
}

// game/server/portal2/prop_paint_bomb.cpp:385
static ConCommand ent_create_paint_bomb_erase_command;
